/*=========================================================================

  Program:   Visualization Toolkit
  Module:    vtkOSPRayIsosurfaceFilter.cxx

  Copyright (c) Ken Martin, Will Schroeder, Bill Lorensen
  All rights reserved.
  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
#include <algorithm>
#include "vtkOSPRayIsosurfaceFilter.h"

#include "vtkCellData.h"
#include "vtkDataSet.h"
#include "vtkFloatArray.h"
#include "vtkInformation.h"
#include "vtkInformationVector.h"
#include "vtkInformation.h"
#include "vtkInformationDoubleKey.h"
#include "vtkInformationDoubleVectorKey.h"
#include "vtkMath.h"
#include "vtkObjectFactory.h"
#include "vtkPointData.h"
#include "vtkSmartPointer.h"

#include "vtkCellData.h"
#include "vtkDataSet.h"
#include "vtkDataSet.h"
#include "vtkIdTypeArray.h"
#include "vtkInformation.h"
#include "vtkInformationVector.h"
#include "vtkObjectFactory.h"
#include "vtkPointData.h"

     
#include <vtkDoubleArray.h>

vtkStandardNewMacro(vtkOSPRayIsosurfaceFilter);

//----------------------------------------------------------------------------
vtkOSPRayIsosurfaceFilter::vtkOSPRayIsosurfaceFilter()
{
}

//----------------------------------------------------------------------------
vtkOSPRayIsosurfaceFilter::~vtkOSPRayIsosurfaceFilter()
{
}

//----------------------------------------------------------------------------
void vtkOSPRayIsosurfaceFilter::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);
}

 void vtkOSPRayIsosurfaceFilter::SetValue(int i, double val)
  {
//    std::cout << __PRETTY_FUNCTION__ << " " << i << " " << val << std::endl;
    NumberOfContours=std::max(NumberOfContours,i+1);
    ContourValues[i]=val;
    Modified();
  }


int vtkOSPRayIsosurfaceFilter::RequestData(vtkInformation* info,
                                    vtkInformationVector** inputVector,
                                    vtkInformationVector* outputVector)
{  

    // get the info objects
  vtkInformation *inInfo = inputVector[0]->GetInformationObject(0);
  vtkInformation *outInfo = outputVector->GetInformationObject(0);

  // get the input and output
  vtkDataSet *input = vtkDataSet::SafeDownCast(
    inInfo->Get(vtkDataObject::DATA_OBJECT()));
  vtkDataSet *output = vtkDataSet::SafeDownCast(
    outInfo->Get(vtkDataObject::DATA_OBJECT()));

  vtkIdType numPts, numCells, id;
  vtkDoubleArray *ptIds;
  vtkIdTypeArray *cellIds;
  vtkPointData *inPD=input->GetPointData(), *outPD=output->GetPointData();
  vtkCellData *inCD=input->GetCellData(), *outCD=output->GetCellData();

  // First, copy the input to the output as a starting point
  output->CopyStructure( input );

if (NumberOfContours)
{
    ptIds = vtkDoubleArray::New();
    ptIds->SetNumberOfValues(NumberOfContours+1);
    ptIds->SetValue(0, NumberOfContours);
    for (int i =0; i < NumberOfContours; i++)
    {
      std::cout << "contour val: " << ContourValues[i] << std::endl;
      ptIds->SetValue(i+1, ContourValues[i]);
    }
    ptIds->SetName("ospIsoValues");
    int idx = outPD->AddArray(ptIds);
    ptIds->Delete();
  }
  if (EnableClip)
  {
    ptIds = vtkDoubleArray::New();
    ptIds->SetNumberOfValues(2);
    ptIds->SetValue(0, ClipValue);
    ptIds->SetValue(1, ClipAxis);
    ptIds->SetName("ospClipValues");
    int idx = outPD->AddArray(ptIds);
    ptIds->Delete();
  }


  outPD->PassData(inPD);
  outCD->PassData(inCD);

  return 1;
}
