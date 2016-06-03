################################################################################
#    HPCC SYSTEMS software Copyright (C) 2012 HPCC Systems®.
#
#    Licensed under the Apache License, Version 2.0 (the "License");
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

cmake_minimum_required(VERSION 2.8)
PROJECT(initfiles)

if ( PLATFORM AND UNIX )
    ADD_SUBDIRECTORY(bash)
    ADD_SUBDIRECTORY(bin)
    ADD_SUBDIRECTORY(etc)
    ADD_SUBDIRECTORY(sbin)
    ADD_SUBDIRECTORY(componentfiles)
endif ()

Install ( DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/examples DESTINATION ${INSTALL_PATH} COMPONENT Runtime USE_SOURCE_PERMISSIONS PATTERN ".svn" EXCLUDE )
Install ( DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/classes DESTINATION ${INSTALL_PATH} COMPONENT Runtime USE_SOURCE_PERMISSIONS PATTERN ".svn" EXCLUDE )
