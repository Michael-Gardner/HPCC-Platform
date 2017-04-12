################################################################################
#    HPCC SYSTEMS software Copyright (C) 2013 HPCC Systems®.
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

# Component: esdlcmd

#####################################################
# Description:
# ------------
#    Cmake Input File for esdlcmd
#####################################################


project( esdlcmd )

include(./../../esp/scm/smcscm.cmake)

set ( SRCS
         ${ESPSCM_GENERATED_DIR}/common_esp.cpp
         ${CMAKE_CURRENT_SOURCE_DIR}/esdlcmd.cpp
         ${CMAKE_CURRENT_SOURCE_DIR}/esdlcmd_shell.cpp
         ${CMAKE_CURRENT_SOURCE_DIR}/esdlcmd_core.cpp
         ${CMAKE_CURRENT_SOURCE_DIR}/esdlcmd_common.cpp
         ${CMAKE_CURRENT_SOURCE_DIR}/esdl2ecl.cpp
         ${CMAKE_CURRENT_SOURCE_DIR}/esdlcmd_monitor.cpp
         ${CMAKE_CURRENT_SOURCE_DIR}/esdl-publish.cpp
         ${ESPSCM_GENERATED_DIR}/ws_esdlconfig_esp.cpp
    )

include_directories (
         ${CMAKE_BINARY_DIR}
         ${CMAKE_BINARY_DIR}/oss
         ./../../system/include
         ./../../system/jlib
         ./../../esp/clients
         ./../../system/security/shared
         ./../../system/mp
         ./../../common/dllserver
         ./../../common/fileview2
         ./../../common/environment
         ./../../esp/bindings
         ./../../esp/bindings/SOAP/xpp
         ./../../esp/platform
         ./../../system/xmllib
         ./../../system/scm
         ./../../system/security/scm
         ./../../esp/platform
         ./../../tools/esdlcomp
         ./../../tools/esdlcmd-xml
         ./../../esp/esdllib
    )

ADD_DEFINITIONS( -D_CONSOLE -DCOMPONENTFILES_DIR="${COMPONENTFILES_PATH}")
if (CMAKE_COMPILER_IS_GNUCXX OR CMAKE_COMPILER_IS_CLANGXX)
    ADD_DEFINITIONS( -O0 )
endif ()

ADD_EXECUTABLE ( esdl ${SRCS} )

add_dependencies ( esdl
                   espscm
                 )

             install ( TARGETS esdl RUNTIME DESTINATION ${EXEC_DIR} COMPONENT tools)
target_link_libraries ( esdl
        jlib
        esphttp
        xmllib
        esdllib
        esdlcomp
    )

if ( UNIX )
    install ( PROGRAMS esdl.install DESTINATION etc/init.d/install COMPONENT Runtime )
    install ( PROGRAMS esdl.uninstall DESTINATION etc/init.d/uninstall COMPONENT Runtime )
endif ( UNIX )
