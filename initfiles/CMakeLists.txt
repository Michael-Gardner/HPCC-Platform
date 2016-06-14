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
project(initfiles)

if(PLATFORM AND UNIX)
    add_subdirectory(packaging)
    add_subdirectory(bin)
    add_subdirectory(etc)
    add_subdirectory(sbin)
    add_subdirectory(componentfiles)
    add_subdirectory(systemd)
endif()

install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/examples DESTINATION ${EXAMPLE_DIR} COMPONENT Runtime USE_SOURCE_PERMISSIONS PATTERN ".svn" EXCLUDE)
install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/classes DESTINATION ${CLASS_DIR} COMPONENT Runtime USE_SOURCE_PERMISSIONS PATTERN ".svn" EXCLUDE)
