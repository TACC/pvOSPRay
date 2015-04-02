/***********************************************************************************************************
Copyright (c) 2015, Carson Brownlee, Texas Advanced Computing Center, Universtiy of Texas at Austin
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided 
that the following conditions are met:

1. Redistributions of source code must retain the above copyright notice, this list of conditions and the
 following disclaimer.

2. Redistributions in binary form must reproduce the above copyright notice, this list of conditions and 
the following disclaimer in the documentation and/or other materials provided with the distribution.

3. Neither the name of the copyright holder nor the names of its contributors may be used to endorse or
 promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED
WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A
PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE 
FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS 
INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN 
IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
***********************************************************************************************************/

/*=========================================================================

  Program:   Visualization Toolkit
  Module:    vtkOSPRayActor.cxx

  Copyright (c) Ken Martin, Will Schroeder, Bill Lorensen
  All rights reserved.
  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
/*=========================================================================

  Program:   VTK/ParaView Los Alamos National Laboratory Modules (PVLANL)
  Module:    vtkOSPRayActor.cxx

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

     #define GL_GLEXT_PROTOTYPES

#include "vtkOSPRay.h"
#include "vtkOSPRayActor.h"
#include "vtkOSPRayManager.h"
#include "vtkOSPRayProperty.h"
#include "vtkOSPRayRenderer.h"
#include "vtkMapper.h"

#include "vtkDataSet.h"
#include "vtkObjectFactory.h"
#include "vtkRendererCollection.h"
#include "vtkTimerLog.h"

// #include <Interface/Scene.h>
// #include <Interface/Context.h>
// #include <Engine/Control/RTRT.h>
// #include <Model/Groups/DynBVH.h>
// #include <Model/Groups/RecursiveGrid.h>
// #include <Model/Groups/Group.h>
// #include <Core/Geometry/AffineTransform.h>

//
//ospray
//
#if USE_OSPRAY
#include "ospray/ospray.h"
// #include "ospray/common/OspCommon.h"
#endif

//VBO includes
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xatom.h>
#include <GL/glx.h>
#include <GL/glu.h>

#include <GL/glext.h>
#include "vtkOpenGL.h"
#include "vtkOpenGLError.h"
#include <map>
#include <algorithm>


#include "vtkInformation.h"
#include "vtkInformationVector.h"


//============================================================================

//This is a helper that exists just to hold on to manta side resources
//long enough for the manta thread to destroy them, whenever that
//threads gets around to it (in a callback)
#if 0
class vtkOSPRayActorThreadCache
{
public:
  vtkOSPRayActorThreadCache(Manta::Group *w,
    Manta::AccelerationStructure *a,
    Manta::Group *g)
  : MantaWorldGroup(w), MantaAS(a), MantaGeom(g)
  {
    this->DebugCntr = vtkOSPRayActorThreadCache::GlobalCntr++;
    //cerr << "MAPR( " << this << ") " << this->DebugCntr << endl;
    //cerr << " AS: " << this->MantaAS << endl;
    //cerr << " WG: " << this->MantaWorldGroup << endl;
    //cerr << " MG: " << this->MantaGeom << endl;
  }

  void FreeMantaResources()
  {
    //cerr << "MAPR(" << this << ") FREE MANTA RESOURCES "
    //    << this->DebugCntr << endl;
    //cerr << " AS: " << this->MantaAS << endl;
    if (this->MantaAS)
    {
      this->MantaAS->setGroup( NULL );
    }
    //cerr << " WG: " << this->MantaWorldGroup << endl;
    if (this->MantaWorldGroup)
    {
      this->MantaWorldGroup->remove(this->MantaAS, false);
    }
    delete this->MantaAS;
    //cerr << " MG: " << this->MantaGeom << endl;
    if (this->MantaGeom)
    {
      this->MantaGeom->shrinkTo(0, true);
    }
    delete this->MantaGeom;

    //WARNING: this class must never be instantiated on the stack.
    //Therefore, it has private unimplemented copy/contructors.
    delete this;
  }

  Manta::Group * MantaWorldGroup;
  Manta::AccelerationStructure * MantaAS;
  Manta::Group * MantaGeom;
  int DebugCntr;
  static int GlobalCntr;
private:
  vtkOSPRayActorThreadCache(const vtkOSPRayActorThreadCache&);  // Not implemented.
  void operator=(const vtkOSPRayActorThreadCache&);  // Not implemented.
};
#endif

// int vtkOSPRayActorThreadCache::GlobalCntr = 0;

//===========================================================================

vtkStandardNewMacro(vtkOSPRayActor);

//----------------------------------------------------------------------------
vtkOSPRayActor::vtkOSPRayActor()
// : Group(0), MantaAS(0)
{
  //cerr << "MA(" << this << ") CREATE" << endl;
  this->OSPRayManager = NULL;
  this->SortType = DYNBVH;
  // this->OSPRayModel = ospNewModel();
  this->OSPRayModel = NULL;
}

//----------------------------------------------------------------------------
// now some Manta resources, ignored previously, can be de-allocated safely
//
vtkOSPRayActor::~vtkOSPRayActor()
{
  //cerr << "MA(" << this << ") DESTROY" << endl;
  if (this->OSPRayManager)
  {
    this->ReleaseGraphicsResources(NULL);
    //cerr << "MA(" << this << " DESTROY " << this->OSPRayManager << " "
    //     << this->OSPRayManager->GetReferenceCount() << endl;
    this->OSPRayManager->Delete();
  }
}

//----------------------------------------------------------------------------
void vtkOSPRayActor::PrintSelf( ostream & os, vtkIndent indent )
{
  this->Superclass::PrintSelf( os, indent );
}

//----------------------------------------------------------------------------
vtkProperty *vtkOSPRayActor::MakeProperty()
{
  return vtkOSPRayProperty::New();
}

//----------------------------------------------------------------------------
void vtkOSPRayActor::ReleaseGraphicsResources( vtkWindow * win )
{
  #if 0
  //cerr << "MA(" << this << ") RELEASE GRAPHICS RESOURCES" << endl;
  if (win)
  {
    this->Superclass::ReleaseGraphicsResources( win );
  }
  if (!this->OSPRayManager)
  {
    //cerr << "MA(" << this << ") NO MGR" << endl;
    return;
  }

  //save off the pointers for the manta thread
  vtkOSPRayActorThreadCache *R =
  new vtkOSPRayActorThreadCache(this->OSPRayManager->GetMantaWorldGroup(),
   this->MantaAS,
   this->Group);
  //cerr << "MA(" << this << ") handed off to MAPR(" << R << ")" << endl;
  //make no further references to them in this thread
  this->MantaAS = NULL;
  this->Group = NULL;

  //ask the manta thread to free them when it can
  // this->OSPRayManager->GetMantaEngine()->
    // addTransaction("cleanup actor",
                   // Manta::Callback::create
                   // (R, &vtkOSPRayActorThreadCache::FreeMantaResources));
    // if (vboPart)
      // this->OSPRayManager->vbos.erase(find(this->OSPRayManager->vbos.begin(), this->OSPRayManager->vbos.end(), vboPart));
#endif
}

//----------------------------------------------------------------------------
void vtkOSPRayActor::Render( vtkRenderer * ren, vtkMapper * mapper )
{

  // cerr << "MA(" << this << ") RENDER" << endl;
  if ( vtkOSPRayRenderer * mantaRenderer = vtkOSPRayRenderer::SafeDownCast( ren ) )
  {
    if (!this->OSPRayManager)
    {
      this->OSPRayManager = mantaRenderer->GetOSPRayManager();
      //cerr << "MA(" << this << " REGISTER " << this->OSPRayManager << " "
      //     << this->OSPRayManager->GetReferenceCount() << endl;
      this->OSPRayManager->Register(this);
    }

    // TODO: be smarter on update or create rather than create every time
    // build transformation (with AffineTransfrom and Instance?)

    // TODO: the way "real FLAT" shading is done right now (by not supplying vertex
    // normals), changing from FLAT to Gouraud shading needs to create a new mesh.

    // vtkDataObject* input = vtkDataObject::GetData(inputVector[0]);
  // vtkTable* output = vtkTable::GetData(outputVector);

  // char *buffer = new char[strlen(this->Format)+1024];
  // strcpy(buffer, "?");


      // mapper->Render(ren, this);


    //check if anything that affect appearence has changed, if so, rebuild manta
    //object so we see it. Don't do it every frame, since it is costly.
    // if (mapper->GetInput() &&
    //   mapper->GetInput()->GetMTime() > this->MeshMTime ||
    //   mapper->GetMTime() > this->MeshMTime ||
    //   this->GetProperty()->GetMTime() > this->MeshMTime ||
    //   this->GetMTime() > this->MeshMTime)
    {
      // if (this->OSPRayModel != NULL)
        // return;  //TODO: THIS IS A HACK!  Why is it rebuilding in demo?
      // update pipeline to get up to date data to show
      // cerr << "actor updating mapper\n";
      mapper->Render(ren, this);
  //     vtkInformation* inputInfo = NULL;
  //     if (mapper->GetInput())
  //     {
  //       inputInfo = mapper->GetInput()->GetInformation();
  // // // vtkInformation* outputInfo = outputVector->GetInformationObject(0);

  //       if (inputInfo && inputInfo->Has(vtkDataObject::DATA_TIME_STEP())
  //         )
  //       {
  //         double time = inputInfo->Get(vtkDataObject::DATA_TIME_STEP());
  //         if (cache[time] != NULL)
  //         {
  //           std::cerr << "using cache at time " << time << "\n";
  //     // this->OSPRayModel = cache[time];

  //           this->OSPRayModel = cache[time];     
  // // return;

  //     // this->MeshMTime.Modified();
  //     // UpdateObjects(ren);
  //         }
  //       }
  //     }

      // this->MeshMTime.Modified();

      // this->OSPRayManager->GetMantaEngine()->addTransaction
        // ("update geometry",
         // Manta::Callback::create(this, &vtkOSPRayActor::UpdateObjects, ren));

      // if (inputInfo && inputInfo->Has(vtkDataObject::DATA_TIME_STEP())
      //   )
      // {
      //   double time = inputInfo->Get(vtkDataObject::DATA_TIME_STEP());
      //   cache[time] = this->OSPRayModel;

      //   cerr << "MA time: " << time << std::endl;
      // }
    }
      UpdateObjects(ren);
  }
}

//----------------------------------------------------------------------------
void vtkOSPRayActor::SetVisibility(int newval)
{
  //cerr << "MA(" << this << ") SET VISIBILITY " << newval << endl;
  if (newval == this->GetVisibility())
  {
    return;
  }
  if (this->OSPRayManager && !newval)
  {
    //this is necessary since Render (and thus UpdateObjects) is not
    //called when visibility is off.
    // this->OSPRayManager->GetMantaEngine()->addTransaction
      // ( "detach geometry",
        // Manta::Callback::create(this, &vtkOSPRayActor::RemoveObjects) );
  }
  this->Superclass::SetVisibility(newval);
}

//----------------------------------------------------------------------------
void vtkOSPRayActor::RemoveObjects()
{
#if 0
  //cerr << "MA(" << this << ") REMOVE OBJECTS" << endl;
  if (!this->OSPRayManager)
  {
    return;
  }

  if (this->MantaAS)
  {
    //cerr << " AS: " << this->MantaAS << endl;
    //cerr << " WG: " << this->OSPRayManager->GetMantaWorldGroup() << endl;
    Manta::Group *grp = this->MantaAS->getGroup();
    for (unsigned int i = 0; i < grp->size(); i++)
    {
      Manta::Group *ig = dynamic_cast<Manta::Group *>(grp->get(i));
      if (ig)
      {
        ig->shrinkTo(0, true);
        //delete ig;
      }
    }
    grp->shrinkTo(0, true);
    //delete grp;
    this->MantaAS->setGroup( NULL );
    this->OSPRayManager->GetMantaWorldGroup()->remove( this->MantaAS, false );
    delete this->MantaAS;
    this->MantaAS = NULL;
  }
  #endif
}

//----------------------------------------------------------------------------
void vtkOSPRayActor::UpdateObjects( vtkRenderer * ren )
{
  #if 1
  // cerr << "MA(" << this << ") UPDATE" << endl;
  vtkOSPRayRenderer * mantaRenderer =
  vtkOSPRayRenderer::SafeDownCast( ren );
  if (!mantaRenderer)
  {
    return;
  }

  //Remove whatever we used to show in the scene
  if (!this->OSPRayManager)
  {
    return;
  }

  if (!this->OSPRayModel)
    return;

  //TODO:
  //We are using Manta's DynBVH, but we never use it Dyn-amically.
  //Instead we delete the old and rebuild a new AS every time something changes,
  //We should either ask the DynBVH to update itself,
  //or try different acceleration structures. Those might be faster - either
  //during sort or during search.

  //Remove what was shown.
  this->RemoveObjects();

  if (!this->GetVisibility())
    return;

  //Add what we are now supposed to show.
  #if 1
  // if (this->Group)
  {
    vtkTimerLog::MarkStartEvent("Execute AccelStructBuild ");
    //Create an acceleration structure for the data and add it to the scene

    //We have to nest to make an AS for each inner group
    //Is there a Manta call we can make to simply recurse while making the AS?
    // this->MantaAS = new Manta::DynBVH(false);
    // //cerr << "MA(" << this << ") CREATE AS " << this->MantaAS << endl;
    // Manta::Group *group = new Manta::Group();
    // for (unsigned int i = 0; i < this->Group->size(); i++)
    // {
    //   Manta::AccelerationStructure *innerAS = NULL;
    //   switch (this->SortType) {
    //     case DYNBVH:
    //     default:
    //     innerAS = new Manta::DynBVH(false);
    //     break;
    //     case RECURSIVEGRID3:
    //     innerAS = new Manta::RecursiveGrid(3);
    //     break;
    //   }

    //   Manta::Group * innerGroup = dynamic_cast<Manta::Group *>
    //   (this->Group->get(i));
    //   if (innerGroup)
    //   {
    //     //cerr << "MA(" << this << ") BVH FOR " << i << " " << innerGroup << endl;
    //     innerAS->setGroup(innerGroup);
    //     group->add(innerAS);
    //     innerAS->rebuild();
    //   }
    //   else
    //   {
    //     //cerr << "MA(" << this << ") SIMPLE " << i << " " << innerGroup << endl;
    //     delete innerAS;
    //     group->add(this->Group->get(i));
    //   }
    // }

    //cerr << "MA(" << this << ") PREPROCESS" << endl;
    // this->MantaAS->setGroup(group);
    // Manta::Group* mantaWorldGroup = this->OSPRayManager->GetMantaWorldGroup();
    // mantaWorldGroup->add(static_cast<Manta::Object *> (this->MantaAS));
    // //cerr << "ME = " << this->OSPRayManager->GetMantaEngine() << endl;
    // //cerr << "LS = " << this->OSPRayManager->GetMantaLightSet() << endl;
    // Manta::PreprocessContext context(this->OSPRayManager->GetMantaEngine(), 0, 1,
    //  this->OSPRayManager->GetMantaLightSet());
    // mantaWorldGroup->preprocess(context);
    //cerr << "PREP DONE" << endl;

    // Manta::AffineTransform mt;
    // mt.initWithIdentity();
    // printf("adding ospray actor objects\n");
    // this->OSPRayModel = ospNewModel();
    OSPGeometry inst = ospNewInstance((((OSPModel)this->OSPRayModel)), osp::affine3f(embree::one));
      /*ospray::affine3f(embree::LinearSpace3f(mt(0,0), mt(0,1), mt(0,2), mt(1,0), mt(1,1), mt(1,2), mt(2,0),mt(2,1),mt(2,2)), embree::Vec3fa(mt(0,3),mt(1,3),mt(2,3))*/
    ospAddGeometry((((OSPModel)this->OSPRayManager->OSPRayModel)),inst);

    vtkTimerLog::MarkEndEvent("Execute AccelStructBuild ");
  }
  #endif
  #endif
}
#if 0
//----------------------------------------------------------------------------
void vtkOSPRayActor::SetGroup( Manta::Group * group )
{
  #if 0
  //cerr << "MA(" << this << ") SET GROUP"
  //     << " WAS " << this->Group
  //     << " NOW " << group << endl;
  if (!this->Group)
  {
    this->Group = group;
    return;
  }

  //save off the pointers for the manta thread
  vtkOSPRayActorThreadCache *R = new vtkOSPRayActorThreadCache(NULL,
   NULL,
   this->Group);

  this->Group = group;
  //ask the manta thread to free them when it can
  // this->OSPRayManager->GetMantaEngine()->
    // addTransaction("change geometry",
                   // Manta::Callback::create
                   // (R, &vtkOSPRayActorThreadCache::FreeMantaResources));
                   #endif

}
#endif
