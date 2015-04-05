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

INCLUDE_DIRECTORIES(../)

ADD_EXECUTABLE(OSPRay_cylinder MACOSX_BUNDLE OSPRay_cylinder.cxx)
TARGET_LINK_LIBRARIES(OSPRay_cylinder vtkOSPRay)

ADD_EXECUTABLE(OSPRay_rendered_file MACOSX_BUNDLE OSPRay_rendered_file.cxx)
TARGET_LINK_LIBRARIES(OSPRay_rendered_file vtkOSPRay)

ADD_EXECUTABLE(OSPRay_rendered_water MACOSX_BUNDLE OSPRay_rendered_water.cxx)
TARGET_LINK_LIBRARIES(OSPRay_rendered_water vtkOSPRay)

