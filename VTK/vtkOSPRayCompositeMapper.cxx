/*=========================================================================

  Program:   Visualization Toolkit
  Module:    vtkOSPRayCompositeMapper.cxx

  Copyright (c) Ken Martin, Will Schroeder, Bill Lorensen
  All rights reserved.
  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
#include "vtkOSPRayCompositeMapper.h"

#include "vtkOSPRayPolyDataMapper.h"
#include "vtkObjectFactory.h"

vtkStandardNewMacro(vtkOSPRayCompositeMapper);

vtkOSPRayCompositeMapper::vtkOSPRayCompositeMapper()
{
}

vtkOSPRayCompositeMapper::~vtkOSPRayCompositeMapper()
{
}

vtkPolyDataMapper * vtkOSPRayCompositeMapper::MakeAMapper()
{
  return vtkOSPRayPolyDataMapper::New();
}

void vtkOSPRayCompositeMapper::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);
}
