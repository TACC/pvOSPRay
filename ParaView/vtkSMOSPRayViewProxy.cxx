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
// A  View that sets up the display pipeline so that it
// works with OSPRay.
#include <QTimer>
#include "vtkCommand.h"
#include "vtkSMOSPRayViewProxy.h"
#include "vtkObjectFactory.h"

#include "vtkClientServerStream.h"
#include "vtkProcessModule.h"
#include "vtkSMInputProperty.h"
#include "vtkSMPropertyHelper.h"
#include "vtkSMSessionProxyManager.h"
#include "vtkSMSession.h"
#include "vtkSMRepresentationProxy.h"
#include "vtkSMSourceProxy.h"

#include "vtkQtProgressiveRenderer.h"
#include "vtkPVGenericRenderWindowInteractor.h"

#include <assert.h>

//-----------------------------------------------------------------------------
vtkStandardNewMacro(vtkSMOSPRayViewProxy);


//-----------------------------------------------------------------------------
vtkSMOSPRayViewProxy::vtkSMOSPRayViewProxy()
{
}

//-----------------------------------------------------------------------------
vtkSMOSPRayViewProxy::~vtkSMOSPRayViewProxy()
{
}

//-----------------------------------------------------------------------------
void vtkSMOSPRayViewProxy::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);
}

//------------------------------------------------------------------------------
void vtkSMOSPRayViewProxy::CreateVTKObjects()
{
  this->Superclass::CreateVTKObjects();

  vtkSMPropertyHelper(this, "UseLight").Set(0);
  vtkSMPropertyHelper(this, "LightSwitch").Set(1);
}

//-----------------------------------------------------------------------------
vtkSMRepresentationProxy* vtkSMOSPRayViewProxy::CreateDefaultRepresentation(
  vtkSMProxy* source, int opport)
{
  if (!source)
    {
    return 0;
    }

    const char* toTry[] = 
    {
      /*"OSPRayUniformGridRepresentation",*/
      "OSPRayGeometryRepresentation",
      NULL
    };

  assert("Session should be set by now" && this->Session);
  vtkSMSessionProxyManager* pxm = this->GetSessionProxyManager();

  // Update with time to avoid domains updating without time later.
  vtkSMSourceProxy* sproxy = vtkSMSourceProxy::SafeDownCast(source);
  if (sproxy)
    {
    double view_time = vtkSMPropertyHelper(this, "ViewTime").GetAsDouble();
    sproxy->UpdatePipeline(view_time);
    }
for(int i=0; toTry[i] != NULL; i++)
{
  // Choose which type of representation proxy to create.
  vtkSMProxy* prototype;
  prototype = pxm->GetPrototypeProxy("representations",
    toTry[i]);
  vtkSMInputProperty *pp = vtkSMInputProperty::SafeDownCast(
    prototype->GetProperty("Input"));
  pp->RemoveAllUncheckedProxies();
  pp->AddUncheckedInputConnection(source, opport);
  bool g = (pp->IsInDomains()>0);
  pp->RemoveAllUncheckedProxies();


//   vtkQtProgressiveRenderer* progressiveRenderer = new vtkQtProgressiveRenderer();
//   this->AddObserver(vtkCommand::UpdateDataEvent,
//       progressiveRenderer, &vtkQtProgressiveRenderer::onViewUpdated);
//   this->GetInteractor()->AddObserver(
//       vtkCommand::StartInteractionEvent,
//       progressiveRenderer, &vtkQtProgressiveRenderer::onStartInteractionEvent);
// this->GetInteractor()->AddObserver(
//       vtkCommand::EndInteractionEvent,
//       progressiveRenderer, &vtkQtProgressiveRenderer::onEndInteractionEvent);

  if (g)
    {
      printf("using OSPRay representation: %s\n", toTry[i]);
    return vtkSMRepresentationProxy::SafeDownCast(
      pxm->NewProxy("representations", toTry[i]));
    }
  }

  return 0;
}
