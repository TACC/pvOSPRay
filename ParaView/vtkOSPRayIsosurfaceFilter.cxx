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


int vtkOSPRayIsosurfaceFilter::RequestData(vtkInformation* info,
                                    vtkInformationVector** inputVector,
                                    vtkInformationVector* outputVector)
{  
  #if 0
// Get the input and output data objects.
  vtkDataSet* input = vtkDataSet::GetData(inputVector[0]);
  vtkDataSet* output = vtkDataSet::GetData(outputVector);

  // // Check the size of the input.
  // vtkIdType numPts = input->GetNumberOfPoints();
  // if(numPts < 1)
  //   {
  //   vtkDebugMacro("No input!");
  //   return 1;
  //   }

  // // Allocate space for the elevation scalar data.
  // vtkSmartPointer<vtkFloatArray> newScalars =
  //   vtkSmartPointer<vtkFloatArray>::New();
  // newScalars->SetNumberOfTuples(numPts);

  // // Set up 1D parametric system and make sure it is valid.
  // double diffVector[3] =
  //   { this->HighPoint[0] - this->LowPoint[0],
  //     this->HighPoint[1] - this->LowPoint[1],
  //     this->HighPoint[2] - this->LowPoint[2] };
  // double length2 = vtkMath::Dot(diffVector, diffVector);
  // if(length2 <= 0)
  //   {
  //   vtkErrorMacro("Bad vector, using (0,0,1).");
  //   diffVector[0] = 0;
  //   diffVector[1] = 0;
  //   diffVector[2] = 1;
  //   length2 = 1.0;
  //   }

  // // Support progress and abort.
  // vtkIdType tenth = (numPts >= 10? numPts/10 : 1);
  // double numPtsInv = 1.0/numPts;
  // int abort = 0;

  // // Compute parametric coordinate and map into scalar range.
  // double diffScalar = this->ScalarRange[1] - this->ScalarRange[0];
  // vtkDebugMacro("Generating elevation scalars!");
  // for(vtkIdType i=0; i < numPts && !abort; ++i)
  //   {
  //   // Periodically update progress and check for an abort request.
  //   if(i % tenth == 0)
  //     {
  //     this->UpdateProgress((i+1)*numPtsInv);
  //     abort = this->GetAbortExecute();
  //     }

  //   // Project this input point into the 1D system.
  //   double x[3];
  //   input->GetPoint(i, x);
  //   double v[3] = { x[0] - this->LowPoint[0],
  //                   x[1] - this->LowPoint[1],
  //                   x[2] - this->LowPoint[2] };
  //   double s = vtkMath::Dot(v, diffVector) / length2;
  //   s = (s < 0.0 ? 0.0 : s > 1.0 ? 1.0 : s);

  //   // Store the resulting scalar value.
  //   newScalars->SetValue(i, this->ScalarRange[0] + s*diffScalar);
  //   }

  // Copy all the input geometry and data to the output.
  output->CopyStructure(input);
  output->GetPointData()->PassData(input->GetPointData());
  output->GetCellData()->PassData(input->GetCellData());

    vtkInformation* outInfo = outputVector->GetInformationObject(0);
  vtkInformationDoubleKey* infoKey =
      new vtkInformationDoubleKey("ospIsoVal", "ospIso");
  infoKey->Set(outInfo, IsoValue);

    vtkInformationDoubleKey* infoKey2 =
      new vtkInformationDoubleKey("ospIsoVal", "ospIso");
   infoKey2->Set(info, IsoValue);


  printf("vtkOSPRayIsosurfaceFilter info:\n");
  output->PrintSelf(std::cout,vtkIndent());
  printf("end vtkOSPRayIsosurfaceFilter info\n");

    printf("vtkOSPRayIsosurfaceFilter info2:\n");
  info->PrintSelf(std::cout,vtkIndent());
  printf("end vtkOSPRayIsosurfaceFilter info2\n");

    printf("vtkOSPRayIsosurfaceFilter info3:\n");
  outputVector->PrintSelf(std::cout,vtkIndent());
  printf("end vtkOSPRayIsosurfaceFilter info3\n");

  #endif

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

  numPts = input->GetNumberOfPoints();
  numCells = input->GetNumberOfCells();

    // Loop over points (if requested) and generate ids
  //
  // if ( numPts > 0 )
  //   {
    ptIds = vtkDoubleArray::New();
    ptIds->SetNumberOfValues(1);

    // for (id=0; id < numPts; id++)
      // {
      ptIds->SetValue(0, IsoValue);
      // }

    ptIds->SetName("ospIsoValues");
    // if ( ! this->FieldData )
    //   {

    printf("adding pointids\n");
      int idx = outPD->AddArray(ptIds);
      outPD->SetActiveAttribute(idx, vtkDataSetAttributes::SCALARS);
      outPD->CopyScalarsOff();
    //   }
    // else
    //   {
    //   outPD->AddArray(ptIds);
    //   outPD->CopyFieldOff("ospIsoPts");
    //   }
    ptIds->Delete();
    // }

  // Loop over cells (if requested) and generate ids
  //
  if ( numCells > 0 )
    {
    cellIds = vtkIdTypeArray::New();
    cellIds->SetNumberOfValues(numCells);

    for (id=0; id < numCells; id++)
      {
      cellIds->SetValue(id, 2);
      }

    cellIds->SetName("ospIsoPts");
    // if ( ! this->FieldData )
      // {
    printf("adding cellids\n");
      int idx = outCD->AddArray(cellIds);
      outCD->SetActiveAttribute(idx, vtkDataSetAttributes::SCALARS);
      outCD->CopyScalarsOff();
    //   }
    // else
    //   {
    //   outCD->AddArray(cellIds);
    //   outCD->CopyFieldOff("ospIsoPts");
    //   }
    cellIds->Delete();
    }

  outPD->PassData(inPD);
  outCD->PassData(inCD);

  printf("vtkOSPRayIsosurfaceFilter info3:\n");
  outputVector->PrintSelf(std::cout,vtkIndent());
  printf("end vtkOSPRayIsosurfaceFilter info3\n");

  printf("vtkOSPRayIsosurfaceFilter info4:\n");
  outCD->PrintSelf(std::cout,vtkIndent());
  printf("end vtkOSPRayIsosurfaceFilter info4\n");

  printf("vtkOSPRayIsosurfaceFilter info5:\n");
  outPD->PrintSelf(std::cout,vtkIndent());
  printf("end vtkOSPRayIsosurfaceFilter info5\n");
  // Add the new scalars array to the output.
  // newScalars->SetName("Elevation");
  // output->GetPointData()->AddArray(newScalars);
  // output->GetPointData()->SetActiveScalars("Elevation");

  return 1;
}