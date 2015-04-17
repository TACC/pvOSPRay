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


// #include <typeinfo.h>

bool vtkOSPRayManager::initialized = false;

vtkStandardNewMacro(vtkOSPRayManager);

//----------------------------------------------------------------------------
vtkOSPRayManager::vtkOSPRayManager()
{
  #if 1
  cerr << "OManager(" << this << ") CREATE" << endl;
  // this->OSPRayEngine = OSPRay::createOSPRay();
  // //TODO: this requires OSPRay >= r2439 but I can't check that programatically
  // this->OSPRayEngine->setDisplayBeforeRender(false);
  // this->OSPRayFactory = new OSPRay::Factory( this->OSPRayEngine );
  // this->Started = false;

  // this->OSPRayScene = NULL;
  // this->OSPRayWorldGroup = NULL;
  // this->OSPRayLightSet = NULL;
  // this->OSPRayCamera = NULL;
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
    // const char* av[] = {"pvOSPRay\0","\0"};
    // const char* av[] = {"pvOSPRay\0","--osp:debug\0"};
    const char* av[] = {"pvOSPRay\0","--osp:verbose\0"};
    ospInit(&ac, av);
  }
  else  //coi
  {
    int ac =2;
    const char* av[] = {"pvOSPRay\0","--osp:coi","\0"};
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
  v = this->OSPRayEngine->numWorkers();
  this->OSPRayEngine->changeNumWorkers(0);
  while (v != 0)
    {
    //cerr << "MX(" << this << ") SYNC " << i++ << " " << v << endl;
    if (this->SyncDisplay)
      {
      this->SyncDisplay->doneRendering();
      v = this->OSPRayEngine->numWorkers();
      if (v != 0)
        {
        this->SyncDisplay->waitOnFrameReady();
        }
      }
    v = this->OSPRayEngine->numWorkers();
    }
  this->OSPRayEngine->finish();
  this->OSPRayEngine->blockUntilFinished();

  //cerr << "MX(" << this << ") SYNC DONE " << i << " " << v << endl;
  //cerr << "MX(" << this << ") wait" << endl;

  if (this->OSPRayLightSet)
    {
    delete this->OSPRayLightSet->getAmbientLight();
    /*
    //let vtkOSPRayLight's delete themselves
    OSPRay::Light *light;
    for (unsigned int i = 0; i < this->OSPRayLightSet->numLights(); i++)
      {
      light = this->OSPRayLightSet->getLight(i);
      delete light;
      }
    */
    }
  delete this->OSPRayLightSet;

  delete this->OSPRayCamera;

  if (this->OSPRayScene)
    {
    delete this->OSPRayScene->getBackground();
    }
  delete this->OSPRayScene;

  delete this->OSPRayWorldGroup;

  delete this->OSPRayFactory;

  //delete this->SyncDisplay; //engine does this

  delete this->OSPRayEngine;

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
    cerr << "WARNING: OSPRay is already initted, ignoring reinitialize." << endl;
    return;
    }
  this->Started = true;

  // create an empty OSPRay scene with background
  this->OSPRayScene = new OSPRay::Scene();
  this->OSPRayScene->getRenderParameters().setMaxDepth( maxDepth );
  this->OSPRayEngine->setScene( this->OSPRayScene );

  OSPRay::ConstantBackground * background = new OSPRay::ConstantBackground(
    OSPRay::Color(OSPRay::RGBColor( bgColor[0], bgColor[1], bgColor[2] )));
  this->OSPRayScene->setBackground( background );

  // create empty world groups
  this->OSPRayWorldGroup = new OSPRay::Group();
  this->OSPRayScene->setObject( this->OSPRayWorldGroup );

  // create empty LightSet with ambient light
  this->OSPRayLightSet = new OSPRay::LightSet();
  this->OSPRayLightSet->setAmbientLight(
    new OSPRay::ConstantAmbient(
    OSPRay::Color(OSPRay::RGBColor( ambient[0], ambient[1], ambient[2] ))));
  this->OSPRayScene->setLights( this->OSPRayLightSet );

  // create the OSPRayCamera singleton,
  // it is the only camera we create per renderer
  this->OSPRayCamera = this->OSPRayFactory->
    createCamera( "pinhole(-normalizeRays -createCornerRays)" );

  // Use SyncDisplay with Null Display to stop OSPRay engine at each frame,
  // the image is combined with OpenGL framebuffer by vtkXOSPRayRenderWindow
  std::vector<std::string> vs;
  this->SyncDisplay = new OSPRay::SyncDisplay( vs );
  this->SyncDisplay->setChild(  new OSPRay::NullDisplay( vs )  );

  //Set image size
  this->ChannelId = this->OSPRayEngine->createChannel
    ( this->SyncDisplay,
      this->OSPRayCamera,
      stereo, size[0], size[1] );

    // ospray::vec2i newSize(size[0],size[1]);
    // ospFramebuffer = ospNewFrameBuffer(newSize,OSP_RGBA_I8);
    #endif
}
