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

//----------------------------------------------------------------------------
vtkOSPRayRenderer::vtkOSPRayRenderer()
//:
  //EngineInited( false ), EngineStarted( false ),
  //IsStereo( false ), OSPRayScene( 0 ), OSPRayWorldGroup( 0 ),
  //OSPRayLightSet( 0 ), OSPRayCamera( 0 ), SyncDisplay( 0 )
{
  //cerr << "MR(" << this << ") CREATE" << endl;

  // Default options
  this->NumberOfWorkers = 1;
  this->EnableShadows = 0;
  this->Samples = 1;
  this->MaxDepth = 5;

  // the default global ambient light created by vtkRenderer is too bright.
  this->SetAmbient( 0.1, 0.1, 0.1 );

  this->OSPRayManager = vtkOSPRayManager::New();



  // _width = _height = 0;
  // setSize(params.width,params.height);
      // Assert(camera != NULL && "could not create camera");
      // ospSet3f(camera,"pos",-1,1,-1);
      // ospSet3f(camera,"dir",+1,-1,+1);
      // ospCommit(camera);

// {
  // static OSPRenderer oRenderer = ospNewRenderer("raycast_eyelight");
  // static OSPRenderer oRenderer = ospNewRenderer("ao16");

  // this->OSPRayRenderer = ospNewRenderer("obj");
  // this->OSPRayRenderer = ospNewRenderer("pathtracer");
  bool ao = EnableAO;
  EnableAO=-1;
  // SetEnableAO(ao);
  if (EnableAO)
    this->OSPRayManager->OSPRayRenderer = (osp::Renderer*)ospNewRenderer("ao4");
  else
    this->OSPRayManager->OSPRayRenderer = (osp::Renderer*)ospNewRenderer("obj");
  OSPRenderer oRenderer = (OSPRenderer)this->OSPRayManager->OSPRayRenderer;
  this->OSPRayManager->OSPRayModel = ospNewModel();  //Carson: note: static needed, it seems they are freed in scope
  OSPModel oModel = (OSPModel)this->OSPRayManager->OSPRayModel;
  this->OSPRayManager->OSPRayCamera = ospNewCamera("perspective");
  OSPCamera oCamera = (OSPCamera)this->OSPRayManager->OSPRayCamera;

  // renderer = ospNewRenderer("ao16");
  // renderer = ospNewRenderer("obj");
  Assert(oRenderer != NULL && "could not create renderer");

  ospSetParam(oRenderer,"world",oModel);
  ospSetParam(oRenderer,"model",oModel);
  ospSetParam(oRenderer,"camera",oCamera);
  ospSet1i(oRenderer,"spp",Samples);
  ospSet3f(oRenderer,"bgColor",0.83,0.35,0.43);
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
  this->ImageSize = -1;

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
  #if 0
  //cerr << "MR(" << this << ")#" << this->GetLayer() << " INIT "
  //     << this->OSPRayManager << endl;
  this->OSPRayManager->StartEngine(this->MaxDepth,
                                  this->GetBackground(),
                                  this->Ambient,
                                  this->IsStereo,
                                  this->GetSize());

  this->OSPRayScene = this->OSPRayManager->GetOSPRayScene();
  this->OSPRayWorldGroup = this->OSPRayManager->GetOSPRayWorldGroup();
  this->OSPRayLightSet = this->OSPRayManager->GetOSPRayLightSet();
  this->OSPRayCamera = this->OSPRayManager->GetOSPRayCamera();
  this->SyncDisplay = this->OSPRayManager->GetSyncDisplay();
  this->ChannelId = this->OSPRayManager->GetChannelId();

  this->EngineInited = true;
  #endif
}

//----------------------------------------------------------------------------
void vtkOSPRayRenderer::SetBackground(double r, double g, double b)
{
  OSPRenderer oRenderer = (OSPRenderer)this->OSPRayManager->OSPRayRenderer;
  ospSet3f(oRenderer,"bgColor",r,g,b);
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
  #if 0
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
    vLight->Render( this, 0 /* not used */ );
    }

  if (noneOn)
    {
    if (this->OSPRayLightSet->numLights()==0 )
      {
      // there is no VTK light nor OSPRayLight defined, create a OSPRay headlight
      cerr
        << "No light defined, creating a headlight at camera position" << endl;
      this->DefaultLight =
        new OSPRay::HeadLight( 0, OSPRay::Color( OSPRay::RGBColor( 1, 1, 1 ) ) );
      this->OSPRayEngine->addTransaction
        ("add headlight",
         OSPRay::Callback::create( this->OSPRayLightSet, &OSPRay::LightSet::add,
                                  this->DefaultLight ) );
      }
    }
  else
    {
    if (this->DefaultLight)
      {
      OSPRay::Callback::create( this->OSPRayLightSet, &OSPRay::LightSet::remove,
                               this->DefaultLight );
      this->DefaultLight = NULL;
      }
    }
  #endif

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
  //cerr << "MR(" << this << ") DeviceRender" << endl;

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
}

//----------------------------------------------------------------------------
// let the renderer display itself appropriately based on its layer index
void vtkOSPRayRenderer::LayerRender()
{

#if 1

  //TODO:
  //this needs to be simplified. Now that UpdateSize happens before this
  //vtk's size and OSPRay's size should always be the same so the ugly
  //conversion and minsize safety check should go away
  int     i, j;
  int     rowLength,  OSPRaySize[2];
  int     minWidth,   minHeight;
  int     hOSPRayDiff, hRenderDiff;
  int     renderPos0[2];
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
  renderPos0[0] = int( renViewport[0] * renWinSize[0] + 0.5f );
  renderPos0[1] = int( renViewport[1] * renWinSize[1] + 0.5f );
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


#if USE_OSPRAY
    ospray::vec2i newSize(renderSize[0],renderSize[1]);
// printf("setSize 2");
    // if (framebuffer) ospFreeFrameBuffer(framebuffer);
// printf("setSize 3");
  static OSPFrameBuffer framebuffer = ospNewFrameBuffer(newSize,OSP_RGBA_I8);
  // memory allocation and acess to the OSPRay image
  int size = renderSize[0]*renderSize[1];
 static  ospray::vec2i oldSize(renderSize[0],renderSize[1]);
  if (this->ImageSize != size)
    {
      if (this->ColorBuffer)
    delete[] this->ColorBuffer;
  if (this->DepthBuffer)
    delete[] this->DepthBuffer;
    this->ImageSize = size;
    this->DepthBuffer = new float[ size ];
    this->ColorBuffer = new float[ size ];
// printf("setSize 2");
    // if (framebuffer) ospFreeFrameBuffer(framebuffer);
// printf("setSize 3");
    }
    if (oldSize[0] != newSize[0] || oldSize[1] != newSize[1])
    {
     framebuffer = ospNewFrameBuffer(newSize,OSP_RGBA_I8);
    }
    oldSize = newSize;
    //TODO: save framebuffer
  // OSPRayBuffer = static_cast< float * >( OSPRayBase->getRawData(0) );

  // update this->ColorBuffer and this->DepthBuffer from the OSPRay
  // RGBA8ZfloatPixel array
  double *clipValues = this->GetActiveCamera()->GetClippingRange();
  double depthScale  = 1.0f / ( clipValues[1] - clipValues[0] );

  //
  //  OSPRay
  //
  #if USE_OSPRAY
  // printf("ospRender\n");
  OSPRenderer renderer = ((OSPRenderer)this->OSPRayManager->OSPRayRenderer);
  // std::cout << "renderer: " << this->OSPRayManager->ospRenderer << std::endl;
  // PRINT(renderer);
  // OSPFramebuffer framebuffer = this->OSPRayManager->ospFramebuffer;
  OSPModel ospModel = ((OSPModel)this->OSPRayManager->OSPRayModel);
  // PRINT(ospModel);
  ospCommit(ospModel);

              //TODO: Need to figure out where we're going to read lighting data from
    //begin light test
    std::vector<OSPLight> pointLights;
    // cout << "msgView: Adding a hard coded directional light as the sun." << endl;
    OSPLight ospLight = ospNewLight(renderer, "DirectionalLight");
    ospSetString(ospLight, "name", "sun" );
    ospSet3f(ospLight, "color", .6, .6, .55);
    ospSet3f(ospLight, "direction", -1, -1, 0);
    ospCommit(ospLight);
    pointLights.push_back(ospLight);
    OSPLight ospLight2 = ospNewLight(renderer, "DirectionalLight");
    ospSetString(ospLight2, "name", "shadow" );
    ospSet3f(ospLight2, "color", .3, .35, .4);
    ospSet3f(ospLight2, "direction", 1, .5, 0);
    ospCommit(ospLight2);
    pointLights.push_back(ospLight);
    OSPData pointLightArray = ospNewData(pointLights.size(), OSP_OBJECT, &pointLights[0], 0);
    ospSetData(renderer, "directionalLights", pointLightArray);
// updateCamera();
  ospCommit(renderer);
  ospCommit(ospModel);
    //end light test

  // printf("render\n");




  ospRenderFrame(framebuffer,renderer);

  float* ospBuffer = (float *) ospMapFrameBuffer(framebuffer);
  #endif

       // this->OSPRayManager->ospModel = ospNewModel();
      // ospSetParam(renderer,"world",this->OSPRayManager->ospModel);
  // ospSetParam(renderer,"model",this->OSPRayManager->ospModel);

  // ospCommit(renderer);
  #endif

  vtkTimerLog::MarkStartEvent("Image Conversion");
  for ( j = 0; j < minHeight; j ++ )
    {
    // there are two floats in each pixel in OSPRay buffer
    int OSPRayIndex = ( ( j + hOSPRayDiff  ) * rowLength     ) * 2;
    // there is only one float in each pixel in the GL RGBA or Z buffer
    int tupleIndex = ( ( j + hRenderDiff ) * renderSize[0] ) * 1;

    for ( i = 0; i < minWidth; i ++ )
      {
      // this->ColorBuffer[ tupleIndex + i ]
                         // = OSPRayBuffer[ OSPRayIndex + i*2     ];
                         #if USE_OSPRAY
      this->ColorBuffer[ tupleIndex + i ]
                         = ospBuffer[ tupleIndex + i  ];
                         // char testBuff[] = {128,128,255,255};
                         // this->ColorBuffer[ tupleIndex + i ]
                         // = *((float*)testBuff);
                         #endif
      // float depthValue   = OSPRayBuffer[ OSPRayIndex + i*2 + 1 ];
      // normalize the depth values to [ 0.0f, 1.0f ], since we are using a
      // software buffer for Z values and never write them to OpenGL buffers,
      // we don't have to clamp them any more
      // TODO: On a second thought, we probably don't even have to normalize Z
      // values at all
      // this->DepthBuffer[ tupleIndex + i ]
                         // = ( depthValue - clipValues[0] ) * depthScale;
      }
    }

  // let layer #0 initialize GL depth buffer
  if ( this->GetLayer() == 0 )
    {
    // this->GetRenderWindow()->
    //   SetZbufferData( renderPos0[0],  renderPos0[1],
    //                   renderPos0[0] + renderSize[0] - 1,
    //                   renderPos0[1] + renderSize[1] - 1,
    //                   this->DepthBuffer );

    this->GetRenderWindow()->
      SetRGBACharPixelData( renderPos0[0],  renderPos0[1],
                            renderPos0[0] + renderSize[0] - 1,
                            renderPos0[1] + renderSize[1] - 1,
                            (unsigned char*)this->ColorBuffer, 0, 0 );
    glFinish();
    }
  else
    {
    //layers on top add the colors of their non background pixels
    unsigned char*  GLbakBuffer = NULL;
    GLbakBuffer = this->GetRenderWindow()->
      GetRGBACharPixelData( renderPos0[0],  renderPos0[1],
                            renderPos0[0] + renderSize[0] - 1,
                            renderPos0[1] + renderSize[1] - 1, 0 );

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
        SetRGBACharPixelData( renderPos0[0],  renderPos0[1],
          renderPos0[0] + renderSize[0] - 1,
          renderPos0[1] + renderSize[1] - 1,
          GLbakBuffer, 0, 0 );
      }

    delete [] GLbakBuffer;
    }
    #if 1
    //
    // ospray
    //
    ospUnmapFrameBuffer(ospBuffer,framebuffer);
    // this->OSPRayManager->ospModel = ospNewModel();
    // ospSetParam(renderer,"world",this->OSPRayManager->ospModel);
    // ospSetParam(renderer,"model",this->OSPRayManager->ospModel);

    // ospCommit(renderer);
  //
  // - ospray
  //
  #endif

  //cerr << "MR(" << this << ") release" << endl;
  vtkTimerLog::MarkEndEvent("Image Conversion");


#endif

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
    printf("using ao\n");
    this->OSPRayManager->OSPRayRenderer = (osp::Renderer*)ospNewRenderer("ao4");
  }
  else
    this->OSPRayManager->OSPRayRenderer = (osp::Renderer*)ospNewRenderer("obj");
  OSPRenderer oRenderer = (OSPRenderer)this->OSPRayManager->OSPRayRenderer;

  Assert(oRenderer != NULL && "could not create renderer");

  ospSetParam(oRenderer,"world",oModel);
  ospSetParam(oRenderer,"model",oModel);
  ospSetParam(oRenderer,"camera",oCamera);

  ospCommit(oRenderer);
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
