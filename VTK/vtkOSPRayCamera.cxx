/*=========================================================================

  Program:   Visualization Toolkit
  Module:    vtkOSPRayCamera.cxx

  Copyright (c) Ken Martin, Will Schroeder, Bill Lorensen
  All rights reserved.
  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
/*=========================================================================

  Program:   VTK/ParaView Los Alamos National Laboratory Modules (PVLANL)
  Module:    vtkOSPRayCamera.cxx

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

#include "vtkOSPRay.h"
#include "vtkOSPRayCamera.h"
#include "vtkOSPRayManager.h"
#include "vtkOSPRayRenderer.h"

#include "vtkObjectFactory.h"

// #include <Interface/Camera.h>
// #include <Engine/Control/RTRT.h>
#include <math.h>

//
//ospray
//
#if USE_OSPRAY
#include "ospray/ospray.h"
#include "ospray/common/OSPCommon.h"
#endif

//
//  vbos
//
#include <GL/glu.h>


vtkStandardNewMacro(vtkOSPRayCamera);

//----------------------------------------------------------------------------
vtkOSPRayCamera::vtkOSPRayCamera()
// : MantaCamera (0)
{
  //TODO: Observe my own modified event, and call OrientCamera then
  //cerr << "MC(" << this << ") CREATE" << endl;
  this->OSPRayManager = NULL;
}

//----------------------------------------------------------------------------
vtkOSPRayCamera::~vtkOSPRayCamera()
{
  //cerr << "MC(" << this << ") DESTROY" << endl;
  if (this->OSPRayManager)
    {
    //cerr << "MC(" << this << ") DESTROY " << this->OSPRayManager << " "
    //     << this->OSPRayManager->GetReferenceCount() << endl;
    this->OSPRayManager->Delete();
    }
}

//----------------------------------------------------------------------------
void vtkOSPRayCamera::OrientMantaCamera(vtkRenderer *ren)
{
  #if 1
  //cerr << "MC(" << this << ") ORIENT" << endl;
  vtkOSPRayRenderer * mantaRenderer = vtkOSPRayRenderer::SafeDownCast(ren);
  if (!mantaRenderer)
    {
    return;
    }

  // if (!this->MantaCamera)
  //   {
  //   this->MantaCamera = mantaRenderer->GetMantaCamera();
  //   if (!this->MantaCamera)
  //     {
  //     return;
  //     }
  //   }

  if (!this->OSPRayManager)
    {
    this->OSPRayManager = mantaRenderer->GetOSPRayManager();
    //cerr << "MC(" << this << ") REGISTER " << this->OSPRayManager << " "
    //     << this->OSPRayManager->GetReferenceCount() << endl;
    this->OSPRayManager->Register(this);
    }

  // for figuring out aspect ratio
  int lowerLeft[2];
  int usize, vsize;
  ren->GetTiledSizeAndOrigin(&usize, &vsize, lowerLeft, lowerLeft + 1);

  double *eye, *lookat, *up, vfov;
  eye    = this->Position;
  lookat = this->FocalPoint;
  up     = this->ViewUp;
  vfov   = this->ViewAngle;

#if 1
  OSPCamera ospCamera = ((OSPCamera)this->OSPRayManager->OSPRayCamera);
    // PRINT(ospCamera);
    if (vsize == 0)
      return;
    ospSetf(ospCamera,"aspect",float(usize)/float(vsize));
  ospSetf(ospCamera,"fovy",vfov);
  // printf("fovy %f fovh %f\n", p.camera_vfov, p.camera_hfov);
  Assert(ospCamera != NULL && "could not create camera");
  ospSet3f(ospCamera,"pos",eye[0], eye[1], eye[2]);
  ospSet3f(ospCamera,"up",up[0], up[1], up[2]);
  ospSet3f(ospCamera,"dir",lookat[0]-eye[0],lookat[1]-eye[1],lookat[2]-eye[2]);
      // ospCommit(camera);
      // embree::Vector3f camPos = embree::Vector3f(p.camera_eye.x(), p.camera_eye.y(), p.camera_eye.z());
  // Manta::Vector lookat = (p.camera_eye + p.camera_dir);
  // embree::Vector3f camLookAt = embree::Vector3f(lookat.x(), lookat.y(), lookat.z());
  // embree::Vector3f camUp = embree::Vector3f(p.camera_up.x(), p.camera_up.y(), p.camera_up.z());
  ospCommit(ospCamera);
  #endif

  // const Manta::BasicCameraData bookmark
  //   (
  //    Manta::Vector(eye[0], eye[1], eye[2]),
  //    Manta::Vector(lookat[0], lookat[1], lookat[2]),
  //    Manta::Vector(up[0], up[1], up[2]),
  //    vfov * usize / vsize, vfov
  //    );

  // mantaRenderer->GetMantaEngine()->addTransaction
    // ("update camera",
     // Manta::Callback::create(this->MantaCamera,
                             // &Manta::Camera::setBasicCameraData, bookmark)
     // );

  #endif
}

//----------------------------------------------------------------------------
// called by Renderer::UpdateCamera()
void vtkOSPRayCamera::Render(vtkRenderer *ren)
{
  if (this->GetMTime() > this->LastRenderTime)
    {
    this->OrientMantaCamera(ren);

    this->LastRenderTime.Modified();
    }
}
