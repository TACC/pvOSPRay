/*=========================================================================

  Program:   Visualization Toolkit
  Module:    vtkOSPRayVolumeMapper.cxx

  Copyright (c) Ken Martin, Will Schroeder, Bill Lorensen
  All rights reserved.
  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
#include "vtkOSPRayVolumeMapper.h"

#include "vtkDataSet.h"
#include "vtkExecutive.h"
#include "vtkGarbageCollector.h"
#include "vtkImageData.h"
#include "vtkInformation.h"


// Construct a vtkOSPRayVolumeMapper with empty scalar input and clipping off.
vtkOSPRayVolumeMapper::vtkOSPRayVolumeMapper()
{
  std::cout << __PRETTY_FUNCTION__ << std::endl;
  int i;

  this->BlendMode = vtkOSPRayVolumeMapper::COMPOSITE_BLEND;

  this->Cropping = 0;
  for ( i = 0; i < 3; i++ )
    {
    this->CroppingRegionPlanes[2*i    ]      = 0;
    this->CroppingRegionPlanes[2*i + 1]      = 1;
    this->VoxelCroppingRegionPlanes[2*i]     = 0;
    this->VoxelCroppingRegionPlanes[2*i + 1] = 1;
    }
  this->CroppingRegionFlags = VTK_CROP_SUBVOLUME;
}

vtkOSPRayVolumeMapper::~vtkOSPRayVolumeMapper()
{
}

void vtkOSPRayVolumeMapper::ConvertCroppingRegionPlanesToVoxels()
{
  double *spacing    = this->GetInput()->GetSpacing();
  int dimensions[3];
  this->GetInput()->GetDimensions(dimensions);
  double origin[3];
  double *bds = this->GetInput()->GetBounds();
  origin[0] = bds[0];
  origin[1] = bds[2];
  origin[2] = bds[4];

  for ( int i = 0; i < 6; i++ )
    {
    this->VoxelCroppingRegionPlanes[i] =
      (this->CroppingRegionPlanes[i] - origin[i/2]) / spacing[i/2];

    this->VoxelCroppingRegionPlanes[i] =
      ( this->VoxelCroppingRegionPlanes[i] < 0 ) ?
      ( 0 ) : ( this->VoxelCroppingRegionPlanes[i] );

    this->VoxelCroppingRegionPlanes[i] =
      ( this->VoxelCroppingRegionPlanes[i] > dimensions[i/2]-1 ) ?
      ( dimensions[i/2]-1 ) : ( this->VoxelCroppingRegionPlanes[i] );
    }
}

void vtkOSPRayVolumeMapper::SetInputData( vtkDataSet *genericInput )
{
  vtkImageData *input =
    vtkImageData::SafeDownCast( genericInput );

  if ( input )
    {
    this->SetInputData( input );
    }
  else
    {
    vtkErrorMacro("The SetInput method of this mapper requires vtkImageData as input");
    }
}

void vtkOSPRayVolumeMapper::SetInputData( vtkImageData *input )
{
  this->SetInputDataInternal(0, input);
}

vtkImageData *vtkOSPRayVolumeMapper::GetInput()
{
  if (this->GetNumberOfInputConnections(0) < 1)
    {
    return 0;
    }
  return vtkImageData::SafeDownCast(
    this->GetExecutive()->GetInputData(0, 0));
}


// Print the vtkOSPRayVolumeMapper
void vtkOSPRayVolumeMapper::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);

  os << indent << "Cropping: " << (this->Cropping ? "On\n" : "Off\n");

  os << indent << "Cropping Region Planes: " << endl
     << indent << "  In X: " << this->CroppingRegionPlanes[0]
     << " to " << this->CroppingRegionPlanes[1] << endl
     << indent << "  In Y: " << this->CroppingRegionPlanes[2]
     << " to " << this->CroppingRegionPlanes[3] << endl
     << indent << "  In Z: " << this->CroppingRegionPlanes[4]
     << " to " << this->CroppingRegionPlanes[5] << endl;

  os << indent << "Cropping Region Flags: "
     << this->CroppingRegionFlags << endl;

  os << indent << "BlendMode: " << this->BlendMode << endl;

  // Don't print this->VoxelCroppingRegionPlanes
}

//----------------------------------------------------------------------------
int vtkOSPRayVolumeMapper::FillInputPortInformation(int port, vtkInformation* info)
{
  if(!this->Superclass::FillInputPortInformation(port, info))
    {
    return 0;
    }
  info->Set(vtkAlgorithm::INPUT_REQUIRED_DATA_TYPE(), "vtkImageData");
  return 1;
}
