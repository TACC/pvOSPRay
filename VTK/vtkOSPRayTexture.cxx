/*=========================================================================

  Program:   Visualization Toolkit
  Module:    vtkOSPRayTexture.cxx

  Copyright (c) Ken Martin, Will Schroeder, Bill Lorensen
  All rights reserved.
  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
/*=========================================================================

  Program:   VTK/ParaView Los Alamos National Laboratory Modules (PVLANL)
  Module:    vtkOSPRayTexture.cxx

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
//This is a helper that exists just to hold on to manta side resources
//long enough for the manta thread to destroy them, whenever that
//threads gets around to it (in a callback)
// class vtkOSPRayTextureThreadCache
// {
// public:
//   vtkOSPRayTextureThreadCache(Manta::Texture<Manta::Color> *mt)
//     : MantaTexture(mt)
//   {
//   }

//   void FreeMantaResources()
//   {
//     delete MantaTexture;
//     //WARNING: this class must never be instantiated on the stack.
//     //Therefore, it has private unimplemented copy/contructors.
//     delete this;
//   }

// private:
//   vtkOSPRayTextureThreadCache(const vtkOSPRayTextureThreadCache&);
//   // Not implemented.
//   void operator=(const vtkOSPRayTextureThreadCache&);
//   // Not implemented.

//   Manta::Texture<Manta::Color> *MantaTexture;
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
  // this->MantaTexture = NULL;
  this->OSPRayTexture = NULL;
}

//----------------------------------------------------------------------------
vtkOSPRayTexture::~vtkOSPRayTexture()
{
  //cerr << "MT( " << this << ") DESTROY " << endl;
  if (this->OSPRayManager)
  {
    this->DeleteMantaTexture();

    //cerr << "MT(" << this << ") DESTROY " << this->OSPRayManager << " "
    //     << this->OSPRayManager->GetReferenceCount() << endl;
    this->OSPRayManager->Delete();
  }
}

//-----------------------------------------------------------------------------
void vtkOSPRayTexture::DeleteMantaTexture()
{
  if (!this->OSPRayTexture)
  {
    return;
  }

  //save off the pointers for the manta thread
  // vtkOSPRayTextureThreadCache *R =
  //   new vtkOSPRayTextureThreadCache(this->MantaTexture);

  // //make no further references to them in this thread
  this->OSPRayTexture = NULL;

  //ask the manta thread to free them when it can
  // this->OSPRayManager->GetMantaEngine()->
    // addTransaction("cleanup texture",
                   // Manta::Callback::create
                   // (R, &vtkOSPRayTextureThreadCache::FreeMantaResources));
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

  this->DeleteMantaTexture();
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

    // Create Manta Image from input
    // Manta::Image *image =
      // new Manta::SimpleImage<Manta::RGB8Pixel> (false, xsize, ysize);
    // Manta::RGB8Pixel *pixels =
      // dynamic_cast<Manta::SimpleImage<Manta::RGB8Pixel> const*>(image)->
      // getRawPixels(0);
    // Manta::RGB8Pixel pixel;
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

    // create Manta texture from the image
    // Manta::ImageTexture<Manta::Color> *imgtexture =
      // new Manta::ImageTexture<Manta::Color>(image, false);
    // imgtexture->setInterpolationMethod(1);

    // this->DeleteMantaTexture();
    // this->MantaTexture = imgtexture;

    // Manta image is copied and converted to internal buffer in the texture,
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


