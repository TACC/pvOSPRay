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

// .NAME vtkOSPRayTexture -
// .SECTION Description
//

#ifndef __vtkOSPRayTexture_h
#define __vtkOSPRayTexture_h

#include "vtkOSPRayModule.h"
#include "vtkTexture.h"
// #include "Interface/Texture.h"

//BTX
#include "vtkWeakPointer.h" // needed for vtkWeakPointer.
//ETX

//BTX
namespace OSPRay {
//class ImageTexture<Color>;
}
//ETX
namespace osp
{
  // class OSPTexture2D;
  class Texture2D;
}

class vtkRenderWindow;
class vtkOSPRayManager;

class VTKOSPRAY_EXPORT vtkOSPRayTexture : public vtkTexture
{
public:
  static vtkOSPRayTexture *New();
  vtkTypeMacro(vtkOSPRayTexture,vtkTexture);
  virtual void PrintSelf(ostream& os, vtkIndent indent);

  // Description:
  // Implement base class method.
  void Load(vtkRenderer *ren);

  // Description:
  // Release any graphics resources that are being consumed by this texture.
  // The parameter window could be used to determine which graphic
  // resources to release. Using the same texture object in multiple
  // render windows is NOT currently supported.
  void ReleaseGraphicsResources(vtkWindow *);
  //BTX
  // vtkSetMacro(OSPRayTexture, OSPRay::Texture<OSPRay::Color>*);
  // vtkGetMacro(OSPRayTexture, OSPRay::Texture<OSPRay::Color>*);
  vtkSetMacro(OSPRayTexture, osp::Texture2D*);
  vtkGetMacro(OSPRayTexture, osp::Texture2D*);
  //ETX

  //BTX
protected:
  vtkOSPRayTexture();
  ~vtkOSPRayTexture();

  vtkTimeStamp   LoadTime;

private:
  vtkOSPRayTexture(const vtkOSPRayTexture&);  // Not implemented.
  void operator=(const vtkOSPRayTexture&);  // Not implemented.

  void DeleteOSPRayTexture();

  // OSPRay::Texture<OSPRay::Color> *OSPRayTexture;
  osp::Texture2D* OSPRayTexture;
  //ETX

  vtkOSPRayManager *OSPRayManager;
};

#endif
