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

include(ParaViewTestingMacros)

if (PARAVIEW_DATA_ROOT AND PARAVIEW_BUILD_QT_GUI AND BUILD_SHARED_LIBS)
  add_client_tests("pv"
    LOAD_PLUGIN "pvOSPRay"
    BASELINE_DIR ${PARAVIEW_DATA_ROOT}/Baseline
    TEST_SCRIPTS ${CMAKE_CURRENT_SOURCE_DIR}/OSPRayView.xml
  )
endif()
