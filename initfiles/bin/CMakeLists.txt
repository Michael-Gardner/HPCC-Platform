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

configure_file("init_dafilesrv.in" "init_dafilesrv" @ONLY)
configure_file("init_eclagent.in" "init_eclagent" @ONLY)
configure_file("init_configesp.in" "init_configesp" @ONLY)
configure_file("init_dali.in" "init_dali" @ONLY)
configure_file("init_dfuserver.in" "init_dfuserver" @ONLY)
configure_file("init_eclccserver.in" "init_eclccserver" @ONLY)
configure_file("init_eclscheduler.in" "init_eclscheduler" @ONLY)
configure_file("init_esp.in" "init_esp" @ONLY)
configure_file("init_ftslave.in" "init_ftslave" @ONLY)
configure_file("init_roxie.in" "init_roxie" @ONLY)
configure_file("init_roxie_cluster.in" "init_roxie_cluster" @ONLY)
configure_file("init_sasha.in" "init_sasha" @ONLY)
configure_file("init_thor.in" "init_thor" @ONLY)
configure_file("init_thorslave.in" "init_thorslave" @ONLY)

FOREACH( iFILES
    ${CMAKE_CURRENT_BINARY_DIR}/init_dafilesrv
    ${CMAKE_CURRENT_BINARY_DIR}/init_eclagent
    ${CMAKE_CURRENT_BINARY_DIR}/init_configesp
    ${CMAKE_CURRENT_BINARY_DIR}/init_dali
    ${CMAKE_CURRENT_BINARY_DIR}/init_dfuserver
    ${CMAKE_CURRENT_BINARY_DIR}/init_eclccserver
    ${CMAKE_CURRENT_BINARY_DIR}/init_eclscheduler
    ${CMAKE_CURRENT_BINARY_DIR}/init_esp
    ${CMAKE_CURRENT_BINARY_DIR}/init_ftslave
    ${CMAKE_CURRENT_BINARY_DIR}/init_roxie
    ${CMAKE_CURRENT_BINARY_DIR}/init_roxie_cluster
    ${CMAKE_CURRENT_BINARY_DIR}/init_sasha
    ${CMAKE_CURRENT_BINARY_DIR}/init_thor
    ${CMAKE_CURRENT_BINARY_DIR}/init_thorslave
)
    install ( PROGRAMS ${iFILES} DESTINATION ${BIN_DIR} COMPONENT Runtime )
ENDFOREACH ( iFILES )
