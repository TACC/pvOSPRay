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

#include "OSPRayViewOptions.h"
#include "ui_OSPRayViewOptions.h"

#include "vtkSMRenderViewProxy.h"
#include "vtkSMPropertyHelper.h"

#include <QHBoxLayout>

#include "OSPRayView.h"

#include "pqActiveView.h"

class OSPRayViewOptions::pqInternal
{
public:
  Ui::OSPRayViewOptions ui;
};

//----------------------------------------------------------------------------
OSPRayViewOptions::OSPRayViewOptions(QWidget *widgetParent)
  : pqOptionsContainer(widgetParent)
{

  this->Internal = new pqInternal();
  this->Internal->ui.setupUi(this);

  QObject::connect(this->Internal->ui.ao,
                  SIGNAL(toggled(bool)),
                  this, SIGNAL(changesAvailable()));
  QObject::connect(this->Internal->ui.progressiveRefinement,
                  SIGNAL(toggled(bool)),
                  this, SIGNAL(changesAvailable()));
  QObject::connect(this->Internal->ui.samples,
                   SIGNAL(valueChanged(int)),
                   this, SIGNAL(changesAvailable()));
  QObject::connect(this->Internal->ui.maxDepth,
                   SIGNAL(valueChanged(int)),
                   this, SIGNAL(changesAvailable()));
}

//----------------------------------------------------------------------------
OSPRayViewOptions::~OSPRayViewOptions()
{
}

//----------------------------------------------------------------------------
void OSPRayViewOptions::setPage(const QString&)
{
}

//----------------------------------------------------------------------------
QStringList OSPRayViewOptions::getPageList()
{
  QStringList ret;
  ret << "OSPRay";
  return ret;
}

//----------------------------------------------------------------------------
void OSPRayViewOptions::applyChanges()
{
  pqView* view = pqActiveView::instance().current();
  pqRenderView *rView = qobject_cast<pqRenderView*>(view);

  int intSetting;
  bool boolSetting;

  vtkSMRenderViewProxy *proxy = rView->getRenderViewProxy();

  boolSetting = this->Internal->ui.ao->isChecked();
  vtkSMPropertyHelper(proxy, "EnableAO").Set(boolSetting);

  boolSetting = this->Internal->ui.progressiveRefinement->isChecked();
  vtkSMPropertyHelper(proxy, "EnableProgressiveRefinement").Set(boolSetting);

  intSetting = this->Internal->ui.samples->value();
  vtkSMPropertyHelper(proxy, "Samples").Set(intSetting);

  intSetting = this->Internal->ui.maxDepth->value();
  vtkSMPropertyHelper(proxy, "MaxDepth").Set(intSetting);
  
  vtkSMPropertyHelper(proxy, "SuppressLOD").Set(1);
}

//----------------------------------------------------------------------------
void OSPRayViewOptions::resetChanges()
{
}
