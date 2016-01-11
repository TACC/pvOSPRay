/*=========================================================================

  Program:   Visualization Toolkit
  Module:    vtkOSPRayVolumeRayCastMapper.h

  Copyright (c) Ken Martin, Will Schroeder, Bill Lorensen
  All rights reserved.
  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/

// .NAME vtkOSPRayVolumeRayCastMapper - A volume renderer based on the OSPRay ray tracer
// .SECTION Description
// This is a software ray caster for rendering volumes in vtkImageData.

// .SECTION see also
// vtkVolumeMapper
//
//  The OSPRayVolumeRayCastMapper creates an OSPRay model and sets a flag for the 
//  active renderer.  When the active OSPRayRenderer renders, it uses the set flag
//  and provided model to render into the scene.
//

//
//  modified 1/11/2016 Carson Brownlee - cleanup of unused functions.
//  modified 12/29/2015 by Carson Brownlee
//
//
//

#ifndef __vtkOSPRayVolumeRayCastMapper_h
#define __vtkOSPRayVolumeRayCastMapper_h

// #include "vtkRenderingVolumeModule.h" // For export macro
#include "vtkVolumeMapper.h"
#include "vtkVolumeRayCastFunction.h" // For vtkVolumeRayCastStaticInfo
                                      // and vtkVolumeRayCastDynamicInfo
#include "vtkOSPRayModule.h"
#include <vector>
#include <map>


namespace osp
{
  class Volume;
  class Model;
  class TransferFunction;
}

class vtkEncodedGradientEstimator;
class vtkEncodedGradientShader;
class vtkMatrix4x4;
class vtkMultiThreader;
class vtkPlaneCollection;
class vtkRenderer;
class vtkTimerLog;
class vtkVolume;
class vtkVolumeRayCastFunction;
class vtkVolumeTransform;
class vtkTransform;
class vtkRayCastImageDisplayHelper;
class vtkOSPRayManager;

struct vtkOSPRayVolumeCacheEntry
{
  osp::Volume* Volume;
  vtkTimeStamp BuildTime;
};


// Forward declaration needed for use by friend declaration below.
VTK_THREAD_RETURN_TYPE OSPRayVolumeRayCastMapper_CastRays( void *arg );

class VTKRENDERINGVOLUME_EXPORT vtkOSPRayVolumeRayCastMapper : public vtkVolumeMapper
{
public:
  static vtkOSPRayVolumeRayCastMapper *New();
  vtkTypeMacro(vtkOSPRayVolumeRayCastMapper,vtkVolumeMapper);
  void PrintSelf( ostream& os, vtkIndent indent );

  // Description:
  // Set/Get the distance between samples.  This variable is only
  // used for sampling ray casting methods.  Methods that compute
  // a ray value by stepping cell-by-cell are not affected by this
  // value.
  vtkSetMacro( SampleDistance, double );
  vtkGetMacro( SampleDistance, double );

  // Description:
  // Get / Set the volume ray cast function. This is used to process
  // values found along the ray to compute a final pixel value.
  // virtual void SetVolumeRayCastFunction(vtkVolumeRayCastFunction*);
  // vtkGetObjectMacro( VolumeRayCastFunction, vtkVolumeRayCastFunction );

  // // Description:
  // // Set / Get the gradient estimator used to estimate normals
  // virtual void SetGradientEstimator(vtkEncodedGradientEstimator *gradest);
  // vtkGetObjectMacro( GradientEstimator, vtkEncodedGradientEstimator );

  // // Description:
  // // Get the gradient shader.
  // vtkGetObjectMacro( GradientShader, vtkEncodedGradientShader );

  // Description:
  // Sampling distance in the XY image dimensions. Default value of 1 meaning
  // 1 ray cast per pixel. If set to 0.5, 4 rays will be cast per pixel. If
  // set to 2.0, 1 ray will be cast for every 4 (2 by 2) pixels.
  vtkSetClampMacro( ImageSampleDistance, double, 0.1, 100.0 );
  vtkGetMacro( ImageSampleDistance, double );

  // Description:
  // This is the minimum image sample distance allow when the image
  // sample distance is being automatically adjusted
  vtkSetClampMacro( MinimumImageSampleDistance, double, 0.1, 100.0 );
  vtkGetMacro( MinimumImageSampleDistance, double );

  // Description:
  // This is the maximum image sample distance allow when the image
  // sample distance is being automatically adjusted
  vtkSetClampMacro( MaximumImageSampleDistance, double, 0.1, 100.0 );
  vtkGetMacro( MaximumImageSampleDistance, double );

  // Description:
  // If AutoAdjustSampleDistances is on, the the ImageSampleDistance
  // will be varied to achieve the allocated render time of this
  // prop (controlled by the desired update rate and any culling in
  // use).
  vtkSetClampMacro( AutoAdjustSampleDistances, int, 0, 1 );
  vtkGetMacro( AutoAdjustSampleDistances, int );
  vtkBooleanMacro( AutoAdjustSampleDistances, int );

  // Description:
  // Set/Get the number of threads to use. This by default is equal to
  // the number of available processors detected.
  void SetNumberOfThreads( int num );
  int GetNumberOfThreads();
  vtkSetMacro(SamplingRate, double);
  vtkGetMacro(SamplingRate, double);

  // Description:
  // If IntermixIntersectingGeometry is turned on, the zbuffer will be
  // captured and used to limit the traversal of the rays.
  vtkSetClampMacro( IntermixIntersectingGeometry, int, 0, 1 );
  vtkGetMacro( IntermixIntersectingGeometry, int );
  vtkBooleanMacro( IntermixIntersectingGeometry, int );

//BTX
  // Description:
  // WARNING: INTERNAL METHOD - NOT INTENDED FOR GENERAL USE
  // Initialize rendering for this volume.
  void Render( vtkRenderer *, vtkVolume * );

  // Description:
  // WARNING: INTERNAL METHOD - NOT INTENDED FOR GENERAL USE
  // Release any graphics resources that are being consumed by this mapper.
  // The parameter window could be used to determine which graphic
  // resources to release.
  void ReleaseGraphicsResources(vtkWindow *);

//ETX

protected:
  vtkOSPRayVolumeRayCastMapper();
  ~vtkOSPRayVolumeRayCastMapper();

  vtkVolumeRayCastFunction     *VolumeRayCastFunction;
  vtkEncodedGradientEstimator  *GradientEstimator;
  vtkEncodedGradientShader     *GradientShader;
  vtkRayCastImageDisplayHelper *ImageDisplayHelper;

  virtual void ReportReferences(vtkGarbageCollector*);

  // The distance between sample points along the ray
  int CellFlag;
  double                       SampleDistance;
  double                       ImageSampleDistance;
  double                       MinimumImageSampleDistance;
  double                       MaximumImageSampleDistance;
  int                          AutoAdjustSampleDistances;

  int                          ScalarDataType;
  void                         *ScalarDataPointer;

  int           IntermixIntersectingGeometry;

  float        *ZBuffer;

//
//OSPRay
//

  int NumberOfThreads;
  vtkOSPRayManager *OSPRayManager;
  osp::Volume* OSPRayVolume;
  osp::Model* OSPRayModel;
  vtkTimeStamp  BuildTime,PropertyTime;
  osp::TransferFunction* transferFunction;
  int NumColors;
  std::vector<float> TFVals, TFOVals;
  bool SharedData;
  bool VolumeAdded;
  double SamplingRate;
  std::map< vtkVolume*, std::map< double, vtkOSPRayVolumeCacheEntry* > > Cache;


private:
  vtkOSPRayVolumeRayCastMapper(const vtkOSPRayVolumeRayCastMapper&);  // Not implemented.
  void operator=(const vtkOSPRayVolumeRayCastMapper&);  // Not implemented.
};

#endif
