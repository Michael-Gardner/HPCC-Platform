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

GENERATE_BASH(processor ${bash-vars} "dafilesrv.in" outFiles)
GENERATE_BASH(processor ${bash-vars} "hpcc-init.in" outFiles)

foreach(iFile
        ${CMAKE_CURRENT_BINARY_DIR}/dafilesrv
        ${CMAKE_CURRENT_BINARY_DIR}/hpcc-init
        ${CMAKE_CURRENT_SOURCE_DIR}/dafilesrv.service
        ${CMAKE_CURRENT_SOURCE_DIR}/dafilesrv@.service
        ${CMAKE_CURRENT_SOURCE_DIR}/hpcc-init.service
        ${CMAKE_CURRENT_SOURCE_DIR}/hpcc-init@.service)
    install(PROGRAMS ${iFILE} DESTINATION ${INIT_DIR} COMPONENT Runtime)
endforeach(iFile)
