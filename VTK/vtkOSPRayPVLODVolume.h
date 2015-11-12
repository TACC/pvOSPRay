/*=========================================================================

  Program:   ParaView
  Module:    vtkOSPRayPVLODVolume.h

  Copyright (c) Kitware, Inc.
  All rights reserved.
  See Copyright.txt or http://www.paraview.org/HTML/Copyright.html for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
// .NAME vtkOSPRayPVLODVolume - an actor that supports multiple levels of detail
// .SECTION Description
// vtkOSPRayPVLODVolume is much like vtkPVLODActor except that it works on
// volumes instead of surfaces.  This just has two mappers: full res and
// LOD, and this actor knows which is which.

// .SECTION see also
// vtkActor vtkRenderer vtkLODProp3D vtkLODActor

#ifndef __vtkOSPRayPVLODVolume_h
#define __vtkOSPRayPVLODVolume_h

#include "vtkVolume.h"
#include "vtkPVVTKExtensionsRenderingModule.h" // needed for export macro

class vtkLODProp3D;
class vtkMapper;

class VTKPVVTKEXTENSIONSRENDERING_EXPORT vtkOSPRayPVLODVolume : public vtkVolume
{
public:
  vtkTypeMacro(vtkOSPRayPVLODVolume,vtkVolume);
  void PrintSelf(ostream& os, vtkIndent indent);

  static vtkOSPRayPVLODVolume *New();

  // Description:
  // This method is used internally by the rendering process.
  virtual int RenderOpaqueGeometry(vtkViewport *viewport);
  virtual int RenderVolumetricGeometry(vtkViewport *viewport);
  virtual int RenderTranslucentPolygonalGeometry( vtkViewport *);

  // Description:
  // Does this prop have some translucent polygonal geometry?
  virtual int HasTranslucentPolygonalGeometry();

  // Description:
  // Release any graphics resources that are being consumed by this actor.
  // The parameter window could be used to determine which graphic
  // resources to release.
  virtual void ReleaseGraphicsResources(vtkWindow *);

  // Description:
  // Set the high res input.  Overloads the virtual vtkVolume method.
  virtual void SetMapper(vtkAbstractVolumeMapper *);

  // Description:
  // This sets the low res input.
  virtual void SetLODMapper(vtkAbstractVolumeMapper *);
  virtual void SetLODMapper(vtkMapper *);

  // Description:
  // Sets the volume propery.  Overloads the virtual vtkVolume method.
  virtual void SetProperty(vtkVolumeProperty *property);

  // Description:
  // Shallow copy of an LOD actor. Overloads the virtual vtkProp method.
  virtual void ShallowCopy(vtkProp *prop);

  // Description:
  // Get the bounds of the current mapper.
  virtual double *GetBounds();

  // Description:
  // Overloads the virtual vtkProp method.
  virtual void SetAllocatedRenderTime(double t, vtkViewport *v);

  // Description:
  // When set, LODMapper, if present it used, otherwise the regular mapper is
  // used.
  vtkSetMacro(EnableLOD, int);
  vtkGetMacro(EnableLOD, int);
protected:
  vtkOSPRayPVLODVolume();
  ~vtkOSPRayPVLODVolume();

  // Description:
  // Since volume mapper are notorious for segfaulting when the scalar array is
  // missing we use this method to validate that we can actually render the
  // data.
  bool CanRender();

  vtkLODProp3D *LODProp;
  int HighLODId;
  int LowLODId;
  int EnableLOD;
  int SelectLOD();
  double MapperBounds[6];
  vtkTimeStamp BoundsMTime;
  virtual void UpdateLODProperty();
private:
  vtkOSPRayPVLODVolume(const vtkOSPRayPVLODVolume&); // Not implemented.
  void operator=(const vtkOSPRayPVLODVolume&); // Not implemented.
};

#endif
