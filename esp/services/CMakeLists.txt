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
ADD_SUBDIRECTORY (common)
ADD_SUBDIRECTORY (ecldirect)
IF (USE_OPENLDAP)
    ADD_SUBDIRECTORY (ws_access)
ENDIF(USE_OPENLDAP)
ADD_SUBDIRECTORY (ws_account)
ADD_SUBDIRECTORY (ws_config)
ADD_SUBDIRECTORY (ws_dfu)
ADD_SUBDIRECTORY (ws_ecl)
ADD_SUBDIRECTORY (ws_fileio)
ADD_SUBDIRECTORY (ws_fs)
ADD_SUBDIRECTORY (ws_machine)
ADD_SUBDIRECTORY (ws_smc)
ADD_SUBDIRECTORY (ws_topology)
ADD_SUBDIRECTORY (ws_workunits)
ADD_SUBDIRECTORY (WsDeploy)
ADD_SUBDIRECTORY (ws_packageprocess)
ADD_SUBDIRECTORY (ws_esdlconfig)
ADD_SUBDIRECTORY (esdl_svc_engine)
if (LOGGING_SERVICE)
ADD_SUBDIRECTORY (ws_loggingservice)
endif()
ADD_SUBDIRECTORY (espcontrol)
