/*=========================================================================

  Program:   Visualization Toolkit
  Module:    vtkOSPRayManager.h

  Copyright (c) Ken Martin, Will Schroeder, Bill Lorensen
  All rights reserved.
  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
// .NAME vtkOSPRayManager - persistant access to Manta engine
// .SECTION Description
// vtkOSPRayManager is a reference counted wrapper around the manta engine.
// Because it is reference counted, it outlives all vtkOSPRay classes that
// reference it. That means that they can safely use it to manage their
// own Manta side resources and that the engine itself will be destructed
// when the wrapper is.

#ifndef __vtkOSPRayManager_h
#define __vtkOSPRayManager_h

#include "vtkObject.h"
#include "vtkOSPRayModule.h"
     #include <vector>


//BTX
namespace Manta {
class Camera;
class Factory;
class Group;
class LightSet;
class MantaInterface;
class Scene;
class SyncDisplay;
};
//ETX

namespace osp
{
class Renderer;
class Model;
class Camera;
}


class VTKOSPRAY_EXPORT vtkOSPRayManager : public vtkObject
{
public:
  static vtkOSPRayManager *New();
  vtkTypeMacro(vtkOSPRayManager,vtkObject);
  virtual void PrintSelf(ostream& os, vtkIndent indent);

  //Description:
  //Called to setup and start the manta ray tracing engine
  void StartEngine(int MaxRayDepth,
                   double *BackGroundColor,
                   double *AmbientRGB,
                   bool IsStereo,
                   int *ViewPortsize);

//BTX
  // Manta::MantaInterface* GetMantaEngine()
  // {
  // return this->MantaEngine;
  // }
  // Manta::Factory* GetMantaFactory()
  // {
  //   return this->MantaFactory;
  // }
  // Manta::Scene* GetMantaScene()
  // {
  //   return this->MantaScene;
  // }
  // Manta::Group* GetMantaWorldGroup()
  // {
  //   return this->MantaWorldGroup;
  // }
  // Manta::LightSet* GetMantaLightSet()
  // {
  //   return this->MantaLightSet;
  // }
  // Manta::Camera* GetMantaCamera()
  // {
  //   return this->MantaCamera;
  // }
  // Manta::SyncDisplay* GetSyncDisplay()
  // {
  //   return this->SyncDisplay;
  // }
  // int GetChannelId()
  // {
  //   return this->ChannelId;
  // }
//ETX

 protected:
  vtkOSPRayManager();
  ~vtkOSPRayManager();

 private:
  vtkOSPRayManager(const vtkOSPRayManager&);  // Not implemented.
  void operator=(const vtkOSPRayManager&);  // Not implemented.

//BTX
  // Manta::MantaInterface * MantaEngine;
  // Manta::Factory * MantaFactory;
  // Manta::Scene * MantaScene;
  // Manta::Group * MantaWorldGroup;
  // Manta::LightSet * MantaLightSet;
  // Manta::Camera * MantaCamera;
  // Manta::SyncDisplay * SyncDisplay;
//ETX
  int ChannelId;
  bool Started;
  static bool initialized;

public:
  //
//  OSPRay vars
//

osp::Model* OSPRayModel;
osp::Renderer*    OSPRayRenderer;
osp::Camera*      OSPRayCamera;

};

#endif
