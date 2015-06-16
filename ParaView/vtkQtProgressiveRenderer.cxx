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
  :delayUpdate(false), renderer(r),disableAutomaticUpdates(false)
{
  Callback = cb; CallbackArg = arg;
  // std::cout << __PRETTY_FUNCTION__ << std::endl;
  // QTimer *timer = new QTimer();
  printf("connecting timer\n");
  QObject::connect(&_pqTimer, SIGNAL(timeout()), this, SLOT(onTimeout()));
  _pqTimer.setSingleShot(true);
  printf("done connecting timer\n");
  // timer->start(10);
  printf("starting timer\n");
  // _pqTimer.start(0);
  printf("done starting timer\n");

    // pqServerManagerModel* smmodel =
    // pqApplicationCore::instance()->getServerManagerModel();
    // if (!smmodel)
    //   printf("no smmodel\n");
  // QObject::connect(smmodel, SIGNAL(viewAdded(pqView*)),
  //   this, SLOT(onViewAdded(pqView*)));

  //   foreach (pqView* view, smmodel->findItems<pqView*>())
  //   {
  //   // this->onViewAdded(view);
  //   }
}
// void vtkQtProgressiveRenderer::SetCallback(void* cb)
// {
//   Callback=cb;
// }

void vtkQtProgressiveRenderer::onTimeout(){
  // printf("timer!\n");
  if (delayUpdate)
    _pqTimer.start(100);
  else
  {
    if (!disableAutomaticUpdates)
    {
      _pqTimer.start(0);
      // printf("calling callback\n");
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
  // std::cout << __PRETTY_FUNCTION__ << std::endl;
  }

  void vtkQtProgressiveRenderer::onStartInteractionEvent()
{
  // std::cout << __PRETTY_FUNCTION__ << std::endl;
  delayUpdate=true;
  renderer->SetSamples(1);
  }
    void vtkQtProgressiveRenderer::onEndInteractionEvent()
{
  // std::cout << __PRETTY_FUNCTION__ << std::endl;
    delayUpdate=false;
  _pqTimer.start(0);
  // renderer->SetSamples(64);
  }
  
