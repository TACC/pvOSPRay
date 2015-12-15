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
#include "vtkOSPRayCamera.h"
#include "vtkOSPRayManager.h"
#include "vtkOSPRayRenderer.h"

#include "vtkObjectFactory.h"

#include "vtkHomogeneousTransform.h"
#include "vtkMatrix4x4.h"
//#include "vtkTransform.h"

#include <math.h>

#ifndef __APPLE__
#include <GL/glu.h>
#else
#include <OpenGL/glu.h>
#endif

vtkStandardNewMacro(vtkOSPRayCamera);

//----------------------------------------------------------------------------

vtkOSPRayCamera::vtkOSPRayCamera()
{
  this->OSPRayManager = NULL;
  this->debugFlag = 0;//set to 1 for printing debug stuff
}

//----------------------------------------------------------------------------
  vtkOSPRayCamera::~vtkOSPRayCamera()
  {
    if (this->OSPRayManager)
    {
      this->OSPRayManager->Delete();
    }
  }

//----------------------------------------------------------------------------
void vtkOSPRayCamera::OrientOSPRayCamera(vtkRenderer *ren)
{
    vtkOSPRayRenderer * OSPRayRenderer = vtkOSPRayRenderer::SafeDownCast(ren);
    if (!OSPRayRenderer)
    {
      return;
    }
    OSPRayRenderer->ClearAccumulation();

    if (!this->OSPRayManager)
    {
    this->OSPRayManager = OSPRayRenderer->GetOSPRayManager();
    this->OSPRayManager->Register(this);
    this->OSPRayManager->stereoCamera = this;
    }
  this->SetupCameraShift();
  this->ShiftCamera();

  // for figuring out aspect ratio
    int lowerLeft[2];
    int usize, vsize;
    ren->GetTiledSizeAndOrigin(&usize, &vsize, lowerLeft, lowerLeft + 1);

    double *eye, *lookat, *up, vfov;
    eye    = this->Position;
    lookat = this->FocalPoint;
    up     = this->ViewUp;
    vfov   = this->ViewAngle;

  if (this->debugFlag){ cout << "OrientOSPRayCamera(): eye( " << eye[0] << ", " << eye[1] << ", " << eye[2] << ")" << endl; }

  double E[4] = { 0.0, 0.0, 0.0, 1.0 };

  E[0] = eye[0];
  E[1] = eye[1];
  E[2] = eye[2];

  if (this->debugFlag){ cout << "OrientOSPRayCamera(): E1( " << E[0] << ", " << E[1] << ", " << E[2] << ", " << E[3] << ")" << endl; }

  // First transform the eye to new position.
  this->EyeTransformMatrix->MultiplyPoint(E, E);

  if (this->debugFlag){ cout << "OrientOSPRayCamera(): E2( " << E[0] << ", " << E[1] << ", " << E[2] << ", " << E[3] << ")" << endl; }

  // Now transform the eye into the screen coordinate system.
  //this->WorldToScreenMatrix->MultiplyPoint(E, E);

  //cout << "OrientOSPRayCamera(): E3( " << E[0] << ", " << E[1] << ", " << E[2] << ", " << E[3] << ")" << endl;

  OSPCamera ospCamera = ((OSPCamera)this->OSPRayManager->OSPRayCamera);
    if (vsize == 0)
      return;
  ospSetf(ospCamera,"aspect",float(usize)/float(vsize));
  ospSetf(ospCamera,"fovy",vfov);
  Assert(ospCamera != NULL && "could not create camera");
  ospSet3f(ospCamera,"pos",eye[0], eye[1], eye[2]);
  ospSet3f(ospCamera,"up",up[0], up[1], up[2]);
  ospSet3f(ospCamera,"dir",lookat[0]-eye[0],lookat[1]-eye[1],lookat[2]-eye[2]);
  ospCommit(ospCamera);

  double pos[3];
  pos[0] = eye[0];
  pos[1] = eye[1];
  pos[2] = eye[2];
  pos[0] = E[0];
  pos[1] = E[1];
  pos[2] = E[2];
  if (this->debugFlag){ cout << "OrientOSPRayCamera(): pos( " << pos[0] << ", " << pos[1] << ", " << pos[2] << ")" << endl; }

  ospSet3f(ospCamera, "pos", pos[0], pos[1], pos[2]);
  ospSet3f(ospCamera,"up",up[0], up[1], up[2]);
  double dir[3];
  dir[0] = lookat[0]-pos[0];
  dir[1] = lookat[1]-pos[1];
  dir[2] = lookat[2]-pos[2];
  ospSet3f(ospCamera,"dir",dir[0],dir[1],dir[2]);
  ospCommit(ospCamera);
  this->UnShiftCamera();
}

void vtkOSPRayCamera::SetupCameraShift(){
  if (this->debugFlag){ cout << " vtkOSPRayCamera::SetupCameraShift()" << endl; }
  int myLeftEye = this->GetLeftEye();
  double myEyeSeparation = this->GetEyeSeparation();
  int stereoFlag = 1;//this flag turns stereo on or off

  /*
  double myEyePosition[3];
  this->GetEyePosition(myEyePosition);
  cout << "vtkOSPRayCamera::SetEyeTransformMatrix(): myEyePosition set to ( " << myEyePosition[0] << ", " << myEyePosition[1] << ", " << myEyePosition[2] << ")" << endl;
  vtkMatrix4x4 * myEyeTransformationMatrix = this->GetEyeTransformMatrix();
  vtkTransform * myTempEyeTransform = vtkTransform::New();
  myTempEyeTransform->SetMatrix(myEyeTransformationMatrix);
  myTempEyeTransform->Inverse();
  vtkMatrix4x4* myInverseMatrix = myTempEyeTransform->GetMatrix();
  */

  //double myEyePosition[3];
  //this->GetEyePosition(myEyePosition);
  //cout << "vtkOSPRayCamera::SetupCameraShift(): myEyePosition set to ( " << myEyePosition[0] << ", " << myEyePosition[1] << ", " << myEyePosition[2] << ")" << endl;

  double *eye, *lookat, *up, vfov;
  eye    = this->Position;

  SavedCameraPosition[0] = eye[0];
  SavedCameraPosition[1] = eye[1];
  SavedCameraPosition[2] = eye[2];
  //SavedCameraPosition[0] = myEyePosition[0];
  //SavedCameraPosition[1] = myEyePosition[1];
  //SavedCameraPosition[2] = myEyePosition[2];

  double myDistance = this->Distance;
  double shiftDistance = 0.0;
  double myScaledEyeSeparation = 0.0;
  if (this->debugFlag){ cout << "SavedCameraPosition set to ( " << SavedCameraPosition[0] << ", " << SavedCameraPosition[1] << ", " << SavedCameraPosition[2] << ")" << endl; }

  if(stereoFlag){
    myScaledEyeSeparation = myEyeSeparation * myDistance;
    shiftDistance = (myScaledEyeSeparation / 2);
    if(myLeftEye==1){
        ShiftedCameraPosition[0] = SavedCameraPosition[0] - shiftDistance;
    } else {
        ShiftedCameraPosition[0] = SavedCameraPosition[0] + shiftDistance;
    }
  } else {
    ShiftedCameraPosition[0] = SavedCameraPosition[0];
  }
  ShiftedCameraPosition[1] = SavedCameraPosition[1];
  ShiftedCameraPosition[2] = SavedCameraPosition[2];
  if (this->debugFlag){ cout << "ShiftedCameraPosition set to ( " << ShiftedCameraPosition[0] << ", " << ShiftedCameraPosition[1] << ", " << ShiftedCameraPosition[2] << ")" << endl; }
  }

void vtkOSPRayCamera::ShiftCamera(){
	if (this->debugFlag){ cout << " vtkOSPRayCamera::ShiftCamera()" << endl; }
  if ((ShiftedCameraPosition[0] != this->Position[0]) ||
        (ShiftedCameraPosition[1] != this->Position[1]) ||
        (ShiftedCameraPosition[2] != this->Position[2])) {
 
        this->Position[0] = ShiftedCameraPosition[0];
        this->Position[1] = ShiftedCameraPosition[1];
        this->Position[2] = ShiftedCameraPosition[2];

		if (this->debugFlag){ cout << " Position set to ( " << this->Position[0] << ", " << this->Position[1] << ", " << this->Position[2] << ")" << endl; }

        this->ComputeViewTransform();
        // recompute the focal distance
        this->ComputeDistance();
        this->ComputeCameraLightTransform();

        double *eye, *lookat, *up, vfov;
        lookat = this->FocalPoint;
        up     = this->ViewUp;
        vfov   = this->ViewAngle;
        eye    = this->Position;
    }
}

void vtkOSPRayCamera::UnShiftCamera(){
  if (this->debugFlag){ cout << " vtkOSPRayCamera::UnShiftCamera()" << endl; }
  if ((SavedCameraPosition[0] != this->Position[0]) ||
        (SavedCameraPosition[1] != this->Position[1]) ||
        (SavedCameraPosition[2] != this->Position[2])) {

        this->Position[0] = SavedCameraPosition[0];
        this->Position[1] = SavedCameraPosition[1];
        this->Position[2] = SavedCameraPosition[2];

		if (this->debugFlag){ cout << " Position set to ( " << this->Position[0] << ", " << this->Position[1] << ", " << this->Position[2] << ")" << endl; }

        this->ComputeViewTransform();
        // recompute the focal distance
        this->ComputeDistance();
        this->ComputeCameraLightTransform();

        double *eye, *lookat, *up, vfov;
        lookat = this->FocalPoint;
        up     = this->ViewUp;
        vfov   = this->ViewAngle;
        eye    = this->Position;
   }
}

//----------------------------------------------------------------------------
// called by Renderer::UpdateCamera()
  void vtkOSPRayCamera::Render(vtkRenderer *ren)
  {
    int lowerLeft[2];
    int usize, vsize;
    ren->GetTiledSizeAndOrigin(&usize, &vsize, lowerLeft, lowerLeft + 1);
    double newAspect = float(usize)/float(vsize);
    if (this->GetMTime() > this->LastRenderTime || (newAspect != this->Aspect) )
    {
      this->Aspect = newAspect;
      this->OrientOSPRayCamera(ren);

      this->LastRenderTime.Modified();

    }
}

void vtkOSPRayCamera::SetPosition(double x, double y, double z)
{
	this->Superclass::SetPosition(x, y, z);
	if (this->debugFlag){ cout << " Position set to ( " << x << ", " << y << ", " << z << ")" << endl; }

	double myEyePostion[3];
	myEyePostion[0] = x;
	myEyePostion[1] = y;
	myEyePostion[2] = z;
	//this->SetEyePosition(myEyePostion);
}

void vtkOSPRayCamera::SetFocalPoint(double x, double y, double z)
{
	this->Superclass::SetFocalPoint(x, y, z);
	if (this->debugFlag){ cout << " FocalPoint set to ( " << x << ", " << y << ", " << z << ")" << endl; }
}

void vtkOSPRayCamera::SetViewUp(double vx, double vy, double vz)
{
	this->Superclass::SetViewUp(vx, vy, vz);
	if (this->debugFlag){ cout << " ViewUp set to ( " << vx << ", " << vy << ", " << vz << ")" << endl; }
}

void vtkOSPRayCamera::SetViewAngle(double angle)
{
	this->Superclass::SetViewAngle(angle);
	if (this->debugFlag){ cout << " ViewAngel set to ( " << angle << ")" << endl; }
}

void vtkOSPRayCamera::SetParallelScale(double scale)
{
	this->Superclass::SetParallelScale(scale);
	if (this->debugFlag){ cout << " ParallelScale set to ( " << scale << ")" << endl; }
}

void vtkOSPRayCamera::SetClippingRange(double dNear, double dFar)
{
	this->Superclass::SetClippingRange(dNear, dFar);
	if (this->debugFlag){ cout << " ClippingRange set to ( " << dNear << ", " << dFar << ")" << endl; }
}

void vtkOSPRayCamera::SetThickness(double thickness)
{
	this->Superclass::SetThickness(thickness);
	if (this->debugFlag){ cout << " Thickness set to ( " << thickness << ")" << endl; }
}

void vtkOSPRayCamera::SetWindowCenter(double x, double y)
{
	this->Superclass::SetWindowCenter(x, y);
	if (this->debugFlag){ cout << " WindowCenter set to ( " << x << ", " << y << ")" << endl; }
}

void vtkOSPRayCamera::SetObliqueAngles(double alpha, double beta)
{
	this->Superclass::SetObliqueAngles(alpha, beta);
	if (this->debugFlag){ cout << " ObliqueAngles set to ( " << alpha << ", " << beta << ")" << endl; }
}

void vtkOSPRayCamera::SetViewShear(double dxdz, double dydz, double center)
{
	this->Superclass::SetViewShear(dxdz, dydz, center);
	if (this->debugFlag){ cout << " ViewShear set to ( " << dxdz << ", " << dydz << ", " << center << ")" << endl; }
}

void vtkOSPRayCamera::SetEyePosition(double eyePosition[3])
{
	this->Superclass::SetEyePosition(eyePosition);
	if (this->debugFlag){ cout << " EyePosition set to ( " << eyePosition[0] << ", " << eyePosition[1] << ", " << eyePosition[2] << ")" << endl; }
}

void vtkOSPRayCamera::SetEyeTransformMatrix(vtkMatrix4x4* matrix)
{
	if (this->debugFlag){ cout << "vtkOSPRayCamera::SetEyeTransformMatrix(vtkMatrix4x4* matrix)" << endl; }
	this->Superclass::SetEyeTransformMatrix(matrix);
	if (this->debugFlag){
		cout << " EyeTransformMatrix set to ( " << matrix->GetElement(0, 0) << ", " << matrix->GetElement(0, 1) << ", " << matrix->GetElement(0, 2) << ", " << matrix->GetElement(0, 3) << ")" << endl;
		cout << " EyeTransformMatrix set to ( " << matrix->GetElement(1, 0) << ", " << matrix->GetElement(1, 1) << ", " << matrix->GetElement(1, 2) << ", " << matrix->GetElement(1, 3) << ")" << endl;
		cout << " EyeTransformMatrix set to ( " << matrix->GetElement(2, 0) << ", " << matrix->GetElement(2, 1) << ", " << matrix->GetElement(2, 2) << ", " << matrix->GetElement(2, 3) << ")" << endl;
		cout << " EyeTransformMatrix set to ( " << matrix->GetElement(3, 0) << ", " << matrix->GetElement(3, 1) << ", " << matrix->GetElement(3, 2) << ", " << matrix->GetElement(3, 3) << ")" << endl;
	}
}

void vtkOSPRayCamera::SetEyeTransformMatrix(const double elements[16])
{
	if (this->debugFlag){ cout << "vtkOSPRayCamera::SetEyeTransformMatrix(const double elements[16])" << endl; }
	this->Superclass::SetEyeTransformMatrix(elements);
	if (this->debugFlag){
		cout << " EyeTransformMatrix set to ( " << elements[0] << ", " << elements[1] << ", " << elements[2] << ", " << elements[3] << ")" << endl;
		cout << " EyeTransformMatrix set to ( " << elements[4] << ", " << elements[5] << ", " << elements[6] << ", " << elements[7] << ")" << endl;
		cout << " EyeTransformMatrix set to ( " << elements[8] << ", " << elements[9] << ", " << elements[10] << ", " << elements[11] << ")" << endl;
		cout << " EyeTransformMatrix set to ( " << elements[12] << ", " << elements[13] << ", " << elements[14] << ", " << elements[15] << ")" << endl;
	}
	this->Modified();
}

void vtkOSPRayCamera::SetModelTransformMatrix(vtkMatrix4x4* matrix)
{
	if (this->debugFlag){ cout << "vtkOSPRayCamera::SetModelTransformMatrix(vtkMatrix4x4* matrix)" << endl; }
	this->Superclass::SetModelTransformMatrix(matrix);
	if (this->debugFlag){
		cout << " ModelTransformMatrix set to ( " << matrix->GetElement(0, 0) << ", " << matrix->GetElement(0, 1) << ", " << matrix->GetElement(0, 2) << ", " << matrix->GetElement(0, 3) << ")" << endl;
		cout << " ModelTransformMatrix set to ( " << matrix->GetElement(1, 0) << ", " << matrix->GetElement(1, 1) << ", " << matrix->GetElement(1, 2) << ", " << matrix->GetElement(1, 3) << ")" << endl;
		cout << " ModelTransformMatrix set to ( " << matrix->GetElement(2, 0) << ", " << matrix->GetElement(2, 1) << ", " << matrix->GetElement(2, 2) << ", " << matrix->GetElement(2, 3) << ")" << endl;
		cout << " ModelTransformMatrix set to ( " << matrix->GetElement(3, 0) << ", " << matrix->GetElement(3, 1) << ", " << matrix->GetElement(3, 2) << ", " << matrix->GetElement(3, 3) << ")" << endl;
	}
}

void vtkOSPRayCamera::SetModelTransformMatrix(const double elements[16])
{
	if (this->debugFlag){ cout << "vtkOSPRayCamera::SetModelTransformMatrix(const double elements[16])" << endl; }
	this->Superclass::SetModelTransformMatrix(elements);
	if (this->debugFlag){
		cout << " ModelTransformMatrix set to ( " << elements[0] << ", " << elements[1] << ", " << elements[2] << ", " << elements[3] << ")" << endl;
		cout << " ModelTransformMatrix set to ( " << elements[4] << ", " << elements[5] << ", " << elements[6] << ", " << elements[7] << ")" << endl;
		cout << " ModelTransformMatrix set to ( " << elements[8] << ", " << elements[9] << ", " << elements[10] << ", " << elements[11] << ")" << endl;
		cout << " ModelTransformMatrix set to ( " << elements[12] << ", " << elements[13] << ", " << elements[14] << ", " << elements[15] << ")" << endl;
	}
}

void vtkOSPRayCamera::SetUserViewTransform(vtkHomogeneousTransform* transform)
{
	if (this->debugFlag){ cout << "vtkOSPRayCamera::SetUserViewTransform(vtkHomogeneousTransform* transform)" << endl; }
	this->Superclass::SetUserViewTransform(transform);
	vtkMatrix4x4* matrix = transform->GetMatrix();
	if (this->debugFlag){
		cout << " UserViewTransform set to ( " << matrix->GetElement(0, 0) << ", " << matrix->GetElement(0, 1) << ", " << matrix->GetElement(0, 2) << ", " << matrix->GetElement(0, 3) << ")" << endl;
		cout << " UserViewTransform set to ( " << matrix->GetElement(1, 0) << ", " << matrix->GetElement(1, 1) << ", " << matrix->GetElement(1, 2) << ", " << matrix->GetElement(1, 3) << ")" << endl;
		cout << " UserViewTransform set to ( " << matrix->GetElement(2, 0) << ", " << matrix->GetElement(2, 1) << ", " << matrix->GetElement(2, 2) << ", " << matrix->GetElement(2, 3) << ")" << endl;
		cout << " UserViewTransform set to ( " << matrix->GetElement(3, 0) << ", " << matrix->GetElement(3, 1) << ", " << matrix->GetElement(3, 2) << ", " << matrix->GetElement(3, 3) << ")" << endl;
	}
}

void vtkOSPRayCamera::SetUserTransform(vtkHomogeneousTransform* transform)
{
	if (this->debugFlag){ cout << "vtkOSPRayCamera::SetUserTransform(vtkHomogeneousTransform* transform)" << endl; }
	this->Superclass::SetUserTransform(transform);
	vtkMatrix4x4* matrix = transform->GetMatrix();
	if (this->debugFlag){
		cout << " UserTransform set to ( " << matrix->GetElement(0, 0) << ", " << matrix->GetElement(0, 1) << ", " << matrix->GetElement(0, 2) << ", " << matrix->GetElement(0, 3) << ")" << endl;
		cout << " UserTransform set to ( " << matrix->GetElement(1, 0) << ", " << matrix->GetElement(1, 1) << ", " << matrix->GetElement(1, 2) << ", " << matrix->GetElement(1, 3) << ")" << endl;
		cout << " UserTransform set to ( " << matrix->GetElement(2, 0) << ", " << matrix->GetElement(2, 1) << ", " << matrix->GetElement(2, 2) << ", " << matrix->GetElement(2, 3) << ")" << endl;
		cout << " UserTransform set to ( " << matrix->GetElement(3, 0) << ", " << matrix->GetElement(3, 1) << ", " << matrix->GetElement(3, 2) << ", " << matrix->GetElement(3, 3) << ")" << endl;
	}
}
