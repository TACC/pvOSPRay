/*=========================================================================

  Program:   Visualization Toolkit
  Module:    vtkOSPRayRenderer.cxx

  Copyright (c) Ken Martin, Will Schroeder, Bill Lorensen
  All rights reserved.
  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
/*=========================================================================

  Program:   VTK/ParaView Los Alamos National Laboratory Modules (PVLANL)
  Module:    vtkOSPRayRenderer.cxx

Copyright (c) 2007, Los Alamos National Security, LLC

All rights reserved.

Copyright 2007. Los Alamos National Security, LLC.
This software was produced under U.S. Government contract DE-AC52-06NA25396
for Los Alamos National Laboratory (LANL), which is operated by
Los Alamos National Security, LLC for the U.S. Department of Energy.
The U.S. Government has rights to use, reproduce, and distribute this software.
NEITHER THE GOVERNMENT NOR LOS ALAMOS NATIONAL SECURITY, LLC MAKES ANY WARRANTY,
EXPRESS OR IMPLIED, OR ASSUMES ANY LIABILITY FOR THE USE OF THIS SOFTWARE.
If software is modified to produce derivative works, such modified software
should be clearly marked, so as not to confuse it with the version available
from LANL.

Additionally, redistribution and use in source and binary forms, with or
without modification, are permitted provided that the following conditions
are met:
-   Redistributions of source code must retain the above copyright notice,
    this list of conditions and the following disclaimer.
-   Redistributions in binary form must reproduce the above copyright notice,
    this list of conditions and the following disclaimer in the documentation
    and/or other materials provided with the distribution.
-   Neither the name of Los Alamos National Security, LLC, Los Alamos National
    Laboratory, LANL, the U.S. Government, nor the names of its contributors
    may be used to endorse or promote products derived from this software
    without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY LOS ALAMOS NATIONAL SECURITY, LLC AND CONTRIBUTORS
"AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
ARE DISCLAIMED. IN NO EVENT SHALL LOS ALAMOS NATIONAL SECURITY, LLC OR
CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

=========================================================================*/

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
//ospray
//
#if USE_OSPRAY
#include "ospray/ospray.h"
#include "ospray/common/OSPCommon.h"
#endif

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
  //IsStereo( false ), MantaScene( 0 ), MantaWorldGroup( 0 ),
  //MantaLightSet( 0 ), MantaCamera( 0 ), SyncDisplay( 0 )
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
  if (EnableAO)
    this->OSPRayManager->OSPRayRenderer = ospNewRenderer("ao4");
  else
    this->OSPRayManager->OSPRayRenderer = ospNewRenderer("obj");
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
  ospSet3f(oRenderer,"bgColor",0.8,0.8,0.8);
  ospCommit(oModel);
  ospCommit(oCamera);
  ospCommit(oRenderer);
  PRINT(oRenderer);
  PRINT(oModel);
  PRINT(oCamera);
  #if 1

  // this->MantaEngine = this->OSPRayManager->GetMantaEngine();
  // this->MantaEngine->changeNumWorkers( this->NumberOfWorkers );

  // this->MantaFactory = this->OSPRayManager->GetMantaFactory();

  this->ColorBuffer = NULL;
  this->DepthBuffer = NULL;
  this->ImageSize = -1;

  // this->MantaFactory->selectImageType( "rgba8zfloat" );

  // this->MantaFactory->selectImageTraverser( "tiled(-square)" );
  // //this->MantaFactory->selectImageTraverser( "deadline()" );

  // this->MantaFactory->selectLoadBalancer( "workqueue" );

  // if (this->EnableShadows)
  //   {
  //   this->MantaFactory->selectShadowAlgorithm( "hard(-attenuateShadows)" );
  //   }
  // else
  //   {
  //   this->MantaFactory->selectShadowAlgorithm( "noshadows" );
  //   }

  // if (this->Samples <= 1)
  //   {
  //   this->MantaFactory->selectPixelSampler( "singlesample" );
  //   }
  // else
  //   {
  //   char buff[80];
  //   sprintf(buff, "regularsample(-numberOfSamples %d)", this->Samples);
  //   this->MantaFactory->selectPixelSampler(buff);
  //   //this->MantaFactory->selectPixelSampler(
  //   //"jittersample(-numberOfSamples 16)");
  //   }

  // this->MantaFactory->selectRenderer( "raytracer" );

  // this->DefaultLight = NULL;
  #endif
}

//----------------------------------------------------------------------------
vtkOSPRayRenderer::~vtkOSPRayRenderer()
{
  //cerr << "MR(" << this << ") DESTROY " << this->OSPRayManager << " "
  //     << this->OSPRayManager->GetReferenceCount() << endl;

  // if (this->DefaultLight && this->MantaLightSet)
  //   {
  //   // Manta::Callback::create( this->MantaLightSet, &Manta::LightSet::remove,
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

  this->MantaScene = this->OSPRayManager->GetMantaScene();
  this->MantaWorldGroup = this->OSPRayManager->GetMantaWorldGroup();
  this->MantaLightSet = this->OSPRayManager->GetMantaLightSet();
  this->MantaCamera = this->OSPRayManager->GetMantaCamera();
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
  // if ((this->Background[0] != r)||
  //     (this->Background[1] != g)||
  //     (this->Background[2] != b))
  // {
  // this->Superclass::SetBackground(r,g,b);
  // this->MantaEngine->addTransaction
    // ( "set background",
      // Manta::Callback::create(this, &vtkOSPRayRenderer::InternalSetBackground));
  // };
}

//----------------------------------------------------------------------------
void vtkOSPRayRenderer::InternalSetBackground()
{
  // double *color = this->GetBackground();
  // Manta::ConstantBackground * background = new Manta::ConstantBackground(
  //   Manta::Color(  Manta::RGBColor( color[0], color[1], color[2] )  )  );

  // delete this->MantaScene->getBackground();
  // this->MantaScene->setBackground( background );
}

//----------------------------------------------------------------------------
void vtkOSPRayRenderer::ClearLights(void)
{
  // this->MantaEngine->addTransaction
  //   ( "clear lights",
  //     Manta::Callback::create( this, &vtkOSPRayRenderer::InternalClearLights));
}

//----------------------------------------------------------------------------
void vtkOSPRayRenderer::Clear()
{
  // printf("vtkOSPRayRenderer::Clear\n");
  // this->OSPRayManager->OSPRayModel = ospNewModel();
  // ospSetParam(this->OSPRayManager->OSPRayRenderer,"world", this->OSPRayManager->OSPRayModel);
  // ospSetParam(this->OSPRayManager->OSPRayRenderer,"model", this->OSPRayManager->OSPRayModel);
  // ospCommit(this->OSPRayManager->OSPRayModel);
  // ospCommit(this->OSPRayManager->OSPRayRenderer);
  // this->MantaEngine->addTransaction
  //   ( "clear lights",
  //     Manta::Callback::create( this, &vtkOSPRayRenderer::InternalClearLights));
}

//----------------------------------------------------------------------------
void vtkOSPRayRenderer::InternalClearLights(void)
{
  // if (this->MantaLightSet)
  //   {
  //   delete this->MantaLightSet->getAmbientLight();
  //   for ( unsigned int i = 0; i < this->MantaLightSet->numLights(); i ++ )
  //     {
  //     Manta::Light *light = this->MantaLightSet->getLight( i );
  //     this->MantaLightSet->remove( light );
  //     delete light;
  //     }
  //   }
}

//----------------------------------------------------------------------------
// Ask lights to load themselves into graphics pipeline.
int vtkOSPRayRenderer::UpdateLights()
{
  #if 0
  // convert VTK lights into Manta lights
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
    //manta lights set intensity to 0.0 if switched off, so render regardless
    vLight->Render( this, 0 /* not used */ );
    }

  if (noneOn)
    {
    if (this->MantaLightSet->numLights()==0 )
      {
      // there is no VTK light nor MantaLight defined, create a Manta headlight
      cerr
        << "No light defined, creating a headlight at camera position" << endl;
      this->DefaultLight =
        new Manta::HeadLight( 0, Manta::Color( Manta::RGBColor( 1, 1, 1 ) ) );
      this->MantaEngine->addTransaction
        ("add headlight",
         Manta::Callback::create( this->MantaLightSet, &Manta::LightSet::add,
                                  this->DefaultLight ) );
      }
    }
  else
    {
    if (this->DefaultLight)
      {
      Manta::Callback::create( this->MantaLightSet, &Manta::LightSet::remove,
                               this->DefaultLight );
      this->DefaultLight = NULL;
      }
    }

  return 0;
  #endif
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
    int     mantaSize[2];
    int*    renderSize  = NULL;
    bool    stereoDummyArg;

    renderSize = this->GetSize();
    this->GetSyncDisplay()->getCurrentImage()->
      getResolution( stereoDummyArg, mantaSize[0], mantaSize[1] );

    if (mantaSize[0] != renderSize[0] ||
        mantaSize[1] != renderSize[1])
      {
      /*
      cerr << "MR(" << this << ") "
           << "Layer: " << this->GetLayer() << ", "
           << "Props: " << this->NumberOfPropsRendered << endl
           << "  MantaSize: " << mantaSize[0] << ", " << mantaSize[1] << ", "
           << "  renderSize: " << renderSize[0] << ", " << renderSize[1] << endl;
      */
      this->GetMantaEngine()->addTransaction
        ("resize",
         Manta::Callback::create
         (this->GetMantaEngine(),
          &Manta::MantaInterface::changeResolution,
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

  vtkTimerLog::MarkStartEvent("Manta Dev Render");

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
  //   this->MantaEngine->beginRendering( false );
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

  vtkTimerLog::MarkEndEvent("Manta Dev Render");
}

//----------------------------------------------------------------------------
// let the renderer display itself appropriately based on its layer index
void vtkOSPRayRenderer::LayerRender()
{

#if 1

  //TODO:
  //this needs to be simplified. Now that UpdateSize happens before this
  //vtk's size and manta's size should always be the same so the ugly
  //conversion and minsize safety check should go away
  int     i, j;
  int     rowLength,  mantaSize[2];
  int     minWidth,   minHeight;
  int     hMantaDiff, hRenderDiff;
  int     renderPos0[2];
  int*    renderSize  = NULL;
  int*    renWinSize  = NULL;
  bool    stereoDumy;
  float*  mantaBuffer = NULL;
  double* renViewport = NULL;
  // const   Manta::SimpleImageBase* mantaBase = NULL;

  // collect some useful info
  renderSize = this->GetSize();
  renWinSize = this->GetRenderWindow()->GetActualSize();
  renViewport= this->GetViewport();
  renderPos0[0] = int( renViewport[0] * renWinSize[0] + 0.5f );
  renderPos0[1] = int( renViewport[1] * renWinSize[1] + 0.5f );
  // this->GetSyncDisplay()->getCurrentImage()->
    // getResolution( stereoDumy, mantaSize[0], mantaSize[1] );
  // mantaBase = dynamic_cast< const Manta::SimpleImageBase * >
    // ( this->GetSyncDisplay()->getCurrentImage() );
  // rowLength = mantaBase->getRowLength();

  // for window re-sizing
  // minWidth    = ( mantaSize[0] < renderSize[0] )
    // ? mantaSize[0] : renderSize[0];
  // minHeight   = ( mantaSize[1] < renderSize[1] )
  minWidth = renderSize[0];
  minHeight =renderSize[1];
  hMantaDiff = 0;
  hRenderDiff = 0;
    // ? mantaSize[1] : renderSize[1];
  // hMantaDiff  = mantaSize[1] - minHeight;
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
  // memory allocation and acess to the Manta image
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
  // mantaBuffer = static_cast< float * >( mantaBase->getRawData(0) );

  // update this->ColorBuffer and this->DepthBuffer from the Manta
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
    // there are two floats in each pixel in Manta buffer
    int mantaIndex = ( ( j + hMantaDiff  ) * rowLength     ) * 2;
    // there is only one float in each pixel in the GL RGBA or Z buffer
    int tupleIndex = ( ( j + hRenderDiff ) * renderSize[0] ) * 1;

    for ( i = 0; i < minWidth; i ++ )
      {
      // this->ColorBuffer[ tupleIndex + i ]
                         // = mantaBuffer[ mantaIndex + i*2     ];
                         #if USE_OSPRAY
      this->ColorBuffer[ tupleIndex + i ]
                         = ospBuffer[ tupleIndex + i  ];
                         // char testBuff[] = {128,128,255,255};
                         // this->ColorBuffer[ tupleIndex + i ]
                         // = *((float*)testBuff);
                         #endif
      // float depthValue   = mantaBuffer[ mantaIndex + i*2 + 1 ];
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
  #if 0
  if (this->NumberOfWorkers == newval)
    {
    return;
    }
  this->NumberOfWorkers = newval;
  this->MantaEngine->addTransaction
    ( "set max depth",
      Manta::Callback::create
      (this, &vtkOSPRayRenderer::InternalSetNumberOfWorkers));
  this->Modified();
  #endif
}

//----------------------------------------------------------------------------
void vtkOSPRayRenderer::InternalSetNumberOfWorkers()
{
  // this->MantaEngine->changeNumWorkers( this->NumberOfWorkers );
}

//----------------------------------------------------------------------------
void vtkOSPRayRenderer::SetEnableShadows( int newval )
{
  #if 0
  if (this->EnableShadows == newval)
    {
    return;
    }

  this->EnableShadows = newval;
  this->MantaEngine->addTransaction
    ( "set shadows",
      Manta::Callback::create(this, &vtkOSPRayRenderer::InternalSetShadows));
  this->Modified();
  #endif
}

//----------------------------------------------------------------------------
void vtkOSPRayRenderer::InternalSetShadows()
{
  #if 0
  if (this->EnableShadows)
    {
    this->MantaFactory->selectShadowAlgorithm( "hard(-attenuateShadows)" );
    }
  else
    {
    this->MantaFactory->selectShadowAlgorithm( "noshadows" );
    }
    #endif
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
  // if (!renderer)
    // return;
  PRINT(renderer);
  Assert(renderer);

  // renderer = ospNewRenderer("ao16");
  // renderer = ospNewRenderer("obj");
  // Assert(oRenderer != NULL && "could not create renderer");

  ospSet1i(renderer,"spp",Samples);
  ospCommit(renderer);

  // this->MantaEngine->addTransaction
    // ( "set samples",
      // Manta::Callback::create(this, &vtkOSPRayRenderer::InternalSetSamples));
  // this->Modified();
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


  // OSPRenderer renderer = ((OSPRenderer)this->OSPRayManager->OSPRayRenderer);
  // if (!renderer)
    // return;
  // PRINT(renderer);
  // Assert(renderer);

  OSPModel oModel = (OSPModel)this->OSPRayManager->OSPRayModel;
  OSPCamera oCamera = (OSPCamera)this->OSPRayManager->OSPRayCamera;

  // _width = _height = 0;
  // setSize(params.width,params.height);
      // Assert(camera != NULL && "could not create camera");
      // ospSet3f(camera,"pos",-1,1,-1);
      // ospSet3f(camera,"dir",+1,-1,+1);
      // ospCommit(camera);

// {
  // static OSPRenderer oRenderer = ospNewRenderer("raycast_eyelight");
  // static OSPRenderer oRenderer = ospNewRenderer("ao16");

  // ospLoadModule("pathtracer");
  // this->OSPRayRenderer = ospNewRenderer("obj");
  // this->OSPRayRenderer = ospNewRenderer("pathtracer");
  if (newval != 0)
  {
    printf("using ao\n");
    this->OSPRayManager->OSPRayRenderer = (osp::Renderer*)ospNewRenderer("ao4");
  }
  else
    this->OSPRayManager->OSPRayRenderer = (osp::Renderer*)ospNewRenderer("obj");
  OSPRenderer oRenderer = (OSPRenderer)this->OSPRayManager->OSPRayRenderer;

  // renderer = ospNewRenderer("ao16");
  // renderer = ospNewRenderer("obj");
  Assert(oRenderer != NULL && "could not create renderer");

  ospSetParam(oRenderer,"world",oModel);
  ospSetParam(oRenderer,"model",oModel);
  ospSetParam(oRenderer,"camera",oCamera);


  // renderer = ospNewRenderer("ao16");
  // renderer = ospNewRenderer("obj");
  // Assert(oRenderer != NULL && "could not create renderer");

  // ospSet1i(renderer,"spp",Samples);
  ospCommit(oRenderer);

  // this->MantaEngine->addTransaction
    // ( "set samples",
      // Manta::Callback::create(this, &vtkOSPRayRenderer::InternalSetSamples));
  // this->Modified();
  #endif
}


//----------------------------------------------------------------------------
void vtkOSPRayRenderer::InternalSetSamples()
{
  #if 0
  if (this->Samples <= 1)
    {
    this->MantaFactory->selectPixelSampler( "singlesample" );
    }
  else
    {
    char buff[80];
    sprintf(buff, "regularsample(-numberOfSamples %d)", this->Samples);
    this->MantaFactory->selectPixelSampler(buff);
    //this->MantaFactory->selectPixelSampler(
    //"jittersample(-numberOfSamples 16)");
    }
    #endif
}

//----------------------------------------------------------------------------
void vtkOSPRayRenderer::SetMaxDepth( int newval )
{
  #if 0
  if (this->MaxDepth == newval)
    {
    return;
    }

  this->MaxDepth = newval;
  this->MantaEngine->addTransaction
    ( "set max depth",
      Manta::Callback::create(this, &vtkOSPRayRenderer::InternalSetMaxDepth));
  this->Modified();
  #endif
}

//----------------------------------------------------------------------------
void vtkOSPRayRenderer::InternalSetMaxDepth()
{
  #if 0
  this->MantaScene->getRenderParameters().setMaxDepth( this->MaxDepth );
  #endif
}
