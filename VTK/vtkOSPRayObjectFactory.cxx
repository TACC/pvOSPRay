/*=========================================================================

  Program:   Visualization Toolkit
  Module:    vtkOSPRayObjectFactory.cxx

  Copyright (c) Ken Martin, Will Schroeder, Bill Lorensen
  All rights reserved.
  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
/*=========================================================================

  Program:   VTK/ParaView Los Alamos National Laboratory Modules (PVLANL)
  Module:    vtkOSPRayObjectFactory.cxx

Copyright (c) 2007, Los Alamos National Security, LLC

All rights reserved.

Copyright 2007. Los Alamos National Security, LLC.
This software was produced under U.S. Government contract DE-AC52-06NA25396
for Los Alamos National Laboratory (LANL), which is operated by
Los Alamos National Security, LLC for the U.S. Department of Energy.
The U.S. Government has rights to use, reproduce, and distribute this software.
NEITHER THE GOVERNMENT NOR LOS ALAMOS NATIONAL SECURITY, LLC MAKES ANY WARRANTY,
EXPRESS OR IMPLIED, OR ASSUMES ANY LIABILITY FOR THE USE OF THIS SOFTWARE.
If software is modified to produce derivative works, such modified software
should be clearly marked, so as not to confuse it with the version available
from LANL.

Additionally, redistribution and use in source and binary forms, with or
without modification, are permitted provided that the following conditions
are met:
-   Redistributions of source code must retain the above copyright notice,
    this list of conditions and the following disclaimer.
-   Redistributions in binary form must reproduce the above copyright notice,
    this list of conditions and the following disclaimer in the documentation
    and/or other materials provided with the distribution.
-   Neither the name of Los Alamos National Security, LLC, Los Alamos National
    Laboratory, LANL, the U.S. Government, nor the names of its contributors
    may be used to endorse or promote products derived from this software
    without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY LOS ALAMOS NATIONAL SECURITY, LLC AND CONTRIBUTORS
"AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
ARE DISCLAIMED. IN NO EVENT SHALL LOS ALAMOS NATIONAL SECURITY, LLC OR
CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

=========================================================================*/
// .NAME vtkOSPRayObjectFactory -
// .SECTION Description
//

#include "vtkOSPRay.h"
#include "vtkOSPRayObjectFactory.h"

#include "vtkDebugLeaks.h"
#include "vtkDynamicLoader.h"
#include "vtkOverrideInformation.h"
#include "vtkOverrideInformationCollection.h"
#include "vtkVersion.h"

#include "vtkOSPRayActor.h"
#include "vtkOSPRayCamera.h"
#include "vtkOSPRayLight.h"
#include "vtkOSPRayPolyDataMapper.h"
#include "vtkOSPRayProperty.h"
#include "vtkOSPRayRenderer.h"
#include "vtkOSPRayTexture.h"

#ifdef VTKMANTA_FOR_PARAVIEW
#include "vtkOSPRayLODActor.h"
#endif

vtkStandardNewMacro(vtkOSPRayObjectFactory);

//----------------------------------------------------------------------------
void vtkOSPRayObjectFactory::PrintSelf(ostream& os, vtkIndent indent)
{
  os << indent << "VTK Manta object factory" << endl;
}

//----------------------------------------------------------------------------
VTK_CREATE_CREATE_FUNCTION(vtkOSPRayActor);
VTK_CREATE_CREATE_FUNCTION(vtkOSPRayCamera);
VTK_CREATE_CREATE_FUNCTION(vtkOSPRayLight);
VTK_CREATE_CREATE_FUNCTION(vtkOSPRayPolyDataMapper);
VTK_CREATE_CREATE_FUNCTION(vtkOSPRayProperty);
VTK_CREATE_CREATE_FUNCTION(vtkOSPRayRenderer);
VTK_CREATE_CREATE_FUNCTION(vtkOSPRayTexture);
#ifdef VTKMANTA_FOR_PARAVIEW
VTK_CREATE_CREATE_FUNCTION(vtkOSPRayLODActor);
#endif

//----------------------------------------------------------------------------
vtkOSPRayObjectFactory::vtkOSPRayObjectFactory()
{
#ifdef VTKMANTA_FOR_PARAVIEW
  this->RegisterOverride("vtkPVLODActor",
                         "vtkOSPRayLODActor",
                         "OSPRay",
                         1,
                         vtkObjectFactoryCreatevtkOSPRayLODActor);
#endif
  this->RegisterOverride("vtkActor",
                         "vtkOSPRayActor",
                         "OSPRay",
                         1,
                         vtkObjectFactoryCreatevtkOSPRayActor);
  this->RegisterOverride("vtkCamera",
                         "vtkOSPRayCamera",
                         "OSPRay",
                         1,
                         vtkObjectFactoryCreatevtkOSPRayCamera);
  this->RegisterOverride("vtkLight",
                         "vtkOSPRayLight",
                         "OSPRay",
                         1,
                         vtkObjectFactoryCreatevtkOSPRayLight);
  this->RegisterOverride("vtkPolyDataMapper",
                         "vtkOSPRayPolyDataMapper",
                         "OSPRay",
                         1,
                         vtkObjectFactoryCreatevtkOSPRayPolyDataMapper);
  this->RegisterOverride("vtkProperty",
                         "vtkOSPRayProperty",
                         "OSPRay",
                         1,
                         vtkObjectFactoryCreatevtkOSPRayProperty);
  this->RegisterOverride("vtkRenderer",
                         "vtkOSPRayRenderer",
                         "OSPRay",
                         1,
                         vtkObjectFactoryCreatevtkOSPRayRenderer);
  this->RegisterOverride("vtkTexture",
                         "vtkOSPRayTexture",
                         "OSPRay",
                         1,
                         vtkObjectFactoryCreatevtkOSPRayTexture);
}

//----------------------------------------------------------------------------
VTK_FACTORY_INTERFACE_IMPLEMENT(vtkOSPRayObjectFactory);

//----------------------------------------------------------------------------
const char *vtkOSPRayObjectFactory::GetVTKSourceVersion()
{
  return VTK_SOURCE_VERSION;
}

//----------------------------------------------------------------------------
const char *vtkOSPRayObjectFactory::GetDescription()
{
  return "VTK Manta Object Factory";
}
