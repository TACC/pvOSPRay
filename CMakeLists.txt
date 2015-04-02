#/***********************************************************************************************************
#Copyright (c) 2015, Carson Brownlee, Texas Advanced Computing Center, Universtiy of Texas at Austin
#All rights reserved.
#
#Redistribution and use in source and binary forms, with or without modification, are permitted provided 
#that the following conditions are met:
#
#1. Redistributions of source code must retain the above copyright notice, this list of conditions and the
# following disclaimer.
#
#2. Redistributions in binary form must reproduce the above copyright notice, this list of conditions and 
#the following disclaimer in the documentation and/or other materials provided with the distribution.
#
#3. Neither the name of the copyright holder nor the names of its contributors may be used to endorse or
# promote products derived from this software without specific prior written permission.
#
#THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED
#WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A
#PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE 
#FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
#LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS 
#INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
#OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN 
#IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
#***********************************************************************************************************/



cmake_minimum_required(VERSION 2.8.8)
MESSAGE("pvOSPRay")
project(OSPRayView)

# FIXME: this is ABSURD!!! Needs to be removed. I don't have time right now.
set (BUILD_AGAINST_PARAVIEW 1)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH}
  "${CMAKE_SOURCE_DIR}/Plugins/pvOSPRay/CMake/")
MESSAGE("pvOSPRay CMAKE_MODULE_PATH" ${CMAKE_MODULE_PATH})


#
# ospray
# 
find_package(OSPRay)
INCLUDE_DIRECTORIES(${OSPRAY_INCLUDE_DIRS})
#
#
# 


# -----------------------------------------------------------------------------
# Process all vtk-modules defined in this plugin. This generates the necessary
# wrappings for all these modules as well.
# -----------------------------------------------------------------------------
pv_process_modules()

# -----------------------------------------------------------------------------
# Build the Paraview plugins
# -----------------------------------------------------------------------------
add_subdirectory(ParaView)
