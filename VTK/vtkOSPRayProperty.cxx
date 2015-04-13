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

#include "ospray/ospray.h"
#include "ospray/common/OSPCommon.h"

#include "vtkOSPRay.h"
#include "vtkOSPRayManager.h"
#include "vtkOSPRayProperty.h"
#include "vtkOSPRayRenderer.h"

#include "vtkObjectFactory.h"

// #include <Core/Color/RGBColor.h>
// #include <Engine/Control/RTRT.h>
// //#include <Model/Materials/AmbientOcclusion.h>
// #include <Model/Materials/Dielectric.h>
// #include <Model/Materials/Flat.h>
// #include <Model/Materials/Lambertian.h>
// #include <Model/Materials/MetalMaterial.h>
// #include <Model/Materials/OrenNayar.h>
// #include <Model/Materials/Phong.h>
// #include <Model/Materials/ThinDielectric.h>
// #include <Model/Materials/Transparent.h>
// #include <Model/Textures/Constant.h>

#include <cstring>


#if 0
//============================================================================
//This is a helper that exists just to hold on to OSPRay side resources
//long enough for the OSPRay thread to destroy them, whenever that
//threads gets around to it (in a callback)
class vtkOSPRayPropertyThreadCache
{
public:
  vtkOSPRayPropertyThreadCache(OSPRay::Material *m,
      OSPRay::Texture<OSPRay::Color> *dT,
      OSPRay::Texture<OSPRay::Color> *sT )
    : OSPRayMaterial(m), DiffuseTexture(dT), SpecularTexture(sT)
  {
    this->DebugCntr = vtkOSPRayPropertyThreadCache::GlobalCntr++;
    //cerr << "MPPR( " << this << ") " << this->DebugCntr << endl;
  }

  void FreeOSPRayResources()
  {
    //cerr << "MPPR(" << this << ") FREE OSPRay RESOURCES "
    //<< this->DebugCntr << endl;
    delete this->OSPRayMaterial;
    delete this->DiffuseTexture;
    delete this->SpecularTexture;

    //WARNING: this class must never be instantiated on the stack.
    //Therefore, it has private unimplemented copy/contructors.
    delete this;
  }

  OSPRay::Material *OSPRayMaterial;
  OSPRay::Texture<OSPRay::Color> *DiffuseTexture;
  OSPRay::Texture<OSPRay::Color> *SpecularTexture;
  int DebugCntr;
  static int GlobalCntr;

private:
  vtkOSPRayPropertyThreadCache(const vtkOSPRayPropertyThreadCache&);  // Not implemented.
  void operator=(const vtkOSPRayPropertyThreadCache&);  // Not implemented.
};
#endif

// int vtkOSPRayPropertyThreadCache::GlobalCntr = 0;

//===========================================================================

vtkStandardNewMacro(vtkOSPRayProperty);

//----------------------------------------------------------------------------
vtkOSPRayProperty::vtkOSPRayProperty()
  // : OSPRayMaterial(0), DiffuseTexture(0), SpecularTexture(0),
    // Reflectance(0.0), Eta(1.52), Thickness(1.0), N(1.0), Nt(1.0)
{
  cerr << "MP(" << this << ") CREATE" << endl;
  this->MaterialType = NULL;
  this->SetMaterialType("default");
  this->OSPRayManager = NULL;
  this->OSPRayMaterial= NULL;//new osp::Material;

  // OSPRenderer renderer = ((OSPRenderer)this->OSPRayManager->OSPRayRenderer);
}

//----------------------------------------------------------------------------
vtkOSPRayProperty::~vtkOSPRayProperty()
{
  if (this->OSPRayManager)
  {
    //cerr << "MP(" << this << ") DESTROY " << this->OSPRayManager << " "
    //     << this->OSPRayManager->GetReferenceCount() << endl;
    this->OSPRayManager->Delete();
  }
  delete[] this->MaterialType;
}

//----------------------------------------------------------------------------
void vtkOSPRayProperty::PrintSelf( ostream & os, vtkIndent indent )
{
  this->Superclass::PrintSelf( os, indent );
}

//------------------------------------------------------------------------------
void vtkOSPRayProperty::ReleaseGraphicsResources(vtkWindow *win)
{
  //cerr << "MP(" << this << ") RELEASE GRAPHICS RESOURCES" << endl;
  this->Superclass::ReleaseGraphicsResources(win);
  if (!this->OSPRayManager)
  {
    return;
  }

  //save off the pointers for the OSPRay thread
  // vtkOSPRayPropertyThreadCache *R =
    // new vtkOSPRayPropertyThreadCache(this->OSPRayMaterial,
                                    // this->DiffuseTexture,
                                    // this->SpecularTexture);
  //make no further references to them in this thread
  // this->OSPRayMaterial = NULL;
  // this->DiffuseTexture = NULL;
  // this->SpecularTexture = NULL;

  //ask the OSPRay thread to free them when it can
  // this->OSPRayManager->GetOSPRayEngine()->addTransaction
    // ( "cleanup property",
      // OSPRay::Callback::create
      // (R, &vtkOSPRayPropertyThreadCache::FreeOSPRayResources));
}

//----------------------------------------------------------------------------
void vtkOSPRayProperty::Render( vtkActor *vtkNotUsed(anActor),
 vtkRenderer * ren)
{
  vtkOSPRayRenderer * OSPRayRenderer = vtkOSPRayRenderer::SafeDownCast( ren );
  if (!OSPRayRenderer)
  {
    return;
  }
  if (!this->OSPRayManager)
  {
    this->OSPRayManager = OSPRayRenderer->GetOSPRayManager();
    //cerr << "MP(" << this << ") REGISTER " << this->OSPRayManager << " "
    //     << this->OSPRayManager->GetReferenceCount() << endl;
    this->OSPRayManager->Register(this);
  }

  double * diffuse  = this->GetDiffuseColor();

  // return;

  if ( this->GetMTime() > this->OSPRayMaterialMTime )
  {
    // std::cerr << "resetting ospray material with diffuse color " << diffuse[0] << " " << diffuse[1] << " " << diffuse[2] << std::endl;
      CreateOSPRayProperty();
    //TODO: this doesn't actually have to be a transaction, other than
    //the deletions
    //TODO: Create should happen before now, whenever the prop is
    //changes actually (see how OSPRayPolyDataMapper creates it)
    // this->OSPRayManager->GetOSPRayEngine()->addTransaction
      // ( "set property",
        // OSPRay::Callback::create(this, &vtkOSPRayProperty::CreateOSPRayProperty));

    this->OSPRayMaterialMTime.Modified();
  }

}

//----------------------------------------------------------------------------
// Implement base class method.
void vtkOSPRayProperty::BackfaceRender( vtkActor * vtkNotUsed( anActor ),
 vtkRenderer * vtkNotUsed( ren ) )
{
  // NOT supported by OSPRay
  //TODO: Do something about it.
  cerr
  << "vtkOSPRayProperty::BackfaceRender(), backface rendering "
  << "is not supported by OSPRay"
  << endl;
}


//----------------------------------------------------------------------------
void vtkOSPRayProperty::CreateOSPRayProperty()
{
  #if 1
  //cerr << "MP(" << this << ") CREATE OSPRay PROPERTY" << endl;

  double * diffuse  = this->GetDiffuseColor();
  double * specular = this->GetSpecularColor();

  //this only happens in a OSPRay thread callback, so this is safe to do - not true

  /*
  if (this->OSPRayMaterial)
    {
    cerr << "DELETING " << this->OSPRayMaterial << endl;
    }
  */
  // delete this->OSPRayMaterial;
  // delete this->DiffuseTexture;
  // delete this->SpecularTexture;

  // this->DiffuseTexture  = new OSPRay::Constant<OSPRay::Color>
  //   (  OSPRay::Color( OSPRay::RGBColor( diffuse[0],  diffuse[1],  diffuse[2]  ) )  );

  // this->SpecularTexture = new OSPRay::Constant<OSPRay::Color>
  //   (  OSPRay::Color( OSPRay::RGBColor( specular[0], specular[1], specular[2] ) )  );

  // A note on OSPRay Materials and shading model:
  // 1. Surface normal is computed at each hit point, if the primitive
  // has curvature (like sphere or triangle with vertex normal), it will
  // be different at each hit point.
  // 2. The Flat material takes account only the surface normal and the
  // color texture of the material. The color texture acts like the emissive
  // color in OpenGL. It is multiplied by the dot product of surface normal
  // and ray direction to get the result color. Since the dot product is
  // different at each point on the primitive the result looks more like
  // Gouraud shading without specular highlight in OpenGL.
  // 3. To get "real" FLAT shading, we have to not to supply vertex normal when
  // we create the mesh.
  // TODO: replace this whole thing with a factory
    if ( strcmp( this->MaterialType, "default" ) == 0 )
    {
    // if lighting is disabled, use EmitMaterial
      if ( this->Interpolation == VTK_FLAT )
      {
      // this->OSPRayMaterial = new OSPRay::Flat( this->DiffuseTexture );
      }
      else
        if ( this->GetOpacity() < 1.0 )
        {
        // this->OSPRayMaterial =
          // new OSPRay::Transparent( this->DiffuseTexture, this->GetOpacity() );
        }
        else
          if ( this->GetSpecular() == 0 )
          {
          // this->OSPRayMaterial = new OSPRay::Lambertian( this->DiffuseTexture );
          }
          else
          {
          // this->OSPRayMaterial =
          //   new OSPRay::Phong( this->DiffuseTexture,
          //                     this->SpecularTexture,
          //                     static_cast<int> ( this->GetSpecularPower() ),
          //                     NULL );
          }
        }
        else
        {
          if ( strcmp( this->MaterialType,  "lambertian" ) == 0 )
          {
      // this->OSPRayMaterial = new OSPRay::Lambertian( this->DiffuseTexture );
          }
          else
            if ( strcmp( this->MaterialType, "phong" ) == 0 )
            {
        // this->OSPRayMaterial =
        //   new OSPRay::Phong( this->DiffuseTexture,
        //                     this->SpecularTexture,
        //                     static_cast<int> ( this->GetSpecularPower() ),
        //                     new OSPRay::Constant<OSPRay::ColorComponent>
        //                     ( this->Reflectance ) );
            }
            else
              if ( strcmp( this->MaterialType, "transparent" ) == 0 )
              {
          // this->OSPRayMaterial
            // = new OSPRay::Transparent( this->DiffuseTexture, this->GetOpacity() );
              }
              else
                if ( strcmp( this->MaterialType, "thindielectric" ) == 0 )
                {
            // this->OSPRayMaterial = new OSPRay::ThinDielectric
            //   (
            //    new OSPRay::Constant<OSPRay::Real>( this->Eta ),
            //    this->DiffuseTexture, this->Thickness, 1 );
                }
                else
                  if ( strcmp( this->MaterialType, "dielectric" ) == 0 )
                  {
              // this->OSPRayMaterial
              //   = new OSPRay::Dielectric( new OSPRay::Constant<OSPRay::Real>( this->N  ),
              //                            new OSPRay::Constant<OSPRay::Real>( this->Nt ),
              //                            this->DiffuseTexture );
                  }
                  else
                    if ( strcmp( this->MaterialType, "metal" ) == 0 )
                    {
                // this->OSPRayMaterial = new OSPRay::MetalMaterial( this->DiffuseTexture );
                    }
                    else
                      if ( strcmp( this->MaterialType, "orennayer" ) == 0 )
                      {
                  // this->OSPRayMaterial = new OSPRay::OrenNayar( this->DiffuseTexture );
                      }
                      else
                      {
                  // just default to phong
                  // this->OSPRayMaterial
                  //   = new OSPRay::Phong( this->DiffuseTexture,
                  //                       this->SpecularTexture,
                  //                       static_cast<int> ( this->GetSpecularPower() ),
                  //                       new OSPRay::Constant<OSPRay::ColorComponent>
                  //                       (
                  //                        this->Reflectance) );
                      }
                    }

          OSPRenderer renderer = ((OSPRenderer)this->OSPRayManager->OSPRayRenderer);

          // FIXME: this check is removed so the material is re-created properly when the renderer is changed
          // need some way to delete the material stored in ospray
          //if (!this->OSPRayMaterial)
            this->OSPRayMaterial = ospNewMaterial(renderer,"OBJMaterial");
          OSPMaterial oMaterial = (OSPMaterial)this->OSPRayMaterial;
          Assert(oMaterial);
          float diffusef[] = {diffuse[0], diffuse[1], diffuse[2]};
// float diffusef[] = {1, 0, 0};
          float specularf[] = {specular[0],specular[1],specular[2]};
          ospSet3fv(oMaterial,"Kd",diffusef);
          ospSet3fv(oMaterial,"Ks",specularf);
          ospSet1f(oMaterial,"Ns",float(this->GetSpecularPower()));
          ospSet1f(oMaterial,"d", float(this->GetOpacity()));

          ospCommit(oMaterial);
          // std::cerr << "creating ospray material with diffuse color " << diffuse[0] << " " << diffuse[1] << " " << diffuse[2] << std::endl;

      // ospMaterial = (osp::Material*)&oMaterial;
      // *(this->ospMaterial) = *((osp::Material*)&oMaterial);
      // PRINT(oMaterial);
      // OSPMaterial mat = *((OSPMaterial*)ospMaterial);
      // PRINT(mat);


  //cerr << "CREATED " << this->OSPRayMaterial << endl;
  #endif
                  }
