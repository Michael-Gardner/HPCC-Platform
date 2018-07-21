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
GENERATE_BASH(processor ${bash-vars} "init_dafilesrv.in" outFiles)
GENERATE_BASH(processor ${bash-vars} "init_eclagent.in" outFiles)
GENERATE_BASH(processor ${bash-vars} "init_dali.in" outFiles)
GENERATE_BASH(processor ${bash-vars} "init_thor.in" outFiles)
GENERATE_BASH(processor ${bash-vars} "init_configesp.in" outFiles)
GENERATE_BASH(processor ${bash-vars} "init_dfuserver.in" outFiles)
GENERATE_BASH(processor ${bash-vars} "init_eclccserver.in" outFiles)
GENERATE_BASH(processor ${bash-vars} "init_eclscheduler.in" outFiles)
GENERATE_BASH(processor ${bash-vars} "init_roxie.in" outFiles)
GENERATE_BASH(processor ${bash-vars} "init_sasha.in" outFiles)
ADD_CUSTOM_TARGET(ProcessFiles-initfiles-bin ALL DEPENDS ${outFiles})

if(USE_SPARK)
    set(spark_start_master "${INSTALL_DIR}/externals/spark-hadoop/sbin/start-master.sh")
    set(spark_stop_master  "${INSTALL_DIR}/externals/spark-hadoop/sbin/stop-master.sh")
    set(spark_start_slave  "${ADMIN_PATH}/spark-slave.sh")
    set(spark_stop_slave   "${INSTALL_DIR}/externals/spark-hadoop/sbin/stop-slave.sh")
endif(USE_SPARK)

configure_file(init_backupnode.in init_backupnode @ONLY)
configure_file(init_esp.in init_esp @ONLY)
configure_file(init_thorslave.in init_thorslave @ONLY)

FOREACH( iFILES
    ${outFiles}
    ${CMAKE_CURRENT_SOURCE_DIR}/init_ftslave
    ${CMAKE_CURRENT_SOURCE_DIR}/init_roxie_cluster
    ${CMAKE_CURRENT_BINARY_DIR}/init_esp
    ${CMAKE_CURRENT_BINARY_DIR}/init_backupnode
    ${CMAKE_CURRENT_BINARY_DIR}/init_thorslave
)
    install ( PROGRAMS ${iFILES} DESTINATION ${EXEC_DIR} COMPONENT Runtime )
ENDFOREACH ( iFILES )
