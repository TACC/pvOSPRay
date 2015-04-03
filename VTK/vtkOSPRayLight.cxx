/*=========================================================================

  Program:   Visualization Toolkit
  Module:    vtkOSPRayLight.cxx

  Copyright (c) Ken Martin, Will Schroeder, Bill Lorensen
  All rights reserved.
  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
/*=========================================================================

  Program:   VTK/ParaView Los Alamos National Laboratory Modules (PVLANL)
  Module:    vtkOSPRayLight.cxx

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

#include "ospray/ospray.h"
#include "ospray/common/OSPCommon.h"

#include "vtkOSPRay.h"
#include "vtkOSPRayLight.h"
#include "vtkOSPRayManager.h"
#include "vtkOSPRayRenderer.h"

#include "vtkObjectFactory.h"

// #include <Interface/Light.h>
// #include <Interface/LightSet.h>
// #include <Interface/Context.h>
// #include <Engine/Control/RTRT.h>
// #include <Model/Lights/PointLight.h>
// #include <Model/Lights/DirectionalLight.h>

#include <math.h>

vtkStandardNewMacro(vtkOSPRayLight);

//----------------------------------------------------------------------------
vtkOSPRayLight::vtkOSPRayLight()
{
  //cerr << "ML(" << this << ") CREATE" << endl;
  // this->MantaLight = NULL;
  this->OSPRayManager = NULL;
}

//----------------------------------------------------------------------------
vtkOSPRayLight::~vtkOSPRayLight()
{
  //cerr << "ML(" << this << ") DESTROY" << endl;
  // delete this->MantaLight;
  if (this->OSPRayManager)
    {
    //cerr << "ML(" << this << ") DESTROY " << this->OSPRayManager << " "
    //     << this->OSPRayManager->GetReferenceCount() << endl;
    this->OSPRayManager->Delete();
    }
}

//----------------------------------------------------------------------------
void vtkOSPRayLight::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);
}

//----------------------------------------------------------------------------
void vtkOSPRayLight::Render(vtkRenderer *ren, int /* not used */)
{
  #if 1
  vtkOSPRayRenderer *renderer = vtkOSPRayRenderer::SafeDownCast(ren);
  if (!renderer)
    {
    return;
    }

  // if (!this->MantaLight)
    {
    CreateLight(ren);
    }
  // else
    {
    // UpdateLight(ren);
    }
    #endif
}

//----------------------------------------------------------------------------
// called in Transaction context, it is safe to modify the engine state here
void vtkOSPRayLight::CreateLight(vtkRenderer *ren)
{
  vtkOSPRayRenderer *mantaRenderer = vtkOSPRayRenderer::SafeDownCast(ren);
  if (!mantaRenderer)
    {
    return;
    }

    if (!this->OSPRayManager)
    {
    this->OSPRayManager = mantaRenderer->GetOSPRayManager();
    //cerr << "ML(" << this << ") REGISTER " << this->OSPRayManager << " "
    //     << this->OSPRayManager->GetReferenceCount() << endl;
    this->OSPRayManager->Register(this);
    }
  
  OSPRenderer renderer = ((OSPRenderer)this->OSPRayManager->OSPRayRenderer);
      std::vector<OSPLight> pointLights;
      std::vector<OSPLight> directionalLights;
    // cout << "msgView: Adding a hard coded directional light as the sun." << endl;


  #if 1
  double *color, *position, *focal, direction[3];

  // Manta Lights only have one "color"
  color    = this->GetDiffuseColor();
  position = this->GetTransformedPosition();
  focal    = this->GetTransformedFocalPoint();

  if (this->GetPositional())
    {
              OSPLight ospLight = ospNewLight(renderer, "OBJPointLight");
    ospSetString(ospLight, "name", "point" );
    ospSet3f(ospLight, "color", color[0],color[1],color[2]);
    ospSet3f(ospLight, "position", position[0],position[1],position[2]);
    ospCommit(ospLight);
    pointLights.push_back(ospLight);
    OSPData pointLightArray = ospNewData(pointLights.size(), OSP_OBJECT, &pointLights[0], 0);
    ospSetData(renderer, "pointLights", pointLightArray); 
    // this->MantaLight = new Manta::PointLight(
    //   Manta::Vector(position[0], position[1], position[2]),
    //   Manta::Color(Manta::RGBColor(color[0],color[1],color[2])));
    }
  else
    {
    // "direction" in Manta means the direction toward light source rather than the
    // direction of rays originate from light source
    direction[0] = position[0] - focal[0];
    direction[1] = position[1] - focal[1];
    direction[2] = position[2] - focal[2];
        OSPLight ospLight = ospNewLight(renderer, "DirectionalLight");
    ospSetString(ospLight, "name", "sun" );
    ospSet3f(ospLight, "color", color[0],color[1],color[2]);
    ospSet3f(ospLight, "direction", direction[0],direction[1],direction[2]);
    ospCommit(ospLight);
    directionalLights.push_back(ospLight);
        OSPData pointLightArray = ospNewData(directionalLights.size(), OSP_OBJECT, &directionalLights[0], 0);
    ospSetData(renderer, "directionalLights", pointLightArray);

    // this->MantaLight = new Manta::DirectionalLight(
    //   Manta::Vector(direction[0], direction[1], direction[2]),
    //   Manta::Color(Manta::RGBColor(color[0],color[1],color[2])));
    }
  // mantaRenderer->GetMantaLightSet()->add(this->MantaLight);
  // if (!this->OSPRayManager)
  //   {
  //   this->OSPRayManager = mantaRenderer->GetOSPRayManager();
  //   //cerr << "ML(" << this << ") REGISTER " << this->OSPRayManager << " "
  //   //     << this->OSPRayManager->GetReferenceCount() << endl;
  //   this->OSPRayManager->Register(this);
  //   }
    #endif
}

//------------------------------------------------------------------------------
// called in Transaction context, it is safe to modify the engine state here
void vtkOSPRayLight::UpdateLight(vtkRenderer *ren)
{
  CreateLight(ren);
  #if 0
  if (!this->MantaLight)
    {
    return;
    }
  double *color, *position, *focal, direction[3];
  double intens = this->GetIntensity();
  double on = (this->GetSwitch()?1.0:0.0);

  // Manta Lights only have one "color"
  color    = this->GetDiffuseColor();
  position = this->GetTransformedPosition();
  focal    = this->GetTransformedFocalPoint();

  double lcolor[3]; //factor in intensity and on/off state for manta API
  lcolor[0] = color[0] * intens * on;
  lcolor[1] = color[1] * intens * on;
  lcolor[2] = color[2] * intens * on;

  if (this->GetPositional())
    {
    Manta::PointLight * pointLight =
      dynamic_cast<Manta::PointLight *>(this->MantaLight);
    if ( pointLight )
        {
        pointLight->setPosition(Manta::Vector(position[0], position[1], position[2]));
        pointLight->setColor(Manta::Color(Manta::RGBColor(lcolor[0],lcolor[1],lcolor[2])));
        }
    else
      {
      vtkWarningMacro(
        << "Changing from Directional to Positional light is not supported by vtkOSPRay" );
      }
    }
  else
    {
    Manta::DirectionalLight * dirLight =
      dynamic_cast<Manta::DirectionalLight *>(this->MantaLight);
    if ( dirLight )
        {
        // "direction" in Manta means the direction toward light source rather than the
        // direction of rays originate from light source
        direction[0] = position[0] - focal[0];
        direction[1] = position[1] - focal[1];
        direction[2] = position[2] - focal[2];
        dirLight->setDirection(Manta::Vector(direction[0], direction[1], direction[2]));
        dirLight->setColor(Manta::Color(Manta::RGBColor(lcolor[0],lcolor[1],lcolor[2])));
        }
    else
      {
      vtkWarningMacro
        (<< "Changing from Positional to Directional light is not supported by vtkOSPRay" );
      }
    }
    #endif
}
