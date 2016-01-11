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

// .NAME vtkOSPRayRenderer - Renderer that uses OSPRay ray tracer instead of GL.
// .SECTION Description
// vtkOSPRayRenderer is a concrete implementation of the abstract class
// vtkRenderer. The renderer uses the Intel OSPRay ray tracing framework
// to render geometry.  Corresponding OSPRay classes for mappers, actors, etc.
// are required for the renderer to display the correct representations.
//

#ifndef __vtkOSPRayRenderer_h
#define __vtkOSPRayRenderer_h

#include "vtkOSPRayModule.h"
#include "vtkOpenGLRenderer.h"

#include "vtkOSPRay.h"

#include "vtkOSPRayRenderable.h"
#include <queue>
#include <utility>
#include <string>


//BTX
namespace OSPRay {
class OSPRayInterface;
class Scene;
class Group;
class LightSet;
class Factory;
class Camera;
class SyncDisplay;
class Light;
}
//ETX

class vtkOSPRayManager;

class VTKOSPRAY_EXPORT vtkOSPRayRenderer : public vtkOpenGLRenderer
{
public:
  static vtkOSPRayRenderer *New();
  vtkTypeMacro(vtkOSPRayRenderer, vtkOpenGLRenderer);
  void PrintSelf(ostream& os, vtkIndent indent);

  //Description:
  // Overridden to use OSPRay callbacks to do the work.
  virtual void SetBackground(double rgb[3])
    { this->Superclass::SetBackground(rgb); }
  virtual void SetBackground(double r, double g, double b);

  //Description:
  //Changes the number of OSPRay rendering threads.
  //More is faster.
  //Default is 1.
  void SetNumberOfWorkers(int);
  vtkGetMacro(NumberOfWorkers, int);

  //Description:
  //Turns on or off shadows.  This only applies when the obj renderer is used in OSPRay
  //Default is off.
  void SetEnableShadows(int);
  vtkGetMacro(EnableShadows, int);

    //Description:
  //Turns on or off Ambient Occlusion.
  //Default is off.
  void SetEnableAO(int);
  vtkGetMacro(EnableAO, int);

  //Description:
  //Turns on or off to use the path tracer.
  //Default is off.
  void SetEnablePathtracing(int);
  vtkGetMacro(EnablePathtracing, int);
  //Description:
  //Turns on or off gradient shading in volumes.
  //Default is off.
  void SetEnableVolumeShading(int);
  vtkGetMacro(EnableVolumeShading, int);

  //Description:
  //Controls multisample (anitaliased) rendering.
  //More looks better, but is slower.
  //Default is 1.
  void SetSamples(int);
  vtkGetMacro(Samples, int);

  vtkGetMacro(Accumulate, bool);
  void SetAccumulate(bool st)
  {
    Accumulate = st;
  }

  //Description:
  //Controls maximum ray bounce depth.
  //More looks better, but is slower.
  //Default is 5 meaning a couple of bounces.
  void SetMaxDepth(int);
  vtkGetMacro(MaxDepth, int);

  // Description:
  // Ask lights to load themselves into graphics pipeline.
  int UpdateLights(void);

  // Description:
  // Turns off all lighting
  void ClearLights(void);


  void Clear();
  // The accumulation buffer is used to accumulate multiple renders for 
  // progressive rendering.  It should be cleared when the frame needs a refresh,
  // such as camera updates.
  void ClearAccumulation();

  //Description:
  //Access to the OSPRay rendered image
  float * GetColorBuffer()
  {
    return this->ColorBuffer;
  }
  float * GetDepthBuffer()
  {
    return this->DepthBuffer;
  }

  //Setup ospray world, camera etc. before
  // renders are called in the geometry
  void PreRender();
  // Description:
  // Concrete render method. Do not call this directly. The pipeline calls
  // it during Renderwindow::Render()
  void DeviceRender();
  //Display a rendered OSPRay framebuffer to the VTK framebuffer for display.
  void LayerRender();

  //Description:
  //All classes that make OSPRay calls should get hold of this and
  //Register it so that the Manager, and thus the OSPRay engine
  //outlive themselves, and thus guarantee that they can safely make
  //OSPRay API calls whenever they need to.
  vtkOSPRayManager* GetOSPRayManager()
  {
    return this->OSPRayManager;
  }

  void SetHasVolume(bool st) { HasVolume=st;}
  void SetProgressiveRenderFlag() {prog_flag = true; }
  void SetClearAccumFlag() {ClearAccumFlag = true; }
  int GetAccumCounter() { return AccumCounter; }
  int GetMaxAccumulation() { return MaxAccum; }
  int GetFrame() { return Frame; }
  void SetComputeDepth(bool use_depth) { 
    if (use_depth == ComputeDepth)
      return;
    ComputeDepth= use_depth;
    FramebufferDirty=true;
  }

  void AddOSPRayRenderable(vtkOSPRayRenderable* inst);

protected:
  vtkOSPRayRenderer();
  ~vtkOSPRayRenderer();

  //lets OSPRay engine know when viewport changes
  void UpdateSize();

  // OSPRay renderer does not support picking.
  virtual void DevicePickRender() {};
  virtual void StartPick(unsigned int vtkNotUsed(pickFromSizev)) {};
  virtual void UpdatePickId() {};
  virtual void DonePick() {};
  virtual unsigned int GetPickedId() { return 0; };
  virtual unsigned int GetNumPickedIds() { return 0; };
  virtual int GetPickedIds( unsigned int vtkNotUsed(atMost), unsigned int * vtkNotUsed(callerBuffer) )
  {
    return 0;
  };
  virtual double GetPickedZ() { return 0.0f; };
  //creates the internal OSPRay renderer object

  void UpdateOSPRayRenderer();

private:
  vtkOSPRayRenderer(const vtkOSPRayRenderer&); // Not implemented.
  void operator=(const vtkOSPRayRenderer&); // Not implemented.

  void InitEngine();

  // Description:
  // Return the OpenGL name of the back left buffer.
  // It is GL_BACK_LEFT if GL is bound to the window-system-provided
  // framebuffer. It is vtkgl::COLOR_ATTACHMENT0_EXT if GL is bound to an
  // application-created framebuffer object (GPU-based offscreen rendering)
  // It is used by vtkOpenGLCamera.
  unsigned int GetBackLeftBuffer();

  // Description:
  // Return the OpenGL name of the back right buffer.
  // It is GL_BACK_RIGHT if GL is bound to the window-system-provided
  // framebuffer. It is vtkgl::COLOR_ATTACHMENT0_EXT+1 if GL is bound to an
  // application-created framebuffer object (GPU-based offscreen rendering)
  // It is used by vtkOpenGLCamera.
  unsigned int GetBackRightBuffer();

  // Description:
  // Return the OpenGL name of the front left buffer.
  // It is GL_FRONT_LEFT if GL is bound to the window-system-provided
  // framebuffer. It is vtkgl::COLOR_ATTACHMENT0_EXT if GL is bound to an
  // application-created framebuffer object (GPU-based offscreen rendering)
  // It is used by vtkOpenGLCamera.
  unsigned int GetFrontLeftBuffer();

  // Description:
  // Return the OpenGL name of the front right buffer.
  // It is GL_FRONT_RIGHT if GL is bound to the window-system-provided
  // framebuffer. It is vtkgl::COLOR_ATTACHMENT0_EXT+1 if GL is bound to an
  // application-created framebuffer object (GPU-based offscreen rendering)
  // It is used by vtkOpenGLCamera.
  unsigned int GetFrontRightBuffer();

  // Description:
  // Return the OpenGL name of the back left buffer.
  // It is GL_BACK if GL is bound to the window-system-provided
  // framebuffer. It is vtkgl::COLOR_ATTACHMENT0_EXT if GL is bound to an
  // application-created framebuffer object (GPU-based offscreen rendering)
  // It is used by vtkOpenGLCamera.
  unsigned int GetBackBuffer();

  // Description:
  // Return the OpenGL name of the front left buffer.
  // It is GL_FRONT if GL is bound to the window-system-provided
  // framebuffer. It is vtkgl::COLOR_ATTACHMENT0_EXT if GL is bound to an
  // application-created framebuffer object (GPU-based offscreen rendering)
  // It is used by vtkOpenGLCamera.
  unsigned int GetFrontBuffer();


  unsigned char *GetRGBACharPixelData(int x,int y,int x2,int y2,
                                              int front);
  int SetRGBACharPixelData(int x, int y, int x2, int y2,
                                   unsigned char *data, int front,
                                   int blend=0,bool left=true);
  unsigned char *GetRGBACharPixelDataRight(int x, int y, int x2, int y2,
                                              int front);
  // int SetRGBACharPixelDataRight(int x,int y, int x2, int y2,
  //                                  unsigned char *data, int front,
  //                                  int blend=0);

  int GetRGBACharPixelData(int x, int y, int x2, int y2, int front,
                           unsigned char* data);
  int GetRGBACharPixelDataRight(int x, int y, int x2, int y2, int front,
                           unsigned char* data);

  //Description:
  // Overriden to help ensure that a OSPRay compatible class is created.
  vtkCamera * MakeCamera();
  std::vector<vtkOSPRayRenderable*> renderables;

  bool IsStereo;
  bool EngineInited;
  bool EngineStarted;

  int ImageX;
  int ImageY;
  OSPFrameBuffer osp_framebuffer;

  float *ColorBuffer;
  float *DepthBuffer;


  int ChannelId;

  vtkOSPRayManager *OSPRayManager;

  int NumberOfWorkers;
  int EnableShadows;
  int EnableAO;
  int EnablePathtracing;
  int EnableVolumeShading;
  int Samples;
  int MaxDepth;
  bool Accumulate;
  int AccumCounter;
  int MaxAccum;
  bool prog_flag;
  int Frame;
  bool ComputeDepth;
  bool FramebufferDirty;
  bool HasVolume;
  bool ClearAccumFlag;

  double backgroundRGB[3];

  unsigned int BackLeftBuffer;
  unsigned int BackRightBuffer;
  unsigned int FrontLeftBuffer;
  unsigned int FrontRightBuffer;
  unsigned int FrontBuffer;
  unsigned int BackBuffer;

  std::queue<std::pair<std::string, double> > Statistics;
  int StatisticFramesPerOutput;
};

#endif
