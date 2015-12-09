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
#include "vtkGenericRenderWindowInteractor.h"
#include "vtkPVSynchronizedRenderer.h"
#include "vtkRenderViewBase.h"
#include "vtkRenderWindowInteractor.h"
#include "vtkMultiProcessController.h"

#include "vtkQtProgressiveRenderer.h"
#include "vtkCommand.h"

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
  EnablePathtracing=-1;
  EnableProgressiveRefinement=-1;
  EnableShadows=-1;
  EnableVolumeShading=-1;
  Samples=-1;
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
  ProgressiveRenderer = NULL;

  this->OrientationWidget->SetParentRenderer(OSPRayRenderer);

  this->SetInteractionMode(INTERACTION_MODE_3D);
  SetEnableProgressiveRefinement(true);
  SetEnableAO(false);
  SetEnablePathtracing(false);
  SetEnableShadows(false);
  SetEnableVolumeShading(false);
  SetSamples(1);
}

//----------------------------------------------------------------------------
vtkPVOSPRayView::~vtkPVOSPRayView()
{
  OSPRayRenderer->Delete();
  if (ProgressiveRenderer)
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

void vtkPVOSPRayView::Update() { 
  if (this->Interactor)
  {
    static bool once = false;
    if (!once)
    {
      once = true;
      int enabledProg = this->EnableProgressiveRefinement;
      EnableProgressiveRefinement = -1;
      SetEnableProgressiveRefinement(true);
      SetEnableProgressiveRefinement(enabledProg);
    }
  }
  this->Superclass::Update();
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

void vtkPVOSPRayView::SetEnablePathtracing(int newval)
{
  if (newval == this->EnablePathtracing)
  {
    return;
  }
  this->EnablePathtracing = newval;
  vtkOSPRayRenderer *renderer = vtkOSPRayRenderer::SafeDownCast(this->RenderView->GetRenderer());
  renderer->SetEnablePathtracing(this->EnablePathtracing);
}

void vtkPVOSPRayView::SetEnableProgressiveRefinement(int newval)
{
 if (this->Interactor && !ProgressiveRenderer)
   CreateProgressiveRenderer();
 if (newval != EnableProgressiveRefinement)
 {
  EnableProgressiveRefinement = newval;
  if (this->Interactor && ProgressiveRenderer)
  {
    if (newval)
    {
      ProgressiveRenderer->resumeAutoUpdates();
    }
    else
    {
      ProgressiveRenderer->stopAutoUpdates();
    }
  }
 }
}

void vtkPVOSPRayView::SetEnableVolumeShading(int newval)
{
  if (newval == this->EnableVolumeShading)
  {
    return;
  }
  this->EnableVolumeShading = newval;
  vtkOSPRayRenderer *renderer = vtkOSPRayRenderer::SafeDownCast(this->RenderView->GetRenderer());
  renderer->SetEnableVolumeShading(this->EnableVolumeShading);
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

void vtkPVOSPRayView::Render (bool interactive, bool skip_rendering)
{
  if (GetUseDistributedRenderingForStillRender())
  {
    std::cerr << "usedistributed\n";
    OSPRayRenderer->SetComputeDepth(true);
  }
  else
    OSPRayRenderer->SetComputeDepth(false);
  this->Superclass::Render(interactive, skip_rendering);
}

void vtkPVOSPRayView::RenderUpdate()
{
  if (GetUseDistributedRenderingForStillRender())
    return;
  if (!GetEnableProgressiveRefinement())
    return;
  // SynchronizeForCollaboration();
	this->OSPRayRenderer->SetProgressiveRenderFlag();
  this->StillRender();
  // this->InteractiveRender();
}

void vtkPVOSPRayView::CreateProgressiveRenderer()
{
  ProgressiveRenderer = new vtkQtProgressiveRenderer(OSPRayRenderer,RenderUpdateCallback, this);
  this->Interactor->AddObserver(
    vtkCommand::StartInteractionEvent,
    ProgressiveRenderer, &vtkQtProgressiveRenderer::onStartInteractionEvent);
  this->Interactor->AddObserver(
    vtkCommand::EndInteractionEvent,
    ProgressiveRenderer, &vtkQtProgressiveRenderer::onEndInteractionEvent);
}
