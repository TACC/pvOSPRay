/*=========================================================================

  Program:   Visualization Toolkit
  Module:    vtkOSPRayPolyDataMapper.cxx

  Copyright (c) Ken Martin, Will Schroeder, Bill Lorensen
  All rights reserved.
  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
/*=========================================================================

  Program:   VTK/ParaView Los Alamos National Laboratory Modules (PVLANL)
  Module:    vtkOSPRayPolyDataMapper.cxx

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

#include "vtkOSPRay.h"
#include "vtkOSPRayActor.h"
#include "vtkOSPRayManager.h"
#include "vtkOSPRayPolyDataMapper.h"
#include "vtkOSPRayProperty.h"
#include "vtkOSPRayRenderer.h"
#include "vtkOSPRayTexture.h"

#include "vtkAppendPolyData.h"
#include "vtkCellArray.h"
#include "vtkCellData.h"
#include "vtkCommand.h"
#include "vtkDataArray.h"
#include "vtkDoubleArray.h"
#include "vtkFloatArray.h"
#include "vtkGenericCell.h"
#include "vtkGlyph3D.h"
#include "vtkImageData.h"
#include "vtkMatrix4x4.h"
#include "vtkObjectFactory.h"
#include "vtkPointData.h"
#include "vtkPoints.h"
#include "vtkPolyData.h"
#include "vtkProperty.h"
#include "vtkSphereSource.h"
#include "vtkTransform.h"
#include "vtkTubeFilter.h"
#include "vtkUnsignedCharArray.h"

#include "vtkInformation.h"
#include "vtkInformationVector.h"

// #include <Engine/Control/RTRT.h>
// #include <Model/Groups/DynBVH.h>
// #include <Model/Groups/Group.h>
// #include <Model/Groups/Mesh.h>
// #include <Model/Primitives/TextureCoordinateCylinder.h>
// #include <Model/Primitives/TextureCoordinateSphere.h>
// #include <Model/Primitives/WaldTriangle.h>
// #include <Model/Materials/Flat.h>
// #include <Model/Materials/Lambertian.h>
// #include <Model/Materials/Transparent.h>
// #include <Model/Materials/Phong.h>
// #include <Model/Textures/TexCoordTexture.h>
// #include <Model/Textures/Constant.h>


// #include <Core/Geometry/Vector.h>
#include <vector>




//
//ospray
//
#if USE_OSPRAY
#include "ospray/ospray.h"
#include "ospray/common/OSPCommon.h"
#endif

#include <math.h>
#include <algorithm>

// class vtkOSPRayPolyDataMapper::Helper {
// public:
//   Helper() {}
//   ~Helper() {}

//   Manta::Material *material;
//   std::vector<Manta::Vector> texCoords;
// };

vtkStandardNewMacro(vtkOSPRayPolyDataMapper);


namespace vtkosp
{
  class Vec3
  {
  public:
    Vec3(float x,float y,float z) {
      vals[0] =x;
      vals[1] = y;
      vals[2] = z;
    }
    float operator[](unsigned int i)const{return vals[i];}
    float x() { return vals[0];}
    float y() { return vals[1];}
    float z() { return vals[2];}

    float vals[3];
  };
  class Vec4
  {
  public:
    Vec4(float x,float y,float z,float w) {
      vals[0] = x;
      vals[1] = y;
      vals[2] = z;
      vals[3] = w;
    }
    float operator[](unsigned int i)const{return vals[i];}
    float x() { return vals[0];}
    float y() { return vals[1];}
    float z() { return vals[2];}
    float w() { return vals[3];}

    float vals[4];
  };
  class Vec2
  {
  public:
    Vec2(float x,float y) {
      vals[0] =x;
      vals[1] = y;
    }
    float operator[](unsigned int i)const{return vals[i];}
    float x() { return vals[0];}
    float y() { return vals[1];}

    float vals[2];
  };
  class Mesh
  {
  public:
    size_t size() { return vertex_indices.size()/3; }
    std::vector<size_t> vertex_indices;
    std::vector<Vec3> vertices;
    std::vector<Vec3> vertexNormals;
    std::vector<Vec2> texCoords;
    std::vector<size_t> texture_indices;
    std::vector<size_t> normal_indices;
    std::vector<Vec4> colors;
  };
}


 int vtkOSPRayPolyDataMapper::timestep=0;//HACK!

//----------------------------------------------------------------------------
// Construct empty object.
vtkOSPRayPolyDataMapper::vtkOSPRayPolyDataMapper()
{
  #if 1
  //cerr << "MM(" << this << ") CREATE" << endl;
  this->InternalColorTexture = NULL;
  this->OSPRayManager = NULL;
  this->PointSize = 1.0;
  this->LineWidth = 1.0;
  this->Representation = VTK_SURFACE;
  // this->MyHelper = new Helper();
  #endif
}

//----------------------------------------------------------------------------
// Destructor (don't call ReleaseGraphicsResources() since it is virtual
vtkOSPRayPolyDataMapper::~vtkOSPRayPolyDataMapper()
{
  #if 1
  //cerr << "MM(" << this << ") DESTROY" << endl;
  if (this->InternalColorTexture)
    {
    this->InternalColorTexture->Delete();
    }

  if (this->OSPRayManager)
    {
    //cerr << "MM(" << this << ") DESTROY "
    //     << this->OSPRayManager << " "
    //     << this->OSPRayManager->GetReferenceCount() << endl;
    this->OSPRayManager->Delete();
    }

  // delete this->MyHelper;
  #endif
}

//----------------------------------------------------------------------------
// Release the graphics resources used by this mapper.  In this case, release
// the display list if any.
void vtkOSPRayPolyDataMapper::ReleaseGraphicsResources(vtkWindow *win)
{
  #if 1
  //cerr << "MM(" << this << ") RELEASE GRAPHICS RESOURCES" << endl;
  this->Superclass::ReleaseGraphicsResources( win );

  if (this->InternalColorTexture)
    {
    this->InternalColorTexture->Delete();
    }
  this->InternalColorTexture = NULL;
  #endif
}

//----------------------------------------------------------------------------
// Receives from Actor -> maps data to primitives
// called by Mapper->Render() (which is called by Actor->Render())
void vtkOSPRayPolyDataMapper::RenderPiece(vtkRenderer *ren, vtkActor *act)
{
  #if 1
  vtkOSPRayRenderer* mantaRenderer =
    vtkOSPRayRenderer::SafeDownCast(ren);
  if (!mantaRenderer)
    {
    return;
    }
  if (!this->OSPRayManager)
    {
    this->OSPRayManager = mantaRenderer->GetOSPRayManager();
    //cerr << "MM(" << this << ") REGISTER " << this->OSPRayManager << " "
    //     << this->OSPRayManager->GetReferenceCount() << endl;
    this->OSPRayManager->Register(this);
    }





  // write geometry, first ask the pipeline to update data
  vtkPolyData *input = this->GetInput();
  // std::cerr << "polydata input: " << input << std::endl;
  if (input == NULL)
    {
    vtkErrorMacro(<< "No input to vtkOSPRayPolyDataMapper!");
    return;
    }
  else
    {
    this->InvokeEvent( vtkCommand::StartEvent, NULL );

    // Static = 1:  this mapper does NOT need to propagate updates to other mappers
    // down the pipeline and therefore saves the time that would be otherwise taken
    if ( !this->Static )
      {
      this->Update();
      }

    this->InvokeEvent( vtkCommand::EndEvent, NULL );

    vtkIdType numPts = input->GetNumberOfPoints();
    if ( numPts == 0 )
      {
      vtkDebugMacro(<< "No points from the input to vtkOSPRayPolyDataMapper!");
      input = NULL;
      return;
      }
    }

  if ( this->LookupTable == NULL )
    {
    this->CreateDefaultLookupTable();
    }

  // TODO: vtkOpenGLPolyDataMapper uses OpenGL clip planes here

  // For vertex coloring, this sets this->Colors as side effect.
  // For texture map coloring, this sets ColorCoordinates
  // and ColorTextureMap as a side effect.
  this->MapScalars( act->GetProperty()->GetOpacity() );

  if ( this->ColorTextureMap )
    {
    if (!this->InternalColorTexture)
      {
      this->InternalColorTexture = vtkOSPRayTexture::New();
      this->InternalColorTexture->RepeatOff();
      }
    this->InternalColorTexture->SetInputData(this->ColorTextureMap);
    }

  // if something has changed, regenerate Manta primitives if required
  if ( this->GetMTime()  > this->BuildTime ||
       input->GetMTime() > this->BuildTime ||
       //act->GetMTime()   > this->BuildTime ||
       act->GetProperty()->GetMTime() > this->BuildTime ||
       act->GetMatrix()->GetMTime() > this->BuildTime
       )
    {
    //this->ReleaseGraphicsResources( ren->GetRenderWindow() );

    // If we are coloring by texture, then load the texture map.
    // Use Map as indicator, because texture hangs around.
    if (this->ColorTextureMap)
      {
      this->InternalColorTexture->Load(ren);
      }

    this->Draw(ren, act);
    this->BuildTime.Modified();
    }

  input = NULL;
  // TODO: deal with timer ??
  #endif
}

//----------------------------------------------------------------------------
void vtkOSPRayPolyDataMapper::DrawPolygons(vtkPolyData *polys,
                                          vtkPoints *ptarray,
                                          vtkosp::Mesh *mesh
                                          /*,
                                          Manta::Group *points,
                                          Manta::Group *lines*/)
{

  // Manta::Material *material = this->MyHelper->material;
  // std::vector<Manta::Vector> &texCoords = this->MyHelper->texCoords;

  int total_triangles = 0;
  vtkCellArray *cells = polys->GetPolys();
  vtkIdType npts = 0, *index = 0, cellNum = 0;

  switch (this->Representation) {
  case VTK_POINTS:
    {
    for ( cells->InitTraversal(); cells->GetNextCell(npts, index); cellNum++ )
      {
      double coord[3];
      // Manta::Vector noTC(0.0,0.0,0.0);
      for (int i = 0; i < npts; i++)
        {
        //TODO: Make option to scale pointsize by scalar
          /*
        ptarray->GetPoint(index[i], coord);
        Manta::TextureCoordinateSphere *sphere =
          new Manta::TextureCoordinateSphere
          (material,
           Manta::Vector(coord[0], coord[1], coord[2]),
           this->PointSize,
           (texCoords.size()?
            texCoords[(this->CellScalarColor?cellNum:index[i])] : noTC)
           );
        points->add(sphere);
        */
        }
      total_triangles ++;
      }
    //cerr << "polygons: # of triangles = " << total_triangles << endl;
    } //VTK_POINTS;
    break;
  case VTK_WIREFRAME:
    {
    double coord0[3];
    double coord1[3];
    // Manta::Vector noTC(0.0,0.0,0.0);
    // Manta::TextureCoordinateCylinder *segment;
    for ( cells->InitTraversal(); cells->GetNextCell(npts, index); cellNum++ )
      {
      ptarray->GetPoint(index[0], coord0);
      for (vtkIdType i = 1; i < npts; i++)
        {
        //TODO: Make option to scale linewidth by scalar
          /*
        ptarray->GetPoint(index[i], coord1);
        segment =
          new Manta::TextureCoordinateCylinder
          (material,
           Manta::Vector(coord0[0], coord0[1], coord0[2]),
           Manta::Vector(coord1[0], coord1[1], coord1[2]),
           this->LineWidth,
           (texCoords.size()?
            texCoords[(this->CellScalarColor?cellNum:index[i-1])] : noTC),
           (texCoords.size()?
            texCoords[(this->CellScalarColor?cellNum:index[i])] : noTC)
           );
        lines->add(segment);
        coord0[0] = coord1[0];
        coord0[1] = coord1[1];
        coord0[2] = coord1[2];
        */
        }
      ptarray->GetPoint(index[0], coord1);
      /*
      segment =
        new Manta::TextureCoordinateCylinder
        (material,
         Manta::Vector(coord0[0], coord0[1], coord0[2]),
         Manta::Vector(coord1[0], coord1[1], coord1[2]),
         this->LineWidth,
         (texCoords.size()?
          texCoords[(this->CellScalarColor?cellNum:index[npts-1])] : noTC),
         (texCoords.size()?
          texCoords[(this->CellScalarColor?cellNum:index[0])] : noTC)
         );
      lines->add(segment);
      */
      }
    } //VTK_WIREFRAME:
    break;
  case VTK_SURFACE:
  default:
    {
    // write polygons with on the fly triangulation, assuming polygons are simple and
    // can be triangulated into "fans"
    for ( cells->InitTraversal(); cells->GetNextCell(npts, index); cellNum++ )
      {
      int triangle[3];

      // the first triangle
      triangle[0] = index[0];
      triangle[1] = index[1];
      triangle[2] = index[2];
      mesh->vertex_indices.push_back(triangle[0]);
      mesh->vertex_indices.push_back(triangle[1]);
      mesh->vertex_indices.push_back(triangle[2]);
      // mesh->face_material.push_back(0);

      if ( !mesh->vertexNormals.empty() )
        {
        mesh->normal_indices.push_back(triangle[0]);
        mesh->normal_indices.push_back(triangle[1]);
        mesh->normal_indices.push_back(triangle[2]);
        }

      if ( !mesh->texCoords.empty() )
        {
        if (this->CellScalarColor)
          {
          mesh->texture_indices.push_back(cellNum);
          mesh->texture_indices.push_back(cellNum);
          mesh->texture_indices.push_back(cellNum);
          }
        else
          {
          mesh->texture_indices.push_back(triangle[0]);
          mesh->texture_indices.push_back(triangle[1]);
          mesh->texture_indices.push_back(triangle[2]);
          }
        }
      total_triangles ++;

      // the remaining triangles, of which
      // each introduces a triangle after extraction
      for ( int i = 3; i < npts; i ++ )
        {
        triangle[1] = triangle[2];
        triangle[2] = index[i];
        mesh->vertex_indices.push_back(triangle[0]);
        mesh->vertex_indices.push_back(triangle[1]);
        mesh->vertex_indices.push_back(triangle[2]);
        // mesh->face_material.push_back(0);

        if ( !mesh->vertexNormals.empty() )
          {
          mesh->normal_indices.push_back(triangle[0]);
          mesh->normal_indices.push_back(triangle[1]);
          mesh->normal_indices.push_back(triangle[2]);
          }

        if ( !mesh->texCoords.empty() )
          {
          if (this->CellScalarColor)
            {
            mesh->texture_indices.push_back(cellNum);
            mesh->texture_indices.push_back(cellNum);
            mesh->texture_indices.push_back(cellNum);
            }
          else
            {
            mesh->texture_indices.push_back(triangle[0]);
            mesh->texture_indices.push_back(triangle[1]);
            mesh->texture_indices.push_back(triangle[2]);
            }
          }
        total_triangles ++;
        }
      }
    //cerr << "polygons: # of triangles = " << total_triangles << endl;

    for ( int i = 0; i < total_triangles; i ++ )
      {
      // mesh->addTriangle( new Manta::WaldTriangle );
      }
#if 0
       ospray::vec3fa* vertices = (ospray::vec3fa*)embree::alignedMalloc(sizeof(ospray::vec3fa)*mesh->vertices.size());
  ospray::vec3i* triangles = (ospray::vec3i*)embree::alignedMalloc(sizeof(ospray::vec3i)*mesh->vertex_indices.size()/3);
  for(size_t i =0;i< mesh->vertex_indices.size();i++)
    vertices[i]== ospray::vec3fa(mesh->vertices[i].x(), mesh->vertices[i].y(),  mesh->vertices[i].z());
  for(size_t i = 0; i < mesh->vertex_indices.size()/3; i++)
    triangles[i] = embree::Vec3i(mesh->vertex_indices[i*3], mesh->vertex_indices[i*3+1], mesh->vertex_indices[i*3+2]);

     OSPMesh ospMesh = ospNewTriangleMesh();
  OSPData position = ospNewData(numPositions,OSP_vec3fa,
    &vertices[0]);
  ospSetData(ospMesh,"position",position);

  // OSPData normal = ospNewData(normals.size(),OSP_vec3fa,
    // &normals[0]);
  // ospSetData(ospMesh,"vertex.normal",normal);


  OSPData index = ospNewData(numTriangles,OSP_vec3i,
   &triangles[0]);
  ospSetData(ospMesh,"index",index);
  // ospCommit(o_current_material);
  // updateMaterial();
  // ospSetMaterial(oren->_data->ospMesh,o_current_material);
  // ospRelease(o_current_material);
  ospCommit(ospMesh);
#endif

  // oren->_data->ospModel = ospNewModel();
  // ospAddGeometry(oren->_data->ospModel,oren->_data->ospMesh);
  // ospCommit(oren->_data->ospModel);

    }//VTK_SURFACE
    break;
  }

}


//----------------------------------------------------------------------------
void vtkOSPRayPolyDataMapper::DrawTStrips(vtkPolyData *polys,
                                         vtkPoints *ptarray,
                                          vtkosp::Mesh *mesh)
                                         // Manta::Mesh *mesh,
                                         // Manta::Group *points,
                                         // Manta::Group *lines)
{
  // Manta::Material *material = this->MyHelper->material;
  // std::vector<Manta::Vector> &texCoords = this->MyHelper->texCoords;

  // total number of triangles
  int total_triangles = 0;

  vtkCellArray *cells = polys->GetStrips();
  vtkIdType npts = 0, *index = 0, cellNum = 0;;

  switch (this->Representation) {
  case VTK_POINTS:
    {
    for ( cells->InitTraversal(); cells->GetNextCell(npts, index); cellNum++ )
      {
      double coord[3];
      // Manta::Vector noTC(0.0,0.0,0.0);
      for (int i = 0; i < npts; i++)
        {
        //TODO: Make option to scale pointsize by scalar
        ptarray->GetPoint(index[i], coord);
        // Manta::TextureCoordinateSphere *sphere =
        //   new Manta::TextureCoordinateSphere
        //   (material,
        //    Manta::Vector(coord[0], coord[1], coord[2]),
        //    this->PointSize,
        //    (texCoords.size()?
        //     texCoords[(this->CellScalarColor?cellNum:index[i])] : noTC)
        //    );
        // points->add(sphere);
        }
      total_triangles ++;
      }
    //cerr << "polygons: # of triangles = " << total_triangles << endl;
    } //VTK_POINTS;
    break;
  case VTK_WIREFRAME:
    {
    double coord0[3];
    double coord1[3];
    double coord2[3];
    // Manta::Vector noTC(0.0,0.0,0.0);
    // Manta::TextureCoordinateCylinder *segment;
    for ( cells->InitTraversal(); cells->GetNextCell(npts, index); cellNum++ )
      {
      //TODO: Make option to scale linewidth by scalar
      // ptarray->GetPoint(index[0], coord0);
      // ptarray->GetPoint(index[1], coord1);
      // segment =
      //   new Manta::TextureCoordinateCylinder
      //   (material,
      //    Manta::Vector(coord0[0], coord0[1], coord0[2]),
      //    Manta::Vector(coord1[0], coord1[1], coord1[2]),
      //    this->LineWidth,
      //    (texCoords.size()?
      //     texCoords[(this->CellScalarColor?cellNum:index[0])] : noTC),
      //    (texCoords.size()?
      //     texCoords[(this->CellScalarColor?cellNum:index[1])] : noTC)
      //    );
      // lines->add(segment);
      for (vtkIdType i = 2; i < npts; i++)
        {
        ptarray->GetPoint(index[i], coord2);
        // segment =
        //   new Manta::TextureCoordinateCylinder
        //   (material,
        //    Manta::Vector(coord1[0], coord1[1], coord1[2]),
        //    Manta::Vector(coord2[0], coord2[1], coord2[2]),
        //    this->LineWidth,
        //    (texCoords.size()?
        //     texCoords[(this->CellScalarColor?cellNum:index[i-1])] : noTC),
        //    (texCoords.size()?
        //     texCoords[(this->CellScalarColor?cellNum:index[i])] : noTC)
        //    );
        // lines->add(segment);
        // segment =
        //   new Manta::TextureCoordinateCylinder
        //   (material,
        //    Manta::Vector(coord2[0], coord2[1], coord2[2]),
        //    Manta::Vector(coord0[0], coord0[1], coord0[2]),
        //    this->LineWidth,
        //    (texCoords.size()?
        //     texCoords[(this->CellScalarColor?cellNum:index[i])] : noTC),
        //    (texCoords.size()?
        //     texCoords[(this->CellScalarColor?cellNum:index[i-2])] : noTC)
        //    );
        // lines->add(segment);
        coord0[0] = coord1[0];
        coord0[1] = coord1[1];
        coord0[2] = coord1[2];
        coord1[0] = coord2[0];
        coord1[1] = coord2[1];
        coord1[2] = coord2[2];
        }
      }
    } //VTK_WIREFRAME:
    break;
  case VTK_SURFACE:
  default:
    {
    for ( cells->InitTraversal(); cells->GetNextCell(npts, index); cellNum++ )
      {
      // count of the i-th triangle in a strip
      int numtriangles2 = 0;

      int triangle[3];
      // the first triangle
      triangle[0] = index[0];
      triangle[1] = index[1];
      triangle[2] = index[2];
      mesh->vertex_indices.push_back( triangle[0] );
      mesh->vertex_indices.push_back( triangle[1] );
      mesh->vertex_indices.push_back( triangle[2] );
      // mesh->face_material.push_back(0);

      if ( !mesh->vertexNormals.empty() )
        {
        mesh->normal_indices.push_back( triangle[0] );
        mesh->normal_indices.push_back( triangle[1] );
        mesh->normal_indices.push_back( triangle[2] );
        }

      if ( !mesh->texCoords.empty() )
        {
        if ( this->CellScalarColor )
          {
          mesh->texture_indices.push_back(cellNum);
          mesh->texture_indices.push_back(cellNum);
          mesh->texture_indices.push_back(cellNum);
          }
        else
          {
          mesh->texture_indices.push_back( triangle[0] );
          mesh->texture_indices.push_back( triangle[1] );
          mesh->texture_indices.push_back( triangle[2] );
          }
        }

      total_triangles ++;
      numtriangles2 ++;

      // the rest of triangles
      for ( int i = 3; i < npts; i ++ )
        {
        int tmp[3];
        if ( numtriangles2 % 2 == 1 )
          {
          // an odd triangle
          tmp[0] = triangle[1];
          tmp[1] = triangle[2];
          tmp[2] = index[i];

          triangle[0] = tmp[0];
          triangle[1] = tmp[2];
          triangle[2] = tmp[1];
          }
        else
          {
          // an even triangle
          tmp[0] = triangle[1];
          tmp[1] = triangle[2];
          tmp[2] = index[i];

          triangle[0] = tmp[1];
          triangle[1] = tmp[0];
          triangle[2] = tmp[2];
          }

        mesh->vertex_indices.push_back( triangle[0] );
        mesh->vertex_indices.push_back( triangle[1] );
        mesh->vertex_indices.push_back( triangle[2] );
        // mesh->face_material.push_back(0);

        if ( !mesh->vertexNormals.empty() )
          {
          mesh->normal_indices.push_back( triangle[0] );
          mesh->normal_indices.push_back( triangle[1] );
          mesh->normal_indices.push_back( triangle[2] );
          }

        if ( !mesh->texCoords.empty() )
          {
          if ( this->CellScalarColor )
            {
            mesh->texture_indices.push_back(cellNum);
            mesh->texture_indices.push_back(cellNum);
            mesh->texture_indices.push_back(cellNum);
            }
          else
            {
            mesh->texture_indices.push_back( triangle[0] );
            mesh->texture_indices.push_back( triangle[1] );
            mesh->texture_indices.push_back( triangle[2] );
            }
          }

        total_triangles ++;
        numtriangles2 ++;
        }
      }

    //cerr << "strips: # of triangles = " << total_triangles << endl;

    // for ( int i = 0; i < total_triangles; i++ )
      // {
      // mesh->addTriangle( new Manta::WaldTriangle );
      // }


    }
  }
}


//----------------------------------------------------------------------------
// Draw method for Manta.
void vtkOSPRayPolyDataMapper::Draw(vtkRenderer *renderer, vtkActor *actor)
{
  #if 1
  // printf("ospPolyDataMapper::Draw\n");
  vtkOSPRayActor *mantaActor =
    vtkOSPRayActor::SafeDownCast(actor);
  if (!mantaActor)
    {
    return;
    }
  vtkOSPRayProperty *mantaProperty =
    vtkOSPRayProperty::SafeDownCast( mantaActor->GetProperty() );
  if (!mantaProperty)
    {
    return;
    }
  vtkPolyData *input = this->GetInput();


  vtkInformation* inputInfo = this->GetInput()->GetInformation();
  // // vtkInformation* outputInfo = outputVector->GetInformationObject(0);

// std::cerr << "ospPDM Actor: " << actor << std::endl; 
  if (inputInfo && inputInfo->Has(vtkDataObject::DATA_TIME_STEP())
    )
    {
    double time = inputInfo->Get(vtkDataObject::DATA_TIME_STEP());
    // cerr << "MA time: " << time << std::endl;
    timestep = time;
    if (mantaActor->cache[time] != NULL)
    {
      std::cerr << "using cache at time " << time << "\n";
      // this->OSPRayModel = cache[time];

  mantaActor->OSPRayModel = mantaActor->cache[time];     
  return;
      
      // this->MeshMTime.Modified();
      // UpdateObjects(ren);
    }
    // return;

  }
  else if (!inputInfo)
  {
    cerr << "MA time: didn't have info\n";
  }
  else
  {
    // cerr << "MA time: didn't have time\n";
    if (mantaActor->cache[timestep] != NULL)
    {
      // std::cerr << "using nontime actor at timestep: " << timestep << std::endl;
      // mantaActor->OSPRayModel = mantaActor->cache[timestep];   
      // return;
    }
    // if (mantaActor->OSPRayModel)
    // {
    //   //     if (!(
    //   // this->GetInput()->GetMTime() > mantaActor->MeshMTime ||
    //   // mantaActor->GetMTime() > mantaActor->MeshMTime ||
    //   // mantaActor->GetProperty()->GetMTime() > mantaActor->MeshMTime ||
    //   // mantaActor->GetMTime() > mantaActor->MeshMTime))
    //     return;
    // }
  }
    mantaActor->MeshMTime.Modified();




  // Compute we need to for color
  this->Representation = mantaProperty->GetRepresentation();

  this->CellScalarColor = false;
  if (( this->ScalarMode == VTK_SCALAR_MODE_USE_CELL_DATA ||
        this->ScalarMode == VTK_SCALAR_MODE_USE_CELL_FIELD_DATA ||
        this->ScalarMode == VTK_SCALAR_MODE_USE_FIELD_DATA ||
        !input->GetPointData()->GetScalars()
        )
      && this->ScalarMode != VTK_SCALAR_MODE_USE_POINT_FIELD_DATA
      )
    {
    this->CellScalarColor = true;
    }

  // this->MyHelper->material = NULL;
  // this->MyHelper->texCoords.clear();
  // Manta::Material *&material = this->MyHelper->material;
  OSPMaterial ospMaterial = NULL;
  vtkosp::Mesh* mesh = new vtkosp::Mesh();
  // std::vector<Manta::Vector> &texCoords = this->MyHelper->texCoords;

      osp::Material* osmat = mantaProperty->GetOSPRayMaterial();
      if (!osmat)
      {
        mantaProperty->CreateMantaProperty();
      ospMaterial = ((OSPMaterial)mantaProperty->GetOSPRayMaterial());
      }
      else
        ospMaterial = ((OSPMaterial)osmat);

  if ( !this->ScalarVisibility || (!this->Colors && !this->ColorCoordinates))
    {
    cerr << "poly colors: Solid color from actor's property" << endl;

      /*
    material = mantaProperty->GetMantaMaterial();
    if(!material)
      {
      mantaProperty->CreateMantaProperty();
      material = mantaProperty->GetMantaMaterial();

      //TODO: the leaks
      mantaProperty->SetMantaMaterial(NULL);
      mantaProperty->SetSpecularTexture(NULL);
      mantaProperty->SetDiffuseTexture(NULL);
      }
      */
    }
  else if (this->Colors)
    {
    cerr << "poly colors: Color scalar values directly (interpolation in color space)" << endl;
    // Manta::Texture<Manta::Color> *texture = new Manta::TexCoordTexture();
    if ( mantaProperty->GetInterpolation() == VTK_FLAT )
      {
      //cerr << "Flat" << endl;
      // material = new Manta::Flat(texture);
      }
    else
      {
      if ( mantaProperty->GetOpacity() < 1.0 )
        {
        //cerr << "Translucent" << endl;
        // material = new Manta::Transparent(texture,
                                          // mantaProperty->GetOpacity());
        }
      else
        {
        if ( mantaProperty->GetSpecular() == 0 )
          {
          //cerr << "non specular" << endl;
          // material = new Manta::Lambertian(texture);
          }
        else
          {
          //cerr << "phong" << endl;
          // double *specular = mantaProperty->GetSpecularColor();
          // Manta::Texture<Manta::Color> *specularTexture =
          //   new Manta::Constant<Manta::Color>
          //   (Manta::Color(Manta::RGBColor(specular[0],
          //                                 specular[1],
          //                                 specular[2])));
          // material =
          //   new Manta::Phong
          //   (texture,
          //    specularTexture,
          //    static_cast<int> (mantaProperty->GetSpecularPower()),
          //    NULL);
          }
        }
      }

    //this table has one RGBA for every point (or cell) in object
    for ( int i = 0; i < this->Colors->GetNumberOfTuples(); i ++ )
      {
        unsigned char *color = this->Colors->GetPointer(4*i);
      // texCoords.push_back
        // (Manta::Vector(color[0]/255.0, color[1]/255.0, color[2]/255.0) );
      // mesh->texCoords.push_back(vtkosp::Vec3(color[0]/255.0, color[1]/255.0, color[2]/255.0));
        mesh->colors.push_back(vtkosp::Vec4(color[0]/255.0,color[1]/255.0,color[2]/255.0,1));
      }
      // printf("texture coords: using rgba every point\n");

    }
  else if (this->ColorCoordinates)
    {
      printf("poly colors: texture coords: using color coordinates for a texture\n");
    //cerr << "interpolate in data space, then color map each pixel" << endl;
    // Manta::Texture<Manta::Color> *texture =
    //   this->InternalColorTexture->GetMantaTexture();
      osp::Texture2D* texture = this->InternalColorTexture->GetOSPRayTexture();
      // PRINT((OSPTexture2D)texture);
      Assert(texture);
      ospSetParam(ospMaterial,"map_Kd",((OSPTexture2D)(texture)));
      ospCommit(ospMaterial);

    // material = new Manta::Lambertian(texture);

    // //this table is a color transfer function with colors that cover the scalar range
    // //I think
    for (int i = 0; i < this->ColorCoordinates->GetNumberOfTuples(); i++)
      {
      double *tcoord = this->ColorCoordinates->GetTuple(i);
    //   texCoords.push_back( Manta::Vector(tcoord[0], 0, 0) );
      mesh->texCoords.push_back(vtkosp::Vec2(tcoord[0],0));
        // mesh->colors.push_back(vtkosp::Vec4(color[0]/255.0,color[1]/255.0,color[2]/255.0,1));
      // printf("texCoord: %f %f\n", tcoord[0], 0);
      }

      // printf("NEED TO IMPLEMENT COLORCOORDINATES\n");
    }
  else if (input->GetPointData()->GetTCoords() && actor->GetTexture() )
    {
      printf("poly colors: texture coords: using texture\n");
      #if 1
    //cerr << "color using actor's texture" << endl;
    vtkOSPRayTexture *osprayTexture =
      vtkOSPRayTexture::SafeDownCast(actor->GetTexture());
    if (osprayTexture)
      {
    //   Manta::Texture<Manta::Color> *texture =
    //     mantaTexture->GetMantaTexture();
    //   material = new Manta::Lambertian(texture);
            ospSetParam(ospMaterial,"map_Kd",((OSPTexture2D)(osprayTexture->GetOSPRayTexture())));
            ospCommit(ospMaterial);
      }

    // // convert texture coordinates to manta format
    vtkDataArray *tcoords = input->GetPointData()->GetTCoords();
    for (int i = 0; i < tcoords->GetNumberOfTuples(); i++)
      {
      double *tcoord = tcoords->GetTuple(i);
      mesh->texCoords.push_back(vtkosp::Vec2(tcoord[0],tcoord[1]));
    //     ( Manta::Vector(tcoord[0], tcoord[1], tcoord[2]) );
      }
#endif
      // printf("NEED TO IMPLEMENT TEXTURES\n");
    }

  // transform point coordinates according to actor's transformation matrix
  //TODO: Use manta instancing to transform instead of doing it brute force here
  //to reduce number of copies
  vtkTransform *transform = vtkTransform::New();
  transform->SetMatrix( actor->GetMatrix() );
  vtkPoints *points = vtkPoints::New();
  transform->TransformPoints( input->GetPoints(), points );

  // obtain the OpenGL-based point size and line width
  // that are specified through vtkProperty
  this->PointSize = mantaProperty->GetPointSize();
  this->LineWidth = mantaProperty->GetLineWidth();
  if (this->PointSize < 1.0)
    {
    this->PointSize = 1.0;
    }
  if (this->LineWidth < 1.0)
    {
    this->LineWidth = 1.0;
    }
  this->PointSize = sqrt(this->PointSize) * 0.010;
  this->LineWidth = sqrt(this->LineWidth) * 0.005;

  //containers for the manta primitives we are going to produce
  // Manta::Group *sphereGroup = new Manta::Group();
  // Manta::Group *tubeGroup = new Manta::Group();
  // Manta::Mesh *mesh = new Manta::Mesh();

  //convert VTK_VERTEX cells to manta spheres
  if ( input->GetNumberOfVerts() > 0 )
    {
    vtkCellArray *ca = input->GetVerts();
    ca->InitTraversal();
    vtkIdType npts;
    vtkIdType *pts;
    vtkPoints *ptarray = points;
    double coord[3];
    vtkIdType cell;
    // Manta::Vector noTC(0.0,0.0,0.0);
    while ((cell = ca->GetNextCell(npts, pts)))
      {
      //TODO: Make option to scale pointsize by scalar

      // ptarray->GetPoint(pts[0], coord);
      // Manta::TextureCoordinateSphere *sphere =
      //   new Manta::TextureCoordinateSphere
      //   (material,
      //    Manta::Vector(coord[0], coord[1], coord[2]),
      //    this->PointSize,
      //    (texCoords.size()?
      //     texCoords[(this->CellScalarColor?cell:pts[0])] : noTC)
      //    );
      // sphereGroup->add(sphere);
      }
    }

  //convert VTK_LINE type cells to manta cylinders
  if ( input->GetNumberOfLines() > 0 )
    {
    // vtkCellArray *ca = input->GetLines();
    // ca->InitTraversal();
    // vtkIdType npts;
    // vtkIdType *pts;
    // vtkPoints *ptarray = points;
    // double coord0[3];
    // double coord1[3];
    // vtkIdType cell;
    // Manta::Vector noTC(0.0,0.0,0.0);
    // while ((cell = ca->GetNextCell(npts, pts)))
    //   {
    //   ptarray->GetPoint(pts[0], coord0);
    //   for (vtkIdType i = 1; i < npts; i++)
    //     {
    //     //TODO: Make option to scale linewidth by scalar
    //     ptarray->GetPoint(pts[i], coord1);
    //     Manta::TextureCoordinateCylinder *segment =
    //       new Manta::TextureCoordinateCylinder
    //       (material,
    //        Manta::Vector(coord0[0], coord0[1], coord0[2]),
    //        Manta::Vector(coord1[0], coord1[1], coord1[2]),
    //        this->LineWidth,
    //        (texCoords.size()?
    //         texCoords[(this->CellScalarColor?cell:pts[0])] : noTC),
    //        (texCoords.size()?
    //         texCoords[(this->CellScalarColor?cell:pts[1])] : noTC)
    //        );
    //     tubeGroup->add(segment);
    //     coord0[0] = coord1[0];
    //     coord0[1] = coord1[1];
    //     coord0[2] = coord1[2];
    //     }
    //   }
    }

  //convert coordinates to manta format
  //TODO: eliminate the copy
  for ( int i = 0; i < points->GetNumberOfPoints(); i++ )
    {
    double *pos = points->GetPoint(i);
    bool wasNan=false;
    int fixIndex = i-1;
    do {
      wasNan = false;
      for(int j=0;j<3;j++)
      {
        if (isnan(pos[j]))
        {
          wasNan=true;
        }
      }
      if (wasNan && fixIndex >= 0)
      pos = points->GetPoint(fixIndex--);
  } while(wasNan == true && fixIndex >= 0);
    mesh->vertices.push_back( vtkosp::Vec3(pos[0], pos[1], pos[2]) );
    }

  // Do flat shading by not supplying vertex normals to manta
  if ( mantaProperty->GetInterpolation() != VTK_FLAT )
    {
    vtkPointData *pointData = input->GetPointData();
    if ( pointData->GetNormals() )
      {
      vtkDataArray *normals = vtkFloatArray::New();
      normals->SetNumberOfComponents(3);
      transform->TransformNormals( pointData->GetNormals(), normals );
      for ( int i = 0; i < normals->GetNumberOfTuples(); i ++ )
        {
        double *normal = normals->GetTuple(i);
        mesh->vertexNormals.push_back( vtkosp::Vec3(normal[0], normal[1], normal[2]) );
        }
      normals->Delete();
      }
    }

  // mesh->materials.push_back(material);
  // mesh->texCoords = texCoords;
  // texCoords.clear();

  // convert polygons to manta format
  if ( input->GetNumberOfPolys() > 0 )
    {
    this->DrawPolygons(input, points, mesh/*, sphereGroup, tubeGroup*/);
    }

  // convert triangle strips to manta format
  if ( input->GetNumberOfStrips() > 0 )
    {
    this->DrawTStrips(input, points, mesh/*, sphereGroup, tubeGroup*/);
    }

  //delete transformed point coordinates
  transform->Delete();
  points->Delete();

  //put everything together into one group
  // Manta::Group *group = new Manta::Group();
  // if(sphereGroup->size())
  //   {
  //   //cerr << "MM(" << this << ")   points " << sphereGroup->size() << endl;
  //   // group->add(sphereGroup);
  //   }
  // else
  //   {
  //   // delete sphereGroup;
  //   }
  // // if(tubeGroup->size())
  //   {
  //   //cerr << "MM(" << this << ")   lines " << tubeGroup->size() << endl;
  //   // group->add(tubeGroup);
  //   }
  // else
  //   {
  //   // delete tubeGroup;
  //   }
  if (mesh->size())
    {
    //cerr << "MM(" << this << ")   polygons " << mesh->size() << endl;
      // group->add(mesh);

    //
    // ospray
    //

      size_t numNormals = mesh->vertexNormals.size();
  size_t numTexCoords = mesh->texCoords.size();
      size_t numPositions = mesh->vertices.size();
      size_t numTriangles = mesh->vertex_indices.size()/3;
  #if USE_OSPRAY

  ospray::vec3fa* vertices = (ospray::vec3fa*)embree::alignedMalloc(sizeof(ospray::vec3fa)*numPositions);
  ospray::vec3i* triangles = (ospray::vec3i*)embree::alignedMalloc(sizeof(ospray::vec3i)*numTriangles);

    for(size_t i = 0; i < numPositions; i++)
  {
    vertices[i] = ospray::vec3fa(mesh->vertices[i].x(), mesh->vertices[i].y(),  mesh->vertices[i].z());
    // vertices[i] = ospray::vec3fa(float(i)*.01,float(i)*.01,float(i)*.01);
    // printf("vert: %f %f %f\n",mesh->vertices[i].x(), mesh->vertices[i].y(), mesh->vertices[i].z());
  }
  // normals.resize(numNormals);
  // for(size_t i = 0; i < numNormals; i++)
    // normals[i] = ospray::vec3fa(mesh->vertexNormals[i].x(), mesh->vertexNormals[i].y(), mesh->vertexNormals[i].z());


      // embree::Vec3i* vertex_indices = (embree::Vec3i*)alignedMalloc(sizeof(embree::Vec3i)*numTriangles);
  // triangles.resize(numTriangles);
  for(size_t i = 0, mi = 0; i < numTriangles; i++, mi+=3)
  {
    triangles[i] = embree::Vec3i(mesh->vertex_indices[mi+0], mesh->vertex_indices[mi+1], mesh->vertex_indices[mi+2]);
    // triangles[i] = embree::Vec3i(0,1,2);
    // printf("indices: %d %d %d\n",mesh->vertex_indices[mi+0], mesh->vertex_indices[mi+1], mesh->vertex_indices[mi+2]);
  }


  OSPGeometry ospMesh = ospNewTriangleMesh();
  OSPData position = ospNewData(numPositions,OSP_FLOAT3A,
    &vertices[0]);
  ospSetData(ospMesh,"position",position);

if (!mesh->normal_indices.empty())
{
  // OSPData normal = ospNewData(normals.size(),OSP_vec3fa,
    // &normals[0]);
  // ospSetData(ospMesh,"vertex.normal",normal);
}


  OSPData index = ospNewData(numTriangles,OSP_INT3,
   &triangles[0]);
  ospSetData(ospMesh,"index",index);

  OSPRenderer renderer = ((OSPRenderer)this->OSPRayManager->OSPRayRenderer);

  if (!mesh->texCoords.empty()) {
    OSPData texcoord = ospNewData(mesh->texCoords.size(), OSP_FLOAT2,
      &mesh->texCoords[0]);
    assert(mesh->texCoords.size() > 0);
    ospSetData(ospMesh,"vertex.texcoord",texcoord);
  }
  if (!mesh->colors.empty())
  {
    std::cerr << "using color coordinates\n";
    // note: to share data use OSP_DATA_SHARED_BUFFER
    OSPData colors =ospNewData(mesh->colors.size(), OSP_FLOAT4,
      &mesh->colors[0]); 
    ospSetData(ospMesh,"vertex.color",colors);
  }


if (!ospMaterial)
{
  // printf("no material specification used, using default\n");
      mantaProperty->CreateMantaProperty();
      ospMaterial = ((OSPMaterial)mantaProperty->GetOSPRayMaterial());
}
// PRINT(ospMaterial);

  ospSetMaterial(ospMesh,ospMaterial);
  ospCommit(ospMesh);

  // static OSPModel ospModel;
  // OSPModel ospModel = ospNewModel();
  mantaActor->OSPRayModel = ospNewModel();
  ospAddGeometry(mantaActor->OSPRayModel,ospMesh);
  ospCommit(mantaActor->OSPRayModel);
    if (inputInfo && inputInfo->Has(vtkDataObject::DATA_TIME_STEP())
    )
    {
    double time = inputInfo->Get(vtkDataObject::DATA_TIME_STEP());
    mantaActor->cache[time] = mantaActor->OSPRayModel;
  }
  else
  {
    mantaActor->cache[timestep] = mantaActor->OSPRayModel;
    std::cerr << "added nontime actor at timestep" << timestep << "\n";
  }
  // mantaActor->ospMesh = ospMesh;
  // mantaActor->OSPRayModel = ((osp::Model*)ospModel);
  printf("added osp mesh num triangles: %d\n", numTriangles);
  #endif


    }
  else
    {
    delete mesh;
    }

  // if (group->size())
  //   {
  //   mantaActor->SetGroup(group);
  //   }
  // else
  //   {
  //   mantaActor->SetGroup(NULL);
  //   delete group;
  //   //cerr << "NOTHING TO SEE" << endl;
  //   }
    #endif
}
