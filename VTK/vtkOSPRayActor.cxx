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

//VBO includes
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xatom.h>
#include <GL/glx.h>
#ifndef __APPLE__
#include <GL/glu.h>
#else
#include <OpenGL/glu.h>
#endif


#include <GL/glext.h>
#include "vtkOpenGL.h"
#include "vtkOpenGLError.h"
#include <map>
#include <algorithm>


#include "vtkInformation.h"
#include "vtkInformationVector.h"


//============================================================================

//This is a helper that exists just to hold on to OSPRay side resources
//long enough for the OSPRay thread to destroy them, whenever that
//threads gets around to it (in a callback)
#if 0
class vtkOSPRayActorThreadCache
{
public:
  vtkOSPRayActorThreadCache(OSPRay::Group *w,
    OSPRay::AccelerationStructure *a,
    OSPRay::Group *g)
  : OSPRayWorldGroup(w), OSPRayAS(a), OSPRayGeom(g)
  {
    this->DebugCntr = vtkOSPRayActorThreadCache::GlobalCntr++;
    //cerr << "MAPR( " << this << ") " << this->DebugCntr << endl;
    //cerr << " AS: " << this->OSPRayAS << endl;
    //cerr << " WG: " << this->OSPRayWorldGroup << endl;
    //cerr << " MG: " << this->OSPRayGeom << endl;
  }

  void FreeOSPRayResources()
  {
    //cerr << "MAPR(" << this << ") FREE OSPRay RESOURCES "
    //    << this->DebugCntr << endl;
    //cerr << " AS: " << this->OSPRayAS << endl;
    if (this->OSPRayAS)
    {
      this->OSPRayAS->setGroup( NULL );
    }
    //cerr << " WG: " << this->OSPRayWorldGroup << endl;
    if (this->OSPRayWorldGroup)
    {
      this->OSPRayWorldGroup->remove(this->OSPRayAS, false);
    }
    delete this->OSPRayAS;
    //cerr << " MG: " << this->OSPRayGeom << endl;
    if (this->OSPRayGeom)
    {
      this->OSPRayGeom->shrinkTo(0, true);
    }
    delete this->OSPRayGeom;

    //WARNING: this class must never be instantiated on the stack.
    //Therefore, it has private unimplemented copy/contructors.
    delete this;
  }

  OSPRay::Group * OSPRayWorldGroup;
  OSPRay::AccelerationStructure * OSPRayAS;
  OSPRay::Group * OSPRayGeom;
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
// : Group(0), OSPRayAS(0)
{
  std::cout << __PRETTY_FUNCTION__ << " " << this << std::endl;
  //cerr << "MA(" << this << ") CREATE" << endl;
  this->OSPRayManager = NULL;
  this->SortType = DYNBVH;
  // this->OSPRayModel = ospNewModel();
  this->OSPRayModel = NULL;
  // this->SetEnableLOD(false);
}

//----------------------------------------------------------------------------
// now some OSPRay resources, ignored previously, can be de-allocated safely
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

  //save off the pointers for the OSPRay thread
  vtkOSPRayActorThreadCache *R =
  new vtkOSPRayActorThreadCache(this->OSPRayManager->GetOSPRayWorldGroup(),
   this->OSPRayAS,
   this->Group);
  //cerr << "MA(" << this << ") handed off to MAPR(" << R << ")" << endl;
  //make no further references to them in this thread
  this->OSPRayAS = NULL;
  this->Group = NULL;

  //ask the OSPRay thread to free them when it can
  // this->OSPRayManager->GetOSPRayEngine()->
    // addTransaction("cleanup actor",
                   // OSPRay::Callback::create
                   // (R, &vtkOSPRayActorThreadCache::FreeOSPRayResources));
    // if (vboPart)
      // this->OSPRayManager->vbos.erase(find(this->OSPRayManager->vbos.begin(), this->OSPRayManager->vbos.end(), vboPart));
#endif
}

//----------------------------------------------------------------------------
void vtkOSPRayActor::Render( vtkRenderer * ren, vtkMapper * mapper )
{
  cerr << __PRETTY_FUNCTION__ << " " << this << endl;
  if ( vtkOSPRayRenderer * OSPRayRenderer = vtkOSPRayRenderer::SafeDownCast( ren ) )
  {
    if (!this->OSPRayManager)
    {
      this->OSPRayManager = OSPRayRenderer->GetOSPRayManager();
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


    //check if anything that affect appearence has changed, if so, rebuild OSPRay
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

      // this->OSPRayManager->GetOSPRayEngine()->addTransaction
        // ("update geometry",
         // OSPRay::Callback::create(this, &vtkOSPRayActor::UpdateObjects, ren));

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
    // this->OSPRayManager->GetOSPRayEngine()->addTransaction
      // ( "detach geometry",
        // OSPRay::Callback::create(this, &vtkOSPRayActor::RemoveObjects) );
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

  if (this->OSPRayAS)
  {
    //cerr << " AS: " << this->OSPRayAS << endl;
    //cerr << " WG: " << this->OSPRayManager->GetOSPRayWorldGroup() << endl;
    OSPRay::Group *grp = this->OSPRayAS->getGroup();
    for (unsigned int i = 0; i < grp->size(); i++)
    {
      OSPRay::Group *ig = dynamic_cast<OSPRay::Group *>(grp->get(i));
      if (ig)
      {
        ig->shrinkTo(0, true);
        //delete ig;
      }
    }
    grp->shrinkTo(0, true);
    //delete grp;
    this->OSPRayAS->setGroup( NULL );
    this->OSPRayManager->GetOSPRayWorldGroup()->remove( this->OSPRayAS, false );
    delete this->OSPRayAS;
    this->OSPRayAS = NULL;
  }
  #endif
}

//----------------------------------------------------------------------------
void vtkOSPRayActor::UpdateObjects( vtkRenderer * ren )
{
  #if 1
  // cerr << "MA(" << this << ") UPDATE" << endl;
  vtkOSPRayRenderer * OSPRayRenderer =
  vtkOSPRayRenderer::SafeDownCast( ren );
  if (!OSPRayRenderer)
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
  //We are using OSPRay's DynBVH, but we never use it Dyn-amically.
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
    //Is there a OSPRay call we can make to simply recurse while making the AS?
    // this->OSPRayAS = new OSPRay::DynBVH(false);
    // //cerr << "MA(" << this << ") CREATE AS " << this->OSPRayAS << endl;
    // OSPRay::Group *group = new OSPRay::Group();
    // for (unsigned int i = 0; i < this->Group->size(); i++)
    // {
    //   OSPRay::AccelerationStructure *innerAS = NULL;
    //   switch (this->SortType) {
    //     case DYNBVH:
    //     default:
    //     innerAS = new OSPRay::DynBVH(false);
    //     break;
    //     case RECURSIVEGRID3:
    //     innerAS = new OSPRay::RecursiveGrid(3);
    //     break;
    //   }

    //   OSPRay::Group * innerGroup = dynamic_cast<OSPRay::Group *>
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
    // this->OSPRayAS->setGroup(group);
    // OSPRay::Group* OSPRayWorldGroup = this->OSPRayManager->GetOSPRayWorldGroup();
    // OSPRayWorldGroup->add(static_cast<OSPRay::Object *> (this->OSPRayAS));
    // //cerr << "ME = " << this->OSPRayManager->GetOSPRayEngine() << endl;
    // //cerr << "LS = " << this->OSPRayManager->GetOSPRayLightSet() << endl;
    // OSPRay::PreprocessContext context(this->OSPRayManager->GetOSPRayEngine(), 0, 1,
    //  this->OSPRayManager->GetOSPRayLightSet());
    // OSPRayWorldGroup->preprocess(context);
    //cerr << "PREP DONE" << endl;

    // OSPRay::AffineTransform mt;
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
void vtkOSPRayActor::SetGroup( OSPRay::Group * group )
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

  //save off the pointers for the OSPRay thread
  vtkOSPRayActorThreadCache *R = new vtkOSPRayActorThreadCache(NULL,
   NULL,
   this->Group);

  this->Group = group;
  //ask the OSPRay thread to free them when it can
  // this->OSPRayManager->GetOSPRayEngine()->
    // addTransaction("change geometry",
                   // OSPRay::Callback::create
                   // (R, &vtkOSPRayActorThreadCache::FreeOSPRayResources));
                   #endif

}
#endif
