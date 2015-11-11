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

cmake_minimum_required(VERSION 2.8.8)
# MESSAGE("pvOSPRay")
project(pvOSPRay)

set (BUILD_AGAINST_PARAVIEW 1)

if(NOT ParaView_SOURCE_DIR)
  find_package(ParaView REQUIRED)
  include("${PARAVIEW_USE_FILE}")
  include (ParaViewPlugins)

  # Load information about VTK/ParaView modules loaded/available.
  pv_setup_module_environment("pvOSPRay")
else()
  # we're building from within ParaView Source.
    include_directories(${VTK_INCLUDE_DIRS})
endif()

# set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH}
#   ${CMAKE_SOURCE_DIR}/Plugins/pvOSPRay/CMake)
# MESSAGE("pvOSPRay CMAKE_MODULE_PATH " ${CMAKE_MODULE_PATH})
# mark_as_advanced(CMAKE_INSTALL_PREFIX)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH}
  ${PROJECT_SOURCE_DIR}/CMake
  )
# MESSAGE("pvOSPRay CMAKE_MODULE_PATH " ${CMAKE_MODULE_PATH})

# if(NOT CMAKE_BUILD_TYPE)
#   set(CMAKE_BUILD_TYPE "Release" CACHE STRING "Choose the build type: Debug Release RelWithDebInfo MinSizeRel." FORCE)
# endif(NOT CMAKE_BUILD_TYPE)


#
# ospray
# 
find_package(OSPRay REQUIRED)
INCLUDE_DIRECTORIES(${OSPRAY_INCLUDE_DIRS})
#
#
# 
find_package(X11 REQUIRED)
INCLUDE_DIRECTORIES(${X11_X11_INCLUDE_PATH})


if (PARAVIEW_ENABLE_QT_SUPPORT)
  # need to set up Qt stuff here because there are Qt dependencies before
  # ParaView requires this minimum version of Qt, and let's do it here before
  # our first call to FindQt4.cmake
  set (QT_REQUIRED TRUE)

  include (ParaViewQtVersion)
  message("qt")
  if (PARAVIEW_QT_VERSION VERSION_GREATER "4")
    message("including qt5")
    set (Qt5_FIND_COMPONENTS
      Help
      Network
      Test
      UiTools
      Widgets
      Xml)
    if(PARAVIEW_USE_QTWEBKIT)
      list(APPEND Qt5_FIND_COMPONENTS WebKitWidgets)
    endif()
    include (ParaViewQt5)
    if (NOT QT5_FOUND)
      message( SEND_ERROR "Qt ${QT_MIN_VERSION} or greater not found. "
        "Please set the Qt5Core_DIR variable." )
    endif ()
  else ()
    find_package (Qt4)
    if (NOT QT4_FOUND)
      message (SEND_ERROR "Qt ${QT_MIN_VERSION} or greater not found. "
        "Please set the QT_QMAKE_EXECUTABLE variable.")
    else ()
      # check is Qtversion  is 4.8.*. If so, we are good. Otherwise we will post a
      # warning of versions (>= 4.7 && < 4.8). However we report errors for any
      # version less than 4.7
      string(REGEX MATCH "^4\\.[8]\\.[0-9]+" qt_version_match "${QTVERSION}")
      if (NOT qt_version_match)
        string(REGEX MATCH "^4\\.[0-6]+\\.[0-9]+" qt_version46_x_tmp "${QTVERSION}")
        if (qt_version46_x_tmp)
          message(SEND_ERROR "Qt ${QTVERSION} not supported. "
            "Please use ${QT_OFFICIAL_VERSION} (you may need to clean your dirtied cache)."
            "Minium required version is ${QT_MIN_VERSION}.")
        else ()
          message(WARNING "Warning: You are using Qt ${QTVERSION}. "
            "Officially supported version is Qt ${QT_OFFICIAL_VERSION}")
        endif ()
      endif ()
    endif ()
  endif ()
else ()
  message(SEND_ERROR "qt required for pvOSPRay build")
endif ()

#SET(PARAVIEW_QT_QMAKE_EXECUTABLE ${QT_QMAKE_EXECUTABLE})
#FIND_PACKAGE(Qt4 REQUIRED)
#INCLUDE(${QT_USE_FILE})

# -----------------------------------------------------------------------------
# Process all vtk-modules defined in this plugin. This generates the necessary
# wrappings for all these modules as well.
# -----------------------------------------------------------------------------
pv_process_modules()

# -----------------------------------------------------------------------------
# Build the Paraview plugins
# -----------------------------------------------------------------------------
add_subdirectory(ParaView)
