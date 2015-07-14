/*=========================================================================

  Program:   ParaView
  Module:    vtkPVOSPRayImageVolumeRepresentation.cxx

  Copyright (c) Kitware, Inc.
  All rights reserved.
  See Copyright.txt or http://www.paraview.org/HTML/Copyright.html for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
#include "vtkPVOSPRayImageVolumeRepresentation.h"

#include "vtkAlgorithmOutput.h"
#include "vtkCommand.h"
#include "vtkExtentTranslator.h"
#include "vtkImageData.h"
#include "vtkInformation.h"
#include "vtkInformationVector.h"
#include "vtkMath.h"
#include "vtkObjectFactory.h"
#include "vtkOutlineSource.h"
#include "vtkPolyDataMapper.h"
#include "vtkPVCacheKeeper.h"
#include "vtkPVLODVolume.h"
#include "vtkPVRenderView.h"
#include "vtkRenderer.h"
#include "vtkSmartPointer.h"
#include "vtkSmartVolumeMapper.h"
#include "vtkStreamingDemandDrivenPipeline.h"
#include "vtkVolumeProperty.h"
#include "vtkAbstractVolumeMapper.h"
#include "vtkPExtentTranslator.h"

#include "vtkOSPRayVolumeRayCastMapper.h"
#include "vtkOSPRayCompositeMapper.h"
#include "vtkOSPRayLODActor.h"
#include "vtkOSPRayActor.h"
#include "vtkOSPRayPolyDataMapper.h"
#include "vtkOSPRayProperty.h"
#include "vtkObjectFactory.h"
#include "vtkVolumeProperty.h"
     #include "vtkPiecewiseFunction.h"
     #include "vtkOSPRayPVLODVolume.h"

#include <map>
#include <string>

#if 0
class vtkOSPRayPVLODVolume : public vtkPVLODVolume
{
public:
  static vtkOSPRayPVLODVolume* New();
  vtkTypeMacro(vtkOSPRayPVLODVolume,vtkPVLODVolume);

  vtkOSPRayPVLODVolume() : vtkPVLODVolume() {}

  virtual int   RenderOpaqueGeometry (vtkViewport *viewport)
  { std::cout << __PRETTY_FUNCTION__ << std::endl; }
virtual int   RenderVolumetricGeometry (vtkViewport *viewport)
  { std::cout << __PRETTY_FUNCTION__ << std::endl; }
virtual int   RenderTranslucentPolygonalGeometry (vtkViewport *)
  { std::cout << __PRETTY_FUNCTION__ << std::endl; }
}


vtkStandardNewMacro(vtkOSPRayPVLODVolume);
#endif

#if 0
vtkStandardNewMacro(vtkPVOSPRayImageVolumeRepresentation);
//----------------------------------------------------------------------------
vtkPVOSPRayImageVolumeRepresentation::vtkPVOSPRayImageVolumeRepresentation()
{
  std::cout << __PRETTY_FUNCTION__ << std::endl;

  // this->VolumeMapper->Delete();
  // this->VolumeMapper = vtkOSPRayVolumeMapper::New();

  // this->VolumeMapper2->Delete();
  this->VolumeMapper = vtkOSPRayVolumeRayCastMapper::New();
  // this->LODMapper->Delete();
  // this->LODMapper = vtkOSPRayVolumeMapper::New();
  // this->Actor->Delete();
  // this->Actor = vtkOSPRayLODActor::New();
  // this->Property->Delete();
  // this->Property = vtkOSPRayProperty::New();

  // this->Actor->SetMapper(this->VolumeMapper);
  // this->Actor->SetLODMapper(this->LODMapper);
  // this->Actor->SetProperty(this->Property);

  // vtkInformation* keys = vtkInformation::New();
  // this->Actor->SetPropertyKeys(keys);
  // keys->Delete();

    this->Property = vtkVolumeProperty::New();

  this->Actor = vtkPVLODVolume::New();
  this->Actor->SetProperty(this->Property);

  this->CacheKeeper = vtkPVCacheKeeper::New();

  this->OutlineSource = vtkOutlineSource::New();
  this->OutlineMapper = vtkPolyDataMapper::New();

  this->ColorArrayName = 0;
  this->ColorAttributeType = POINT_DATA;
  this->Cache = vtkImageData::New();

  this->CacheKeeper->SetInputData(this->Cache);
  this->Actor->SetLODMapper(this->OutlineMapper);

  vtkMath::UninitializeBounds(this->DataBounds);

  /*
  this->VolumeMapper = vtkSmartVolumeMapper::New();
  this->Property = vtkVolumeProperty::New();

  this->Actor = vtkPVLODVolume::New();
  this->Actor->SetProperty(this->Property);

  this->CacheKeeper = vtkPVCacheKeeper::New();

  this->OutlineSource = vtkOutlineSource::New();
  this->OutlineMapper = vtkPolyDataMapper::New();

  this->ColorArrayName = 0;
  this->ColorAttributeType = POINT_DATA;
  this->Cache = vtkImageData::New();

  this->CacheKeeper->SetInputData(this->Cache);
  this->Actor->SetLODMapper(this->OutlineMapper);

  vtkMath::UninitializeBounds(this->DataBounds);
  */
}

//----------------------------------------------------------------------------
vtkPVOSPRayImageVolumeRepresentation::~vtkPVOSPRayImageVolumeRepresentation()
{
  this->VolumeMapper->Delete();
  // this->Property->Delete();
  // this->Actor->Delete();
  // this->OutlineSource->Delete();
  // this->OutlineMapper->Delete();
  // this->CacheKeeper->Delete();

  // this->SetColorArrayName(0);

  // this->Cache->Delete();
}

//----------------------------------------------------------------------------
int vtkPVOSPRayImageVolumeRepresentation::FillInputPortInformation(
  int, vtkInformation* info)
{
  info->Set(vtkAlgorithm::INPUT_REQUIRED_DATA_TYPE(), "vtkImageData");
  info->Set(vtkAlgorithm::INPUT_IS_OPTIONAL(), 1);
  return 1;
}

//----------------------------------------------------------------------------
int vtkPVOSPRayImageVolumeRepresentation::ProcessViewRequest(
  vtkInformationRequestKey* request_type,
  vtkInformation* inInfo, vtkInformation* outInfo)
{
  if (!this->Superclass::ProcessViewRequest(request_type, inInfo, outInfo))
    {
    return 0;
    }
  if (request_type == vtkPVView::REQUEST_UPDATE())
    {
    vtkPVRenderView::SetPiece(inInfo, this,
      this->OutlineSource->GetOutputDataObject(0));
    outInfo->Set(vtkPVRenderView::NEED_ORDERED_COMPOSITING(), 1);

    vtkPVRenderView::SetGeometryBounds(inInfo, this->DataBounds);

    vtkPVOSPRayImageVolumeRepresentation::PassOrderedCompositingInformation(
      this, inInfo);
    }
  else if (request_type == vtkPVView::REQUEST_RENDER())
    {
    this->UpdateMapperParameters();

    vtkAlgorithmOutput* producerPort = vtkPVRenderView::GetPieceProducer(inInfo, this);
    if (producerPort)
      {
      this->OutlineMapper->SetInputConnection(producerPort);
      }
    }
  return 1;
}

//----------------------------------------------------------------------------
void vtkPVOSPRayImageVolumeRepresentation::PassOrderedCompositingInformation(
  vtkPVDataRepresentation* self, vtkInformation* inInfo)
{
  // The KdTree generation code that uses the image cuts needs to be updated
  // bigtime. But due to time shortage, I'm leaving the old code as is. We
  // will get back to it later.
 //  if (self->GetNumberOfInputConnections(0) == 1)
 //    {
 //    vtkAlgorithmOutput* connection = self->GetInputConnection(0, 0);
 //    vtkAlgorithm* inputAlgo = connection->GetProducer();
 //    vtkStreamingDemandDrivenPipeline* sddp =
 //      vtkStreamingDemandDrivenPipeline::SafeDownCast(inputAlgo->GetExecutive());
 //    vtkExtentTranslator* translator =
 //      sddp->GetExtentTranslator(connection->GetIndex());
 // //   vtkStreamingDemandDrivenPipeline::GetExtentTranslator(sddp->GetExecutive()->GetOutputPort());

 //    int extent[6] = {1, -1, 1, -1, 1, -1};
 //    sddp->GetWholeExtent(sddp->GetOutputInformation(connection->GetIndex()),
 //      extent);

 //    double origin[3], spacing[3];
 //    vtkImageData* image = vtkImageData::SafeDownCast(
 //      inputAlgo->GetOutputDataObject(connection->GetIndex()));
 //    image->GetOrigin(origin);
 //    image->GetSpacing(spacing);
 //    vtkPVRenderView::SetOrderedCompositingInformation(
 //      inInfo, self, translator, extent, origin, spacing);
 //    }

  //Carson: taking out for now, not sure parallel works with ospray volume rendering yet anyway
}

//----------------------------------------------------------------------------
int vtkPVOSPRayImageVolumeRepresentation::RequestData(vtkInformation* request,
    vtkInformationVector** inputVector, vtkInformationVector* outputVector)
{
  vtkMath::UninitializeBounds(this->DataBounds);

  // Pass caching information to the cache keeper.
  this->CacheKeeper->SetCachingEnabled(this->GetUseCache());
  this->CacheKeeper->SetCacheTime(this->GetCacheKey());

  if (inputVector[0]->GetNumberOfInformationObjects()==1)
    {
    vtkImageData* input = vtkImageData::GetData(inputVector[0], 0);
    if (!this->GetUsingCacheForUpdate())
      {
      this->Cache->ShallowCopy(input);
      }
    this->CacheKeeper->Update();

    this->Actor->SetEnableLOD(0);
    this->VolumeMapper->SetInputConnection(
      this->CacheKeeper->GetOutputPort());

    this->OutlineSource->SetBounds(vtkImageData::SafeDownCast(
        this->CacheKeeper->GetOutputDataObject(0))->GetBounds());
    this->OutlineSource->GetBounds(this->DataBounds);
    this->OutlineSource->Update();
    }
  else
    {
    // when no input is present, it implies that this processes is on a node
    // without the data input i.e. either client or render-server, in which case
    // we show only the outline.
    this->VolumeMapper->RemoveAllInputs();
    this->Actor->SetEnableLOD(1);
    }

  return this->Superclass::RequestData(request, inputVector, outputVector);
}

//----------------------------------------------------------------------------
bool vtkPVOSPRayImageVolumeRepresentation::IsCached(double cache_key)
{
  return this->CacheKeeper->IsCached(cache_key);
}

//----------------------------------------------------------------------------
void vtkPVOSPRayImageVolumeRepresentation::MarkModified()
{
  if (!this->GetUseCache())
    {
    // Cleanup caches when not using cache.
    this->CacheKeeper->RemoveAllCaches();
    }
  this->Superclass::MarkModified();
}

//----------------------------------------------------------------------------
bool vtkPVOSPRayImageVolumeRepresentation::AddToView(vtkView* view)
{
  // FIXME: Need generic view API to add props.
  vtkPVRenderView* rview = vtkPVRenderView::SafeDownCast(view);
  if (rview)
    {
    rview->GetRenderer()->AddActor(this->Actor);
    return true;
    }
  return false;
}

//----------------------------------------------------------------------------
bool vtkPVOSPRayImageVolumeRepresentation::RemoveFromView(vtkView* view)
{
  vtkPVRenderView* rview = vtkPVRenderView::SafeDownCast(view);
  if (rview)
    {
    rview->GetRenderer()->RemoveActor(this->Actor);
    return true;
    }
  return false;
}

//----------------------------------------------------------------------------
void vtkPVOSPRayImageVolumeRepresentation::UpdateMapperParameters()
{
  this->VolumeMapper->SelectScalarArray(this->ColorArrayName);
  switch (this->ColorAttributeType)
    {
  case CELL_DATA:
    this->VolumeMapper->SetScalarMode(VTK_SCALAR_MODE_USE_CELL_FIELD_DATA);
    break;

  case POINT_DATA:
  default:
    this->VolumeMapper->SetScalarMode(VTK_SCALAR_MODE_USE_POINT_FIELD_DATA);
    break;
    }
  this->Actor->SetMapper(this->VolumeMapper);
}

//----------------------------------------------------------------------------
void vtkPVOSPRayImageVolumeRepresentation::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);
}


//***************************************************************************
// Forwarded to Actor.

//----------------------------------------------------------------------------
void vtkPVOSPRayImageVolumeRepresentation::SetOrientation(double x, double y, double z)
{
  this->Actor->SetOrientation(x, y, z);
}

//----------------------------------------------------------------------------
void vtkPVOSPRayImageVolumeRepresentation::SetOrigin(double x, double y, double z)
{
  this->Actor->SetOrigin(x, y, z);
}

//----------------------------------------------------------------------------
void vtkPVOSPRayImageVolumeRepresentation::SetPickable(int val)
{
  this->Actor->SetPickable(val);
}
//----------------------------------------------------------------------------
void vtkPVOSPRayImageVolumeRepresentation::SetPosition(double x , double y, double z)
{
  this->Actor->SetPosition(x, y, z);
}
//----------------------------------------------------------------------------
void vtkPVOSPRayImageVolumeRepresentation::SetScale(double x, double y, double z)
{
  this->Actor->SetScale(x, y, z);
}

//----------------------------------------------------------------------------
void vtkPVOSPRayImageVolumeRepresentation::SetVisibility(bool val)
{
  this->Superclass::SetVisibility(val);
  this->Actor->SetVisibility(val? 1 : 0);
}

//***************************************************************************
// Forwarded to vtkVolumeProperty.
//----------------------------------------------------------------------------
void vtkPVOSPRayImageVolumeRepresentation::SetInterpolationType(int val)
{
  this->Property->SetInterpolationType(val);
}

//----------------------------------------------------------------------------
void vtkPVOSPRayImageVolumeRepresentation::SetColor(vtkColorTransferFunction* lut)
{
  std::cout << __PRETTY_FUNCTION__ << "\n";
  this->Property->SetColor(lut);
}

//----------------------------------------------------------------------------
void vtkPVOSPRayImageVolumeRepresentation::SetScalarOpacity(vtkPiecewiseFunction* pwf)
{
  std::cout << __PRETTY_FUNCTION__ << " SetScalarOpaicty\n";
  this->Property->SetScalarOpacity(pwf);

  float tfOVals[256];
  pwf->GetTable(0,300, 12, tfOVals);
      std::cout << "tfOVals:\n";
  for(int i=0;i<12;i++)
    std::cout << tfOVals[i] << " " << std::endl;
  std::cout << "\n\n";
}

//----------------------------------------------------------------------------
void vtkPVOSPRayImageVolumeRepresentation::SetScalarOpacityUnitDistance(double val)
{
  this->Property->SetScalarOpacityUnitDistance(val);
}

//----------------------------------------------------------------------------
void vtkPVOSPRayImageVolumeRepresentation::SetAmbient(double val)
{
  this->Property->SetAmbient(val);
}

//----------------------------------------------------------------------------
void vtkPVOSPRayImageVolumeRepresentation::SetDiffuse(double val)
{
  this->Property->SetDiffuse(val);
}

//----------------------------------------------------------------------------
void vtkPVOSPRayImageVolumeRepresentation::SetSpecular(double val)
{
  this->Property->SetSpecular(val);
}

//----------------------------------------------------------------------------
void vtkPVOSPRayImageVolumeRepresentation::SetSpecularPower(double val)
{
  this->Property->SetSpecularPower(val);
}

//----------------------------------------------------------------------------
void vtkPVOSPRayImageVolumeRepresentation::SetShade(bool val)
{
  this->Property->SetShade(val);
}

//----------------------------------------------------------------------------
void vtkPVOSPRayImageVolumeRepresentation::SetIndependantComponents(bool val)
{
  this->Property->SetIndependentComponents(val);
}

//----------------------------------------------------------------------------
void vtkPVOSPRayImageVolumeRepresentation::SetRequestedRenderMode(int mode)
{
  // this->VolumeMapper->SetRequestedRenderMode(mode);
}
#endif

vtkStandardNewMacro(vtkPVOSPRayImageVolumeRepresentation);
//----------------------------------------------------------------------------
vtkPVOSPRayImageVolumeRepresentation::vtkPVOSPRayImageVolumeRepresentation()
{
    // std::cout << __PRETTY_FUNCTION__ << std::endl;
  // this->VolumeMapper = vtkSmartVolumeMapper::New();
  this->VolumeMapper = vtkOSPRayVolumeRayCastMapper::New();
  // this->Property = vtkVolumeProperty::New();

  this->Property = vtkVolumeProperty::New();

  // this->Actor = vtkPVLODVolume::New();
  this->Actor = vtkOSPRayPVLODVolume::New();
  // Actor->SetSelectedLODID(-1);
  this->Actor->SetProperty(this->Property);

  this->CacheKeeper = vtkPVCacheKeeper::New();

  this->OutlineSource = vtkOutlineSource::New();
  this->OutlineMapper = vtkPolyDataMapper::New();

  this->Cache = vtkImageData::New();

  this->CacheKeeper->SetInputData(this->Cache);
  // this->Actor->SetLODMapper(this->OutlineMapper);

  vtkMath::UninitializeBounds(this->DataBounds);
  this->DataSize = 0;

  this->ColorArrayName = 0;
  this->ColorAttributeType = POINT_DATA;


  // // this->VolumeMapper->Delete();
  // // this->VolumeMapper = vtkOSPRayVolumeMapper::New();

  // // this->VolumeMapper2->Delete();
  // this->VolumeMapper = vtkOSPRayVolumeRayCastMapper::New();
  // // this->LODMapper->Delete();
  // // this->LODMapper = vtkOSPRayVolumeMapper::New();
  // // this->Actor->Delete();
  // // this->Actor = vtkOSPRayLODActor::New();
  // // this->Property->Delete();
  // // this->Property = vtkOSPRayProperty::New();

  // // this->Actor->SetMapper(this->VolumeMapper);
  // // this->Actor->SetLODMapper(this->LODMapper);
  // // this->Actor->SetProperty(this->Property);

  // // vtkInformation* keys = vtkInformation::New();
  // // this->Actor->SetPropertyKeys(keys);
  // // keys->Delete();

  //   this->Property = vtkVolumeProperty::New();

  // this->Actor = vtkPVLODVolume::New();
  // this->Actor->SetProperty(this->Property);

  // this->CacheKeeper = vtkPVCacheKeeper::New();

  // this->OutlineSource = vtkOutlineSource::New();
  // this->OutlineMapper = vtkPolyDataMapper::New();

  // this->ColorArrayName = 0;
  // this->ColorAttributeType = POINT_DATA;
  // this->Cache = vtkImageData::New();

  // this->CacheKeeper->SetInputData(this->Cache);
  // this->Actor->SetLODMapper(this->OutlineMapper);

  // vtkMath::UninitializeBounds(this->DataBounds);
}

//----------------------------------------------------------------------------
vtkPVOSPRayImageVolumeRepresentation::~vtkPVOSPRayImageVolumeRepresentation()
{
  this->VolumeMapper->Delete();
  this->Property->Delete();
  this->Actor->Delete();
  this->OutlineSource->Delete();
  this->OutlineMapper->Delete();
  this->CacheKeeper->Delete();

  this->Cache->Delete();
}

//----------------------------------------------------------------------------
int vtkPVOSPRayImageVolumeRepresentation::FillInputPortInformation(
  int, vtkInformation* info)
{
  info->Set(vtkAlgorithm::INPUT_REQUIRED_DATA_TYPE(), "vtkImageData");
  info->Set(vtkAlgorithm::INPUT_IS_OPTIONAL(), 1);
  return 1;
}

//----------------------------------------------------------------------------
int vtkPVOSPRayImageVolumeRepresentation::ProcessViewRequest(
  vtkInformationRequestKey* request_type,
  vtkInformation* inInfo, vtkInformation* outInfo)
{
  if (!this->Superclass::ProcessViewRequest(request_type, inInfo, outInfo))
    {
    return 0;
    }
  if (request_type == vtkPVView::REQUEST_UPDATE())
    {
    vtkPVRenderView::SetPiece(inInfo, this,
      this->OutlineSource->GetOutputDataObject(0),
      this->DataSize);
    // BUG #14792.
    // We report this->DataSize explicitly since the data being "delivered" is
    // not the data that should be used to make rendering decisions based on
    // data size.
    outInfo->Set(vtkPVRenderView::NEED_ORDERED_COMPOSITING(), 1);

    vtkPVRenderView::SetGeometryBounds(inInfo, this->DataBounds);

    vtkPVOSPRayImageVolumeRepresentation::PassOrderedCompositingInformation(
      this, inInfo);

    vtkPVRenderView::SetRequiresDistributedRendering(inInfo, this, true);
    }
  else if (request_type == vtkPVView::REQUEST_UPDATE_LOD())
    {
    vtkPVRenderView::SetRequiresDistributedRenderingLOD(inInfo, this, true);
    }
  else if (request_type == vtkPVView::REQUEST_RENDER())
    {
    this->UpdateMapperParameters();

    vtkAlgorithmOutput* producerPort = vtkPVRenderView::GetPieceProducer(inInfo, this);
    if (producerPort)
      {
      this->OutlineMapper->SetInputConnection(producerPort);
      }
    }
  return 1;
}

//----------------------------------------------------------------------------
void vtkPVOSPRayImageVolumeRepresentation::PassOrderedCompositingInformation(
  vtkPVDataRepresentation* self, vtkInformation* inInfo)
{
  // The KdTree generation code that uses the image cuts needs to be updated
  // bigtime. But due to time shortage, I'm leaving the old code as is. We
  // will get back to it later.
  (void)inInfo;
  if (self->GetNumberOfInputConnections(0) == 1)
    {
    vtkAlgorithmOutput* connection = self->GetInputConnection(0, 0);
    vtkAlgorithm* inputAlgo = connection->GetProducer();
    vtkStreamingDemandDrivenPipeline* sddp =
      vtkStreamingDemandDrivenPipeline::SafeDownCast(inputAlgo->GetExecutive());

    int extent[6] = {1, -1, 1, -1, 1, -1};
    sddp->GetWholeExtent(sddp->GetOutputInformation(connection->GetIndex()),
      extent);
    double origin[3], spacing[3];
    vtkImageData* image = vtkImageData::SafeDownCast(
      inputAlgo->GetOutputDataObject(connection->GetIndex()));
    image->GetOrigin(origin);
    image->GetSpacing(spacing);

    vtkNew<vtkPExtentTranslator> translator;
    translator->GatherExtents(image);
    vtkPVRenderView::SetOrderedCompositingInformation(
      inInfo, self, translator.GetPointer(), extent, origin, spacing);
    }
}

//----------------------------------------------------------------------------
int vtkPVOSPRayImageVolumeRepresentation::RequestData(vtkInformation* request,
    vtkInformationVector** inputVector, vtkInformationVector* outputVector)
{
  vtkMath::UninitializeBounds(this->DataBounds);
  this->DataSize = 0;

  // Pass caching information to the cache keeper.
  this->CacheKeeper->SetCachingEnabled(this->GetUseCache());
  this->CacheKeeper->SetCacheTime(this->GetCacheKey());

  if (inputVector[0]->GetNumberOfInformationObjects()==1)
    {
    vtkImageData* input = vtkImageData::GetData(inputVector[0], 0);
    if (!this->GetUsingCacheForUpdate())
      {
      this->Cache->ShallowCopy(input);
      }
    this->CacheKeeper->Update();

    printf("lod disable\n");
    this->Actor->SetEnableLOD(0);
    this->VolumeMapper->SetInputConnection(
      this->CacheKeeper->GetOutputPort());

    this->OutlineSource->SetBounds(vtkImageData::SafeDownCast(
        this->CacheKeeper->GetOutputDataObject(0))->GetBounds());
    this->OutlineSource->GetBounds(this->DataBounds);
    this->OutlineSource->Update();

    this->DataSize = this->CacheKeeper->GetOutputDataObject(0)->GetActualMemorySize();
    }
  else
    {
    // when no input is present, it implies that this processes is on a node
    // without the data input i.e. either client or render-server, in which case
    // we show only the outline.
    this->VolumeMapper->RemoveAllInputs();
    // printf("lod enable\n");
    this->Actor->SetEnableLOD(1);
    }

  return this->Superclass::RequestData(request, inputVector, outputVector);
}

//----------------------------------------------------------------------------
bool vtkPVOSPRayImageVolumeRepresentation::IsCached(double cache_key)
{
  return this->CacheKeeper->IsCached(cache_key);
}

//----------------------------------------------------------------------------
void vtkPVOSPRayImageVolumeRepresentation::MarkModified()
{
  if (!this->GetUseCache())
    {
    // Cleanup caches when not using cache.
    this->CacheKeeper->RemoveAllCaches();
    }
  this->Superclass::MarkModified();
}

//----------------------------------------------------------------------------
bool vtkPVOSPRayImageVolumeRepresentation::AddToView(vtkView* view)
{
  // FIXME: Need generic view API to add props.
  vtkPVRenderView* rview = vtkPVRenderView::SafeDownCast(view);
  if (rview)
    {
    rview->GetRenderer()->AddActor(this->Actor);
    return true;
    }
  return false;
}

//----------------------------------------------------------------------------
bool vtkPVOSPRayImageVolumeRepresentation::RemoveFromView(vtkView* view)
{
  vtkPVRenderView* rview = vtkPVRenderView::SafeDownCast(view);
  if (rview)
    {
    rview->GetRenderer()->RemoveActor(this->Actor);
    return true;
    }
  return false;
}

//----------------------------------------------------------------------------
void vtkPVOSPRayImageVolumeRepresentation::UpdateMapperParameters()
{
  // std::cout << __PRETTY_FUNCTION__ << std::endl;
  const char* colorArrayName = NULL;
  int fieldAssociation = vtkDataObject::FIELD_ASSOCIATION_POINTS;

  vtkInformation *info = this->GetInputArrayInformation(0);
  if (info &&
    info->Has(vtkDataObject::FIELD_ASSOCIATION()) &&
    info->Has(vtkDataObject::FIELD_NAME()))
    {
    colorArrayName = info->Get(vtkDataObject::FIELD_NAME());
    fieldAssociation = info->Get(vtkDataObject::FIELD_ASSOCIATION());
    }

  this->VolumeMapper->SelectScalarArray(colorArrayName);
  switch (fieldAssociation)
    {
  case vtkDataObject::FIELD_ASSOCIATION_CELLS:
    this->VolumeMapper->SetScalarMode(VTK_SCALAR_MODE_USE_CELL_FIELD_DATA);
    break;

  case vtkDataObject::FIELD_ASSOCIATION_NONE:
    this->VolumeMapper->SetScalarMode(VTK_SCALAR_MODE_USE_FIELD_DATA);
    break;

  case vtkDataObject::FIELD_ASSOCIATION_POINTS:
  default:
    this->VolumeMapper->SetScalarMode(VTK_SCALAR_MODE_USE_POINT_FIELD_DATA);
    break;
    }

// printf("setting actor mapper to VolumeMapper\n");
 // this->VolumeMapper->PrintSelf(std::cout, vtkIndent());
 vtkOSPRayVolumeRayCastMapper* ospMapper = vtkOSPRayVolumeRayCastMapper::SafeDownCast(this->VolumeMapper);
 // if (ospMapper)
  // printf("using corrrect osp mapper\n");
  this->Actor->SetMapper(this->VolumeMapper);
  // this->Actor->SetLODMapper(this->VolumeMapper);
  // this is necessary since volume mappers don't like empty arrays.
  this->Actor->SetVisibility(colorArrayName != NULL && colorArrayName[0] != 0);
  this->Actor->SetVisibility(1);
  //Carson: Hack!
  // std::cout << "is visible?: " << this->Actor->GetVisibility() << std::endl;
}

//----------------------------------------------------------------------------
void vtkPVOSPRayImageVolumeRepresentation::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);
}


//***************************************************************************
// Forwarded to Actor.

//----------------------------------------------------------------------------
void vtkPVOSPRayImageVolumeRepresentation::SetOrientation(double x, double y, double z)
{
  this->Actor->SetOrientation(x, y, z);
}

//----------------------------------------------------------------------------
void vtkPVOSPRayImageVolumeRepresentation::SetOrigin(double x, double y, double z)
{
  this->Actor->SetOrigin(x, y, z);
}

//----------------------------------------------------------------------------
void vtkPVOSPRayImageVolumeRepresentation::SetPickable(int val)
{
  this->Actor->SetPickable(val);
}
//----------------------------------------------------------------------------
void vtkPVOSPRayImageVolumeRepresentation::SetPosition(double x , double y, double z)
{
  this->Actor->SetPosition(x, y, z);
}
//----------------------------------------------------------------------------
void vtkPVOSPRayImageVolumeRepresentation::SetScale(double x, double y, double z)
{
  this->Actor->SetScale(x, y, z);
}

//----------------------------------------------------------------------------
void vtkPVOSPRayImageVolumeRepresentation::SetVisibility(bool val)
{
  this->Superclass::SetVisibility(val);
  this->Actor->SetVisibility(val? 1 : 0);
}

//***************************************************************************
// Forwarded to vtkVolumeProperty.
//----------------------------------------------------------------------------
void vtkPVOSPRayImageVolumeRepresentation::SetInterpolationType(int val)
{
  this->Property->SetInterpolationType(val);
}

//----------------------------------------------------------------------------
void vtkPVOSPRayImageVolumeRepresentation::SetColor(vtkColorTransferFunction* lut)
{
  this->Property->SetColor(lut);
}

//----------------------------------------------------------------------------
void vtkPVOSPRayImageVolumeRepresentation::SetScalarOpacity(vtkPiecewiseFunction* pwf)
{
  this->Property->SetScalarOpacity(pwf);
}

//----------------------------------------------------------------------------
void vtkPVOSPRayImageVolumeRepresentation::SetScalarOpacityUnitDistance(double val)
{
  this->Property->SetScalarOpacityUnitDistance(val);
}

//----------------------------------------------------------------------------
void vtkPVOSPRayImageVolumeRepresentation::SetAmbient(double val)
{
  this->Property->SetAmbient(val);
}

//----------------------------------------------------------------------------
void vtkPVOSPRayImageVolumeRepresentation::SetDiffuse(double val)
{
  this->Property->SetDiffuse(val);
}

//----------------------------------------------------------------------------
void vtkPVOSPRayImageVolumeRepresentation::SetSpecular(double val)
{
  this->Property->SetSpecular(val);
}

//----------------------------------------------------------------------------
void vtkPVOSPRayImageVolumeRepresentation::SetSpecularPower(double val)
{
  this->Property->SetSpecularPower(val);
}

//----------------------------------------------------------------------------
void vtkPVOSPRayImageVolumeRepresentation::SetShade(bool val)
{
  this->Property->SetShade(val);
}

//----------------------------------------------------------------------------
void vtkPVOSPRayImageVolumeRepresentation::SetIndependantComponents(bool val)
{
  this->Property->SetIndependentComponents(val);
}

//----------------------------------------------------------------------------
void vtkPVOSPRayImageVolumeRepresentation::SetRequestedRenderMode(int mode)
{
  // this->VolumeMapper->SetRequestedRenderMode(mode);
}

