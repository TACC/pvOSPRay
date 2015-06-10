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
  find_package(ParaView)
  include("${PARAVIEW_USE_FILE}")
  include (ParaViewPlugins)

  # Load information about VTK/ParaView modules loaded/available.
  pv_setup_module_environment("pvOSPRay")
else()
  # we're building from within ParaView Source.
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

FIND_PACKAGE(Qt4 REQUIRED)
INCLUDE(${QT_USE_FILE})

# -----------------------------------------------------------------------------
# Process all vtk-modules defined in this plugin. This generates the necessary
# wrappings for all these modules as well.
# -----------------------------------------------------------------------------
pv_process_modules()

# -----------------------------------------------------------------------------
# Build the Paraview plugins
# -----------------------------------------------------------------------------
add_subdirectory(ParaView)
