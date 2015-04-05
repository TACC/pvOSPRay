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

#include "OSPRayDisplay.h"
#include "ui_OSPRayDisplay.h"

// Qt Includes.
#include <QVBoxLayout>

// ParaView Includes.
#include "pqDisplayPanel.h"
#include "pqPropertyLinks.h"
#include "vtkSMPVRepresentationProxy.h"
#include "pqSignalAdaptors.h"
#include "pqActiveObjects.h"
#include "OSPRayView.h"

#include <iostream>

class OSPRayDisplay::pqInternal
{
public:
  Ui::OSPRayDisplay ui;
  pqPropertyLinks links;
  pqSignalAdaptorComboBox *strAdapt;
};

//-----------------------------------------------------------------------------
OSPRayDisplay::OSPRayDisplay(pqDisplayPanel* panel)
  : Superclass(panel)
{
  QWidget* frame = new QWidget(panel);
  this->Internal = new pqInternal;
  this->Internal->ui.setupUi(frame);
  QVBoxLayout* l = qobject_cast<QVBoxLayout*>(panel->layout());
  l->addWidget(frame);

  this->Internal->strAdapt =
    new pqSignalAdaptorComboBox(this->Internal->ui.material);

  OSPRayView* mView = qobject_cast<OSPRayView*>
    (pqActiveObjects::instance().activeView());
  if (!mView)
    {
    frame->setEnabled(false);
    return;
    }

  pqRepresentation *rep = panel->getRepresentation();
  vtkSMPVRepresentationProxy *mrep = vtkSMPVRepresentationProxy::SafeDownCast
    (rep->getProxy());
  if (!mrep)
    {
    frame->setEnabled(false);
    return;
    }

  vtkSMProperty *prop = mrep->GetProperty("MaterialType");
  //have to use a helper class because pqPropertyLinks won't map directly
  this->Internal->links.addPropertyLink(
    this->Internal->strAdapt,
    "currentText",
    SIGNAL(currentTextChanged(const QString&)),
    mrep,
    prop);

  prop = mrep->GetProperty("Reflectance");
  this->Internal->links.addPropertyLink(
    this->Internal->ui.reflectance,
    "value",
    SIGNAL(valueChanged(double)),
    mrep,
    prop);

  prop = mrep->GetProperty("Thickness");
  this->Internal->links.addPropertyLink(
    this->Internal->ui.thickness,
    "value",
    SIGNAL(valueChanged(double)),
    mrep,
    prop);

  prop = mrep->GetProperty("Eta");
  this->Internal->links.addPropertyLink(
    this->Internal->ui.eta,
    "value",
    SIGNAL(valueChanged(double)),
    mrep,
    prop);

  prop = mrep->GetProperty("N");
  this->Internal->links.addPropertyLink(
    this->Internal->ui.n,
    "value",
    SIGNAL(valueChanged(double)),
    mrep,
    prop);

  prop = mrep->GetProperty("Nt");
  this->Internal->links.addPropertyLink(
    this->Internal->ui.nt,
    "value",
    SIGNAL(valueChanged(double)),
    mrep,
    prop);
}

//-----------------------------------------------------------------------------
OSPRayDisplay::~OSPRayDisplay()
{
  delete this->Internal->strAdapt;
  delete this->Internal;
}
