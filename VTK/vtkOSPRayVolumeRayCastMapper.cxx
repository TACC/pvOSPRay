/*=========================================================================

Program:   Visualization Toolkit
Module:    vtkOSPRayVolumeRayCastMapper.cxx

Copyright (c) Ken Martin, Will Schroeder, Bill Lorensen
All rights reserved.
See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

 This software is distributed WITHOUT ANY WARRANTY; without even
 the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
 PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
#include "vtkOSPRayVolumeRayCastMapper.h"

#include "ospray/ospray.h"

#include "vtkCamera.h"
#include "vtkDataArray.h"
#include "vtkEncodedGradientEstimator.h"
#include "vtkEncodedGradientShader.h"
#include "vtkFiniteDifferenceGradientEstimator.h"
#include "vtkGarbageCollector.h"
#include "vtkGraphicsFactory.h"
#include "vtkImageData.h"
#include "vtkMath.h"
#include "vtkMultiThreader.h"
#include "vtkObjectFactory.h"
#include "vtkPlaneCollection.h"
#include "vtkPointData.h"
#include "vtkRenderWindow.h"
#include "vtkRenderer.h"
#include "vtkTimerLog.h"
#include "vtkTransform.h"
#include "vtkVolumeProperty.h"
#include "vtkVolumeRayCastFunction.h"
#include "vtkRayCastImageDisplayHelper.h"
#include "vtkStreamingDemandDrivenPipeline.h"
#include "vtkColorTransferFunction.h"
#include "vtkPiecewiseFunction.h"
#include "vtkTimerLog.h"
#include "vtkInformation.h"
#include "vtkInformationVector.h"
#include "vtkInformation.h"
#include "vtkInformationDoubleKey.h"
#include "vtkInformationDoubleVectorKey.h"

#include "vtkOSPRayRenderer.h"
#include "vtkOSPRayManager.h"

#include <math.h>
#include <algorithm>

vtkStandardNewMacro(vtkOSPRayVolumeRayCastMapper);

// Construct a new vtkOSPRayVolumeRayCastMapper with default values
 vtkOSPRayVolumeRayCastMapper::vtkOSPRayVolumeRayCastMapper()
 {
  this->VolumeAdded=false;
  this->NumColors = 128;
  this->SampleDistance             =  1.0;
  this->ImageSampleDistance        =  1.0;
  this->MinimumImageSampleDistance =  1.0;
  this->MaximumImageSampleDistance = 10.0;
  this->AutoAdjustSampleDistances  =  1;

  this->ZBuffer                = NULL;

  this->IntermixIntersectingGeometry = 1;

  this->OSPRayManager = vtkOSPRayManager::Singleton();

  this->SharedData = false;
  if (SharedData)
    OSPRayVolume = ospNewVolume("shared_structured_volume");
  else
    OSPRayVolume = ospNewVolume("block_bricked_volume");
  transferFunction = ospNewTransferFunction("piecewise_linear");
  ospCommit(transferFunction);
  SamplingRate=0.25;
}

// Destruct a vtkOSPRayVolumeRayCastMapper - clean up any memory used
vtkOSPRayVolumeRayCastMapper::~vtkOSPRayVolumeRayCastMapper()
{
}


void vtkOSPRayVolumeRayCastMapper::SetNumberOfThreads( int num )
{
  NumberOfThreads = num;
}

int vtkOSPRayVolumeRayCastMapper::GetNumberOfThreads()
{
  return NumberOfThreads;
}

void vtkOSPRayVolumeRayCastMapper::ReleaseGraphicsResources(vtkWindow *)
{
}

void vtkOSPRayVolumeRayCastMapper::Render( vtkRenderer *ren, vtkVolume *vol )
{
  // make sure that we have scalar input and update the scalar input
  if ( this->GetInput() == NULL )
  {
    vtkErrorMacro(<< "No Input!");
    return;
  }
  else
  {
    this->GetInputAlgorithm()->UpdateInformation();
    vtkStreamingDemandDrivenPipeline::SetUpdateExtentToWholeExtent(
      this->GetInputInformation());
    this->GetInputAlgorithm()->Update();
  }
  // vol->UpdateTransferFunctions( ren );

  //
  // OSPRay
  //

  vtkOSPRayRenderer* OSPRayRenderer =
  vtkOSPRayRenderer::SafeDownCast(ren);
  if (!OSPRayRenderer)
  {
    return;
  }
  OSPRayModel = this->OSPRayManager->OSPRayVolumeModel;

  OSPRenderer renderer = this->OSPRayManager->OSPRayVolumeRenderer;

  vtkImageData *data = this->GetInput();
  vtkDataArray * scalars = this->GetScalars(data, this->ScalarMode,
    this->ArrayAccessMode, this->ArrayId, this->ArrayName, this->CellFlag);

  void* ScalarDataPointer =
  this->GetInput()->GetPointData()->GetScalars()->GetVoidPointer(0);
  int ScalarDataType =
  this->GetInput()->GetPointData()->GetScalars()->GetDataType();

  int dim[3];
  data->GetDimensions(dim);

  size_t typeSize = 0;
  std::string voxelType;
  if (ScalarDataType == VTK_FLOAT)
  {
    typeSize = sizeof(float);
    voxelType = "float";
  }
  else if (ScalarDataType == VTK_UNSIGNED_CHAR)
  {
    typeSize = sizeof(unsigned char);
    voxelType = "uchar";
  }
  else if (ScalarDataType == VTK_DOUBLE)
  {
    typeSize = sizeof(double);
    voxelType = "double";
  }
  else
  {
    std::cerr << "ERROR: Unsupported data type for ospray volumes, current supported data types are: " 
     << " float, uchar, double\n";
     return;
  }

  //
  // Cache timesteps
  //
  double timestep=-1;
  vtkInformation *inputInfo = this->GetInput()->GetInformation();
  // // std::cout << __PRETTY_FUNCTION__ << " (" << this << ") " << "actor: (" <<
  // // OSPRayActor << ") mode: (" << OSPRayActor->OSPRayModel << ") " << std::endl;
  if (inputInfo && inputInfo->Has(vtkDataObject::DATA_TIME_STEP()))
  {
    //std::cerr << "has timestep\n";
    timestep = inputInfo->Get(vtkDataObject::DATA_TIME_STEP());
    //std::cerr << "timestep time: " << timestep << std::endl;
  }
  vtkOSPRayVolumeCacheEntry* cacheEntry = Cache[vol][timestep];
  if (!cacheEntry)
  {
    cacheEntry = new vtkOSPRayVolumeCacheEntry();
    if (SharedData)
      OSPRayVolume = ospNewVolume("shared_structured_volume");
    else
      OSPRayVolume = ospNewVolume("block_bricked_volume");
    cacheEntry->Volume = OSPRayVolume;
    Cache[vol][timestep] = cacheEntry;

    //
    // Send Volumetric data to OSPRay
    //

    char* buffer = NULL;
    size_t sizeBytes = dim[0]*dim[1]*dim[2] *typeSize;

    buffer = (char*)ScalarDataPointer;

    ospSet3i(OSPRayVolume, "dimensions", dim[0], dim[1], dim[2]);
    double origin[3];
    vol->GetOrigin(origin);
    double *bds = data->GetBounds();
    origin[0] = bds[0];
    origin[1] = bds[2];
    origin[2] = bds[4];

    double spacing[3];
    data->GetSpacing(spacing);
    ospSet3f(OSPRayVolume, "gridOrigin", origin[0], origin[1], origin[2]);
    ospSet3f(OSPRayVolume, "gridSpacing", spacing[0],spacing[1],spacing[2]);
    ospSetString(OSPRayVolume, "voxelType", voxelType.c_str());
    if (SharedData)
    {
      OSPData voxelData = ospNewData(sizeBytes, OSP_UCHAR, ScalarDataPointer, OSP_DATA_SHARED_BUFFER);
      ospSetData(OSPRayVolume, "voxelData", voxelData);
    }
    else
    {
			osp::vec3i ll, uu;
			ll.x = 0, ll.y = 0, ll.z = 0;
			uu.x = dim[0], uu.y = dim[1], uu.z = dim[2];
      ospSetRegion(OSPRayVolume, ScalarDataPointer, ll, uu);
    }
  }
  OSPRayVolume = cacheEntry->Volume;

  // test for modifications to volume properties
  if (vol->GetProperty()->GetMTime() > PropertyTime)
  {
    OSPRayRenderer->SetClearAccumFlag();
    vtkVolumeProperty* volProperty = vol->GetProperty();
    vtkColorTransferFunction* colorTF = volProperty->GetRGBTransferFunction(0);
    vtkPiecewiseFunction *scalarTF = volProperty->GetScalarOpacity(0);
    int numNodes = colorTF->GetSize();
    double* tfData = colorTF->GetDataPointer();

    TFVals.resize(NumColors*3);
    TFOVals.resize(NumColors);
    scalarTF->GetTable(data->GetScalarRange()[0],data->GetScalarRange()[1], NumColors, &TFOVals[0]);
    colorTF->GetTable(data->GetScalarRange()[0],data->GetScalarRange()[1], NumColors, &TFVals[0]);

    OSPData colorData = ospNewData(NumColors, OSP_FLOAT3, &TFVals[0]);// TODO: memory leak?  does ospray manage this>
    ospSetData(transferFunction, "colors", colorData);
    OSPData tfAlphaData = ospNewData(NumColors, OSP_FLOAT, &TFOVals[0]);
    ospSetData(transferFunction, "opacities", tfAlphaData);
    ospCommit(transferFunction);
    ospSet1i(OSPRayVolume, "gradientShadingEnabled", volProperty->GetShade());
    PropertyTime.Modified();
  }

  // test for modifications to input
  if (this->GetInput()->GetMTime() > this->BuildTime)
  {

    std::vector<float> isoValues;
    if (this->GetInput()->GetPointData()->GetScalars("ospIsoValues"))
    {
      int num = this->GetInput()->GetPointData()->GetScalars("ospIsoValues")->GetComponent(0,0);
      for( int i=0; i < num; i++)
      {
        float isoValue = this->GetInput()->GetPointData()->GetScalars("ospIsoValues")->GetComponent(0,i+1);
        isoValues.push_back(isoValue);
      }
    }

    if (isoValues.size())
    {
      OSPData isovaluesData = ospNewData(isoValues.size(), OSP_FLOAT, &isoValues[0]);
      ospSetData(OSPRayVolume, "isovalues", isovaluesData);
    }

    if (this->GetInput()->GetPointData()->GetScalars("ospClipValues"))
    {
      float clipValue = this->GetInput()->GetPointData()->GetScalars("ospClipValues")->GetComponent(0,0);
      int clipAxis = this->GetInput()->GetPointData()->GetScalars("ospClipValues")->GetComponent(0,1);

      std::cout << "clipValue: " << clipValue << std::endl;
      std::cout << "clipAxis: " << clipAxis << std::endl;

      float uu[3], ll[3];
			uu[0] = dim[0], uu[1] = dim[1], uu[2] = dim[2];
			ll[0] = 0, ll[0] = 0, ll[0] = 0;
			if (clipAxis >= 0 && clipAxis <= 2)
        uu[clipAxis] = clipValue;
      ospSet3fv(OSPRayVolume, "volumeClippingBoxLower", ll);
      ospSet3fv(OSPRayVolume, "volumeClippingBoxUpper", uu);
    }

    ospSet2f(transferFunction, "valueRange", data->GetScalarRange()[0], data->GetScalarRange()[1]);

    //! Commit the transfer function only after the initial colors and alphas have been set (workaround for Qt signalling issue).
    ospCommit(transferFunction);

    //TODO: manage memory

    ospSetObject((OSPObject)OSPRayVolume, "transferFunction", transferFunction);
    this->BuildTime.Modified();
  }
  if (SamplingRate == 0.0f)
  {
    //automatically determine sampling rate, for now just a simple switch
    int maxBound = std::max(dim[0],dim[1]);
    maxBound = std::max(maxBound,dim[2]);
    if (maxBound < 1000)
    {
      float s = 1000.0f - maxBound;
      s = (s/1000.0f*4.0f + 0.25f);
      ospSet1f(OSPRayVolume, "samplingRate", s);
    }
    else
      ospSet1f(OSPRayVolume, "samplingRate", 0.25f);
  }
  else
    ospSet1f(OSPRayVolume, "samplingRate", SamplingRate);
  ospCommit(OSPRayVolume);
  ospAddVolume(OSPRayModel,(OSPVolume)OSPRayVolume);
  // if (!VolumeAdded)
    // VolumeAdded = true;
  ospCommit(OSPRayModel);
  ospSetObject(renderer, "model", OSPRayModel);
  ospCommit(renderer);
  this->OSPRayManager->OSPRayVolumeModel = OSPRayModel;

  OSPRayRenderer->SetHasVolume(true);
}



// Print method for vtkOSPRayVolumeRayCastMapper
void vtkOSPRayVolumeRayCastMapper::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);

  os << indent << "Sample Distance: " << this->SampleDistance << "\n";
  os << indent << "Image Sample Distance: "
  << this->ImageSampleDistance << "\n";
  os << indent << "Minimum Image Sample Distance: "
  << this->MinimumImageSampleDistance << "\n";
  os << indent << "Maximum Image Sample Distance: "
  << this->MaximumImageSampleDistance << "\n";
  os << indent << "Auto Adjust Sample Distances: "
  << this->AutoAdjustSampleDistances << "\n";
  os << indent << "Intermix Intersecting Geometry: "
  << (this->IntermixIntersectingGeometry ? "On\n" : "Off\n");
}

//----------------------------------------------------------------------------
void vtkOSPRayVolumeRayCastMapper::ReportReferences(vtkGarbageCollector* collector)
{
  this->Superclass::ReportReferences(collector);
}
