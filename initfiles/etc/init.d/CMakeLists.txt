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

configure_file(dafilesrv.in dafilesrv @ONLY)
configure_file(hpcc-init.in hpcc-init @ONLY)
configure_file(hpcc-common.in hpcc-common @ONLY)

set(initScripts
  ${CMAKE_CURRENT_BINARY_DIR}/dafilesrv
  ${CMAKE_CURRENT_SOURCE_DIR}/dafilesrv.service
  ${CMAKE_CURRENT_SOURCE_DIR}/dafilesrv@.service
  ${CMAKE_CURRENT_BINARY_DIR}/hpcc-init
  ${CMAKE_CURRENT_SOURCE_DIR}/hpcc-init.service
  ${CMAKE_CURRENT_SOURCE_DIR}/hpcc-init@.service)

set(bashScripts
  ${CMAKE_CURRENT_BINARY_DIR}/hpcc-common
  ${CMAKE_CURRENT_SOURCE_DIR}/hpcc-pid
  ${CMAKE_CURRENT_SOURCE_DIR}/hpcc-lock
  ${CMAKE_CURRENT_SOURCE_DIR}/hpcc-init-functions
  ${CMAKE_CURRENT_SOURCE_DIR}//hpcc-export-path)

add_custom_target(ProcessFiles-initfiles-bash-etc-init.d ALL DEPENDS ${initScripts} ${bashScripts})

foreach(iFile ${initScripts})
  install(PROGRAMS ${iFile} DESTINATION /etc/init.d COMPONENT Runtime)
endforeach()

foreach(bFile ${bashScripts})
  install(PROGRAMS ${bFile} DESTINATION ${SBIN_PATH} COMPONENT Runtime)
endforeach()
