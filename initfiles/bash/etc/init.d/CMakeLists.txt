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


configure_file("dafilesrv.in" dafilesrv @ONLY)
configure_file("install-init.in" install-init @ONLY)
configure_file("hpcc-init.in" hpcc-init @ONLY)
configure_file("hpcc_common.in" hpcc_common @ONLY)
configure_file("uninstall-init.in" uninstall-init @ONLY)

FOREACH( oFILES
    ${CMAKE_CURRENT_BINARY_DIR}/install-init
    ${CMAKE_CURRENT_BINARY_DIR}/hpcc_common
    ${CMAKE_CURRENT_BINARY_DIR}/uninstall-init
    ${CMAKE_CURRENT_SOURCE_DIR}/pid.sh
    ${CMAKE_CURRENT_SOURCE_DIR}/lock.sh
    ${CMAKE_CURRENT_SOURCE_DIR}/init-functions
    ${CMAKE_CURRENT_SOURCE_DIR}/export-path
    ${CMAKE_CURRENT_SOURCE_DIR}/hpcc-init.service
    ${CMAKE_CURRENT_SOURCE_DIR}/hpcc-init@.service
    ${CMAKE_CURRENT_SOURCE_DIR}/dafilesrv.service
    ${CMAKE_CURRENT_SOURCE_DIR}/dafilesrv@.service
)
    install ( PROGRAMS ${oFILES} DESTINATION ${SHARE_PATH}/etc/init.d COMPONENT Runtime )
ENDFOREACH ( oFILES )

foreach( iFILES
    ${CMAKE_CURRENT_BINARY_DIR}/dafilesrv
    ${CMAKE_CURRENT_BINARY_DIR}/hpcc-init
)
    # install directly to /etc/init.d
    install ( PROGRAMS ${iFILES} DESTINATION /etc/init.d COMPONENT Runtime )
endforeach()

if ( PLATFORM )
    install ( PROGRAMS hpcc-init.install DESTINATION ${SHARE_PATH}/etc/init.d/install COMPONENT Runtime )
    install ( PROGRAMS hpcc-init.uninstall DESTINATION ${SHARE_PATH}/etc/init.d/uninstall COMPONENT Runtime )
endif()
