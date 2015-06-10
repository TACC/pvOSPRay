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

#include "vtkActor.h"
#include "vtkCuller.h"
#include "vtkLight.h"
#include "vtkLightCollection.h"
#include "vtkObjectFactory.h"
#include "vtkRendererCollection.h"
#include "vtkRenderWindow.h"
#include "vtkTimerLog.h"


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

#include "vtkImageData.h"
#include "vtkPNGWriter.h"
#include "vtkPolyDataMapper.h"
#include "vtkPolyData.h"

#include <vtkSmartPointer.h>
#include <vtkCommand.h>
#include <vtkRenderer.h>
#include <vtkRendererCollection.h>
#include <vtkRenderWindow.h>
#include <vtkRenderWindowInteractor.h>

//
//  VBOs
//
#if USE_VBOS
#include "vtkOSPRayActor.h"
#include <GL/glu.h>
#include <assert.h>
#endif


#include <string>

   vtkStandardNewMacro(vtkOSPRayRenderer);

   class vtkTimerCallback : public vtkCommand
{
  public:
    static vtkTimerCallback *New()
    {
      vtkTimerCallback *cb = new vtkTimerCallback;
      cb->TimerCount = 0;
      return cb;
    }
 
    virtual void Execute(vtkObject *vtkNotUsed(caller), unsigned long eventId,
                         void *vtkNotUsed(callData))
    {
      if (vtkCommand::TimerEvent == eventId)
        {
        ++this->TimerCount;
        }
        std::cout << "timer " << this->TimerCount << std::endl;
    }
 
  private:
    int TimerCount;
 
};

//----------------------------------------------------------------------------
   vtkOSPRayRenderer::vtkOSPRayRenderer()
//:
  //EngineInited( false ), EngineStarted( false ),
  //IsStereo( false ), OSPRayScene( 0 ), OSPRayWorldGroup( 0 ),
  //OSPRayLightSet( 0 ), OSPRayCamera( 0 ), SyncDisplay( 0 )
   {
  std::cout << __PRETTY_FUNCTION__ << std::endl;
    hasVolumeHack= false;
  //cerr << "MR(" << this << ") CREATE" << endl;

    this->EngineInited=false;
  // Default options
    this->NumberOfWorkers = 1;
    this->EnableShadows = 0;
    this->Samples = 1;
    this->MaxDepth = 5;

  this->ImageX = -1;
  this->ImageY = -1;

  this->backgroundRGB[0] = 0.0;
  this->backgroundRGB[1] = 0.0;
  this->backgroundRGB[2] = 0.0;

  // the default global ambient light created by vtkRenderer is too bright.
    this->SetAmbient( 0.1, 0.1, 0.1 );

    this->OSPRayManager = vtkOSPRayManager::New();

  this->OSPRayManager->OSPRayModel = ospNewModel();  //Carson: note: static needed, it seems they are freed in scope
  OSPModel oModel = (OSPModel)this->OSPRayManager->OSPRayModel;
  this->OSPRayManager->OSPRayCamera = ospNewCamera("perspective");
  OSPCamera oCamera = (OSPCamera)this->OSPRayManager->OSPRayCamera;

  bool ao = EnableAO;
  EnableAO=-1;
  SetEnableAO(ao);
  OSPRenderer oRenderer = (OSPRenderer)this->OSPRayManager->OSPRayRenderer;

  this->OSPRayManager->OSPRayVolumeRenderer = (osp::Renderer*)ospNewRenderer("raycast_volume_renderer");
  this->OSPRayManager->OSPRayDynamicModel = ospNewModel();  
  OSPRenderer vRenderer = (OSPRenderer)this->OSPRayManager->OSPRayVolumeRenderer;
  ospSet3f(vRenderer, "bgColor", backgroundRGB[0], backgroundRGB[1], backgroundRGB[2]);
  OSPModel vModel = (OSPModel)this->OSPRayManager->OSPRayDynamicModel;
  ospSetObject(vRenderer, "dynamic_model", vModel);

  ospSetParam(vRenderer,"world",vModel);
  ospSetParam(vRenderer,"model",vModel);
  ospSetParam(vRenderer,"camera",oCamera);

  // renderer = ospNewRenderer("ao16");
  // renderer = ospNewRenderer("obj");
  Assert(oRenderer != NULL && "could not create renderer");

  ospSetParam(oRenderer,"world",oModel);
  ospSetParam(oRenderer,"model",oModel);
  ospSetParam(oRenderer,"camera",oCamera);
  ospSet1i(oRenderer,"spp",Samples);
  ospSet3f(oRenderer,"bgColor",0.83,0.35,0.43);
  ospSet1f(oRenderer,"epsilon", 10e-5);
  ospCommit(oModel);
  ospCommit(oCamera);
  ospCommit(oRenderer);
  PRINT(oRenderer);
  PRINT(oModel);
  PRINT(oCamera);
  #if 1

  // this->OSPRayEngine = this->OSPRayManager->GetOSPRayEngine();
  // this->OSPRayEngine->changeNumWorkers( this->NumberOfWorkers );

  // this->OSPRayFactory = this->OSPRayManager->GetOSPRayFactory();

  this->ColorBuffer = NULL;
  this->DepthBuffer = NULL;
  // this->ImageSize = -1;
  this->osp_framebuffer = NULL;

  // this->OSPRayFactory->selectImageType( "rgba8zfloat" );

  // this->OSPRayFactory->selectImageTraverser( "tiled(-square)" );
  // //this->OSPRayFactory->selectImageTraverser( "deadline()" );

  // this->OSPRayFactory->selectLoadBalancer( "workqueue" );

  // if (this->EnableShadows)
  //   {
  //   this->OSPRayFactory->selectShadowAlgorithm( "hard(-attenuateShadows)" );
  //   }
  // else
  //   {
  //   this->OSPRayFactory->selectShadowAlgorithm( "noshadows" );
  //   }

  // if (this->Samples <= 1)
  //   {
  //   this->OSPRayFactory->selectPixelSampler( "singlesample" );
  //   }
  // else
  //   {
  //   char buff[80];
  //   sprintf(buff, "regularsample(-numberOfSamples %d)", this->Samples);
  //   this->OSPRayFactory->selectPixelSampler(buff);
  //   //this->OSPRayFactory->selectPixelSampler(
  //   //"jittersample(-numberOfSamples 16)");
  //   }

  // this->OSPRayFactory->selectRenderer( "raytracer" );

  // this->DefaultLight = NULL;
  #endif


}

//----------------------------------------------------------------------------
vtkOSPRayRenderer::~vtkOSPRayRenderer()
{
  if (this->osp_framebuffer)
  {
    ospFreeFrameBuffer(this->osp_framebuffer);
    this->osp_framebuffer = NULL;
  }

  if (this->ColorBuffer)
  {
   delete[] this->ColorBuffer;
    this->ColorBuffer = NULL;
  }

  if (this->DepthBuffer)
  {
    delete[] this->DepthBuffer;
    this->DepthBuffer = NULL;
  }
  //cerr << "MR(" << this << ") DESTROY " << this->OSPRayManager << " "
  //     << this->OSPRayManager->GetReferenceCount() << endl;

  // if (this->DefaultLight && this->OSPRayLightSet)
  //   {
  //   // OSPRay::Callback::create( this->OSPRayLightSet, &OSPRay::LightSet::remove,
  //                            // this->DefaultLight );
  //   this->DefaultLight = NULL;
  //   }

  // this->OSPRayManager->Delete();

  // if (this->ColorBuffer)
  //   {
  //   delete[] this->ColorBuffer;
  //   }
  // if (this->DepthBuffer)
  //   {
  //   delete[] this->DepthBuffer;
  //   }
}

//----------------------------------------------------------------------------
void vtkOSPRayRenderer::InitEngine()
{
      // vtkRenderWindowInteractor* iren = vtkRenderWindowInteractor::New(); 
   // iren->SetRenderWindow(this->GetRenderWindow());
  //  vtkRenderWindowInteractor* iren = this->GetRenderWindow()->GetInteractor();
  //  // iren->Initialize();

  //  // iren->AddObserver TimerEvent {if {$val == 0} exit}

  //  // Sign up to receive TimerEvent
  // vtkSmartPointer<vtkTimerCallback> cb = 
  //   vtkSmartPointer<vtkTimerCallback>::New();
  // iren->AddObserver(vtkCommand::TimerEvent, cb);


  //  iren->CreateRepeatingTimer(1000);

  //  printf("starting timer!\n");
  //  iren->Start();
  //  printf("started timer!\n");
   this->EngineInited = true;
}

//----------------------------------------------------------------------------
void vtkOSPRayRenderer::SetBackground(double r, double g, double b)
{
  OSPRenderer oRenderer = (OSPRenderer)this->OSPRayManager->OSPRayRenderer;
  OSPRenderer vRenderer = (OSPRenderer)this->OSPRayManager->OSPRayVolumeRenderer;
  ospSet3f(oRenderer,"bgColor",r,g,b);
  ospSet3f(vRenderer,"bgColor",r,g,b);

  backgroundRGB[0] = r;
  backgroundRGB[1] = g;
  backgroundRGB[2] = b;
}

//----------------------------------------------------------------------------
void vtkOSPRayRenderer::ClearLights(void)
{
}

//----------------------------------------------------------------------------
void vtkOSPRayRenderer::Clear()
{
}

//----------------------------------------------------------------------------
// Ask lights to load themselves into graphics pipeline.
int vtkOSPRayRenderer::UpdateLights()
{

  OSPRenderer renderer = ((OSPRenderer)this->OSPRayManager->OSPRayRenderer);
      // std::vector<OSPLight> pointLights;
      // std::vector<OSPLight> directionalLights;
  std::vector<OSPLight> lights;

      // std::vector<OSPLight> pointLights;
    // cout << "msgView: Adding a hard coded directional light as the sun." << endl;
      /*
    OSPLight ospLight = ospNewLight(renderer, "DirectionalLight");
    ospSetString(ospLight, "name", "sun" );
    ospSet3f(ospLight, "color", .6, .6, .55);
    ospSet3f(ospLight, "direction", -1, -1, 0);
    ospCommit(ospLight);
    directionalLights.push_back(ospLight);
    OSPLight ospLight2 = ospNewLight(renderer, "DirectionalLight");
    ospSetString(ospLight2, "name", "shadow" );
    ospSet3f(ospLight2, "color", .3, .35, .4);
    ospSet3f(ospLight2, "direction", 1, .5, 0);
    ospCommit(ospLight2);
    directionalLights.push_back(ospLight);
    */

  // convert VTK lights into OSPRay lights
    vtkCollectionSimpleIterator sit;
    this->Lights->InitTraversal( sit );

  // TODO: schedule ClearLight here?
    vtkLight *vLight = NULL;
    bool noneOn = true;
    for ( this->Lights->InitTraversal( sit );
      ( vLight = this->Lights->GetNextLight( sit ) ) ; )
    {
      if ( vLight->GetSwitch() )
      {
        noneOn = false;
      }
    //OSPRay lights set intensity to 0.0 if switched off, so render regardless
    // vLight->Render( this, 0 /* not used */ );
      vtkLight* light = vLight;

      double *color, *position, *focal, direction[3];

  // OSPRay Lights only have one "color"
      color    = light->GetDiffuseColor();
      position = light->GetTransformedPosition();
      focal    = light->GetTransformedFocalPoint();

      if (light->GetPositional())
      {
        OSPLight ospLight = ospNewLight(renderer, "OBJPointLight");
        ospSetString(ospLight, "name", "point" );
        ospSet3f(ospLight, "color", color[0],color[1],color[2]);
        ospSet3f(ospLight, "position", position[0],position[1],position[2]);
        ospCommit(ospLight);
        lights.push_back(ospLight);
    // std::cout << " adding point light" << color[0] << " " << color[1] << " " << color[2] << " \n";
    // OSPData pointLightArray = ospNewData(pointLights.size(), OSP_OBJECT, &pointLights[0], 0);
    // ospSetData(renderer, "pointLights", pointLightArray); 
    // this->OSPRayLight = new OSPRay::PointLight(
    //   OSPRay::Vector(position[0], position[1], position[2]),
    //   OSPRay::Color(OSPRay::RGBColor(color[0],color[1],color[2])));
      }
      else
      {
    // "direction" in OSPRay means the direction toward light source rather than the
    // direction of rays originate from light source
        direction[0] = position[0] - focal[0];
        direction[1] = position[1] - focal[1];
        direction[2] = position[2] - focal[2];
        OSPLight ospLight = ospNewLight(renderer, "DirectionalLight");
        ospSetString(ospLight, "name", "directional" );
        ospSet3f(ospLight, "color", color[0]*0.4,color[1]*0.4,color[2]*0.4);
        osp::vec3f dir(-direction[0],-direction[1],-direction[2]);
        dir = normalize(dir); 
        ospSet3f(ospLight, "direction", dir.x,dir.y,dir.z);
        ospCommit(ospLight);
        lights.push_back(ospLight);
    // std::cout << " adding directional light" << color[0] << " " << color[1] << " " << color[2] << 
    // direction[0] << " " << direction[1] << " " << direction[2] << " \n";
        // OSPData pointLightArray = ospNewData(directionalLights.size(), OSP_OBJECT, &directionalLights[0], 0);
    // ospSetData(renderer, "directionalLights", pointLightArray);

    // this->OSPRayLight = new OSPRay::DirectionalLight(
    //   OSPRay::Vector(direction[0], direction[1], direction[2]),
    //   OSP
      }

      if (noneOn)
      {
    // if (this->OSPRayLightSet->numLights()==0 )
        {
      // there is no VTK light nor OSPRayLight defined, create a OSPRay headlight
          cerr
          << "No light defined, creating a headlight at camera position" << endl;
      // this->DefaultLight =
      //   new OSPRay::HeadLight( 0, OSPRay::Color( OSPRay::RGBColor( 1, 1, 1 ) ) );
      // this->OSPRayEngine->addTransaction
      //   ("add headlight",
      //    OSPRay::Callback::create( this->OSPRayLightSet, &OSPRay::LightSet::add,
      //                             this->DefaultLight ) );
        }
      }
      else
      {
      }
    }
{
            OSPLight ospLight = ospNewLight(renderer, "DirectionalLight");
        ospSetString(ospLight, "name", "sun" );
        ospSet3f(ospLight, "color", 0.5,0.5,.5);
        ospSet3f(ospLight, "direction", 1,1,1);
        ospCommit(ospLight);
        // lights.push_back(ospLight);
      }
      {
          double* color = this->Ambient;
        OSPLight ospLight = ospNewLight(renderer, "AmbientLight");
    ospSetString(ospLight, "name", "ambient" );
    ospSet3f(ospLight, "color", .5,.5,.5);
    ospSet1f(ospLight, "intensity", 1.0f);
    // ospSet3f(ospLight, "direction", direction[0],direction[1],direction[2]);
    ospCommit(ospLight);
    lights.push_back(ospLight);
  }


    OSPData lightsArray = ospNewData(lights.size(), OSP_OBJECT, &lights[0], 0);
    // ospSetData(renderer, "directionalLights", directionalLightsArray);
    ospSetData(renderer, "lights",lightsArray);
    ospCommit(renderer);


    // OSPData pointLightArray = ospNewData(pointLights.size(), OSP_OBJECT, &pointLights[0], 0);
    // ospSetData(renderer, "pointLights", pointLightArray); 

    return 0;
  }

//----------------------------------------------------------------------------
  vtkCamera* vtkOSPRayRenderer::MakeCamera()
  {
    return vtkOSPRayCamera::New();
  }

//----------------------------------------------------------------------------
  void vtkOSPRayRenderer::UpdateSize()
  {
  #if 0
  if (this->EngineStarted)
    {
    int     OSPRaySize[2];
    int*    renderSize  = NULL;
    bool    stereoDummyArg;

    renderSize = this->GetSize();
    this->GetSyncDisplay()->getCurrentImage()->
      getResolution( stereoDummyArg, OSPRaySize[0], OSPRaySize[1] );

    if (OSPRaySize[0] != renderSize[0] ||
        OSPRaySize[1] != renderSize[1])
      {
      /*
      cerr << "MR(" << this << ") "
           << "Layer: " << this->GetLayer() << ", "
           << "Props: " << this->NumberOfPropsRendered << endl
           << "  OSPRaySize: " << OSPRaySize[0] << ", " << OSPRaySize[1] << ", "
           << "  renderSize: " << renderSize[0] << ", " << renderSize[1] << endl;
      */
      this->GetOSPRayEngine()->addTransaction
        ("resize",
         OSPRay::Callback::create
         (this->GetOSPRayEngine(),
          &OSPRay::OSPRayInterface::changeResolution,
          0, renderSize[0], renderSize[1],
          true));
      this->GetSyncDisplay()->doneRendering();
      this->GetSyncDisplay()->waitOnFrameReady();
      }
          // ospray::vec2i newSize(renderSize[0],renderSize[1]);
    // ospFramebuffer = ospNewFrameBuffer(newSize,OSP_RGBA_I8);
    }
    #endif
  }

//----------------------------------------------------------------------------
  void vtkOSPRayRenderer::DeviceRender()
  {
  cerr << "MR(" << this << ") DeviceRender" << endl;

  // In ParaView, we are wasting time in rendering the "sync layer" with
  // empty background image just to be dropped in LayerRender(). We just
  // don't start the engine with sync layer.
  // TODO: this may not be the right way to check if it is a sync layer
    if (this->GetLayer() != 0 && this->GetActors()->GetNumberOfItems() == 0)
    {
      return;
    }

    vtkTimerLog::MarkStartEvent("OSPRay Dev Render");

    if (!this->EngineInited )
    {
      this->InitEngine();
    }

    vtkTimerLog::MarkStartEvent("Geometry");

    this->Clear();

    this->UpdateSize();


    hasVolumeHack = false;
    OSPRenderer oRenderer = (OSPRenderer)this->OSPRayManager->OSPRayRenderer;
  this->OSPRayManager->OSPRayModel = ospNewModel();  //Carson: note: static needed, it seems they are freed in scope
  OSPModel oModel = (OSPModel)this->OSPRayManager->OSPRayModel;
  // this->OSPRayManager->OSPRayCamera = ospNewCamera("perspective");
  OSPCamera oCamera = (OSPCamera)this->OSPRayManager->OSPRayCamera;
  ospSetParam(oRenderer,"world",oModel);
  ospSetParam(oRenderer,"model",oModel);
  ospSetParam(oRenderer,"camera",oCamera);


  // this->OSPRayManager->OSPRayModel = ospNewModel();
  // ospSetParam(this->OSPRayManager->OSPRayRenderer,"world", this->OSPRayManager->OSPRayModel);
  // ospSetParam(this->OSPRayManager->OSPRayRenderer,"model", this->OSPRayManager->OSPRayModel);
  ospCommit(this->OSPRayManager->OSPRayModel);
  ospCommit(this->OSPRayManager->OSPRayRenderer);

  // call camera::Render()
  this->UpdateCamera();

  // TODO: call ClearLights here?

  // call Light::Render()
  this->UpdateLightGeometry();
  this->UpdateLights();

  // if (!this->EngineStarted)
  //   {
  //   this->OSPRayEngine->beginRendering( false );
  //   this->EngineStarted = true;
  //   this->GetSyncDisplay()->waitOnFrameReady();
  //   }

  this->UpdateGeometry();

  vtkTimerLog::MarkEndEvent("Geometry");

  vtkTimerLog::MarkStartEvent("Total LayerRender");
    // this->GetSyncDisplay()->doneRendering();
  // this->GetSyncDisplay()->waitOnFrameReady();
  // ospCommit(this->OSPRayManager->OSPRayModel);
  this->LayerRender();

  vtkTimerLog::MarkEndEvent("Total LayerRender");

  vtkTimerLog::MarkEndEvent("OSPRay Dev Render");

//
//try to hack in updates for progressive rendering!
//
  // this->GetRenderWindow()->Modified();
//       vtkActorCollection* actors = this->GetActors();
//   printf("num actors: %d\n", this->GetActors()->GetNumberOfItems());
//   actors->InitTraversal();
//   for(vtkIdType i = 0; i < actors->GetNumberOfItems(); i++)
//   {
//   vtkActor* act = actors->GetNextActor();
//   if (act)
//   {
//     printf("found actor %d\n", i);
//   act->Modified();
//   vtkPolyDataMapper* mapper = dynamic_cast<vtkPolyDataMapper*>(act->GetMapper());
//   if (mapper)
//   {
//     printf("found mapper\n");
//   vtkPolyData* dat = mapper->GetInput();
//   if (dat)
//   {
//   printf("updating actor\n");
//   dat->Modified();
// }

// }
// }
//   // act->Update();
//   }
}

//----------------------------------------------------------------------------
// let the renderer display itself appropriately based on its layer index
void vtkOSPRayRenderer::LayerRender()
{
  //TODO:
  //this needs to be simplified. Now that UpdateSize happens before this
  //vtk's size and OSPRay's size should always be the same so the ugly
  //conversion and minsize safety check should go away
  int     i, j;
  int     rowLength,  OSPRaySize[2];
  int     minWidth,   minHeight;
  int     hOSPRayDiff, hRenderDiff;
  int     renderPos[2];
  int*    renderSize  = NULL;
  int*    renWinSize  = NULL;
  bool    stereoDumy;
  float*  OSPRayBuffer = NULL;
  double* renViewport = NULL;
  // const   OSPRay::SimpleImageBase* OSPRayBase = NULL;

  // collect some useful info
  renderSize = this->GetSize();
  renWinSize = this->GetRenderWindow()->GetActualSize();
  renViewport= this->GetViewport();
  renderPos[0] = int( renViewport[0] * renWinSize[0] + 0.5f );
  renderPos[1] = int( renViewport[1] * renWinSize[1] + 0.5f );
  //
  // this->GetSyncDisplay()->getCurrentImage()->
    // getResolution( stereoDumy, OSPRaySize[0], OSPRaySize[1] );
  // OSPRayBase = dynamic_cast< const OSPRay::SimpleImageBase * >
    // ( this->GetSyncDisplay()->getCurrentImage() );
  // rowLength = OSPRayBase->getRowLength();

  // for window re-sizing
  // minWidth    = ( OSPRaySize[0] < renderSize[0] )
    // ? OSPRaySize[0] : renderSize[0];
  // minHeight   = ( OSPRaySize[1] < renderSize[1] )
  minWidth = renderSize[0];
  minHeight =renderSize[1];
  hOSPRayDiff = 0;
  hRenderDiff = 0;
    // ? OSPRaySize[1] : renderSize[1];
  // hOSPRayDiff  = OSPRaySize[1] - minHeight;
  // hRenderDiff = renderSize[1] - minHeight;

  // vtkTimerLog::MarkStartEvent("ThreadSync");
  // let the render threads draw what we've asked them to
  // this->GetSyncDisplay()->doneRendering();
  // this->GetSyncDisplay()->waitOnFrameReady();
  // vtkTimerLog::MarkEndEvent("ThreadSync");

  // memory allocation and acess to the OSPRay image
  int size = renderSize[0]*renderSize[1];
  if (this->ImageX != renderSize[0] || this->ImageY != renderSize[1])
  {
    this->ImageX = renderSize[0];
    this->ImageY = renderSize[1];

    if (this->ColorBuffer) delete[] this->ColorBuffer;
    this->ColorBuffer = new float[ size ];

    if (this->DepthBuffer) delete[] this->DepthBuffer;
    this->DepthBuffer = new float[ size ];

    if (this->osp_framebuffer) ospFreeFrameBuffer(this->osp_framebuffer);
    this->osp_framebuffer = ospNewFrameBuffer(osp::vec2i(renderSize[0], renderSize[1]), OSP_RGBA_I8, OSP_FB_COLOR | OSP_FB_DEPTH);
  }

  if (hasVolumeHack)
  {
   OSPRenderer vRenderer = (OSPRenderer)this->OSPRayManager->OSPRayVolumeRenderer;
   OSPModel vdModel = (OSPModel)this->OSPRayManager->OSPRayDynamicModel;
   ospSetObject(vRenderer, "dynamic_model", vdModel);
   OSPModel vModel = (OSPModel)this->OSPRayManager->OSPRayVolumeModel;
   OSPCamera oCamera = (OSPCamera)this->OSPRayManager->OSPRayCamera;

   ospSetParam(vRenderer,"world",vModel);
   ospSetParam(vRenderer,"dynamic_model",vdModel);
   ospSetParam(vRenderer,"model",vModel);
   ospSetParam(vRenderer,"camera",oCamera);

   ospCommit(vModel);
   ospCommit(vdModel);
   ospCommit(vRenderer);

   ospRenderFrame(this->osp_framebuffer,vRenderer);
  }
  else
  {
    OSPRenderer renderer = ((OSPRenderer)this->OSPRayManager->OSPRayRenderer);
    OSPModel ospModel = ((OSPModel)this->OSPRayManager->OSPRayModel);

    ospCommit(renderer);
    ospCommit(ospModel);

    ospRenderFrame(this->osp_framebuffer,renderer);
  }

  double *clipValues = this->GetActiveCamera()->GetClippingRange();
  double viewAngle = this->GetActiveCamera()->GetViewAngle();

  //
  // Closest point is center of near clipping plane - farthest is
  // corner of far clipping plane
  double clipMin = clipValues[0];
  double clipMax = clipValues[1] / pow(cos(viewAngle / 2.0), 2.0);
  double clipDiv = 1.0 / (clipMax - clipMin);

  const void *b = ospMapFrameBuffer(this->osp_framebuffer, OSP_FB_DEPTH);

  float *s = (float *)b;
  float *d = this->DepthBuffer;
  for (int i = 0; i < size; i++, s++, d++)
    *d = isinf(*s) ? 1.0 : (*s - clipMin) * clipDiv;

  ospUnmapFrameBuffer(b, this->osp_framebuffer);

  this->GetRenderWindow()->MakeCurrent();
  glDepthFunc(GL_ALWAYS);

  this->GetRenderWindow()->SetZbufferData(renderPos[0], renderPos[1],
            renderPos[0] + renderSize[0] - 1, renderPos[1] + renderSize[1] - 1, this->DepthBuffer);

  const void* rgba = ospMapFrameBuffer(this->osp_framebuffer);
  memcpy((void *)this->ColorBuffer, rgba, size*sizeof(float));
  ospUnmapFrameBuffer(rgba, this->osp_framebuffer);


  vtkTimerLog::MarkStartEvent("Image Conversion");

  // let layer #0 initialize GL depth buffer
  if ( this->GetLayer() == 0 )
  {
    this->GetRenderWindow()->
      SetRGBACharPixelData( renderPos[0],  renderPos[1],
                            renderPos[0] + renderSize[0] - 1,
                            renderPos[1] + renderSize[1] - 1,
                            (unsigned char*)this->ColorBuffer, 0, 0 );
    glFinish();
  }
  else
  {
    //layers on top add the colors of their non background pixels
    unsigned char*  GLbakBuffer = NULL;
    GLbakBuffer = this->GetRenderWindow()->
      GetRGBACharPixelData( renderPos[0],  renderPos[1],
                            renderPos[0] + renderSize[0] - 1,
                            renderPos[1] + renderSize[1] - 1, 0 );

    bool anyhit = false;
    unsigned char *optr = GLbakBuffer;
    unsigned char *iptr = (unsigned char*)this->ColorBuffer;
    float *zptr = this->DepthBuffer;
    for ( j = 0; j < renderSize[1]; j++)
    {
      for ( i = 0; i < renderSize[0]; i++)
        {
        const float z = *zptr;
        if (z > 0 && z < 1.0)
          {
          anyhit = true;
          *(optr+0) = *(iptr+0);
          *(optr+1) = *(iptr+1);
          *(optr+2) = *(iptr+2);
          *(optr+3) = *(iptr+3);
          }
        optr+=4;
        iptr+=4;
        zptr++;
        }
    }

    if (anyhit)
      {
      // submit the modified RGB colors to GL BACK buffer
      this->GetRenderWindow()->
        SetRGBACharPixelData( renderPos[0],  renderPos[1],
          renderPos[0] + renderSize[0] - 1,
          renderPos[1] + renderSize[1] - 1,
          GLbakBuffer, 0, 0 );
      }

    delete [] GLbakBuffer;
  }

  vtkTimerLog::MarkEndEvent("Image Conversion");
}

//----------------------------------------------------------------------------
void vtkOSPRayRenderer::PrintSelf( ostream& os, vtkIndent indent )
{
  this->Superclass::PrintSelf( os, indent );
}

//----------------------------------------------------------------------------
void vtkOSPRayRenderer::SetNumberOfWorkers( int newval )
{
  if (this->NumberOfWorkers == newval)
  {
    return;
  }
}

//----------------------------------------------------------------------------
void vtkOSPRayRenderer::SetEnableShadows( int newval )
{
  if (this->EnableShadows == newval)
  {
    return;
  }

  OSPRenderer renderer = ((OSPRenderer)this->OSPRayManager->OSPRayRenderer);
  ospSet1i(renderer,"shadowsEnabled", this->EnableShadows);
  ospCommit(renderer);

  this->EnableShadows = newval;
}

//----------------------------------------------------------------------------
void vtkOSPRayRenderer::SetSamples( int newval )
{
  std::cerr << "vtkOSPRayRenderer::SetSamples " << newval << "\n";
  #if 1
  if (this->Samples == newval || newval < 1)
  {
    return;
  }

  this->Samples = newval;


  OSPRenderer renderer = ((OSPRenderer)this->OSPRayManager->OSPRayRenderer);

  PRINT(renderer);
  Assert(renderer);

  ospSet1i(renderer,"spp",Samples);
  ospCommit(renderer);

  #endif
}

//----------------------------------------------------------------------------
void vtkOSPRayRenderer::SetEnableAO( int newval )
{
  std::cerr << "vtkOSPRayRenderer::SetEnableAO " << newval << "\n";
  #if 1
  if (this->EnableAO == newval)
  {
    return;
  }

  this->EnableAO = newval;

  OSPModel oModel = (OSPModel)this->OSPRayManager->OSPRayModel;
  OSPCamera oCamera = (OSPCamera)this->OSPRayManager->OSPRayCamera;

  if (newval != 0)
  {
    std::cout << "using ao4" << std::endl;
    this->OSPRayManager->OSPRayRenderer = (osp::Renderer*)ospNewRenderer("ao4");
  }
  else
  {
    std::cout << "using obj" << std::endl;
    this->OSPRayManager->OSPRayRenderer = (osp::Renderer*)ospNewRenderer("obj");
  // this->OSPRayManager->OSPRayRenderer =  (osp::Renderer*)ospNewRenderer("raycast_volume_renderer");
  }
  OSPRenderer oRenderer = (OSPRenderer)this->OSPRayManager->OSPRayRenderer;

  Assert(oRenderer != NULL && "could not create renderer");

  // ospCommit(oRenderer);
  ospSetParam(oRenderer,"dynamic_model",ospNewModel());
  ospSetParam(oRenderer,"world",oModel);
  ospSetParam(oRenderer,"model",oModel);
  ospSetParam(oRenderer,"camera",oCamera);
  ospCommit(oRenderer);

  ospSet1i(oRenderer,"spp",Samples);

  ospCommit(oRenderer);

  // SetBackground(backgroundRGB[0], backgroundRGB[1], backgroundRGB[2]);

  cout << "new renderer: " << static_cast<void*>(oRenderer) << ", samples: " << this->Samples << endl;

  // loop through actors and set mtime to force update
  // this will force the actors to create new materials that will be
  // associated with the new renderer
  vtkActorCollection *actorList = this->GetActors();
  actorList->InitTraversal();

  int numActors = actorList->GetNumberOfItems();
  for(int i=0; i<numActors; i++) {
    vtkActor *a = actorList->GetNextActor();
    a->Modified();
  }

  #endif
}

//----------------------------------------------------------------------------
void vtkOSPRayRenderer::SetMaxDepth( int newval )
{
  if (this->MaxDepth == newval)
  {
    return;
  }

  this->MaxDepth = newval;
}
