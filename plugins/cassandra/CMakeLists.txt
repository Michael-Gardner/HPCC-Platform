################################################################################
#    HPCC SYSTEMS software Copyright (C) 2014 HPCC Systems®.
#
#    Licensed under the Apache License, Version 2.0 (the "License");
#    you may not use this file except in compliance with the License.
#    You may obtain a copy of the License at
#
#       http://www.apache.org/licenses/LICENSE-2.0
#
#    Unless required by applicable law or agreed to in writing, software #    distributed under the License is distributed on an "AS IS" BASIS,
#    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
#    See the License for the specific language governing permissions and
#    limitations under the License.
################################################################################

# Component: cassandraembed

#####################################################
# Description:
# ------------
#    Cmake Input File for cassandraembed
#####################################################

project(cassandraembed)
# There is not yet a standard package for cassandra cpp client, and only very modern distros have libuv-dev
# When there is (and the distros catch up) we may want to add them as dependencies here
# until then, we build the required libraries from source
# Build libuv, required by the cassandra driver but not available on all distros
if(USE_CASSANDRA)
    if(NOT EXISTS "${PROJECT_SOURCE_DIR}/libuv/README.md")
        message(FATAL_ERROR
            "   The libuv submodule is not available.\
            This normally indicates that the git submodule has not been fetched.\
            Please run git submodule update --init --recursive")
    endif()
    if(APPLE)
        add_custom_command(
            OUTPUT ${PROJECT_BINARY_DIR}/libuv.dylib
            COMMAND ${MAKE} builddir_name=${PROJECT_BINARY_DIR}
            WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}/libuv)
        add_custom_target(
            libuv ALL
            DEPENDS ${PROJECT_BINARY_DIR}/libuv.dylib)
        set(LIBUV_LIBRARY ${PROJECT_BINARY_DIR}/libuv.dylib)
    else()
        add_custom_command(
            OUTPUT ${PROJECT_BINARY_DIR}/libuv.a
            COMMAND ${MAKE} builddir_name=${PROJECT_BINARY_DIR}
            WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}/libuv)
        add_custom_target(libuv-make
            DEPENDS ${PROJECT_BINARY_DIR}/libuv.a)
        add_custom_command(
            OUTPUT ${PROJECT_BINARY_DIR}/libuv.so.0.10
            COMMAND ${CMAKE_COMMAND} -E rename ${PROJECT_BINARY_DIR}/libuv.so ${PROJECT_BINARY_DIR}/libuv.so.0.10)
        add_custom_target(libuv-mv
            DEPENDS libuv-make ${PROJECT_BINARY_DIR}/libuv.so.0.10)
        add_custom_command(
            OUTPUT ${PROJECT_BINARY_DIR}/libuv.so
            COMMAND ${CMAKE_COMMAND} -E create_symlink ${PROJECT_BINARY_DIR}/libuv.so.0.10 ${PROJECT_BINARY_DIR}/libuv.so)
        add_custom_target(libuv ALL
            DEPENDS libuv-mv ${PROJECT_BINARY_DIR}/libuv.so)
        set(LIBUV_LIBRARY ${PROJECT_BINARY_DIR}/libuv.so)
    endif()
    set(LIBUV_INCLUDE_DIR ${PROJECT_SOURCE_DIR}/libuv/include/)

    # Build the Cassandra cpp driver, only presently available as source
    if(NOT EXISTS "${PROJECT_SOURCE_DIR}/cpp-driver/CMakeLists.txt")
        message(FATAL_ERROR
            "   The cpp-driver submodule is not available.\
            This normally indicates that the git submodule has not been fetched.\
            Please run git submodule update --init --recursive")
    endif()
    include(ExternalProject)
    ExternalProject_Add(generate-cassandra
        DEPENDS     libuv
        SOURCE_DIR  ${CMAKE_CURRENT_SOURCE_DIR}/cpp-driver
        BINARY_DIR  ${CMAKE_CURRENT_BINARY_DIR}/cassandra
        CMAKE_ARGS  -DCASS_INSTALL_HEADER=OFF
        -DCASS_BUILD_STATIC=OFF
        -DCASS_BUILD_EXAMPLES=OFF
        -DCASS_INSTALL_PKG_CONFIG=OFF
        -DCMAKE_CXX_FLAGS:string=-Wno-format-nonliteral
        -DLIBUV_LIBRARY=${LIBUV_LIBRARY}
        -DLIBUV_INCLUDE_DIR=${LIBUV_INCLUDE_DIR}
        BUILD_COMMAND ${MAKE}
        INSTALL_COMMAND "")
    add_library(cassandra SHARED IMPORTED)
    set_property(TARGET cassandra
        PROPERTY IMPORTED_LOCATION ${CMAKE_CURRENT_BINARY_DIR}/libuv.so)
    add_dependencies(cassandra generate-cassandra)

    install(CODE "set(ENV{LD_LIBRARY_PATH} \"\$ENV{LD_LIBRARY_PATH}:${PROJECT_BINARY_DIR}:${PROJECT_BINARY_DIR}/cassandra\")")
    if(APPLE)
        install(
            FILES ${PROJECT_BINARY_DIR}/libuv.dylib
            DESTINATION ${LIB_DIR}
            COMPONENT Runtime)
    else()
        install(
            FILES ${PROJECT_BINARY_DIR}/libuv.so.0.10
            DESTINATION ${LIB_DIR}
            PERMISSIONS OWNER_WRITE OWNER_READ OWNER_EXECUTE GROUP_READ GROUP_EXECUTE WORLD_READ WORLD_EXECUTE
            COMPONENT Runtime)
    endif()

    ADD_PLUGIN(cassandraembed)

    set(SRCS
        cassandraembed.cpp
        cassandrawu.cpp)

    include_directories(
        ./../../system/include
        ./../../rtl/eclrtl
        ./../../roxie/roxiemem
        ./../../rtl/include
        ./../../rtl/nbcd
        ./../../common/deftype
        ./../../common/workunit
        ./../../system/jlib
        ./../../system/security/shared
        ./../../system/mp
        ./../../dali/base
        ./cpp-driver/include)

    add_definitions(-D_USRDLL -DCASSANDRAEMBED_EXPORTS)

    HPCC_ADD_LIBRARY(cassandraembed SHARED ${SRCS})

    install(
        TARGETS cassandraembed
        DESTINATION plugins
        COMPONENT Runtime)

    install(
        TARGETS cassandraembed
        DESTINATION ${LIB_DIR}
        COMPONENT Runtime)

    target_link_libraries(
        cassandraembed
        cassandra
        eclrtl
        roxiemem
        dalibase
        workunit
        jlib)

    if(PLATFORM OR CLIENTTOOLS_ONLY)
        install(
            FILES ${CMAKE_CURRENT_SOURCE_DIR}/cassandra.ecllib
            DESTINATION plugins
            COMPONENT Runtime)
    endif()
endif(USE_CASSANDRA)
