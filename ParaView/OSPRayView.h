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

// .NAME OSPRayView - QT GUI interface to a OSPRay Rendered 3D View
// .SECTION Description
// This is the Qt layer interface to the OSPRay Rendered 3D view.

#ifndef _OSPRayView_h
#define _OSPRayView_h

#include "pqRenderView.h"

class OSPRayView : public pqRenderView
{
  Q_OBJECT
  typedef pqRenderView Superclass;
public:
  static QString OSPRayViewType() { return "OSPRayView"; }
  static QString OSPRayViewTypeName() { return "OSPRayViewType Rendered 3D View"; }

  /// constructor takes a bunch of init stuff and must have this signature to
  /// satisfy pqView
  OSPRayView(
         const QString& viewtype,
         const QString& group,
         const QString& name,
         vtkSMViewProxy* viewmodule,
         pqServer* server,
         QObject* p);
  ~OSPRayView();

protected:

private:
  OSPRayView(const OSPRayView&); // Not implemented.
  void operator=(const OSPRayView&); // Not implemented.

};

#endif // _OSPRayView_h
