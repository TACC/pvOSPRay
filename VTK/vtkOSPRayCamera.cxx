/* ======================================================================================= 
   Copyright 2014-2015 Texas Advanced Computing Center, The University of Texas at Austin  
   All rights reserved.
                                                                                           
   Licensed under the BSD 3-Clause License, (the "License"); you may not use this file     
   except in compliance with the License.                                                  
   A copy of the License is included with this software in the file LICENSE.               
   If your copy does not contain the License, you may obtain a copy of the License at:     
                                                                                           
       http://opensource.org/licenses/BSD-3-Clause                                         
                                                                                           
   Unless required by applicable law or agreed to in writing, software distributed under   
   the License is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY 
   KIND, either express or implied.                                                        
   See the License for the specific language governing permissions and limitations under   
   limitations under the License.

   pvOSPRay is derived from VTK/ParaView Los Alamos National Laboratory Modules (PVLANL)
   Copyright (c) 2007, Los Alamos National Security, LLC
   ======================================================================================= */

#include "ospray/ospray.h"
#include "ospray/common/OSPCommon.h"

#include "vtkOSPRay.h"
#include "vtkOSPRayCamera.h"
#include "vtkOSPRayManager.h"
#include "vtkOSPRayRenderer.h"

#include "vtkObjectFactory.h"

// #include <Interface/Camera.h>
// #include <Engine/Control/RTRT.h>
#include <math.h>

//
//  vbos
//
#include <GL/glu.h>   


vtkStandardNewMacro(vtkOSPRayCamera);

//----------------------------------------------------------------------------
vtkOSPRayCamera::vtkOSPRayCamera()
// : OSPRayCamera (0)
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
void vtkOSPRayCamera::OrientOSPRayCamera(vtkRenderer *ren)
{
  #if 1
  //cerr << "MC(" << this << ") ORIENT" << endl;
  vtkOSPRayRenderer * OSPRayRenderer = vtkOSPRayRenderer::SafeDownCast(ren);
  if (!OSPRayRenderer)
    {
    return;
    }
    OSPRayRenderer->ClearAccumulation();

  // if (!this->OSPRayCamera)
  //   {
  //   this->OSPRayCamera = OSPRayRenderer->GetOSPRayCamera();
  //   if (!this->OSPRayCamera)
  //     {
  //     return;
  //     }
  //   }

  if (!this->OSPRayManager)
    {
    this->OSPRayManager = OSPRayRenderer->GetOSPRayManager();
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
  ospSetf(ospCamera,"fovy",vfov); // GDA This needs to come out with next OSPRay release
  // printf("fovy %f fovh %f\n", p.camera_vfov, p.camera_hfov);
  Assert(ospCamera != NULL && "could not create camera");
  ospSet3f(ospCamera,"pos",eye[0], eye[1], eye[2]);
  ospSet3f(ospCamera,"up",up[0], up[1], up[2]);
  ospSet3f(ospCamera,"dir",lookat[0]-eye[0],lookat[1]-eye[1],lookat[2]-eye[2]);
      // ospCommit(camera);
      // embree::Vector3f camPos = embree::Vector3f(p.camera_eye.x(), p.camera_eye.y(), p.camera_eye.z());
  // OSPRay::Vector lookat = (p.camera_eye + p.camera_dir);
  // embree::Vector3f camLookAt = embree::Vector3f(lookat.x(), lookat.y(), lookat.z());
  // embree::Vector3f camUp = embree::Vector3f(p.camera_up.x(), p.camera_up.y(), p.camera_up.z());
  ospCommit(ospCamera);
  #endif

  // const OSPRay::BasicCameraData bookmark
  //   (
  //    OSPRay::Vector(eye[0], eye[1], eye[2]),
  //    OSPRay::Vector(lookat[0], lookat[1], lookat[2]),
  //    OSPRay::Vector(up[0], up[1], up[2]),
  //    vfov * usize / vsize, vfov
  //    );

  // OSPRayRenderer->GetOSPRayEngine()->addTransaction
    // ("update camera",
     // OSPRay::Callback::create(this->OSPRayCamera,
                             // &OSPRay::Camera::setBasicCameraData, bookmark)
     // );

  #endif
}

//----------------------------------------------------------------------------
// called by Renderer::UpdateCamera()
void vtkOSPRayCamera::Render(vtkRenderer *ren)
{
  if (this->GetMTime() > this->LastRenderTime)
    {
    this->OrientOSPRayCamera(ren);

    this->LastRenderTime.Modified();

    }
}
