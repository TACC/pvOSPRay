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

// .NAME vtkPVOSPRayView VTK level view that uses OSPRay instead of GL
// .SECTION Description
// A 3D view that uses the OSPRay ray tracer instead of openGL for rendering

#ifndef __vtkPVOSPRayView_h
#define __vtkPVOSPRayView_h

#include "vtkPVRenderView.h"

class vtkDataRepresentation;

class VTK_EXPORT vtkPVOSPRayView : public vtkPVRenderView
{
public:
  static vtkPVOSPRayView* New();
  vtkTypeMacro(vtkPVOSPRayView, vtkPVRenderView);
  void PrintSelf(ostream& os, vtkIndent indent);

  // Description:
  // Initialize the view with an identifier. Unless noted otherwise, this method
  // must be called before calling any other methods on this class.
  // @CallOnAllProcessess
  virtual void Initialize(unsigned int id);

  // Description:
  //Controls number of render threads.
  virtual void SetThreads(int val);
  vtkGetMacro(Threads, int);

  // Description:
  // Parameters that controls ray tracing quality
  // Defaults are for minimal quality and maximal speed.
  virtual void SetEnableShadows(int val);
  vtkGetMacro(EnableShadows, int);
  virtual void SetEnableAO(int val);
  vtkGetMacro(EnableAO, int);
  virtual void SetSamples(int val);
  vtkGetMacro(Samples, int);
  virtual void SetMaxDepth(int val);
  vtkGetMacro(MaxDepth, int);

  // Overridden to ensure that we always use an vtkOpenGLCamera of the 2D
  // renderer.
  virtual void SetActiveCamera(vtkCamera*);

//BTX
protected:
  vtkPVOSPRayView();
  ~vtkPVOSPRayView();

  int EnableShadows;
  int EnableAO;
  int Threads;
  int Samples;
  int MaxDepth;

private:
  vtkPVOSPRayView(const vtkPVOSPRayView&); // Not implemented
  void operator=(const vtkPVOSPRayView&); // Not implemented
//ETX
};

#endif // __vtkPVOSPRayView_h
