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

SET(outFiles)

GENERATE_BASH(processor ${bash-vars} "dafilesrv.in" outFiles)
GENERATE_BASH(processor ${bash-vars} "install-init.in" outFiles)
GENERATE_BASH(processor ${bash-vars} "hpcc-init.in" outFiles)
GENERATE_BASH(processor ${bash-vars} "hpcc_common.in" outFiles)
GENERATE_BASH(processor ${bash-vars} "uninstall-init.in" outFiles)

if(NOT USE_FHS)
    set(__ADMIN_PATH "etc/init.d")
    set(__INIT_PATH "etc/init.d")
else()
    set(__INIT_PATH "/etc/init.d")
endif()

ADD_CUSTOM_TARGET(ProcessFiles-initfiles-bash-etc-init.d ALL DEPENDS ${outFiles})
FOREACH( oFILES
    ${CMAKE_CURRENT_BINARY_DIR}/install-init
    ${CMAKE_CURRENT_BINARY_DIR}/hpcc_common
    ${CMAKE_CURRENT_BINARY_DIR}/uninstall-init
    ${CMAKE_CURRENT_SOURCE_DIR}/pid.sh
    ${CMAKE_CURRENT_SOURCE_DIR}/lock.sh
    ${CMAKE_CURRENT_SOURCE_DIR}/init-functions
    ${CMAKE_CURRENT_SOURCE_DIR}/export-path
)
    install ( PROGRAMS ${oFILES} DESTINATION ${__ADMIN_PATH} COMPONENT Runtime )
ENDFOREACH ( oFILES )

foreach( iFILES
    ${CMAKE_CURRENT_SOURCE_DIR}/hpcc-init.service
    ${CMAKE_CURRENT_SOURCE_DIR}/hpcc-init@.service
    ${CMAKE_CURRENT_SOURCE_DIR}/dafilesrv.service
    ${CMAKE_CURRENT_SOURCE_DIR}/dafilesrv@.service
    ${CMAKE_CURRENT_BINARY_DIR}/dafilesrv
    ${CMAKE_CURRENT_BINARY_DIR}/hpcc-init
)
    install ( PROGRAMS ${iFILES} DESTINATION ${__INIT_PATH} COMPONENT Runtime )
endforeach()

if ( PLATFORM )
    install ( PROGRAMS hpcc-init.install DESTINATION ${SHARE_PATH}/etc/init.d/install COMPONENT Runtime )
    install ( PROGRAMS hpcc-init.uninstall DESTINATION ${SHARE_PATH}/etc/init.d/uninstall COMPONENT Runtime )
endif()
