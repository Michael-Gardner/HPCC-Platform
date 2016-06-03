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

configure_file("hpcc_setenv.in" "hpcc_setenv" @ONLY)
configure_file("complete-uninstall.sh.in" "complete-uninstall" @ONLY)
configure_file("keygen.sh.in" "keygen.sh" @ONLY)
configure_file("add_conf_settings.sh.in" "add_conf_settings.sh" @ONLY)
configure_file("rm_conf_settings.sh.in" "rm_conf_settings.sh" @ONLY)
configure_file("configmgr.in" "configmgr" @ONLY)
configure_file("install-cluster.sh.in" "install-cluster.sh" @ONLY)
configure_file("hpcc-push.sh.in" "hpcc-push.sh" @ONLY)
configure_file("hpcc-run.sh.in" "hpcc-run.sh" @ONLY)
configure_file("remote-install-engine.sh.in" "remote-install-engine.sh" @ONLY)
configure_file("deploy-java-files.sh.in" "deploy-java-files.sh" @ONLY)

configure_file("prerm.in" "prerm" @ONLY)

configure_file("regex.awk.in.cmake" "regex.awk" @ONLY)

FOREACH( oFILES
    ${CMAKE_CURRENT_BINARY_DIR}/hpcc_setenv
    ${CMAKE_CURRENT_BINARY_DIR}/complete-uninstall
    ${CMAKE_CURRENT_BINARY_DIR}/keygen.sh
    ${CMAKE_CURRENT_BINARY_DIR}/add_conf_settings.sh
    ${CMAKE_CURRENT_BINARY_DIR}/rm_conf_settings.sh
    ${CMAKE_CURRENT_BINARY_DIR}/configmgr
    ${CMAKE_CURRENT_BINARY_DIR}/install-cluster.sh
    ${CMAKE_CURRENT_BINARY_DIR}/hpcc-push.sh
    ${CMAKE_CURRENT_BINARY_DIR}/hpcc-run.sh
    ${CMAKE_CURRENT_BINARY_DIR}/remote-install-engine.sh
    ${CMAKE_CURRENT_BINARY_DIR}/deploy-java-files.sh
    ${CMAKE_CURRENT_BINARY_DIR}/regex.awk
    ${CMAKE_CURRENT_SOURCE_DIR}/alter_confs.sh
    ${CMAKE_CURRENT_SOURCE_DIR}/cluster_script.py
    ${CMAKE_CURRENT_SOURCE_DIR}/deploy-java-files.exp
    ${CMAKE_CURRENT_SOURCE_DIR}/install-hpcc.exp
)
    install ( PROGRAMS ${oFILES} DESTINATION ${SBIN_DIR} COMPONENT Runtime )
ENDFOREACH ( oFILES )

install( DIRECTORY hpcc DESTINATION ${SBIN_DIR} COMPONENT Runtime
         USE_SOURCE_PERMISSIONS
         PATTERN *.pyc EXCLUDE )
