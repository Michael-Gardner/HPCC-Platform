################################################################################
#    HPCC SYSTEMS software Copyright (C) 2016 HPCC Systems®.
#
#    All rights reserved. This program is free software: you can redistribute it
#    and/or modify
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

set(componentList
    dafilesrv
    eclagent
    dali
    thor
    configesp
    dfuserver
    eclccserver
    eclscheduler
    esp
    roxie
    sasha
    thorslave)

set(dafilesrv "dafilesrv" "-L ${LOG_PATH} -I %i -D" "" "Facilitate file transfers across a hpcc cluster.")
set(dali "daserver" "" "" "Stores metadata on queries, workunits, data.")
set(dfuserver "dfuserver" "" "" "data file information store.")
set(eclagent "agentexec" "" "" "ecl workunit controller.")
set(eclccserver "eclccserver" "" "" "compilation server for ecl to c++ to machine code.")
set(eclscheduler "eclscheduler" "" "" "scheduler for ecl workunits.")
#TODO: look into snmpid=$$ for esp processArgs
set(esp "esp" "" "" "Handles remote calls to the hpccsystems-platform.")
set(roxie "roxie" "--topology=RoxieTopology.xml --logfile --restarts=2 --stdlog=0 --daemon --instance %i" "" "Allows for queries of structured data.")
set(sasha "saserver" "" "" "Metadata in-memory store.")
#TODO: thor environment variables
set(thor "thormaster_lcr" "MASTER=\${THORMASTER}:\${THORMASTERPORT}" "" "Coalesces raw data into structured data that can be effectiently served up by roxie.")
set(thorslave "thorslave_lcr" "master=\${master}:\${masterport} slave=.:\${slaveport} slavenum=\${slavenum} slaveprocessnum=\${slave} logDir=${LOG_PATH}/%i" "" "Slave for thor processing")


foreach(component ${componentList})
    list(GET "${component}" 0 processName)
    list(GET "${component}" 1 processArgs)
    list(GET "${component}" 2 dependencies)
    list(GET "${component}" 3 description)
    configure_file(component@instance.service.in ${component}@.service @ONLY)
    install(FILES ${CMAKE_CURRENT_BINARY_DIR}/${component}@.service DESTINATION ${SYSTEMD_PATH} COMPONENT Systemd)
endforeach()

configure_file(hpcc-init.service.in hpcc-init.service @ONLY)
install(PROGRAMS ${CMAKE_CURRENT_BINARY_DIR}/hpcc-init.service DESTINATION ${INIT_PATH} COMPONENT Systemd)
