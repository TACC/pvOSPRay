/*=========================================================================

  Program:   Visualization Toolkit
  Module:    vtkOSPRayManager.cxx

  Copyright (c) Ken Martin, Will Schroeder, Bill Lorensen
  All rights reserved.
  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/


#include "vtkOSPRayManager.h"
#include "vtkObjectFactory.h"
#include "vtkOSPRay.h"

// #include <Core/Color/Color.h>
// #include <Core/Color/ColorDB.h>
// #include <Core/Color/RGBColor.h>
// #include <Engine/Control/RTRT.h>
// #include <Engine/Display/NullDisplay.h>
// #include <Engine/Display/SyncDisplay.h>
// #include <Engine/Factory/Create.h>
// #include <Engine/Factory/Factory.h>
// #include <Image/SimpleImage.h>
// #include <Interface/Context.h>
// #include <Interface/Light.h>
// #include <Interface/LightSet.h>
// #include <Interface/Scene.h>
// #include <Interface/Object.h>
// #include <Model/AmbientLights/ConstantAmbient.h>
// #include <Model/Backgrounds/ConstantBackground.h>
// #include <Model/Groups/Group.h>
// #include <Model/Lights/HeadLight.h>

#if USE_OSPRAY
//
//ospray
//
#include "ospray/ospray.h"
// #include "ospray/common/ospcommon.h"
// #include "ospray/common/OspCommon.h"
     #endif

// #include <typeinfo.h>

bool vtkOSPRayManager::initialized = false;

vtkStandardNewMacro(vtkOSPRayManager);

//----------------------------------------------------------------------------
vtkOSPRayManager::vtkOSPRayManager()
{
  #if 1
  cerr << "OManager(" << this << ") CREATE" << endl;
  // this->MantaEngine = Manta::createManta();
  // //TODO: this requires Manta >= r2439 but I can't check that programatically
  // this->MantaEngine->setDisplayBeforeRender(false);
  // this->MantaFactory = new Manta::Factory( this->MantaEngine );
  // this->Started = false;

  // this->MantaScene = NULL;
  // this->MantaWorldGroup = NULL;
  // this->MantaLightSet = NULL;
  // this->MantaCamera = NULL;
  // this->SyncDisplay = NULL;
  // this->ChannelId = 0;


#if 1
  // OSPRayModel = ospNewModel();
  // OSPRayRenderer = ospNewRenderer();
  // OSPRayCamera = ospNewCamera("perspective");
    // printf("%s::%s\n",typeid(*this).name(),__FUNCTION__);
  if (!initialized)
  {
    initialized=true;
  if (1)
  {
    int ac =1;
    const char* av[] = {"gluray\0","\0"};
    // const char* av[] = {"gluray\0","--osp:debug\0"};
    ospInit(&ac, av);
  }
  else  //coi
  {
    int ac =2;
    const char* av[] = {"gluray\0","--osp:coi","\0"};
    ospInit(&ac, av);
  }
  //ospLoadModule("pathtracer");
}



    // ospray::vec2i newSize(512,512);
    // ospFramebuffer = ospNewFrameBuffer(newSize,OSP_RGBA_I8);

  // ospModel = ((osp::Model*)&oModel);
  // ospRenderer = ((osp::Renderer*)&oRenderer);
  // ospCamera = ((osp::Camera*)&oCamera);
// }

  // oRenderer = *((OSPRenderer*)ospRenderer);
  // PRINT(oRenderer);
  // OSPRenderer renderer = *((OSPRenderer*)this->ospRenderer);
  // std::cout << "renderer: " << this->ospRenderer << std::endl;
  // PRINT(renderer);

  // OSPCamera camera = *((OSPCamera*)this->ospCamera);
  // PRINT(camera);

    #endif
    #endif
}

//----------------------------------------------------------------------------
vtkOSPRayManager::~vtkOSPRayManager()
{
  #if 0
  //cerr << "MX(" << this << ") DESTROY" << endl;
  int v =-1;
  //TODO: This is screwy but the only way I've found to get it to consistently
  //shutdown without hanging.
  //int i = 0;
  v = this->MantaEngine->numWorkers();
  this->MantaEngine->changeNumWorkers(0);
  while (v != 0)
    {
    //cerr << "MX(" << this << ") SYNC " << i++ << " " << v << endl;
    if (this->SyncDisplay)
      {
      this->SyncDisplay->doneRendering();
      v = this->MantaEngine->numWorkers();
      if (v != 0)
        {
        this->SyncDisplay->waitOnFrameReady();
        }
      }
    v = this->MantaEngine->numWorkers();
    }
  this->MantaEngine->finish();
  this->MantaEngine->blockUntilFinished();

  //cerr << "MX(" << this << ") SYNC DONE " << i << " " << v << endl;
  //cerr << "MX(" << this << ") wait" << endl;

  if (this->MantaLightSet)
    {
    delete this->MantaLightSet->getAmbientLight();
    /*
    //let vtkOSPRayLight's delete themselves
    Manta::Light *light;
    for (unsigned int i = 0; i < this->MantaLightSet->numLights(); i++)
      {
      light = this->MantaLightSet->getLight(i);
      delete light;
      }
    */
    }
  delete this->MantaLightSet;

  delete this->MantaCamera;

  if (this->MantaScene)
    {
    delete this->MantaScene->getBackground();
    }
  delete this->MantaScene;

  delete this->MantaWorldGroup;

  delete this->MantaFactory;

  //delete this->SyncDisplay; //engine does this

  delete this->MantaEngine;

  //cerr << "MX(" << this << ") good night Gracie" << endl;
  #endif

}

//----------------------------------------------------------------------------
void vtkOSPRayManager::PrintSelf( ostream& os, vtkIndent indent )
{
  this->Superclass::PrintSelf( os, indent );
}

//----------------------------------------------------------------------------
void vtkOSPRayManager::StartEngine(int maxDepth,
                                  double *bgColor,
                                  double *ambient,
                                  bool stereo,
                                  int *size
                                  )
{
  #if 0
  //cerr << "MX(" << this << ") START" << endl;
  if (this->Started)
    {
    cerr << "WARNING: Manta is already initted, ignoring reinitialize." << endl;
    return;
    }
  this->Started = true;

  // create an empty Manta scene with background
  this->MantaScene = new Manta::Scene();
  this->MantaScene->getRenderParameters().setMaxDepth( maxDepth );
  this->MantaEngine->setScene( this->MantaScene );

  Manta::ConstantBackground * background = new Manta::ConstantBackground(
    Manta::Color(Manta::RGBColor( bgColor[0], bgColor[1], bgColor[2] )));
  this->MantaScene->setBackground( background );

  // create empty world groups
  this->MantaWorldGroup = new Manta::Group();
  this->MantaScene->setObject( this->MantaWorldGroup );

  // create empty LightSet with ambient light
  this->MantaLightSet = new Manta::LightSet();
  this->MantaLightSet->setAmbientLight(
    new Manta::ConstantAmbient(
    Manta::Color(Manta::RGBColor( ambient[0], ambient[1], ambient[2] ))));
  this->MantaScene->setLights( this->MantaLightSet );

  // create the mantaCamera singleton,
  // it is the only camera we create per renderer
  this->MantaCamera = this->MantaFactory->
    createCamera( "pinhole(-normalizeRays -createCornerRays)" );

  // Use SyncDisplay with Null Display to stop Manta engine at each frame,
  // the image is combined with OpenGL framebuffer by vtkXMantaRenderWindow
  std::vector<std::string> vs;
  this->SyncDisplay = new Manta::SyncDisplay( vs );
  this->SyncDisplay->setChild(  new Manta::NullDisplay( vs )  );

  //Set image size
  this->ChannelId = this->MantaEngine->createChannel
    ( this->SyncDisplay,
      this->MantaCamera,
      stereo, size[0], size[1] );

    // ospray::vec2i newSize(size[0],size[1]);
    // ospFramebuffer = ospNewFrameBuffer(newSize,OSP_RGBA_I8);
    #endif
}
