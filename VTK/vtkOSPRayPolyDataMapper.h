/*=========================================================================

  Program:   Visualization Toolkit
  Module:    vtkOSPRayPolyDataMapper.h

  Copyright (c) Ken Martin, Will Schroeder, Bill Lorensen
  All rights reserved.
  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
/*=========================================================================

  Program:   VTK/ParaView Los Alamos National Laboratory Modules (PVLANL)
  Module:    vtkOSPRayPolyDataMapper.h

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
// .NAME vtkOSPRayPolyDataMapper -
// .SECTION Description
//
// .NAME vtkOSPRayPolyDataMapper - a PolyDataMapper for the Manta library
// .SECTION Description
// vtkOSPRayPolyDataMapper is a subclass of vtkPolyDataMapper.
// vtkOSPRayPolyDataMapper is a geometric PolyDataMapper for the Manta
// Raytracer library.

#ifndef __vtkOSPRayPolyDataMapper_h
#define __vtkOSPRayPolyDataMapper_h

#include "vtkOSPRayModule.h"
#include "vtkPolyDataMapper.h"
#include "vtkOSPRayTexture.h"

#include <map>

class vtkSphereSource;
class vtkGlyph3D;
class vtkTubeFilter;
class vtkAppendPolyData;

//BTX
namespace Manta {
class Mesh;
class Group;
}
//ETX
class vtkCellArray;
class vtkPoints;
class vtkProperty;
class vtkRenderWindow;
class vtkOSPRayRenderer;
class vtkOSPRayManager;

namespace vtkosp
{
  class Mesh;
}
namespace osp
{
  class Model;
}

class VTKOSPRAY_EXPORT vtkOSPRayPolyDataMapper : public vtkPolyDataMapper
{
public:
  static vtkOSPRayPolyDataMapper *New();
  vtkTypeMacro(vtkOSPRayPolyDataMapper,vtkPolyDataMapper);
  //virtual void PrintSelf(ostream& os, vtkIndent indent);

  // Description:
  // Implement superclass render method.
  virtual void RenderPiece(vtkRenderer *ren, vtkActor *a);

  // Description:
  // Release any graphics resources that are being consumed by this mapper.
  // The parameter window could be used to determine which graphic
  // resources to release.
  void ReleaseGraphicsResources(vtkWindow *);

  // Description:
  // Draw method for Manta.
  virtual void Draw(vtkRenderer *ren, vtkActor *a);

protected:
  vtkOSPRayPolyDataMapper();
  ~vtkOSPRayPolyDataMapper();

  //BTX
  void DrawPolygons(vtkPolyData *, vtkPoints *,
                    vtkosp::Mesh * /*, Manta::Group *, Manta::Group * */);
  void DrawTStrips(vtkPolyData *, vtkPoints *,
                    vtkosp::Mesh* /*, Manta::Mesh *, Manta::Group *, Manta::Group * */);
  //ETX

private:
  vtkOSPRayPolyDataMapper(const vtkOSPRayPolyDataMapper&); // Not implemented.
  void operator=(const vtkOSPRayPolyDataMapper&); // Not implemented.

  vtkOSPRayManager *OSPRayManager;

  vtkOSPRayTexture* InternalColorTexture;
  int Representation;
  double PointSize;
  double LineWidth;
  std::map<int, osp::Model*> cache;
  bool CellScalarColor;
  static int timestep;
//BTX
  class Helper;
  Helper *MyHelper;
//ETX
};

#endif
