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

include_directories(
  ${VTK_INCLUDE_DIRS}
  ${pvOSPRay_SOURCE_DIR}/VTK
  ${pvOSPRay_SOURCE_DIR}/VTK/vtkOSPRay
  ${pvOSPRay_BINARY_DIR}/VTK/vtkOSPRay
)

IF(PARAVIEW_BUILD_QT_GUI)
  INCLUDE_DIRECTORIES(
    ${PARAVIEW_GUI_INCLUDE_DIRS}
    )
  SET (pvOSPRay_SRCS
      OSPRayView.h
      OSPRayDisplay.h
      vtkQtProgressiveRenderer.h
      vtkOSPRayIsosurfaceFilter.h
      )
  IF (PARAVIEW_QT_VERSION VERSION_GREATER "4")
    INCLUDE (ParaViewQt5)
    QT5_WRAP_UI(UI_SRCS
      OSPRayDisplay.ui
      )
    QT5_WRAP_CPP(MOC_SRCS
      ${pvOSPRay_SRCS})
  ELSE ()
    INCLUDE (${QT_USE_FILE})
    QT4_WRAP_UI(UI_SRCS
      OSPRayDisplay.ui
      )
    QT4_WRAP_CPP(MOC_SRCS
      ${pvOSPRay_SRCS})
  ENDIF ()

  add_pqproxy(
    VIEW_IFACE VIEW_IFACE_SRCS
    TYPE OSPRayView
    XML_GROUP views
    XML_NAME OSPRay)
ENDIF()

LINK_DIRECTORIES ( ${OSPRAY_BUILD_DIR} )

add_paraview_plugin(
  pvOSPRay "2.0"
  REQUIRED_ON_SERVER
  REQUIRED_ON_CLIENT
  SERVER_MANAGER_SOURCES
    vtkPVOSPRayView.cxx
    vtkSMOSPRayViewProxy.cxx
    vtkOSPRayIsosurfaceFilter.cxx
  SERVER_MANAGER_XML
    OSPRayView.xml
    OSPRayIsosurfaceFilter.xml
  GUI_INTERFACES
    ${VIEW_IFACE} ${OPTIONS_IFACE} ${DISPLAY_IFACE}
  GUI_SOURCES
    ${VIEW_IFACE_SRCS} ${OPTIONS_IFACE_SRCS} ${DISPLAY_IFACE_SRCS} ${UI_SRCS}
    ${MOC_SRCS}
    OSPRayView.cxx
    vtkQtProgressiveRenderer.cxx
    OSPRayDisplay.cxx
    # OSPRayViewOptions.cxx
)
target_link_libraries(pvOSPRay ${OSPRAY_LIBRARIES} ${QT_LIBRARIES} ${VTK_LIBRARIES} ${PARAVIEW_LIBRARIES})

# if (BUILD_TESTING)
  # add_subdirectory(Testing)
# endif()
