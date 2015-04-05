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
#include "vtkOSPRayRenderer.h"
#include "vtkOSPRayTexture.h"

#include "vtkHomogeneousTransform.h"
#include "vtkImageData.h"
#include "vtkLookupTable.h"
#include "vtkObjectFactory.h"
#include "vtkPointData.h"
#include "vtkTransform.h"

// #include <Engine/Control/RTRT.h>
// #include <Image/SimpleImage.h>
// #include <Model/Textures/ImageTexture.h>



#include <math.h>

//==============================================================================
//This is a helper that exists just to hold on to OSPRay side resources
//long enough for the OSPRay thread to destroy them, whenever that
//threads gets around to it (in a callback)
// class vtkOSPRayTextureThreadCache
// {
// public:
//   vtkOSPRayTextureThreadCache(OSPRay::Texture<OSPRay::Color> *mt)
//     : OSPRayTexture(mt)
//   {
//   }

//   void FreeOSPRayResources()
//   {
//     delete OSPRayTexture;
//     //WARNING: this class must never be instantiated on the stack.
//     //Therefore, it has private unimplemented copy/contructors.
//     delete this;
//   }

// private:
//   vtkOSPRayTextureThreadCache(const vtkOSPRayTextureThreadCache&);
//   // Not implemented.
//   void operator=(const vtkOSPRayTextureThreadCache&);
//   // Not implemented.

//   OSPRay::Texture<OSPRay::Color> *OSPRayTexture;
// };

//==============================================================================

vtkStandardNewMacro(vtkOSPRayTexture);

//----------------------------------------------------------------------------
// Initializes an instance, generates a unique index.
vtkOSPRayTexture::vtkOSPRayTexture()
: OSPRayTexture(NULL)
{
  //cerr << "MT( " << this << ") CREATE " << endl;
  this->OSPRayManager = NULL;
  // this->OSPRayTexture = NULL;
  this->OSPRayTexture = NULL;
}

//----------------------------------------------------------------------------
vtkOSPRayTexture::~vtkOSPRayTexture()
{
  //cerr << "MT( " << this << ") DESTROY " << endl;
  if (this->OSPRayManager)
  {
    this->DeleteOSPRayTexture();

    //cerr << "MT(" << this << ") DESTROY " << this->OSPRayManager << " "
    //     << this->OSPRayManager->GetReferenceCount() << endl;
    this->OSPRayManager->Delete();
  }
}

//-----------------------------------------------------------------------------
void vtkOSPRayTexture::DeleteOSPRayTexture()
{
  if (!this->OSPRayTexture)
  {
    return;
  }

  //save off the pointers for the OSPRay thread
  // vtkOSPRayTextureThreadCache *R =
  //   new vtkOSPRayTextureThreadCache(this->OSPRayTexture);

  // //make no further references to them in this thread
  this->OSPRayTexture = NULL;

  //ask the OSPRay thread to free them when it can
  // this->OSPRayManager->GetOSPRayEngine()->
    // addTransaction("cleanup texture",
                   // OSPRay::Callback::create
                   // (R, &vtkOSPRayTextureThreadCache::FreeOSPRayResources));
}

//-----------------------------------------------------------------------------
// Release the graphics resources used by this texture.
void vtkOSPRayTexture::ReleaseGraphicsResources(vtkWindow *win)
{
  //cerr << "MT( " << this << ") RELEASE GRAPHICS RESOURCES " << endl;
  this->Superclass::ReleaseGraphicsResources( win );
  if (!this->OSPRayManager)
  {
    return;
  }

  this->DeleteOSPRayTexture();
}

//----------------------------------------------------------------------------
void vtkOSPRayTexture::Load(vtkRenderer *ren)
{
  // return;
  #if 1
  // printf("loading osp texture\n");

  //cerr << "MT(" << this << ") LOAD " << endl;
  vtkImageData *input = this->GetInput();

  vtkOSPRayRenderer* renderer =
  vtkOSPRayRenderer::SafeDownCast(ren);
  if (!renderer)
  {
    return;
  }
  if (!this->OSPRayManager)
  {
    this->OSPRayManager = renderer->GetOSPRayManager();
    //cerr << "MT(" << this << ") REGISTER " << this->OSPRayManager << " "
    //     << this->OSPRayManager->GetReferenceCount() << endl;
    this->OSPRayManager->Register(this);
  }

  if (this->GetMTime() > this->LoadTime.GetMTime() ||
    input->GetMTime()> this->LoadTime.GetMTime() ||
      (this->GetLookupTable() && this->GetLookupTable()->GetMTime() > this->LoadTime.GetMTime()) //||
      /*renWin != this->RenderWindow.GetPointer() || renWin->GetContextCreationTime() > this->LoadTime*/)
      {
        int bytesPerPixel=4;
        int size[3];
        vtkDataArray *scalars;
        unsigned char *dataPtr;
        int xsize, ysize;

    // Get the scalars the user choose to color with.
        scalars = this->GetInputArrayToProcess(0, input);
    // make sure scalars are non null
        if (!scalars)
        {
          vtkErrorMacro(<< "No scalar values found for texture input!");
          return;
        }

    // get some info
        input->GetDimensions(size);

        if (input->GetNumberOfCells() == scalars->GetNumberOfTuples())
        {
      // we are using cell scalars. Adjust image size for cells.
          for (int kk = 0; kk < 3; kk++)
          {
            if (size[kk] > 1)
            {
              size[kk]--;
            }
          }
        }

        bytesPerPixel = scalars->GetNumberOfComponents();

    // make sure using unsigned char data of color scalars type
        if (this->MapColorScalarsThroughLookupTable ||
          scalars->GetDataType() != VTK_UNSIGNED_CHAR)
        {
          printf("color mapping\n");
          dataPtr = this->MapScalarsToColors(scalars);
          bytesPerPixel = 4;
        }
        else
        {
          printf("color chararray\n");
          dataPtr = static_cast<vtkUnsignedCharArray *> (scalars)->GetPointer(0);
        }

    // we only support 2d texture maps right now
    // so one of the three sizes must be 1, but it
    // could be any of them, so lets find it
        if (size[0] == 1)
        {
          xsize = size[1];
          ysize = size[2];
        }
        else
        {
          xsize = size[0];
          if (size[1] == 1)
          {
            ysize = size[2];
          }
          else
          {
            ysize = size[1];
            if (size[2] != 1)
            {
              vtkErrorMacro(<< "3D texture maps currently are not supported!");
              return;
            }
          }
        }
      printf("creating new texture size %d %d\n", xsize, ysize);
      for(int i =0; i < xsize*bytesPerPixel; i++)
      {
        // std::cerr << int(dataPtr[i]) << " ";
      }
      printf("\n\ncolor values: \n");

    // Create OSPRay Image from input
    // OSPRay::Image *image =
      // new OSPRay::SimpleImage<OSPRay::RGB8Pixel> (false, xsize, ysize);
    // OSPRay::RGB8Pixel *pixels =
      // dynamic_cast<OSPRay::SimpleImage<OSPRay::RGB8Pixel> const*>(image)->
      // getRawPixels(0);
    // OSPRay::RGB8Pixel pixel;
        struct OColor { unsigned char r,g,b; };
        OColor* pixels = new OColor[xsize*ysize];
        for (int v = 0; v < ysize; v++)
        {
          for (int u = 0; u < xsize; u++)
          {
            unsigned char *color = &dataPtr[(v*xsize+u)*bytesPerPixel];
            OColor pixel;
            pixel.r = color[0];
            pixel.g = color[1];
            pixel.b = color[2];
            pixels[v*xsize + u] = pixel;
            // printf("[%d %d %d] ", pixel.r, pixel.g, pixel.b);
          }
          // printf("\n");
        }

    // create OSPRay texture from the image
    // OSPRay::ImageTexture<OSPRay::Color> *imgtexture =
      // new OSPRay::ImageTexture<OSPRay::Color>(image, false);
    // imgtexture->setInterpolationMethod(1);

    // this->DeleteOSPRayTexture();
    // this->OSPRayTexture = imgtexture;

    // OSPRay image is copied and converted to internal buffer in the texture,
    // delete the image
    // delete image;

        OSPDataType type = OSP_VOID_PTR;

    // if (msgTex->depth == 1) {
    //   if( msgTex->channels == 3 ) type = OSP_UCHAR3;
    //   if( msgTex->channels == 4 ) type = OSP_UCHAR4;
    // } else if (msgTex->depth == 4) {
    //   if( msgTex->channels == 3 ) type = OSP_FLOAT3;
    //   if( msgTex->channels == 4 ) type = OSP_FLOAT3A;
    // }
        if (bytesPerPixel == 4)
        {
          // printf("bytesperpixel 4\n");
          // type = OSP_FLOAT3;
          type = OSP_UCHAR3;
        }
        else
        {
          printf("error! bytesperpixel !=4\n");
          Assert(0);
          type = OSP_UCHAR3;
        }

        // printf("creating osp texture %d %d\n", xsize, ysize);
        this->OSPRayTexture = (osp::Texture2D*)ospNewTexture2D(xsize,
         ysize,
         type,
         pixels,
         0);

        ospCommit((OSPTexture2D)this->OSPRayTexture);
        // PRINT((OSPTexture2D)this->OSPRayTexture);

        this->LoadTime.Modified();
      }
    #endif
    }

//----------------------------------------------------------------------------
    void vtkOSPRayTexture::PrintSelf(ostream& os, vtkIndent indent)
    {
      this->Superclass::PrintSelf(os,indent);
    }


