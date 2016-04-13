################################################################################
#    HPCC SYSTEMS software Copyright (C) 2014 HPCC Systems®.
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

# Component: memcached

#####################################################
# Description:
# ------------
#    Cmake Input File for memcached
#####################################################

project( memcached )

if (MEMCACHED)
    #ADD_PLUGIN(memcached PACKAGES LIBMEMCACHED MINVERSION 1.0.10)
        # Currently libmemcached versions are not sufficient on ubuntu 12.04 and 14.04 LTS
        # until then, we build the required libraries from source
        add_custom_command(
            OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/libmemcached-1.0.10
            COMMAND wget -q https://launchpad.net/libmemcached/1.0/1.0.10/+download/libmemcached-1.0.10.tar.gz
            COMMAND tar -xzf ${CMAKE_CURRENT_BINARY_DIR}/libmemcached-1.0.10.tar.gz
            WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})
        add_custom_target(fetch-libmemcached
            DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/libmemcached-1.0.10
            WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
            COMMENT "Fetching and unpacking libmemcached source") 
        add_custom_target(configure-libmemcached
            COMMAND ./configure > /dev/null
            DEPENDS fetch-libmemcached
            WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/libmemcached-1.0.10
            COMMENT "Configuring libmemcached")
        add_custom_target(build-libmemcached
            COMMAND make > /dev/null
            DEPENDS configure-libmemcached
            WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/libmemcached-1.0.10
            COMMENT "building libmemcached")
        add_library(libmemcached SHARED IMPORTED)
        add_dependencies(libmemcached build-libmemcached) 
        set_target_properties(libmemcached
            PROPERTIES IMPORTED_LOCATION ${CMAKE_CURRENT_BINARY_DIR}/libmemcached-1.0.10/libmemcached/.libs/libmemcached.so)
        set(LIBMEMCACHED_INCLUDE_DIR ${CMAKE_CURRENT_BINARY_DIR}/libmemcached-1.0.10)
        install(FILES ${CMAKE_CURRENT_BINARY_DIR}/libmemcached-1.0.10/libmemcached/.libs/libmemcached.so DESTINATION plugins)

        set(
            SRCS
            memcachedplugin.hpp
            memcachedplugin.cpp)

        include_directories(
            ./../../system/include
            ./../../rtl/eclrtl
            ./../../rtl/include
            ./../../common/deftype
            ./../../system/jlib
            ${LIBMEMCACHED_INCLUDE_DIR})

        add_definitions(-D_USRDLL -DECL_MEMCACHED_EXPORTS)

        #libmemcached-1.0.18 includes the header <cinttypes> which translates to <bits/cinttypes> requiring ISO C++ 2011 standard to build.
        #All lesser versions explicitly include <tr1/cinttypes> which does not.
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=gnu++0x")

        HPCC_ADD_LIBRARY(memcached SHARED ${SRCS})
        if(${CMAKE_VERSION} VERSION_LESS "2.8.9")
            message(WARNING "Cannot set NO_SONAME. shlibdeps will give warnings when package is installed")
        elseif(NOT APPLE)
            set_target_properties(memcached PROPERTIES NO_SONAME 1)
        endif()

        install(
            TARGETS memcached
            DESTINATION plugins)

        target_link_libraries(
            memcached
            eclrtl
            jlib
            libmemcached)
endif()

if(PLATFORM)
    install(
        FILES ${CMAKE_CURRENT_SOURCE_DIR}/lib_memcached.ecllib
        DESTINATION plugins
        COMPONENT Runtime)
endif()
