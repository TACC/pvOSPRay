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

// .NAME vtkOSPRayViewProxy - view setup for vtkOSPRay
// .SECTION Description
// A  View that sets up the display pipeline so that it works with OSPRay.
// This class causes OSPRay specific representations to be created and
// also initializes the client server wrapped vtkOSPRay library for
// paraview.

#ifndef __vtkSMOSPRayViewProxy_h
#define __vtkSMOSPRayViewProxy_h

#include "vtkSMRenderViewProxy.h"

class vtkSMRepresentationProxy;

class VTK_EXPORT vtkSMOSPRayViewProxy : public vtkSMRenderViewProxy
{
public:
  static vtkSMOSPRayViewProxy* New();
  vtkTypeMacro(vtkSMOSPRayViewProxy, vtkSMRenderViewProxy);
  void PrintSelf(ostream& os, vtkIndent indent);

  // Description:
  // Create a default representation for the given output port of source proxy.
  // Returns a new proxy.
  virtual vtkSMRepresentationProxy* CreateDefaultRepresentation(
    vtkSMProxy*, int opport);

  // Description:
  virtual bool IsSelectionAvailable() { return false; }


//BTX
protected:
  vtkSMOSPRayViewProxy();
  ~vtkSMOSPRayViewProxy();

  virtual void CreateVTKObjects();

private:

  vtkSMOSPRayViewProxy(const vtkSMOSPRayViewProxy&); // Not implemented.
  void operator=(const vtkSMOSPRayViewProxy&); // Not implemented.

//ETX
};


#endif // __vtkSMOSPRayViewProxy_h
