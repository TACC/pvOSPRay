#include "vtkQtProgressiveRenderer.h"
#include "pqActiveObjects.h"
#include "pqApplicationCore.h"
#include "pqCoreTestUtility.h"
#include "pqEventPlayer.h"
#include "pqWidgetEventPlayer.h"
#include "pqServer.h"
#include "pqServerManagerModel.h"
#include "pqView.h"
#include "vtkCommand.h"
#include "vtkPVGenericRenderWindowInteractor.h"
#include "vtkPVView.h"
#include "vtkSMRenderViewProxy.h"
#include "vtkSMSession.h"

#include "vtkOSPRayRenderer.h"
#include <iostream>
#include <stdio.h>

using namespace std;

vtkQtProgressiveRenderer::vtkQtProgressiveRenderer(vtkOSPRayRenderer* r,void (*cb)(void*), void* arg,QObject* parent)
  :delayUpdate(false), renderer(r),disableAutomaticUpdates(false), Samples(1)
{
  Callback = cb; CallbackArg = arg;
  QObject::connect(&_pqTimer, SIGNAL(timeout()), this, SLOT(onTimeout()));
  _pqTimer.setSingleShot(true);
}

vtkQtProgressiveRenderer::~vtkQtProgressiveRenderer()
{
	_pqTimer.stop();
}

void vtkQtProgressiveRenderer::onTimeout(){
  if (delayUpdate)
    _pqTimer.start(100);
  else
  {
    if (!disableAutomaticUpdates)
    {
      _pqTimer.start(0);
     Callback(CallbackArg);
    }
  }
}

  void vtkQtProgressiveRenderer::stopAutoUpdates()
  {
    disableAutomaticUpdates=true;
  }
  void vtkQtProgressiveRenderer::resumeAutoUpdates()
  {
    disableAutomaticUpdates=false;
    _pqTimer.start(100);
  }

void vtkQtProgressiveRenderer::onViewAdded(pqView* view)
{
  vtkSMRenderViewProxy* rvProxy =
    vtkSMRenderViewProxy::SafeDownCast(view->getProxy());
  if (rvProxy)
    {
    rvProxy->AddObserver(vtkCommand::UpdateDataEvent,
      this, &vtkQtProgressiveRenderer::onViewUpdated);
    rvProxy->GetInteractor()->AddObserver(
      vtkCommand::StartInteractionEvent,
      this, &vtkQtProgressiveRenderer::onStartInteractionEvent);
    rvProxy->GetInteractor()->AddObserver(
      vtkCommand::EndInteractionEvent,
      this, &vtkQtProgressiveRenderer::onEndInteractionEvent);
    }
}

void vtkQtProgressiveRenderer::onViewUpdated()
{
}

  void vtkQtProgressiveRenderer::onStartInteractionEvent()
{
  delayUpdate=true;
  Samples = renderer->GetSamples();
  renderer->SetSamples(1);
  }
    void vtkQtProgressiveRenderer::onEndInteractionEvent()
{
    delayUpdate=false;
  _pqTimer.start(0);
  renderer->SetSamples(Samples);
  }
  
