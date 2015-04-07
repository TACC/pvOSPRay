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

#include "vtkOSPRayVolumeMapper.h"

#include "vtkDataSet.h"
#include "vtkExecutive.h"
#include "vtkGarbageCollector.h"
#include "vtkImageData.h"
#include "vtkInformation.h"


vtkStandardNewMacro(vtkOSPRayVolumeMapper);


// Construct a vtkOSPRayVolumeMapper with empty scalar input and clipping off.
vtkOSPRayVolumeMapper::vtkOSPRayVolumeMapper()
{
  std::cout << __PRETTY_FUNCTION__ << std::endl;
  int i;

  this->BlendMode = vtkOSPRayVolumeMapper::COMPOSITE_BLEND;

  this->Cropping = 0;
  for ( i = 0; i < 3; i++ )
    {
    this->CroppingRegionPlanes[2*i    ]      = 0;
    this->CroppingRegionPlanes[2*i + 1]      = 1;
    this->VoxelCroppingRegionPlanes[2*i]     = 0;
    this->VoxelCroppingRegionPlanes[2*i + 1] = 1;
    }
  this->CroppingRegionFlags = VTK_CROP_SUBVOLUME;
}

vtkOSPRayVolumeMapper::~vtkOSPRayVolumeMapper()
{
}

void vtkOSPRayVolumeMapper::Render(vtkRenderer *ren, vtkVolume *vol)
{

}

void vtkOSPRayVolumeMapper::ConvertCroppingRegionPlanesToVoxels()
{
  double *spacing    = this->GetInput()->GetSpacing();
  int dimensions[3];
  this->GetInput()->GetDimensions(dimensions);
  double origin[3];
  double *bds = this->GetInput()->GetBounds();
  origin[0] = bds[0];
  origin[1] = bds[2];
  origin[2] = bds[4];

  for ( int i = 0; i < 6; i++ )
    {
    this->VoxelCroppingRegionPlanes[i] =
      (this->CroppingRegionPlanes[i] - origin[i/2]) / spacing[i/2];

    this->VoxelCroppingRegionPlanes[i] =
      ( this->VoxelCroppingRegionPlanes[i] < 0 ) ?
      ( 0 ) : ( this->VoxelCroppingRegionPlanes[i] );

    this->VoxelCroppingRegionPlanes[i] =
      ( this->VoxelCroppingRegionPlanes[i] > dimensions[i/2]-1 ) ?
      ( dimensions[i/2]-1 ) : ( this->VoxelCroppingRegionPlanes[i] );
    }
}

void vtkOSPRayVolumeMapper::SetInputData( vtkDataSet *genericInput )
{
  vtkImageData *input =
    vtkImageData::SafeDownCast( genericInput );

  if ( input )
    {
    this->SetInputData( input );
    }
  else
    {
    vtkErrorMacro("The SetInput method of this mapper requires vtkImageData as input");
    }
}

void vtkOSPRayVolumeMapper::SetInputData( vtkImageData *input )
{
  this->SetInputDataInternal(0, input);
}

vtkImageData *vtkOSPRayVolumeMapper::GetInput()
{
  if (this->GetNumberOfInputConnections(0) < 1)
    {
    return 0;
    }
  return vtkImageData::SafeDownCast(
    this->GetExecutive()->GetInputData(0, 0));
}


// Print the vtkOSPRayVolumeMapper
void vtkOSPRayVolumeMapper::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);

  os << indent << "Cropping: " << (this->Cropping ? "On\n" : "Off\n");

  os << indent << "Cropping Region Planes: " << endl
     << indent << "  In X: " << this->CroppingRegionPlanes[0]
     << " to " << this->CroppingRegionPlanes[1] << endl
     << indent << "  In Y: " << this->CroppingRegionPlanes[2]
     << " to " << this->CroppingRegionPlanes[3] << endl
     << indent << "  In Z: " << this->CroppingRegionPlanes[4]
     << " to " << this->CroppingRegionPlanes[5] << endl;

  os << indent << "Cropping Region Flags: "
     << this->CroppingRegionFlags << endl;

  os << indent << "BlendMode: " << this->BlendMode << endl;

  // Don't print this->VoxelCroppingRegionPlanes
}

//----------------------------------------------------------------------------
int vtkOSPRayVolumeMapper::FillInputPortInformation(int port, vtkInformation* info)
{
  if(!this->Superclass::FillInputPortInformation(port, info))
    {
    return 0;
    }
  info->Set(vtkAlgorithm::INPUT_REQUIRED_DATA_TYPE(), "vtkImageData");
  return 1;
}
