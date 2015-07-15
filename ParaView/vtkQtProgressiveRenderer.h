#ifndef __vtkQtProgressiveRenderer_h
#define __vtkQtProgressiveRenderer_h

#include <QObject>
#include <QTimer>
#include "pqTimer.h"

class pqView;
class vtkOSPRayRenderer;

class vtkQtProgressiveRenderer : public QObject
{
  Q_OBJECT
public:
  vtkQtProgressiveRenderer(vtkOSPRayRenderer* renderer,void (*cb)(void*), void* arg,QObject* parent=0);
  virtual ~vtkQtProgressiveRenderer();

  void stopAutoUpdates();
  void resumeAutoUpdates();

public slots:
  void onTimeout();
  void onViewAdded(pqView* view);
  void onStartInteractionEvent();
  void onEndInteractionEvent();
  void onViewUpdated();
private:
  bool delayUpdate;
  pqTimer _pqTimer;
  bool disableAutomaticUpdates;
  vtkOSPRayRenderer* renderer;
  void (*Callback)(void*);
  void* CallbackArg;
  int Samples;
};

#endif
