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
ADD_SUBDIRECTORY (esdlcomp)
ADD_SUBDIRECTORY (hidl)
ADD_SUBDIRECTORY (esdlcmd-xml)
ADD_SUBDIRECTORY (esdlcmd)
ADD_SUBDIRECTORY (backupnode)
IF (USE_OPENLDAP)
ADD_SUBDIRECTORY (addScopes)
ADD_SUBDIRECTORY (initldap)
ENDIF(USE_OPENLDAP)
ADD_SUBDIRECTORY (combine)
ADD_SUBDIRECTORY (dumpkey)
ADD_SUBDIRECTORY (keydiff)
ADD_SUBDIRECTORY (pstart)
ADD_SUBDIRECTORY (pskill)
ADD_SUBDIRECTORY (testsocket)
ADD_SUBDIRECTORY (swapnode)
ADD_SUBDIRECTORY (vkey)
ADD_SUBDIRECTORY (wuget)
ADD_SUBDIRECTORY (wutool)
ADD_SUBDIRECTORY (copyexp)
ADD_SUBDIRECTORY (genht)

# start-stop-daemon is linux and apple only.
if ( CMAKE_SYSTEM MATCHES Linux OR APPLE)
    message ("-- Building start-stop-daemon")
    ADD_SUBDIRECTORY (start-stop-daemon)
endif()
