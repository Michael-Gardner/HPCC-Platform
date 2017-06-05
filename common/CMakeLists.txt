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
ADD_SUBDIRECTORY (deftype)
ADD_SUBDIRECTORY (dllserver)
ADD_SUBDIRECTORY (environment)
if ("${BUILD_LEVEL}" STREQUAL "INTERNAL")
    ADD_SUBDIRECTORY (fileview2)
else ()
    ADD_SUBDIRECTORY (fileview2)
endif ()
ADD_SUBDIRECTORY (monitoring)
ADD_SUBDIRECTORY (remote)
ADD_SUBDIRECTORY (roxiecommlib)
ADD_SUBDIRECTORY (thorhelper)
ADD_SUBDIRECTORY (workunit)
ADD_SUBDIRECTORY (wuwebview)
