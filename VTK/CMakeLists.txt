## ======================================================================================= ##
## Copyright 2014-2015 Texas Advanced Computing Center, The University of Texas at Austin  ##
## All rights reserved.                                                                    ##
##                                                                                         ##
## Licensed under the BSD 3-Clause License, (the "License"); you may not use this file     ##
## except in compliance with the License.                                                  ##
## A copy of the License is included with this software in the file LICENSE.               ##
## If your copy does not contain the License, you may obtain a copy of the License at:     ##
##                                                                                         ##
##     http://opensource.org/licenses/BSD-3-Clause                                         ##
##                                                                                         ##
## Unless required by applicable law or agreed to in writing, software distributed under   ##
## the License is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY ##
## KIND, either express or implied.                                                        ##
## See the License for the specific language governing permissions and limitations under   ##
## limitations under the License.                                                          ##
## ======================================================================================= ##

# vtkOSPRay is an interface from VTK's rendering classes to OSPRay,
# a "real time" Ray Tracer.

# It creates a library that includes OSPRay specific subclasses
# of VTK's rendering classes, Renderer, Camera, etc.
# These can be instantiated by name, or by using object factory overloading
# at post compile time (ie, set VTK_AUTOLOAD_PATH to vtkOSPRay's build
# directory and run a normal vtk app)

project(vtkOSPRay)



#Configure vtkOSPRay source proper ############################################

# Define a flag
if (BUILD_AGAINST_PARAVIEW)
  add_definitions(-DVTKOSPRAY_FOR_PARAVIEW)
endif(BUILD_AGAINST_PARAVIEW)

# Set vtkOSPRay source files.
SET ( OSPRAY_CLASSES
  vtkOSPRay/vtkOSPRayActor.cxx
  vtkOSPRay/vtkOSPRayCamera.cxx
  vtkOSPRay/vtkOSPRayLight.cxx
  vtkOSPRay/vtkOSPRayManager.cxx
  vtkOSPRay/vtkOSPRayPolyDataMapper.cxx
  vtkOSPRay/vtkOSPRayProperty.cxx
  vtkOSPRay/vtkOSPRayRenderer.cxx
  vtkOSPRay/vtkOSPRayTestSource.cxx
  vtkOSPRay/vtkOSPRayTexture.cxx
  vtkOSPRay/vtkOSPRayVolumeRayCastMapper.h
  vtkOSPRay/vtkOSPRayVolumeRayCastMapper.cxx
  vtkOSPRayPVLODVolume.cxx
  vtkOSPRayAMRVolumeRepresentation.h
  vtkOSPRayAMRVolumeRepresentation.cxx
  )

IF (BUILD_AGAINST_PARAVIEW)
  add_definitions(-DVTKOSPRAY_FOR_PARAVIEW)
  # paraview uses its own LOD actor, put it in here
  SET ( OSPRAY_CLASSES ${OSPRAY_CLASSES}
    vtkOSPRayLODActor.cxx
    vtkPVOSPRayRepresentation.cxx
    vtkPVOSPRayRepresentation.h
    vtkPVOSPRayOutlineRepresentation.cxx
    vtkPVOSPRayOutlineRepresentation.h
    vtkPVOSPRayImageVolumeRepresentation.cxx
    vtkPVOSPRayImageVolumeRepresentation.h
    vtkOSPRayCompositeMapper.cxx)
ENDIF (BUILD_AGAINST_PARAVIEW)

add_definitions(
  -DvtkCommonCore_EXPORTS
  -DvtkPVClientServerCoreRendering_EXPORTS
  -DvtkRenderingVolume_EXPORTS
  -DvtkPVVTKExtensionsRendering_EXPORTS
  )

set (${vtk-module}_NO_HeaderTest 1)

set(OSPRAY_SRCS
 ${OSPRAY_CLASSES}
 )

vtk_module_library(${vtk-module} ${OSPRAY_SRCS} ${MOCSrcs} ${UISrcs})
find_package(OSPRay)

# setup include paths
INCLUDE_DIRECTORIES(${CMAKE_CURRENT_BINARY_DIR})
INCLUDE_DIRECTORIES(${PROJECT_SOURCE_DIR})
INCLUDE_DIRECTORIES(${PROJECT_SOURCE_DIR}/vtkOSPRay)
INCLUDE_DIRECTORIES(${PROJECT_BINARY_DIR})
INCLUDE_DIRECTORIES(${PROJECT_BINARY_DIR}/vtkOSPRay)
INCLUDE_DIRECTORIES(${PROJECT_BINARY_DIR}/Base)
INCLUDE_DIRECTORIES(${PROJECT_SOURCE_DIR}/Base)

if(VTK_USE_X)
 vtk_module_link_libraries(${vtk-module}
   LINK_PRIVATE ${OSPRAY_LIBRARIES} ${QT_LIBRARIES} ${X11_LIBRARIES} ${X11_Xt_LIB})
elseif(VTK_USE_COCOA)
  vtk_module_link_libraries(${vtk-module} LINK_PRIVATE ${OSPRAY_LIBRARIES} ${QT_LIBRARIES} "-framework OpenGL -framework Cocoa")
  target_link_libraries(${vtk-module} LINK_PRIVATE ${OSPRAY_LIBRARIES} ${QT_LIBRARIES} ${VTK_LIBRARIES})
endif()

# OSPRay_Core uses MMTime which is in it's own special library.
if(WIN32)
  TARGET_LINK_LIBRARIES(${PROJECT_NAME} LINK_PRIVATE Winmm)
endif()

include(CheckCXXCompilerFlag)
CHECK_CXX_COMPILER_FLAG("-std=c++11" CXX11_SUPPORTED)
CHECK_CXX_COMPILER_FLAG("-std=c++0x" CXX0X_SUPPORTED)
if(CXX11_SUPPORTED)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
elseif(CXX0X_SUPPORTED)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++0x")
else()
  message(ERROR "OSPRay requires c++11 support.  The compiler ${CMAKE_CXX_COMPILER} does not appear to support it.")
endif()
