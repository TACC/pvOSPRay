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

#include <QTimer>
#include "vtkPVOSPRayView.h"

#include "vtkCamera.h"
#include "vtkDataRepresentation.h"
#include "vtkOSPRayCamera.h"
#include "vtkOSPRayLight.h"
#include "vtkOSPRayRenderer.h"
#include "vtkObjectFactory.h"
#include "vtkPVAxesWidget.h"
#include "vtkPVGenericRenderWindowInteractor.h"
#include "vtkPVSynchronizedRenderer.h"
#include "vtkRenderViewBase.h"

#include "vtkQtProgressiveRenderer.h"
#include "vtkCommand.h"
#include "vtkPVGenericRenderWindowInteractor.h"

static void RenderUpdateCallback(void* pvView)
{
  vtkPVOSPRayView* view = (vtkPVOSPRayView*)pvView;
  if (view)
    view->RenderUpdate();
}

vtkStandardNewMacro(vtkPVOSPRayView);
//----------------------------------------------------------------------------
vtkPVOSPRayView::vtkPVOSPRayView()
{
  this->SynchronizedRenderers->SetDisableIceT(true);
  EnableAO=-1;
  OSPRayRenderer = vtkOSPRayRenderer::New();
  this->RenderView->SetRenderer(OSPRayRenderer);

  vtkOSPRayCamera *OSPRayCamera = vtkOSPRayCamera::New();
  OSPRayRenderer->SetActiveCamera(OSPRayCamera);
  OSPRayCamera->ParallelProjectionOff();
  OSPRayCamera->Delete();

  OSPRayRenderer->SetUseDepthPeeling(0);

  this->Light->Delete();
  this->Light = vtkOSPRayLight::New();
  this->Light->SetAmbientColor(1, 1, 1);
  this->Light->SetSpecularColor(1, 1, 1);
  this->Light->SetDiffuseColor(1, 1, 1);
  this->Light->SetIntensity(1.0);
  this->Light->SetLightType(2); // CameraLight


  OSPRayRenderer->AddLight(this->Light);
  OSPRayRenderer->SetAutomaticLightCreation(0);

  if (this->Interactor)
    {
      ProgressiveRenderer = new vtkQtProgressiveRenderer(OSPRayRenderer,RenderUpdateCallback, this);
      this->Interactor->AddObserver(
        vtkCommand::StartInteractionEvent,
        ProgressiveRenderer, &vtkQtProgressiveRenderer::onStartInteractionEvent);
      this->Interactor->AddObserver(
        vtkCommand::EndInteractionEvent,
        ProgressiveRenderer, &vtkQtProgressiveRenderer::onEndInteractionEvent);
    }

  this->OrientationWidget->SetParentRenderer(OSPRayRenderer);


  this->SetInteractionMode(INTERACTION_MODE_3D);



}

//----------------------------------------------------------------------------
vtkPVOSPRayView::~vtkPVOSPRayView()
{
  OSPRayRenderer->Delete();
  delete ProgressiveRenderer;
}

//----------------------------------------------------------------------------
void vtkPVOSPRayView::SetActiveCamera(vtkCamera* camera)
{
  this->GetRenderer()->SetActiveCamera(camera);
}

//----------------------------------------------------------------------------
void vtkPVOSPRayView::Initialize(unsigned int id)
{
  this->Superclass::Initialize(id);

  vtkOpenGLRenderer *glrenderer = vtkOpenGLRenderer::SafeDownCast
    (this->RenderView->GetRenderer());
  if(glrenderer)
    {
    glrenderer->SetPass(NULL);
    }
}

//----------------------------------------------------------------------------
void vtkPVOSPRayView::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);
}

//-----------------------------------------------------------------------------
void vtkPVOSPRayView::SetThreads(int newval)
{
}

//-----------------------------------------------------------------------------
void vtkPVOSPRayView::SetEnableShadows(int newval)
{
  if (newval == this->EnableShadows)
    {
    return;
    }
  this->EnableShadows = newval;
  vtkOSPRayRenderer *OSPRayRenderer = vtkOSPRayRenderer::SafeDownCast
    (this->RenderView->GetRenderer());
  OSPRayRenderer->SetEnableShadows(this->EnableShadows);
}
void vtkPVOSPRayView::SetEnableAO(int newval)
{
  if (newval == this->EnableAO)
    {
    return;
    }
  this->EnableAO = newval;
  vtkOSPRayRenderer *renderer = vtkOSPRayRenderer::SafeDownCast(this->RenderView->GetRenderer());
  renderer->SetEnableAO(this->EnableAO);
}

void vtkPVOSPRayView::SetEnableProgressiveRefinement(int newval)
{
  if (newval != EnableProgressiveRefinement)
  {
    EnableProgressiveRefinement = newval;
    if (this->Interactor)
    {
      if (newval)
        ProgressiveRenderer->resumeAutoUpdates();        
      else
        ProgressiveRenderer->stopAutoUpdates();
    }
  }
}

//-----------------------------------------------------------------------------
void vtkPVOSPRayView::SetSamples(int newval)
{
  if (newval == this->Samples)
    {
    return;
    }
  this->Samples = newval;
  vtkOSPRayRenderer *renderer = vtkOSPRayRenderer::SafeDownCast(this->RenderView->GetRenderer());
  renderer->SetSamples(Samples);
}

//-----------------------------------------------------------------------------
void vtkPVOSPRayView::SetMaxDepth(int newval)
{
}


void vtkPVOSPRayView::RenderUpdate()
{
	this->OSPRayRenderer->SetProgressiveRenderFlag();
  this->StillRender();
}
