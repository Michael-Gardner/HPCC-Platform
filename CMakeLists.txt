###############################################################################
#    HPCC SYSTEMS software Copyright(C)2012 HPCC Systems®.
#
#    Licensed under the Apache License, Version 2.0(the "License");
#    you may not use this file except in compliance with the License.
#    You may obtain a copy of the License at
#
#       http://www.apache.org/licenses/LICENSE-2.0
#
#    Unless required by applicable law or agreed to in writing, software
#    distributed under the License is distributed on an "AS IS" BASIS,
#    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
#    See the License for the specific language governing permissions and
#    limitations under the License.
################################################################################
# Description:
# ------------
#           This file is the top level handling for #           cmake based compilation and build process.
#
#   To build for Linux:
#   TODO: build directions
################################################################################

project(hpccsystems)

cmake_minimum_required(VERSION 3.8)

set(TOP_LEVEL_PROJECT ON)
if(NOT CMAKE_PROJECT_NAME STREQUAL PROJECT_NAME)
    set(TOP_LEVEL_PROJECT OFF)
endif(NOT CMAKE_PROJECT_NAME STREQUAL PROJECT_NAME)

include(CTest)
enable_testing()

set(HPCC_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR})
set(CMAKE_MODULE_PATH ${HPCC_SOURCE_DIR}/cmake_modules)
include(version.cmake)

###
## Build Level
###
if(NOT BUILD_LEVEL)
    set(BUILD_LEVEL "COMMUNITY")
endif(NOT BUILD_LEVEL)
if ("${BUILD_LEVEL}" STREQUAL "COMMUNITY")
    set ( HPCC_PROJECT_ABBR  "CC")
endif()

include(cmake_modules/functions.cmake)
include(cmake_modules/optionDefaults.cmake)
include(cmake_modules/compilerLinkerSetup.cmake)
include(cmake_modules/commonSetup.cmake)

################################################################################
## Project Subdirectories
add_subdirectory(system)
add_subdirectory(common)
add_subdirectory(tools)
add_subdirectory(esp)
add_subdirectory(rtl)
add_subdirectory(roxie)
add_subdirectory(dali)
add_subdirectory(plugins)
add_subdirectory(ecl)

################################################################################
## Packaging
set(CPACK_GENERATOR "DEB")
set(CPACK_DEB_COMPONENT_INSTALL ON)
set(CPACK_DEBIAN_PACKAGE_SHLIBDEPS ON)

set(CPACK_PACKAGE_CONTACT "HPCCSystems <ossdevelopment@lexisnexis.com>")

set(CPACK_COMPONENT_Runtime_DISPLAY_NAME "Runtime Libraries")
set(CPACK_COMPONENT_Runtime_DESCRIPTION "Runtime libraries for the hpcc systems platform")
set(CPACK_COMPONENT_Runtime_REQUIRED TRUE)

#uninstall target
configure_file(
    "${CMAKE_CURRENT_SOURCE_DIR}/cmake_uninstall.cmake.in"
    "${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake"
    IMMEDIATE @ONLY)
add_custom_target(uninstall
    COMMAND ${CMAKE_COMMAND} -P ${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake)

include(CPack)
include(CPackComponent)
################################################################################
