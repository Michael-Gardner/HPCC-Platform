################################################################################
#    HPCC SYSTEMS software Copyright (C) 2018 HPCC Systems®.
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


# - Try to find the node program
# Once done this will define
#
#  NODE_FOUND - system has the MYSQL library
#  NODE_EXECUTABLE

find_program(NODE_EXECUTABLE node)
find_program(NPM_EXECUTABLE npm)
if(PACKAGE_FIND_VERSION)
    execute_process(COMMAND ${NODE_EXECUTABLE} --version
        OUTPUT_VARIABLE _VERSION
        RESULT_VARIABLE _NODE_RC)
    if(NOT _NODE_RC) # executed successfully
        string(REPLACE "v" "" NODE_VERSION_STRING "${_VERSION}")
        if("${NODE_VERSION_STRING}" VERSION_LESS "${PACKAGE_FIND_VERSION}")
            set(NODE_VERSION NOTFOUND)
            set(NODE_ERROR_MESSAGE "Node version ${NODE_VERSION_STRING} is too old.  Please install NodeJS as per https://github.com/hpcc-systems/HPCC-Platform/wiki/Building-HPCC#prerequisites")
        else()
            set(NODE_VERSION FOUND)
        endif()
    else()
        # running node --version failed
        set(NODE_ERROR_MESSAGE "Unable to locate node/npm.  Please install NodeJS as per https://github.com/hpcc-systems/HPCC-Platform/wiki/Building-HPCC#prerequisites")
    endif()
else()
    set(NODE_VERSION FOUND) # default
endif()

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(NODE "${NODE_ERROR}"
    NODE_EXECUTABLE
    NPM_EXECUTABLE
    NODE_VERSION
    )

mark_as_advanced(NODE_EXECUTABLE NPM_EXECUTABLE)
