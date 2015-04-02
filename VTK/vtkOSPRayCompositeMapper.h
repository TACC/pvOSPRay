/*=========================================================================

  Program:   Visualization Toolkit
  Module:    vtkCompositePolyDataMapper.h

  Copyright (c) Ken Martin, Will Schroeder, Bill Lorensen
  All rights reserved.
  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
// .NAME vtkOSPRayCompositeMapper - MantaMapper for composite data
// .SECTION Description
// This class is an adapter between composite data produced by the data
// processing pipeline and the non composite capable vtkOSPRayPolyDataMapper.

#ifndef __vtkOSPRayCompositeMapper_h
#define __vtkOSPRayCompositeMapper_h

#include "vtkCompositePolyDataMapper.h"
#include "vtkOSPRayModule.h"
class vtkPolyDataMapper;

class VTKOSPRAY_EXPORT vtkOSPRayCompositeMapper :
  public vtkCompositePolyDataMapper
{

public:
  static vtkOSPRayCompositeMapper *New();
  vtkTypeMacro(vtkOSPRayCompositeMapper, vtkCompositePolyDataMapper);
  virtual void PrintSelf(ostream& os, vtkIndent indent);

protected:
  vtkOSPRayCompositeMapper();
  ~vtkOSPRayCompositeMapper();

  // Description:
  // Need to define the type of data handled by this mapper.
  virtual vtkPolyDataMapper * MakeAMapper();

private:
  vtkOSPRayCompositeMapper(const vtkOSPRayCompositeMapper&);  // Not implemented.
  void operator=(const vtkOSPRayCompositeMapper&);    // Not implemented.
};

#endif
