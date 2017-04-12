###############################################################################
#    HPCC SYSTEMS software Copyright (C) 2017 HPCC Systems®.
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

project(hpccsystems-platform)

if(UNIX AND NOT APPLE)
    cmake_minimum_required(VERSION 3.3.2)
else()
    cmake_minimum_required(VERSION 2.8.11)
endif()

include(CTest)
enable_testing()

set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake_modules/")

include(version.cmake)

###
## Build Level
###
if(NOT BUILD_LEVEL)
    set(BUILD_LEVEL "COMMUNITY")
endif(NOT BUILD_LEVEL)
if ("${BUILD_LEVEL}" STREQUAL "COMMUNITY")
    set ( HPCC_PROJECT_ABBR  "CC")
endif()

## Config Block
###
option(PREFIX "Set the install prefix")
option(EXEC_PREFIX "Set the execution prefix")
option(CONFIG_PREFIX "Set the configuration prefix")
option(DIR_NAME "Set the install directory name")
option(LIB_DIR "Set the library install dir")
option(EXEC_DIR "Set the executable install dir")
option(COMPONENTFILES_DIR "Set the componentfiles dir")
option(ADMIN_DIR "Set the admin dir")
option(PLUGINS_DIR "Set the plugins dir")
option(CONFIG_SOURCE_DIR "Set the configuration source dir")
option(RUNTIME_DIR "Set the runtime dir")
option(HOME_DIR "Set the home dir")
option(LOCK_DIR "Set the lock dir")
option(PID_DIR "Set the pid dir")
option(LOG_DIR "Set the log dir")
option(RUNTIME_USER "Set the runtime username")
option(RUNTIME_GROUP "Set the runtime group")
option(ENV_XML_FILE "Set the environment xml file name.")
option(ENV_CONF_FILE "Set the environment conf file name.")
option(LICENSE_FILE "Set the license file to use.")
option(DESTDIR "Set the alternate root installation path.")

include(${CMAKE_MODULE_PATH}optionDefaults.cmake)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_subdirectory(common)
add_subdirectory(system)
add_subdirectory(esp)
add_subdirectory(tools)
add_subdirectory(dali)
add_subdirectory(rtl)
add_subdirectory(roxie)

SET (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -frtti -fPIC -fmessage-length=0 -Werror=format -Wformat-security -Wformat-nonliteral -pthread -Wuninitialized")







set(VER_SEPARATOR "-")
if("${stagever}" MATCHES "^rc[0-9]+$")
    set(VER_SEPARATOR "~")
endif()

set(package_prefix "hpccsystems")

set(CPACK_DEB_PACKAGE_COMPONENT ON)

set(CPACK_PACKAGE_VERSION_MAJOR ${majorver})
set(CPACK_PACKAGE_VERSION_MINOR ${minorver})
set(CPACK_PACKAGE_VERSION_PATCH ${point}${VER_SEPARATOR}${stagever})
set(CPACK_PACKAGE_VERSION ${CPACK_PACKAGE_VERSION_MAJOR}.${CPACK_PACKAGE_VERSION_MINOR}.${CPACK_PACKAGE_VERSION_PATCH})
set(CPACK_PACKAGE_CONTACT "HPCCSystems <ossdevelopment@lexisnexis.com>")
set(CPACK_SOURCE_GENERATOR TGZ)

set(CPACK_RPM_PACKAGE_VERSION "${version}")
set(CPACK_RPM_PACKAGE_RELEASE "${stagever}")
set(CPACK_RPM_PACKAGE_VENDOR "HPCC Systems®")
set(CPACK_PACKAGE_VENDOR "HPCC Systems®")
set(CPACK_PACKAGE_VENDOR_WITHOUT_TRADEMARK "HPCC Systems")
set(CPACK_DEBIAN_PACKAGE_SHLIBDEPS ON)

include(CPack)

