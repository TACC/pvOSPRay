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

#ifdef VTK_OPENGL2
  #include "vtk_glew.h"
#endif

#include "vtkRenderingOpenGLConfigure.h"
#include "ospray/ospray.h"

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
#include "vtkOpenGLRenderWindow.h"
#include "vtkTimerLog.h"

#include "vtkImageData.h"
#include "vtkPNGWriter.h"
#include "vtkPolyDataMapper.h"
#include "vtkPolyData.h"
#include "vtkMultiProcessController.h"

#include <vtkSmartPointer.h>
#include <vtkCommand.h>
#include <vtkRenderer.h>
#include <vtkRendererCollection.h>
#include <vtkRenderWindow.h>
#include <vtkRenderWindowInteractor.h>


#include "vtkOSPRayActor.h"

//  VBOs
//
// #if USE_VBOS
// #include <GL/glu.h>
// #include <assert.h>
// #endif

/* DrawBufferMode */
#define GL_NONE                           0
#define GL_FRONT_LEFT                     0x0400
#define GL_FRONT_RIGHT                    0x0401
#define GL_BACK_LEFT                      0x0402
#define GL_BACK_RIGHT                     0x0403
#define GL_FRONT                          0x0404
#define GL_BACK                           0x0405
#define GL_LEFT                           0x0406
#define GL_RIGHT                          0x0407
#define GL_FRONT_AND_BACK                 0x0408

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
  }

private:
  int TimerCount;

};

//----------------------------------------------------------------------------
vtkOSPRayRenderer::vtkOSPRayRenderer()
:
prog_flag(false),
Accumulate(false)
{
  Frame=0;
  HasVolume= false;
  ClearAccumFlag=false;

  ComputeDepth = false;
  FramebufferDirty = true;

  this->EngineInited=false;
  this->NumberOfWorkers = 1;
  this->EnableShadows = -1;
  this->Samples = 1;
  this->MaxDepth = 5;
  this->EnableVolumeShading = 0;

  this->ImageX = -1;
  this->ImageY = -1;

  this->backgroundRGB[0] = 0.0;
  this->backgroundRGB[1] = 0.0;
  this->backgroundRGB[2] = 0.0;
  AccumCounter=0;
  MaxAccum=1024;
  this->SetAmbient( 0.1, 0.1, 0.1 );

  this->OSPRayManager = vtkOSPRayManager::Singleton();

  OSPModel oModel = (OSPModel)this->OSPRayManager->OSPRayModel;
  OSPCamera oCamera = (OSPCamera)this->OSPRayManager->OSPRayCamera;
  this->EnableAO=false;
  bool ao = EnableAO;
  EnableAO=-1;
  SetEnableAO(ao);
  OSPRenderer oRenderer = (OSPRenderer)this->OSPRayManager->OSPRayRenderer;
  OSPRenderer vRenderer = (OSPRenderer)this->OSPRayManager->OSPRayVolumeRenderer;
  ospSet3f(vRenderer, "bgColor", backgroundRGB[0], backgroundRGB[1], backgroundRGB[2]);
  OSPModel vModel = (OSPModel)this->OSPRayManager->OSPRayVolumeModel;
  SetEnableShadows(0);

  ospSetObject(vRenderer,"world",vModel);
  ospSetObject(vRenderer,"model",vModel);
  ospSetObject(vRenderer,"camera",oCamera);
  ospCommit(vRenderer);

#if !defined(Assert)
#define Assert if (0)
#endif

  Assert(oRenderer != NULL && "could not create renderer");
  Assert(vRenderer != NULL && "could not create renderer");

  ospSetObject(oRenderer,"world",oModel);
  ospSetObject(oRenderer,"model",oModel);
  ospSetObject(oRenderer,"camera",oCamera);
  ospSet1i(oRenderer,"spp",Samples);
  ospSet3f(oRenderer,"bgColor",1,1,1);
  ospCommit(oModel);
  ospCommit(oCamera);
  ospCommit(oRenderer);

  this->ColorBuffer = NULL;
  this->DepthBuffer = NULL;
  this->osp_framebuffer = NULL;

  this->BackLeftBuffer = static_cast<unsigned int>(GL_BACK_LEFT);
  this->BackRightBuffer = static_cast<unsigned int>(GL_BACK_RIGHT);
  this->FrontLeftBuffer = static_cast<unsigned int>(GL_FRONT_LEFT);
  this->FrontRightBuffer = static_cast<unsigned int>(GL_FRONT_RIGHT);
  this->BackBuffer = static_cast<unsigned int>(GL_BACK);
  this->FrontBuffer = static_cast<unsigned int>(GL_FRONT);

  StatisticFramesPerOutput = 100;
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

  this->BackLeftBuffer = 0;
  this->BackRightBuffer = 0;
  this->FrontLeftBuffer = 0;
  this->FrontRightBuffer = 0;
  this->BackBuffer = 0;
  this->FrontBuffer = 0;

}

//----------------------------------------------------------------------------
void vtkOSPRayRenderer::InitEngine()
{
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
void vtkOSPRayRenderer::ClearAccumulation()
{
  if (osp_framebuffer)
    ospFrameBufferClear(osp_framebuffer, OSP_FB_COLOR | (ComputeDepth ? OSP_FB_DEPTH : 0) | OSP_FB_ACCUM);
  AccumCounter=0;
}


//----------------------------------------------------------------------------
// Ask lights to load themselves into graphics pipeline.
int vtkOSPRayRenderer::UpdateLights()
{
  OSPRenderer renderer = ((OSPRenderer)this->OSPRayManager->OSPRayRenderer);
  OSPRenderer vRenderer = ((OSPRenderer)this->OSPRayManager->OSPRayVolumeRenderer);
  std::vector<OSPLight> lights;


  // convert VTK lights into OSPRay lights
  vtkCollectionSimpleIterator sit;
  this->Lights->InitTraversal( sit );

  vtkLight *vLight = NULL;
  bool noneOn = true;
  for ( this->Lights->InitTraversal( sit );
       ( vLight = this->Lights->GetNextLight( sit ) ) ; )
  {
    if ( vLight->GetSwitch() )
    {
      noneOn = false;
    }
    vtkLight* light = vLight;

    double *color, *position, *focal;

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
    }
    else
    {
      OSPLight ospLight = ospNewLight(renderer, "DirectionalLight");
      ospSetString(ospLight, "name", "directional" );
      float scale = 0.8;
      ospSet3f(ospLight, "color", color[0]*scale,color[1]*scale,color[2]*scale);

      float x = position[0] - focal[0];
      float y = position[1] - focal[1];
      float z = position[2] - focal[2];
			float d = x*x + y*y + z*z;

			if (d != 0.0)
				d = 1.0 / d;

      ospSet3f(ospLight, "direction", x*d, y*d, z*d);
      ospCommit(ospLight);
      lights.push_back(ospLight);
    }

    if (noneOn)
    {
      {
        cerr
        << "No light defined, creating a headlight at camera position" << endl;
      }
    }
    else
    {
    }
  }
  {

  }
  {
  }

  OSPData lightsArray = ospNewData(lights.size(), OSP_OBJECT, &lights[0], 0);
  ospSetData(renderer, "lights",lightsArray);
  ospSetData(vRenderer, "lights",lightsArray);
  ospCommit(renderer);

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
}

void vtkOSPRayRenderer::PreRender()
{
  if ((!prog_flag) || ClearAccumFlag)
  {
    if (osp_framebuffer){
      //disable clearing the framebuffer for now
      ;//ospFrameBufferClear(osp_framebuffer, OSP_FB_COLOR | (ComputeDepth ? OSP_FB_DEPTH : 0) | OSP_FB_ACCUM);
    }
    AccumCounter=0;
    ClearAccumFlag=false;
  }
  else {
    prog_flag = false;
  }

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


  HasVolume = false;
  OSPRenderer oRenderer = (OSPRenderer)this->OSPRayManager->OSPRayRenderer;
  this->OSPRayManager->OSPRayModel = ospNewModel();
  this->OSPRayManager->OSPRayVolumeModel = this->OSPRayManager->OSPRayModel;  //TODO: the volume and geometry are now managed in the same model object, can remove volumemodel entirely
  OSPModel oModel = (OSPModel)this->OSPRayManager->OSPRayModel;
  OSPCamera oCamera = (OSPCamera)this->OSPRayManager->OSPRayCamera;
  ospSetObject(oRenderer,"world",oModel);
  ospSetObject(oRenderer,"model",oModel);
  ospSetObject(oRenderer,"camera",oCamera);


  ospCommit(this->OSPRayManager->OSPRayModel);
  ospCommit(this->OSPRayManager->OSPRayRenderer);

  this->UpdateCamera();


  this->UpdateLightGeometry();
  this->UpdateLights();
}

//----------------------------------------------------------------------------
void vtkOSPRayRenderer::DeviceRender()
{
  // glClearColor(0,0,0,0);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);  //DEBUG: OPENGL2 backend needs buffers cleared
  // std::cerr << "vtkOSPRayRenderer(" << this << ")::DeviceRender\n";
  static vtkTimerLog* timer = vtkTimerLog::New();
  timer->StartTimer();
  PreRender();

  this->UpdateGeometry();

  vtkTimerLog::MarkEndEvent("Geometry");

  vtkTimerLog::MarkStartEvent("Total LayerRender");
  this->LayerRender();

  vtkTimerLog::MarkEndEvent("Total LayerRender");

  vtkTimerLog::MarkEndEvent("OSPRay Dev Render");
  Frame++;
  timer->StopTimer();
  #if GATHER_STATS
  Statistics.push(std::make_pair("DeviceRender", timer->GetElapsedTime()));
  if ( (StatisticFramesPerOutput > 0) && ( (Frame % StatisticFramesPerOutput) == 0) )
  {
    while (!Statistics.empty())
    {
      std::cout << Statistics.front().first << " " << Statistics.front().second;
      Statistics.pop();
    }
  }
  #endif
}

//----------------------------------------------------------------------------
// let the renderer display itself appropriately based on its layer index
void vtkOSPRayRenderer::LayerRender()
{
  int     i, j;
  int     rowLength,  OSPRaySize[2];
  int     renderPos[2];
  int*    renderSize  = NULL;
  int     renWinSize[2];
  bool    stereoDumy;
  float*  OSPRayBuffer = NULL;
  double* renViewport = NULL;

	vtkCamera * activeCamera = this->GetActiveCamera();
	if (!activeCamera)
		return;

  int myLeftEye = activeCamera->GetLeftEye();

  //printf("LR:myLeftEye=%d\n", myLeftEye);
  int myStereoCapableWindow = this->GetRenderWindow()->GetStereoCapableWindow();
  //printf("LR:myStereoCapableWindow=%d\n", myStereoCapableWindow);
  int myStereoMode = this->GetRenderWindow()->GetStereoRender();
  //printf("LR:myStereoMode=%d\n", myStereoMode);

  // collect some useful info
  renderSize = this->GetSize();

  renViewport= this->GetViewport();
  float renderPosFloat[2];
  renderPosFloat[0] = renViewport[0] * renWinSize[0] + 0.5f;
  renderPosFloat[1] = renViewport[1] * renWinSize[1] + 0.5f;
  renderPos[0] = int( renderPosFloat[0] );
  renderPos[1] = int( renderPosFloat[1] );

	renWinSize[0] = this->GetRenderWindow()->GetActualSize()[0] * (renViewport[2] - renViewport[0]);
	renWinSize[1] = this->GetRenderWindow()->GetActualSize()[1] * (renViewport[3] - renViewport[1]);

  int size = renWinSize[0]*renWinSize[1];

#if 0
	std::cerr << "vp: " << renViewport[0] << " " << renViewport[1] << " " << renViewport[2] << " " << renViewport[3] << "\n";
	std::cerr << "renderSize: " << renderSize[0] << " " << renderSize[1] << "\n";
	std::cerr << "renWinSize: " << renWinSize[0] << " " << renWinSize[1] << "\n";
#endif

  if (this->ImageX != renWinSize[0] || this->ImageY != renWinSize[1] || FramebufferDirty )
  {
    FramebufferDirty = false;
    this->ImageX = renWinSize[0];
    this->ImageY = renWinSize[1];

    if (this->ColorBuffer) delete[] this->ColorBuffer;
    this->ColorBuffer = new float[ size ];

    if (this->DepthBuffer) delete[] this->DepthBuffer;
    this->DepthBuffer = new float[ size ];

    if (this->osp_framebuffer) ospFreeFrameBuffer(this->osp_framebuffer);
    this->osp_framebuffer = ospNewFrameBuffer(osp::vec2i{renWinSize[0], renWinSize[1]}, OSP_RGBA_I8, OSP_FB_COLOR | (ComputeDepth ? OSP_FB_DEPTH : 0) | OSP_FB_ACCUM);
    ospFrameBufferClear(osp_framebuffer, OSP_FB_COLOR | (ComputeDepth ? OSP_FB_DEPTH : 0) | OSP_FB_ACCUM);
    AccumCounter=0;
  }
  if (HasVolume && !EnableAO)
  {
    OSPRenderer vRenderer = (OSPRenderer)this->OSPRayManager->OSPRayVolumeRenderer;
    OSPModel vModel = (OSPModel)this->OSPRayManager->OSPRayVolumeModel;
    OSPCamera oCamera = (OSPCamera)this->OSPRayManager->OSPRayCamera;

    ospSetObject(vRenderer,"world",vModel);
    ospSetObject(vRenderer,"model",vModel);
    ospSetObject(vRenderer,"camera",oCamera);
    if (ComputeDepth)
      ospSet1i(vRenderer, "backgroundEnabled",0);

    ospCommit(vModel);
    ospCommit(vRenderer);

    ospRenderFrame(this->osp_framebuffer,vRenderer,OSP_FB_COLOR|OSP_FB_ACCUM|(ComputeDepth?OSP_FB_DEPTH:0));
    AccumCounter++;
  }
  else
  {
    OSPRenderer renderer = ((OSPRenderer)this->OSPRayManager->OSPRayRenderer);
    OSPModel ospModel = ((OSPModel)this->OSPRayManager->OSPRayModel);
    if (ComputeDepth)
      ospSet1i(renderer, "backgroundEnabled",0);
    ospCommit(renderer);
    ospCommit(ospModel);

    ospRenderFrame(this->osp_framebuffer,renderer,OSP_FB_COLOR|OSP_FB_ACCUM|(ComputeDepth?OSP_FB_DEPTH:0));
    AccumCounter++;
  }

  //
  // Copy Depth Buffer
  //
  if (ComputeDepth)
  {
    // if (this->OSPRayManager->stereoCamera!=NULL){
    //  //printf("LR:Shifting Camera\n");
    //  this->OSPRayManager->stereoCamera->ShiftCamera();
    // }

    // if (this->OSPRayManager->stereoCamera!=NULL){
    //  //printf("LR:UnShifting Camera\n");
    //  this->OSPRayManager->stereoCamera->UnShiftCamera();
    // }
    double *clipValues = activeCamera->GetClippingRange();
    double viewAngle = activeCamera->GetViewAngle();

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

    //disable setting the Zbuffer for now
    //this->GetRenderWindow()->SetZbufferData(renderPos[0], renderPos[1],
    //                                        renderPos[0] + renderSize[0] - 1, renderPos[1] + renderSize[1] - 1, this->DepthBuffer);
    if (!b)
      std::cerr << "ERROR: no depth from ospray\n";
    else
    {
      float *s = (float *)b;
      float *d = this->DepthBuffer;
      for (int i = 0; i < size; i++, s++, d++)
        *d = std::isinf(*s) ? 1.0 : (*s - clipMin) * clipDiv;
      ospUnmapFrameBuffer(b, this->osp_framebuffer);

      this->GetRenderWindow()->MakeCurrent();
      int gldepth;
      glGetIntegerv(GL_DEPTH_FUNC, &gldepth);
      glDepthFunc(GL_ALWAYS);

      //Carson: TODO: use drawpixels if we can, setting it through the renderwindow seems to be quite slow
      this->GetRenderWindow()->SetZbufferData(renderPos[0], renderPos[1],
                                              renderPos[0] + renWinSize[0] - 1, renderPos[1] + renWinSize[1] - 1, this->DepthBuffer);
      glDepthFunc(gldepth);
    }
  }
  //
  // Copy RGBA Buffer
  //

  const void* rgba = ospMapFrameBuffer(this->osp_framebuffer);
  memcpy((void *)this->ColorBuffer, rgba, size*sizeof(float));  //Carson - this copy is unecessary for layer0
  vtkTimerLog::MarkStartEvent("Image Conversion");

  //debug: color by opacity
  // float* d = (float*)ColorBuffer;
  // for(size_t i=0;i<size;i++)
  // {
  //   unsigned char* c = (unsigned char*)(&d[i]);
  //   c[0]=c[3];
  //   c[1]=c[3];
  //   c[2]=c[3];
  // }

  // let layer #0 initialize GL depth buffer
  if ( this->GetLayer() == 0 )
  {
      // this->GetRenderWindow()->SetRGBACharPixelData( renderPos[0],  renderPos[1],
      //                      renderPos[0] + renderSize[0] - 1,
      //                      renderPos[1] + renderSize[1] - 1,
      //                     (unsigned char*)this->ColorBuffer, 0, ComputeDepth);
      SetRGBACharPixelData( renderPos[0],  renderPos[1],
                           renderPos[0] + renderSize[0] - 1,
                           renderPos[1] + renderSize[1] - 1,
                          (unsigned char*)this->ColorBuffer, 0, ComputeDepth,myLeftEye==1 );
  }
  else
  {
    //layers on top add the colors of their non background pixels
    unsigned char*  GLbakBuffer = NULL;
    if (myLeftEye==1){
        //GLbakBuffer = this->GetRenderWindow()->
        GLbakBuffer = this->
        GetRGBACharPixelData( renderPos[0],  renderPos[1],
                            renderPos[0] + renWinSize[0] - 1,
                            renderPos[1] + renWinSize[1] - 1, 0 );
    } else {
        //GLbakBuffer = this->GetRenderWindow()->
        GLbakBuffer = this->
        GetRGBACharPixelDataRight( renderPos[0],  renderPos[1],
                            renderPos[0] + renWinSize[0] - 1,
                            renderPos[1] + renWinSize[1] - 1, 0 );
    }
    GLbakBuffer = this->GetRenderWindow()->
    GetRGBACharPixelData( renderPos[0],  renderPos[1],
                         renderPos[0] + renWinSize[0] - 1,
                         renderPos[1] + renWinSize[1] - 1, 0 );
    bool anyhit = false;
    unsigned char *optr = GLbakBuffer;
    unsigned char *iptr = (unsigned char*)this->ColorBuffer;
    float *zptr = this->DepthBuffer;
    for ( j = 0; j < renWinSize[1]; j++)
    {
      for ( i = 0; i < renWinSize[0]; i++)
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
      // submit the modified RGB colors to GL BACK bufferEyeTransformMatrix
      if (myLeftEye==1){
        //this->GetRenderWindow()->
        this->
        SetRGBACharPixelData( renderPos[0],  renderPos[1],
                            renderPos[0] + renWinSize[0] - 1,
                            renderPos[1] + renWinSize[1] - 1,
                            GLbakBuffer, 0, 0,true );
      } else {
        //this->GetRenderWindow()->
        this->
        SetRGBACharPixelData( renderPos[0],  renderPos[1],
                            renderPos[0] + renWinSize[0] - 1,
                            renderPos[1] + renWinSize[1] - 1,
                            GLbakBuffer, 0, 0,false );
      }
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

void vtkOSPRayRenderer::AddOSPRayRenderable(vtkOSPRayRenderable* inst)
{
  ospAddGeometry((OSPModel)this->OSPRayManager->OSPRayModel,inst->instance);
  renderables.push_back(inst);
}

//----------------------------------------------------------------------------
void vtkOSPRayRenderer::SetEnableShadows( int newval )
{
  if (this->EnableShadows == newval)
  {
    return;
  }
  this->EnableShadows = newval;

  OSPRenderer renderer = ((OSPRenderer)this->OSPRayManager->OSPRayRenderer);
  ospSet1i(renderer,"shadowsEnabled", this->EnableShadows);
  ospCommit(renderer);
}

//----------------------------------------------------------------------------
void vtkOSPRayRenderer::SetSamples( int newval )
{
  if (this->Samples == newval || newval < 1)
  {
    return;
  }

  this->Samples = newval;


  OSPRenderer renderer = ((OSPRenderer)this->OSPRayManager->OSPRayRenderer);

  Assert(renderer);

  ospSet1i(renderer,"spp",Samples);
  ospCommit(renderer);

  OSPRenderer vRenderer = ((OSPRenderer)this->OSPRayManager->OSPRayVolumeRenderer);

  Assert(vRenderer);

  ospSet1i(vRenderer,"spp",Samples);
  ospCommit(vRenderer);

}

//----------------------------------------------------------------------------
void vtkOSPRayRenderer::SetEnableAO( int newval )
{
  if (this->EnableAO == newval)
  {
    return;
  }

  this->EnableAO = newval;

  UpdateOSPRayRenderer();

}

//----------------------------------------------------------------------------
void vtkOSPRayRenderer::SetEnablePathtracing( int newval )
{
  if (this->EnablePathtracing == newval)
  {
    return;
  }

  this->EnablePathtracing = newval;

  UpdateOSPRayRenderer();

}


void vtkOSPRayRenderer::SetEnableVolumeShading( int newval )
{
  EnableVolumeShading = newval;
}

void vtkOSPRayRenderer::UpdateOSPRayRenderer()
{
  OSPModel oModel = (OSPModel)this->OSPRayManager->OSPRayModel;
  OSPCamera oCamera = (OSPCamera)this->OSPRayManager->OSPRayCamera;

  if (EnableAO != 0)
  {
    this->OSPRayManager->OSPRayRenderer = (osp::Renderer*)ospNewRenderer("ao4");
  }
  else if (EnablePathtracing != 0)
  {
    this->OSPRayManager->OSPRayRenderer = (osp::Renderer*)ospNewRenderer("pathtracer");
  }
  else
  {
    // this->OSPRayManager->OSPRayRenderer = (osp::Renderer*)ospNewRenderer("obj");
    // this->OSPRayManager->OSPRayRenderer = (osp::Renderer*)ospNewRenderer("raycast_volume_renderer");
    //this->OSPRayManager->OSPRayRenderer = this->OSPRayManager->OSPRayVolumeRenderer;
    this->OSPRayManager->OSPRayRenderer = (osp::Renderer*)ospNewRenderer("raycast_volume_renderer");
    //this->OSPRayManager->OSPRayRenderer = (osp::Renderer*)ospNewRenderer("scivis");
    // this->OSPRayManager->OSPRayRenderer = (osp::Renderer*)ospNewRenderer("obj");
  }
  OSPRenderer oRenderer = (OSPRenderer)this->OSPRayManager->OSPRayRenderer;

  Assert(oRenderer != NULL && "could not create renderer");

  ospSetObject(oRenderer,"dynamic_model",ospNewModel());
  ospSetObject(oRenderer,"world",oModel);
  ospSetObject(oRenderer,"model",oModel);
  ospSetObject(oRenderer,"camera",oCamera);
  ospCommit(oRenderer);

  ospSet1i(oRenderer,"spp",Samples);
  ospSet1f(oRenderer,"epsilon", 10e-2);
  ospSet1i(oRenderer,"shadowsEnabled", this->EnableShadows);

  ospCommit(oRenderer);
  SetBackground(backgroundRGB[0],backgroundRGB[1],backgroundRGB[2]);

  vtkActorCollection *actorList = this->GetActors();
  actorList->InitTraversal();

  int numActors = actorList->GetNumberOfItems();
  for(int i=0; i<numActors; i++) {
    vtkActor *a = actorList->GetNextActor();
    a->Modified();
  }
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

// ----------------------------------------------------------------------------
// Description:
// Return the OpenGL name of the back left buffer.
// It is GL_BACK_LEFT if GL is bound to the window-system-provided
// framebuffer. It is GL_COLOR_ATTACHMENT0_EXT if GL is bound to an
// application-created framebuffer object (GPU-based offscreen rendering)
// It is used by vtkOpenGLCamera.
unsigned int vtkOSPRayRenderer::GetBackLeftBuffer()
{
  return this->BackLeftBuffer;
}

// ----------------------------------------------------------------------------
// Description:
// Return the OpenGL name of the back right buffer.
// It is GL_BACK_RIGHT if GL is bound to the window-system-provided
// framebuffer. It is GL_COLOR_ATTACHMENT0_EXT+1 if GL is bound to an
// application-created framebuffer object (GPU-based offscreen rendering)
// It is used by vtkOpenGLCamera.
unsigned int vtkOSPRayRenderer::GetBackRightBuffer()
{
  return this->BackRightBuffer;
}

// ----------------------------------------------------------------------------
// Description:
// Return the OpenGL name of the front left buffer.
// It is GL_FRONT_LEFT if GL is bound to the window-system-provided
// framebuffer. It is GL_COLOR_ATTACHMENT0_EXT if GL is bound to an
// application-created framebuffer object (GPU-based offscreen rendering)
// It is used by vtkOpenGLCamera.
unsigned int vtkOSPRayRenderer::GetFrontLeftBuffer()
{
  return this->FrontLeftBuffer;
}

// ----------------------------------------------------------------------------
// Description:
// Return the OpenGL name of the front right buffer.
// It is GL_FRONT_RIGHT if GL is bound to the window-system-provided
// framebuffer. It is GL_COLOR_ATTACHMENT0_EXT+1 if GL is bound to an
// application-created framebuffer object (GPU-based offscreen rendering)
// It is used by vtkOpenGLCamera.
unsigned int vtkOSPRayRenderer::GetFrontRightBuffer()
{
  return this->FrontRightBuffer;
}

// ----------------------------------------------------------------------------
// Description:
// Return the OpenGL name of the back left buffer.
// It is GL_BACK if GL is bound to the window-system-provided
// framebuffer. It is GL_COLOR_ATTACHMENT0_EXT if GL is bound to an
// application-created framebuffer object (GPU-based offscreen rendering)
// It is used by vtkOpenGLCamera.
unsigned int vtkOSPRayRenderer::GetBackBuffer()
{
  return this->BackBuffer;
}

// ----------------------------------------------------------------------------
// Description:
// Return the OpenGL name of the front left buffer.
// It is GL_FRONT if GL is bound to the window-system-provided
// framebuffer. It is GL_COLOR_ATTACHMENT0_EXT if GL is bound to an
// application-created framebuffer object (GPU-based offscreen rendering)
// It is used by vtkOpenGLCamera.
unsigned int vtkOSPRayRenderer::GetFrontBuffer()
{
  return this->FrontBuffer;
}

int vtkOSPRayRenderer::GetRGBACharPixelData(int x1, int y1,
                                                int x2, int y2,
                                                int front,
                                                unsigned char* data)
{
  int     y_low, y_hi;
  int     x_low, x_hi;
  int     width, height;


  // set the current window
  this->GetRenderWindow()->MakeCurrent();


  if (y1 < y2)
    {
    y_low = y1;
    y_hi  = y2;
    }
  else
    {
    y_low = y2;
    y_hi  = y1;
    }


  if (x1 < x2)
    {
    x_low = x1;
    x_hi  = x2;
    }
  else
    {
    x_low = x2;
    x_hi  = x1;
    }


  // Must clear previous errors first.
  while(glGetError() != GL_NO_ERROR)
    {
    ;
    }

  if (front)
    {
    glReadBuffer(static_cast<GLenum>(this->GetFrontLeftBuffer()));
    }
  else
    {
    glReadBuffer(static_cast<GLenum>(this->GetBackLeftBuffer()));
    }

  width  = abs(x_hi - x_low) + 1;
  height = abs(y_hi - y_low) + 1;

  glDisable( GL_SCISSOR_TEST );

  glReadPixels( x_low, y_low, width, height, GL_RGBA, GL_UNSIGNED_BYTE,
                data);

  if (glGetError() != GL_NO_ERROR)
    {
    return VTK_ERROR;
    }
  else
    {
    return VTK_OK;
    }

}

int vtkOSPRayRenderer::GetRGBACharPixelDataRight(int x1, int y1,
                                                int x2, int y2,
                                                int front,
                                                unsigned char* data)
{
  int     y_low, y_hi;
  int     x_low, x_hi;
  int     width, height;


  // set the current window
  this->GetRenderWindow()->MakeCurrent();


  if (y1 < y2)
    {
    y_low = y1;
    y_hi  = y2;
    }
  else
    {
    y_low = y2;
    y_hi  = y1;
    }


  if (x1 < x2)
    {
    x_low = x1;
    x_hi  = x2;
    }
  else
    {
    x_low = x2;
    x_hi  = x1;
    }


  // Must clear previous errors first.
  while(glGetError() != GL_NO_ERROR)
    {
    ;
    }

  if (front)
    {
    glReadBuffer(static_cast<GLenum>(this->GetFrontRightBuffer()));
    }
  else
    {
    glReadBuffer(static_cast<GLenum>(this->GetBackRightBuffer()));
    }

  width  = abs(x_hi - x_low) + 1;
  height = abs(y_hi - y_low) + 1;

  glDisable( GL_SCISSOR_TEST );

  glReadPixels( x_low, y_low, width, height, GL_RGBA, GL_UNSIGNED_BYTE,
                data);

  if (glGetError() != GL_NO_ERROR)
    {
    return VTK_ERROR;
    }
  else
    {
    return VTK_OK;
    }

}

unsigned char *vtkOSPRayRenderer::GetRGBACharPixelData(int x1, int y1,
                                                           int x2, int y2,
                                                           int front)
{
  int     y_low, y_hi;
  int     x_low, x_hi;
  int     width, height;

  if (y1 < y2)
    {
    y_low = y1;
    y_hi  = y2;
    }
  else
    {
    y_low = y2;
    y_hi  = y1;
    }


  if (x1 < x2)
    {
    x_low = x1;
    x_hi  = x2;
    }
  else
    {
    x_low = x2;
    x_hi  = x1;
    }

  width  = abs(x_hi - x_low) + 1;
  height = abs(y_hi - y_low) + 1;

  unsigned char *data = new unsigned char[ (width*height)*4 ];
  this->GetRGBACharPixelData(x1, y1, x2, y2, front, data);

  return data;
}

unsigned char *vtkOSPRayRenderer::GetRGBACharPixelDataRight(int x1, int y1,
                                                           int x2, int y2,
                                                           int front)
{
  int     y_low, y_hi;
  int     x_low, x_hi;
  int     width, height;

  if (y1 < y2)
    {
    y_low = y1;
    y_hi  = y2;
    }
  else
    {
    y_low = y2;
    y_hi  = y1;
    }


  if (x1 < x2)
    {
    x_low = x1;
    x_hi  = x2;
    }
  else
    {
    x_low = x2;
    x_hi  = x1;
    }

  width  = abs(x_hi - x_low) + 1;
  height = abs(y_hi - y_low) + 1;

  unsigned char *data = new unsigned char[ (width*height)*4 ];
  this->GetRGBACharPixelDataRight(x1, y1, x2, y2, front, data);

  return data;
}

int vtkOSPRayRenderer::SetRGBACharPixelData(int x1, int y1, int x2,
                                                int y2, unsigned char *data,
                                                int front, int blend, bool left)
{
  // set the current window
  this->GetRenderWindow()->MakeCurrent();

  // Error checking
  // Must clear previous errors first.
  while(glGetError() != GL_NO_ERROR)
    {
    ;
    }

  GLint buffer;
  glGetIntegerv(GL_DRAW_BUFFER, &buffer);

  if (front)
    {
      if (left)
        glDrawBuffer(this->GetFrontLeftBuffer());
      else
        glDrawBuffer(this->GetFrontRightBuffer());
    }
  else
    {
      if (left)
        glDrawBuffer(this->GetBackLeftBuffer());
      else
        glDrawBuffer(this->GetBackRightBuffer());
    }

  // Disable writing on the z-buffer.
  glDepthMask(GL_FALSE);
  glDisable(GL_DEPTH_TEST);

  #ifndef VTK_OPENGL2
    int     y_low, y_hi;
    int     x_low, x_hi;
    int     width, height;

     // write out a row of pixels
    glViewport(0, 0, this->Size[0], this->Size[1]);
    glMatrixMode( GL_MODELVIEW );
    glPushMatrix();
    glLoadIdentity();
    glMatrixMode( GL_PROJECTION );
    glPushMatrix();
    glLoadIdentity();
    glRasterPos3f( (2.0 * static_cast<GLfloat>(x_low) / this->Size[0] - 1),
                   (2.0 * static_cast<GLfloat>(y_low) / this->Size[1] - 1),
                   -1.0 );
    glMatrixMode( GL_PROJECTION );
    glPopMatrix();
    glMatrixMode( GL_MODELVIEW );
    glPopMatrix();

    glDisable( GL_ALPHA_TEST );
    glDisable( GL_SCISSOR_TEST );

    // Turn of texturing in case it is on - some drivers have a problem
    // getting / setting pixels with texturing enabled.
    glDisable( GL_TEXTURE_2D );

    if (y1 < y2)
      {
      y_low = y1;
      y_hi  = y2;
      }
    else
      {
      y_low = y2;
      y_hi  = y1;
      }


    if (x1 < x2)
      {
      x_low = x1;
      x_hi  = x2;
      }
    else
      {
      x_low = x2;
      x_hi  = x1;
      }


    width  = abs(x_hi-x_low);
    height = abs(y_hi-y_low);
  #endif

  if (!blend)
    {
    glDisable(GL_BLEND);

    #ifdef VTK_OPENGL2
      ((vtkOpenGLRenderWindow*)this->GetRenderWindow())->DrawPixels(x1,y1,x2,y2,4,VTK_UNSIGNED_CHAR,data);
    #else
           // glDrawPixels( x2, y2, GL_RGBA, GL_UNSIGNED_BYTE,
           //         data);
     ((vtkOpenGLRenderWindow*)this->GetRenderWindow())->SetRGBACharPixelData(x1,y1,x2,
                                      y2, data,
                                      front, blend);
    #endif
    glEnable(GL_BLEND);
    }
  else
    {
      // glEnable( GL_ALPHA_TEST );
      // glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
      // glBlendEquation(GL_FUNC_ADD);
      // glDisable(GL_DEPTH_TEST);
      #ifdef VTK_OPENGL2
        ((vtkOpenGLRenderWindow*)this->GetRenderWindow())->DrawPixels(x1,y1,x2,y2,4,VTK_UNSIGNED_CHAR,data);
      #else
           // glDrawPixels( width, height, GL_RGBA, GL_UNSIGNED_BYTE,
           //         data);
       ((vtkOpenGLRenderWindow*)this->GetRenderWindow())->SetRGBACharPixelData(x1,y1,x2,
                                              y2, data,
                                              front, blend);
      #endif
    }

  // Renenable writing on the z-buffer.
  glDepthMask(GL_TRUE);
  glEnable(GL_DEPTH_TEST);

  // This seems to be necessary for the image to show up
  glFlush();

  glDrawBuffer(buffer);

  if (glGetError() != GL_NO_ERROR)
    {
    return VTK_ERROR;
    }
  else
    {
    return VTK_OK;
    }
}
