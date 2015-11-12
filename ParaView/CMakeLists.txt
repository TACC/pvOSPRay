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
# message("pvOSPRay_SOURCE_DIR " ${pvOSPRay_SOURCE_DIR})
# message("pvOSPRay_BINARY_DIR " ${pvOSPRay_BINARY_DIR})

IF(PARAVIEW_BUILD_QT_GUI)
  INCLUDE_DIRECTORIES(
    ${PARAVIEW_GUI_INCLUDE_DIRS}
    )
  INCLUDE(${QT_USE_FILE})
  QT4_WRAP_CPP(MOC_SRCS
    OSPRayView.h
    OSPRayDisplay.h
    # OSPRayViewOptions.h
    vtkQtProgressiveRenderer.h
    vtkOSPRayIsosurfaceFilter.h
    )

  QT4_WRAP_UI(UI_SRCS
    OSPRayDisplay.ui
    # OSPRayViewOptions.ui
    )

  # ADD_PARAVIEW_VIEW_MODULE(
  #   VIEW_IFACE VIEW_IFACE_SRCS
  #   VIEW_TYPE OSPRayView
  #   VIEW_NAME OSPRay
  #   VIEW_XML_GROUP views
  #   VIEW_XML_NAME OSPRay
  #   )

  # ADD_PARAVIEW_VIEW_OPTIONS(
  #   OPTIONS_IFACE OPTIONS_IFACE_SRCS
  #   VIEW_TYPE OSPRayView
  #   ACTIVE_VIEW_OPTIONS pqActiveRenderViewOptions
  #   GLOBAL_VIEW_OPTIONS OSPRayViewOptions
  #   )

  # ADD_PARAVIEW_DISPLAY_PANEL_DECORATOR(
  #   DISPLAY_IFACE DISPLAY_IFACE_SRCS
  #   CLASS_NAME OSPRayDisplay
  #   PANEL_TYPES pqDisplayProxyEditor)

ADD_PQPROXY(
    VIEW_IFACE VIEW_IFACE_SRCS
    TYPE OSPRayView
    XML_GROUP views
    XML_NAME OSPRay
    )

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
