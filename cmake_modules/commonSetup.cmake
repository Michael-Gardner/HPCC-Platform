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
#
# File  : commonSetup.cmake
#
################################################################################
# Description:
# ------------
# sets up various cmake options.
################################################################################

macro_ensure_out_of_source_build("The LexisNexis Hpcc requires an out of source build.
Please remove the directory ${CMAKE_BINARY_DIR}/CMakeFiles
and the file ${CMAKE_BINARY_DIR}/CMakeCache.txt,
then create a separate build directory and run 'cmake path_to_source [options]' there.")

cmake_policy ( SET CMP0011 NEW )
if (NOT (CMAKE_MAJOR_VERSION LESS 3))
    cmake_policy ( SET CMP0026 OLD )
    if (NOT (CMAKE_MINOR_VERSION LESS 1))
        cmake_policy ( SET CMP0054 NEW )
    endif()
endif()

option(CLIENTTOOLS "Enable the building/inclusion of a Client Tools component." ON)
option(PLATFORM "Enable the building/inclusion of a Platform component." ON)
option(DEVEL "Enable the building/inclusion of a Development component." OFF)
option(CLIENTTOOLS_ONLY "Enable the building of Client Tools only." OFF)
option(INCLUDE_PLUGINS "Enable the building of platform and all plugins for testing purposes" OFF)
option(USE_CASSANDRA "Include the Cassandra plugin in the base package" ON)
option(PLUGIN "Enable building of a plugin" OFF)
option(USE_SHLIBDEPS "Enable the use of dpkg-shlibdeps on ubuntu packaging" OFF)

option(SIGN_MODULES "Enable signing of ecl standard library modules" OFF)
option(USE_CPPUNIT "Enable unit tests (requires cppunit)" OFF)
option(USE_OPENLDAP "Enable OpenLDAP support (requires OpenLDAP)" ON)
option(USE_ICU "Enable unicode support (requires ICU)" ON)
option(USE_BOOST_REGEX "Configure use of boost regex" ON)
# USE_C11_REGEX is only checked if USE_BOOST_REGEX is OFF
# to disable REGEX altogether turn both off
option(USE_C11_REGEX "Configure use of c++11 std::regex" ON)
option(Boost_USE_STATIC_LIBS "Use boost_regex static library for RPM BUILD" OFF)
option(USE_OPENSSL "Configure use of OpenSSL" ON)
option(USE_ZLIB "Configure use of zlib" ON)
option(USE_CBLAS "Configure use of cblas" ON)
if (WIN32)
    option(USE_GIT "Configure use of GIT (Hooks)" OFF)
else()
    option(USE_GIT "Configure use of GIT (Hooks)" ON)
endif()
option(USE_LIBARCHIVE "Configure use of libarchive" ON)
option(USE_URIPARSER "Configure use of uriparser" OFF)
if (APPLE OR WIN32)
    option(USE_NUMA "Configure use of numa" OFF)
else()
    option(USE_NUMA "Configure use of numa" ON)
endif()
option(USE_NATIVE_LIBRARIES "Search standard OS locations for thirdparty libraries" ON)
option(USE_GIT_DESCRIBE "Use git describe to generate build tag" ON)
option(CHECK_GIT_TAG "Require git tag to match the generated build tag" OFF)
option(USE_XALAN "Configure use of xalan" OFF)
option(USE_APR "Configure use of Apache Software Foundation (ASF) Portable Runtime (APR) libraries" ON)
option(USE_LIBXSLT "Configure use of libxslt" ON)
option(MAKE_DOCS "Create documentation at build time." OFF)
option(MAKE_DOCS_ONLY "Create a base build with only docs." OFF)
option(DOCS_DRUPAL "Create Drupal HTML Docs" OFF)
option(DOCS_EPUB "Create EPUB Docs" OFF)
option(DOCS_MOBI "Create Mobi Docs" OFF)
option(DOCS_AUTO "DOCS automation" OFF)
option(USE_RESOURCE "Use resource download in ECLWatch" OFF)
option(GENERATE_COVERAGE_INFO "Generate coverage info for gcov" OFF)
option(USE_SIGNED_CHAR "Build system with default char type is signed" OFF)
option(USE_UNSIGNED_CHAR "Build system with default char type is unsigned" OFF)
option(USE_MYSQL "Enable mysql support" ON)
option(USE_OPTIONAL "Automatically disable requested features with missing dependencies" ON)
option(JLIB_ONLY  "Build JLIB for other projects such as Configurator, Ganglia Monitoring, etc" OFF)
# Generates code that is more efficient, but will cause problems if target platforms do not support it.
if (CMAKE_SIZEOF_VOID_P EQUAL 8)
    option(USE_INLINE_TSC "Inline calls to read TSC (time stamp counter)" ON)
else()
    option(USE_INLINE_TSC "Inline calls to read TSC (time stamp counter)" OFF)
endif()
if (APPLE OR WIN32)
    option(USE_TBB "Enable Threading Building Block support" OFF)
else()
    option(USE_TBB "Enable Threading Building Block support" ON)
    option(USE_TBBMALLOC "Enable Threading Building Block scalable allocator proxy support" ON)
endif()
option(LOGGING_SERVICE "Configure use of logging service" ON)

option(MAKE_CONFIGURATOR "Build Configurator" ON)
option(CONFIGURATOR_LIB "Build Configurator static library (.a)" OFF)

if ( CONFIGURATOR_LIB )
    set( MAKE_CONFIGURATOR ON )
endif()


MACRO(SET_PLUGIN_PACKAGE plugin)
    string(TOLOWER "${plugin}" pname)
    if(DEFINED pluginname)
        message(FATAL_ERROR "Cannot enable ${pname}, already declared ${pluginname}")
    else()
        set(pluginname "${pname}")
    endif()
    foreach(p in ${PLUGINS_LIST})
        if(NOT "${p}" STREQUAL "${plugin}" AND ${p})
            message(FATAL_ERROR "Cannot declare multiple plugins in a plugin package")
        endif()
    endforeach()
    set(PLUGIN ON)
    set(CLIENTTOOLS OFF)
    set(PLATFORM OFF)
    set(INCLUDE_PLUGINS OFF)
    set(SIGN_MODULES OFF)
    set(USE_OPTIONAL OFF) # Force failure if we can't find the plugin dependencies
ENDMACRO()

# Plugin options
set(PLUGINS_LIST
    REMBED
    V8EMBED
    MEMCACHED
    PY2EMBED
    PY3EMBED
    REDIS
    SQS
    MYSQLEMBED
    JAVAEMBED
    SQLITE3EMBED
    KAFKA
    COUCHBASEEMBED
    EXAMPLEPLUGIN)
foreach(plugin ${PLUGINS_LIST})
    option(${plugin} "Create a package with ONLY the ${plugin} plugin" OFF)
    option(INCLUDE_${plugin} "Include ${plugin} within package for testing" OFF)
    option(SUPPRESS_${plugin} "Suppress ${plugin} from INCLUDE_PLUGINS build" OFF)
    # Plugin Release build for individual package
    if(${plugin})
        SET_PLUGIN_PACKAGE("${plugin}")
        # Development build with all plugins for testing
        # Development build with addition of plugin
    elseif((INCLUDE_PLUGINS OR INCLUDE_${plugin}) AND (NOT SUPPRESS_${plugin}) AND (NOT PLUGIN))
        set(${plugin} ON)
    endif()
endforeach()
#"cmake -DEXAMPLEPLUGIN=ON <path-to/HPCC-Platform/>" will configure the plugin makefiles to be built with "make".

set(LIBMEMCACHED_MINVERSION "1.0.10")

if (SIGN_MODULES)
    message(STATUS "GPG signing check")
    execute_process(COMMAND bash "-c" "gpg --version | awk 'NR==1{print $3}'"
        OUTPUT_VARIABLE GPG_VERSION
        OUTPUT_STRIP_TRAILING_WHITESPACE
        ERROR_QUIET)
    set(GPG_COMMAND_STR "gpg")
    if(${GPG_VERSION} VERSION_GREATER "2.1")
        set(GPG_COMMAND_STR "${GPG_COMMAND_STR} --pinentry-mode loopback --batch --no-tty")
    else()
        set(GPG_COMMAND_STR "${GPG_COMMAND_STR} --batch --no-tty")
    endif()
    if(DEFINED SIGN_MODULES_PASSPHRASE)
        set(GPG_COMMAND_STR "${GPG_COMMAND_STR} --passphrase ${SIGN_MODULES_PASSPHRASE}")
    endif()
    if(DEFINED SIGN_MODULES_KEYID)
        set(GPG_COMMAND_STR "${GPG_COMMAND_STR} --default-key ${SIGN_MODULES_KEYID}")
    endif()
    set(GPG_COMMAND_STR "${GPG_COMMAND_STR} --output sm_keycheck.asc --clearsign sm_keycheck.tmp")
    execute_process(COMMAND rm -f sm_keycheck.tmp sm_keycheck.asc TIMEOUT 5
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR} OUTPUT_QUIET ERROR_QUIET)
    execute_process(COMMAND touch sm_keycheck.tmp TIMEOUT 5
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR} RESULT_VARIABLE t_rc
        OUTPUT_QUIET ERROR_QUIET)
    if(NOT "${t_rc}" STREQUAL "0")
        message(FATAL_ERROR "Failed to create sm_keycheck.tmp for signing")
    endif()
    execute_process(
        COMMAND bash "-c" "${GPG_COMMAND_STR}"
        TIMOUT 120
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
        RESULT_VARIABLE rc_var
        OUTPUT_VARIABLE out_var
        ERROR_VARIABLE err_var
        )
    if(NOT "${rc_var}" STREQUAL "0")
        message(STATUS "GPG signing check - failed")
        message(FATAL_ERROR "gpg signing of std ecllibrary unsupported in current environment. \
        If you wish to build without a signed std ecllibrary add -DSIGN_MODULES=OFF to your \
        cmake invocation.\n${err_var}")
    else()
        message(STATUS "GPG signing check - done")
    endif()
endif()

if ( USE_XALAN AND USE_LIBXSLT )
    set(USE_LIBXSLT OFF)
endif()
if ( USE_LIBXSLT )
    set(USE_LIBXML2 ON)
endif()
if ( USE_XALAN )
    set(USE_XERCES ON)
endif()

if ( MAKE_DOCS AND CLIENTTOOLS_ONLY )
    set( MAKE_DOCS OFF )
endif()

if ( MAKE_DOCS_ONLY AND NOT CLIENTTOOLS_ONLY )
    set( MAKE_DOCS ON )
    if ( USE_DOCS_AUTO )
        set ( DOCS_AUTO  ON)
    endif()
endif()

if ( CLIENTTOOLS_ONLY )
    set(PY2EMBED ON)
    set(PLATFORM OFF)
    set(DEVEL OFF)
endif()

option(PORTALURL "Set url to hpccsystems portal download page")

if ( NOT PORTALURL )
    set( PORTALURL "http://hpccsystems.com/download" )
endif()

set(CMAKE_MODULE_PATH "${HPCC_SOURCE_DIR}/cmake_modules/")

if(UNIX AND SIGN_MODULES)
    execute_process(COMMAND bash "-c" "gpg --version | awk 'NR==1{print $3}'"
        OUTPUT_VARIABLE GPG_VERSION
        OUTPUT_STRIP_TRAILING_WHITESPACE
        ERROR_QUIET)
    message(STATUS "gpg version ${GPG_VERSION}")
    #export gpg public key used for signing to new installation
    add_custom_command(OUTPUT ${CMAKE_BINARY_DIR}/pub.key
        COMMAND bash "-c" "gpg --output=${CMAKE_BINARY_DIR}/pub.key --batch --no-tty --export ${SIGN_MODULES_KEYID}"
        WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
        COMMENT "Exporting public key for eclcc signed modules to ${CMAKE_BINARY_DIR}/pub.key"
        VERBATIM
        )
    add_custom_target(export-stdlib-pubkey ALL
        DEPENDS ${CMAKE_BINARY_DIR}/pub.key
        WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
        )
    install(FILES ${CMAKE_BINARY_DIR}/pub.key DESTINATION .${CONFIG_DIR}/rpmnew  COMPONENT Runtime)
    install(PROGRAMS ${CMAKE_MODULE_PATH}publickey.install DESTINATION etc/init.d/install COMPONENT Runtime)
endif()


##########################################################

# common compiler/linker flags

if ("${CMAKE_BUILD_TYPE}" STREQUAL "")
    set ( CMAKE_BUILD_TYPE "Release" )
elseif (NOT "${CMAKE_BUILD_TYPE}" MATCHES "^Debug$|^Release$|^RelWithDebInfo$")
    message (FATAL_ERROR "Unknown build type ${CMAKE_BUILD_TYPE}")
endif ()
message ("-- Making ${CMAKE_BUILD_TYPE} system")

if (CMAKE_SIZEOF_VOID_P EQUAL 8)
    set ( ARCH64BIT 1 )
else ()
    set ( ARCH64BIT 0 )
endif ()
message ("-- 64bit architecture is ${ARCH64BIT}")

set (CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -D_DEBUG -DDEBUG")

IF (USE_INLINE_TSC)
    add_definitions (-DINLINE_GET_CYCLES_NOW)
ENDIF()

set (CMAKE_THREAD_PREFER_PTHREAD 1)
find_package(Threads)
IF (NOT THREADS_FOUND)
    message(FATAL_ERROR "No threading support found")
ENDIF()

if ("${CMAKE_CXX_COMPILER_ID}" MATCHES "Clang")
    set (CMAKE_COMPILER_IS_CLANGXX 1)
endif()
if ("${CMAKE_C_COMPILER_ID}" MATCHES "Clang")
    set (CMAKE_COMPILER_IS_CLANG 1)
endif()
if ((CMAKE_COMPILER_IS_GNUCC OR CMAKE_COMPILER_IS_CLANG) AND (NOT ${CMAKE_C_COMPILER_VERSION} MATCHES "[0-9]+\\.[0-9]+\\.[0-9]+"))
    execute_process(COMMAND ${CMAKE_C_COMPILER} -dumpversion OUTPUT_VARIABLE CMAKE_C_COMPILER_VERSION OUTPUT_STRIP_TRAILING_WHITESPACE)
endif ()
if ((CMAKE_COMPILER_IS_GNUCXX OR CMAKE_COMPILER_IS_CLANGXX) AND (NOT ${CMAKE_CXX_COMPILER_VERSION} MATCHES "[0-9]+\\.[0-9]+\\.[0-9]+"))
    execute_process(COMMAND ${CMAKE_CXX_COMPILER} -dumpversion OUTPUT_VARIABLE CMAKE_CXX_COMPILER_VERSION OUTPUT_STRIP_TRAILING_WHITESPACE)
endif()
if (CMAKE_COMPILER_IS_GNUCC AND NOT CMAKE_BUILD_TYPE STREQUAL "Debug")
    add_definitions (-fvisibility=hidden)
endif ()
if (CMAKE_COMPILER_IS_CLANGXX)
    execute_process( COMMAND ${CMAKE_CXX_COMPILER} --version OUTPUT_VARIABLE clang_full_version_string )
    if (${clang_full_version_string} MATCHES "Apple LLVM version ([0-9]+\\.[0-9]+\\.[0-9]+).*")
        string (REGEX REPLACE "Apple LLVM version ([0-9]+\\.[0-9]+\\.[0-9]+).*" "\\1" APPLE_CLANG_VERSION ${clang_full_version_string})
    endif()
    if (${clang_full_version_string} MATCHES ".*based on LLVM ([0-9]+\\.[0-9]+).*")
        string (REGEX REPLACE ".*based on LLVM ([0-9]+\\.[0-9]+).*" "\\1" CLANG_VERSION ${clang_full_version_string})
    else()
        if (${clang_full_version_string} MATCHES ".*clang version ([0-9]+\\.[0-9]+).*")
            string (REGEX REPLACE ".*clang version ([0-9]+\\.[0-9]+).*" "\\1" CLANG_VERSION ${clang_full_version_string})
        endif()
    endif()
endif ()

if (WIN32)
    # On windows, the vcproj generator generates both windows and debug build capabilities, and the release mode is appended to the directory later
    # This output location matches what our existing windows release scripts expect - might make sense to move out of tree sometime though
    set ( EXECUTABLE_OUTPUT_PATH "${CMAKE_BINARY_DIR}/bin" )
    set ( LIBRARY_OUTPUT_PATH "${CMAKE_BINARY_DIR}/bin" )
    # Workaround CMake's odd decision to default windows stack size to 10000000
    STRING(REGEX REPLACE "/STACK:[0-9]+" "" CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS}")
    SET (CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} /LARGEADDRESSAWARE")

    if (${ARCH64BIT} EQUAL 1)
        add_definitions(/bigobj)
    endif ()

    if ("${CMAKE_BUILD_TYPE}" MATCHES "Debug")
        if (${ARCH64BIT} EQUAL 0)
            add_definitions(/ZI)
        else()
            add_definitions(/Zi)
        endif ()
    endif ()
    if ("${GIT_COMMAND}" STREQUAL "")
        set ( GIT_COMMAND "git.cmd" )
    endif ()

    SET (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /EHa")

    if (USE_SIGNED_CHAR AND USE_UNSIGNED_CHAR )
        message (FATAL_ERROR "Can't use USE_SIGNED_CHAR and USE_UNSIGNED_CHAR together!")
    else()
        if (USE_SIGNED_CHAR)
            message ("Build system with signed char type.")
            # This is default for MSVC
        endif ()
        if (USE_UNSIGNED_CHAR )
            message ("Build system with unsigned char type.")
            add_definitions(/J)
        endif ()
    endif ()
else ()
    if (NOT CMAKE_USE_PTHREADS_INIT)
        message (FATAL_ERROR "pthreads support not detected")
    endif ()
    set ( EXECUTABLE_OUTPUT_PATH "${CMAKE_BINARY_DIR}/${CMAKE_BUILD_TYPE}/bin" )
    set ( LIBRARY_OUTPUT_PATH "${CMAKE_BINARY_DIR}/${CMAKE_BUILD_TYPE}/libs" )

    if (CMAKE_COMPILER_IS_GNUCXX OR CMAKE_COMPILER_IS_CLANGXX)
        message ("Using compiler: ${CMAKE_CXX_COMPILER_ID} :: ${CMAKE_CXX_COMPILER_VERSION} :: ${CLANG_VERSION} :: ${APPLE_CLANG_VERSION}")
        SET (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -frtti -fPIC -fmessage-length=0 -Werror=format -Wformat-security -Wformat-nonliteral -pthread -Wuninitialized")
        SET (CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -rdynamic")
        SET (CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -g -fno-inline-functions")
        if (CMAKE_COMPILER_IS_GNUCXX)
            SET (CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -g -fno-default-inline")
            if (CMAKE_CXX_COMPILER_VERSION VERSION_GREATER 4.2.4 OR CMAKE_CXX_COMPILER_VERSION VERSION_EQUAL 4.2.4)
                SET (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Werror=return-type -Werror=format-nonliteral")
            endif ()
            if (CMAKE_CXX_COMPILER_VERSION VERSION_GREATER 4.4.0 OR CMAKE_CXX_COMPILER_VERSION VERSION_EQUAL 4.4.0)
                SET (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-psabi")
            endif ()
            if (CMAKE_CXX_COMPILER_VERSION VERSION_GREATER 2.95.3 OR CMAKE_CXX_COMPILER_VERSION VERSION_EQUAL 2.95.3)
                SET (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wparentheses")
            endif ()
        endif ()
        SET (CMAKE_CXX_FLAGS_RELWITHDEBINFO "${CMAKE_CXX_FLAGS_RELEASE}")
        SET (CMAKE_C_FLAGS_RELWITHDEBINFO "${CMAKE_C_FLAGS_RELEASE}")
        SET (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
        if (GENERATE_COVERAGE_INFO)
            message ("Build system with coverage.")
            SET (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fprofile-arcs -ftest-coverage")
        endif()
        # Handle forced default char type
        if (USE_SIGNED_CHAR AND USE_UNSIGNED_CHAR )
            message (FATAL_ERROR "Can't use USE_SIGNED_CHAR and USE_UNSIGNED_CHAR together!")
        else()
            if (USE_SIGNED_CHAR)
                message ("Build system with signed char type.")
                SET (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fsigned-char")
            endif ()
            if (USE_UNSIGNED_CHAR )
                message ("Build system with unsigned char type.")
                SET (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -funsigned-char")
            endif ()
        endif ()
    endif ()
    if (CMAKE_COMPILER_IS_CLANGXX)
        SET (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Werror=logical-op-parentheses -Werror=bool-conversions -Werror=return-type -Werror=comment")
        if (APPLE)
            SET (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -stdlib=libc++")
            SET (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-deprecated-declarations")
        endif ()
        SET (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}  -Werror=bitwise-op-parentheses -Werror=tautological-compare")
        SET (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}  -Wno-switch-enum -Wno-format-zero-length -Wno-switch")
        SET (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}  -Qunused-arguments")  # Silence messages about pthread not being used when linking...
        SET (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}  -Wno-inconsistent-missing-override -Wno-unknown-warning-option")  # Until we fix them all, whcih would be a huge task...
        if (CLANG_VERSION VERSION_GREATER 3.6 OR CLANG_VERSION VERSION_EQUAL 3.6 OR APPLE_CLANG_VERSION VERSION_GREATER 6.0)
            SET (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-pointer-bool-conversion")
        endif()
    endif()
    # All of these are defined in platform.h too, but need to be defned before any system header is included
    ADD_DEFINITIONS (-D_LARGEFILE_SOURCE=1 -D_LARGEFILE64_SOURCE=1 -D_FILE_OFFSET_BITS=64 -D__USE_LARGEFILE64=1 -D__USE_FILE_OFFSET64=1)
    if ("${GIT_COMMAND}" STREQUAL "")
        set ( GIT_COMMAND "git" )
    endif ()
endif ()

set ( SCM_GENERATED_DIR ${CMAKE_BINARY_DIR}/generated )



##################################################################

# Build tag generation

set(projname ${HPCC_PROJECT})
set(majorver ${HPCC_MAJOR})
set(minorver ${HPCC_MINOR})
set(point ${HPCC_POINT})
if ( "${HPCC_MATURITY}" STREQUAL "release" )
    set(stagever "${HPCC_SEQUENCE}")
else()
    set(stagever "${HPCC_MATURITY}${HPCC_SEQUENCE}")
endif()
set(version ${majorver}.${minorver}.${point})

IF ("${CMAKE_BUILD_TYPE}" STREQUAL "Debug")
    set( stagever "${stagever}Debug" )
ENDIF ("${CMAKE_BUILD_TYPE}" STREQUAL "Debug")

###########################################################################

if(USE_OPTIONAL)
    message(WARNING "USE_OPTIONAL set - missing dependencies for optional features will automatically disable them")
endif()

if(NOT "${EXTERNALS_DIRECTORY}" STREQUAL "")
    message(STATUS "Using externals directory at ${EXTERNALS_DIRECTORY}")
endif()

IF ( NOT MAKE_DOCS_ONLY )
    IF (CMAKE_COMPILER_IS_GNUCXX)
        IF ("${CMAKE_CXX_COMPILER_VERSION}" VERSION_LESS "4.7.3")
            MESSAGE(FATAL_ERROR "You need Gnu c++ version 4.7.3 or later to build this project (version ${CMAKE_CXX_COMPILER_VERSION} detected)")
        ENDIF()
    ENDIF()
ENDIF()
###########################################################################

# External library setup - some can be optionally selected based on USE_xxx flags, some are required

if(MAKE_DOCS)
    find_package(XSLTPROC REQUIRED)
    add_definitions(-D_USE_XSLTPROC)
    find_package(FOP REQUIRED)
    add_definitions(-D_USE_FOP)

    if(DOCS_AUTO)
        if("${CONFIGURATOR_DIRECTORY}" STREQUAL "")
            set(MAKE_CONFIGURATOR ON)
            set(JLIB_ONLY ON)
            set(CONFIGURATOR_DIRECTORY ${EXECUTABLE_OUTPUT_PATH})
        endif()
    endif()
endif()

set(BUILD_TAG "${projname}_${version}-${stagever}")
if(USE_GIT_DESCRIBE OR CHECK_GIT_TAG)
    FETCH_GIT_TAG(${CMAKE_SOURCE_DIR} ${projname}_${version} GIT_BUILD_TAG)
    message(STATUS "Git tag is '${GIT_BUILD_TAG}'")
    if(NOT "${GIT_BUILD_TAG}" STREQUAL "${BUILD_TAG}")
        if(CHECK_GIT_TAG)
            message(FATAL_ERROR "Git tag '${GIT_BUILD_TAG}' does not match source version '${BUILD_TAG}'")
        else()
            if(NOT "${GIT_BUILD_TAG}" STREQUAL "") # probably means being built from a tarball...
                set(BUILD_TAG "${BUILD_TAG}[${GIT_BUILD_TAG}]")
            endif()
        endif()
    endif()
endif()
message(STATUS "Build tag is '${BUILD_TAG}'")
if(NOT "${BASE_BUILD_TAG}" STREQUAL "")
    set(BASE_BUILD_TAG "${BUILD_TAG}")
endif()
message(STATUS "Base build tag is '${BASE_BUILD_TAG}'")
configure_file(${HPCC_SOURCE_DIR}/build-config.h.cmake "build-config.h")

if(USE_XERCES)
    find_package(XERCES REQUIRED)
    add_definitions (-D_USE_XERCES)
endif(USE_XERCES)

if(USE_LIBXML2)
    find_package(LIBXML2 REQUIRED)
    add_definitions (-D_USE_LIBXML2)
endif(USE_LIBXML2)

if(USE_CBLAS)
    find_package(CBLAS REQUIRED)
    add_definitions(-D_USE_CBLAS)
endif(USE_CBLAS)

if(USE_ZLIB)
    find_package(ZLIB REQUIRED)
    add_definitions (-D_USE_ZLIB)
endif(USE_ZLIB)

if(USE_LIBARCHIVE)
    if(WIN32)
        if(NOT USE_ZLIB)
            message(FATAL ERROR "LIBARCHIVE requires ZLIB")
        endif(NOT USE_ZLIB)
        find_package(BZip2 REQUIRED)
        add_definitions(-D_USE_BZIP2)
    endif(WIN32)
    find_package(LIBARCHIVE REQUIRED)
    add_definitions (-D_USE_LIBARCHIVE)
endif(USE_LIBARCHIVE)

if(USE_URIPARSER)
    find_package(Uriparser REQUIRED)
    add_definitions (-D_USE_URIPARSER)
endif(USE_URIPARSER)

if(USE_BOOST_REGEX)
    find_package(BOOST_REGEX REQUIRED)
    add_definitions(-D_USE_BOOST_REGEX)
else(USE_BOOST_REGEX)
    if(USE_C11_REGEX)
        if((NOT CMAKE_COMPILER_IS_GNUCC) OR (CMAKE_CXX_COMPILER_VERSION VERSION_GREATER 4.9.0))
            message(STATUS "C11_REGEX enabled")
            add_definitions(-D_USE_C11_REGEX)
        else()
            message(STATUS "C11_REGEX requested but not supported on this platform")
        endif()
    else(USE_C11_REGEX)
        message(STATUS "NO REGEX requested")
    endif(USE_C11_REGEX)
endif(USE_BOOST_REGEX)

if(USE_OPENSSL)
    find_package(OpenSSL REQUIRED)
    add_definitions(-D_USE_OPENSSL)
    include_directories(${OPENSSL_INCLUDE_DIR})
    link_directories(${OPENSSL_LIBRARY_DIR})
endif(USE_OPENSSL)

if(USE_MYSQL_REPOSITORY)
    find_package(MYSQL REQUIRED)
    add_definitions(-D_USE_MYSQL_REPOSITORY)
else()
    add_definitions(-D_NO_MYSQL_REPOSITORY)
endif(USE_MYSQL_REPOSITORY)

if(USE_APR)
    find_package(APR REQUIRED)
    add_definitions(-D_USE_APR)
    include_directories(${APR_INCLUDE_DIR})
    link_directories(${APR_LIBRARY_DIR})
    include_directories(${APRUTIL_INCLUDE_DIR})
    link_directories(${APRUTIL_LIBRARY_DIR})
else()
    add_definitions(-D_NO_APR)
endif(USE_APR)

if(USE_NUMA)
    find_package(NUMA REQUIRED)
    add_definitions(-D_USE_NUMA)
endif()

if(USE_TBB)
    message(STATUS "Enabled use of TBB")
    add_definitions (-D_USE_TBB)
endif(USE_TBB)
if(USE_TBBMALLOC)
    message(STATUS "Enabled use of TBBMALLOC")
    add_definitions(-D_USE_TBBMALLOC)
endif(USE_TBBMALLOC)

if(USE_LIBXSLT)
    find_package(LIBXSLT REQUIRED)
    add_definitions(-D_USE_LIBXSLT)
endif(USE_LIBXSLT)

if(USE_CPPUNIT)
    find_package(CPPUNIT REQUIRED)
    add_definitions(-D_USE_CPPUNIT)
    include_directories(${CPPUNIT_INCLUDE_DIR})
else()
    set(CPPUNIT_INCLUDE_DIR "")
    set(CPPUNIT_LIBRARIES "")
endif(USE_CPPUNIT)

if(USE_ICU)
    find_package(ICU REQUIRED)
    add_definitions(-D_USE_ICU)
    include_directories(${ICU_INCLUDE_DIR})
endif(USE_ICU)

if(USE_OPENLDAP)
    find_package(OPENLDAP REQUIRED)
    add_definitions (-D_USE_OPENLDAP)
else()
    add_definitions (-D_NO_LDAP)
endif(USE_OPENLDAP)

###########################################################################
###
## The following sets the install directories and names.
###
if ( PLATFORM OR PLUGIN )
    set ( CMAKE_INSTALL_PREFIX "${INSTALL_DIR}" )
else ( )
    set ( CMAKE_INSTALL_PREFIX "${INSTALL_DIR}/${version}/clienttools" )
endif ( PLATFORM OR PLUGIN )
if(APPLE)
    set(CMAKE_MACOSX_RPATH ON)
endif()
set (CMAKE_SKIP_BUILD_RPATH  FALSE)
set (CMAKE_BUILD_WITH_INSTALL_RPATH FALSE)
set (CMAKE_INSTALL_RPATH "${CMAKE_INSTALL_PREFIX}/${LIB_DIR};${CMAKE_INSTALL_PREFIX}/${PLUGINS_DIR}")
set (CMAKE_INSTALL_RPATH_USE_LINK_PATH TRUE)

