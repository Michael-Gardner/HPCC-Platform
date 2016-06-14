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
GENERATE_BASH(processor ${bash-vars} "hpcc_setenv.in" outFiles)
GENERATE_BASH(processor ${bash-vars} "complete-uninstall.sh.in" outFiles)
GENERATE_BASH(processor ${bash-vars} "keygen.sh.in" outFiles)
GENERATE_BASH(processor ${bash-vars} "add_conf_settings.sh.in" outFiles)
GENERATE_BASH(processor ${bash-vars} "rm_conf_settings.sh.in" outFiles)
GENERATE_BASH(processor ${bash-vars} "configmgr.in" outFiles)
GENERATE_BASH(processor ${bash-vars} "install-cluster.sh.in" outFiles)
GENERATE_BASH(processor ${bash-vars} "hpcc-push.sh.in" outFiles)
GENERATE_BASH(processor ${bash-vars} "hpcc-run.sh.in" outFiles)
GENERATE_BASH(processor ${bash-vars} "remote-install-engine.sh.in" outFiles)
GENERATE_BASH(processor ${bash-vars} "deploy-java-files.sh.in" outFiles)
GENERATE_BASH(processor ${bash-vars} "hpcc_common.in" outFiles)


ADD_CUSTOM_TARGET(ProcessFiles-initfiles-sbin ALL DEPENDS ${outFiles})
configure_file("${CMAKE_CURRENT_SOURCE_DIR}/regex.awk.in.cmake" "${CMAKE_CURRENT_BINARY_DIR}/regex.awk")

FOREACH( oFILES
    ${outFiles}
    ${CMAKE_CURRENT_BINARY_DIR}/regex.awk
    ${CMAKE_CURRENT_SOURCE_DIR}/alter_confs.sh
    ${CMAKE_CURRENT_SOURCE_DIR}/cluster_script.py
    ${CMAKE_CURRENT_SOURCE_DIR}/deploy-java-files.exp
    ${CMAKE_CURRENT_SOURCE_DIR}/install-hpcc.exp
    ${CMAKE_CURRENT_BINARY_DIR}/hpcc_common
    ${CMAKE_CURRENT_SOURCE_DIR}/pid.sh
    ${CMAKE_CURRENT_SOURCE_DIR}/lock.sh
    ${CMAKE_CURRENT_SOURCE_DIR}/export-path
    ${CMAKE_CURRENT_SOURCE_DIR}/init-functions
    ${CMAKE_CURRENT_SOURCE_DIR}/lsb-base-logging.sh
)
    install ( PROGRAMS ${oFILES} DESTINATION ${SBIN_DIR} COMPONENT Runtime )
ENDFOREACH ( oFILES )

install( DIRECTORY hpcc DESTINATION ${SBIN_DIR} COMPONENT Runtime
         USE_SOURCE_PERMISSIONS
         PATTERN *.pyc EXCLUDE )
