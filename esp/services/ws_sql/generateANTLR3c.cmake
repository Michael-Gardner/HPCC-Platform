################################################################################
#    HPCC SYSTEMS software Copyright(C) 2013 HPCC Systems.
#
#    Licensed under the Apache License, Version 2.0(the "License");
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

# Generate libantlr3c to be shipped with platform code

# guard
if(NOT ANTLR3c_GENERATED)
    set(ANTLR3c_GENERATED ON CACHE BOOL "Is ANTLR3c generated yet")

    if(NOT ANTLR3C_VER)
        set(ANTLR3C_VER "3.4")
    endif()
    message(STATUS "Target ANTLR3C Version: ${ANTLR3C_VER}")

    set(ANTLRcCONFIGURE_COMMAND_PARAMS "--silent" "--disable-antlrdebug")
    if(WIN32)
        set(ANTLR3c_lib "antlr3c.lib")
    else()
        set(ANTLR3c_lib "libantlr3c.so")
    endif()

    if(UNIX)
        if(${ARCH64BIT} EQUAL 1)
            set(ANTLRcCONFIGURE_COMMAND_PARAMS ${ANTLRcCONFIGURE_COMMAND_PARAMS} "--enable-64bit")
            set(osdir "x86_64-linux-gnu")
        else()
            set(osdir "i386-linux-gnu")
        endif()
    elseif(WIN32)
        set(osdir "lib")
    else()
        set(osdir "unknown")
    endif()

    set(ANTLR3_URL "http://www.antlr3.org")
    set(ANTLR3c_DOWNLOAD_URL ${ANTLR3_URL}/download/C)
    set(ANTLRcPACKAGENAME "libantlr3c-${ANTLR3C_VER}")
    set(ANTLRcPACKAGE ${ANTLRcPACKAGENAME}.tar.gz)
    set(ANTLRcSOURCELOCATION ${CMAKE_CURRENT_BINARY_DIR}/ANTLRC)
    set(ANTLRcSEXPANDEDOURCELOCATION ${ANTLRcSOURCELOCATION}/${ANTLRcPACKAGENAME})
    set(ANTLRcBUILDLOCATION ${ANTLRcSEXPANDEDOURCELOCATION}/antlr3cbuild)
    set(ANTLRcBUILDLOCATION_LIBRARY ${ANTLRcBUILDLOCATION}/lib)
    set(ANTLRcBUILDLOCATION_INCLUDE ${ANTLRcBUILDLOCATION}/include)
    set(ANTLR_LICENSE_NAME "${CMAKE_CURRENT_BINARY_DIR}/ANTLR-LICENSE.txt")
    set(ANTLR_LICENSE_URL "https://raw.github.com/antlr/antlr4/master/LICENSE.txt")

    add_custom_command(
        OUTPUT ${ANTLRcSOURCELOCATION}/${ANTLRcPACKAGE}
        COMMAND wget "${ANTLR3c_DOWNLOAD_URL}/${ANTLRcPACKAGE}"
        WORKING_DIRECTORY ${ANTLRcSOURCELOCATION}
        )

    add_custom_target(${ANTLRcPACKAGENAME}-fetch DEPENDS ${ANTLRcSOURCELOCATION}/${ANTLRcPACKAGE} ${ANTLR_LICENSE_NAME})

    add_custom_command(
        OUTPUT ${ANTLR_LICENSE_NAME}
        COMMAND wget --no-check-certificate -O ./ANTLR-LICENSE.txt ${ANTLR_LICENSE_URL}
        WORKING_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}"
        )

    add_custom_command(
        OUTPUT ${ANTLRcSOURCELOCATION}/${ANTLRcPACKAGENAME}-expand.sentinel
        COMMAND ${CMAKE_COMMAND} -E tar xjf ${ANTLRcSOURCELOCATION}/${ANTLRcPACKAGE}
        COMMAND ${CMAKE_COMMAND} -E touch ${ANTLRcSOURCELOCATION}/${ANTLRcPACKAGENAME}-expand.sentinel
        DEPENDS ${ANTLRcSOURCELOCATION}/${ANTLRcPACKAGE}
        WORKING_DIRECTORY ${ANTLRcSOURCELOCATION}
        )
    add_custom_target(${ANTLRcPACKAGENAME}-expand DEPENDS ${ANTLRcPACKAGENAME}-fetch ${ANTLRcSOURCELOCATION}/${ANTLRcPACKAGENAME}-expand.sentinel)

    include(ExternalProject)
    ExternalProject_Add(
        libantlr3c_external
        DOWNLOAD_COMMAND ""
        SOURCE_DIR ${ANTLRcSEXPANDEDOURCELOCATION}
        CONFIGURE_COMMAND ${ANTLRcSEXPANDEDOURCELOCATION}/configure ${ANTLRcCONFIGURE_COMMAND_PARAMS} --prefix=${ANTLRcBUILDLOCATION}
        PREFIX ${ANTLRcSOURCELOCATION}
        BUILD_COMMAND $(MAKE)
        BUILD_IN_SOURCE 1
        )

    add_dependencies(libantlr3c_external ${ANTLRcPACKAGENAME}-expand)

    add_library(libantlr3c SHARED IMPORTED GLOBAL)
    set_property(TARGET libantlr3c PROPERTY IMPORTED_LOCATION ${ANTLRcBUILDLOCATION_LIBRARY}/${ANTLR3c_lib})

    add_dependencies(libantlr3c libantlr3c_external)

    set(ANTLR3c_INCLUDE_DIR ${ANTLRcBUILDLOCATION_INCLUDE})
    set(ANTLR3c_LIBRARIES libantlr3c)

    message(STATUS "Expecting antlr3.h in ${ANTLRcBUILDLOCATION_INCLUDE}")
    find_path(ANTLR3c_INCLUDE_DIR NAMES antlr3.h
        PATHS ${ANTLRcBUILDLOCATION_INCLUDE} NO_DEFAULT_PATH)

    message(STATUS "Looking for ${ANTLR3c_lib} in ${ANTLRcBUILDLOCATION_LIBRARY}")
    find_library(ANTLR3c_LIBRARIES NAMES ${ANTLR3c_lib}
        PATHS ${ANTLRcBUILDLOCATION_LIBRARY} NO_DEFAULT_PATH)

    install(FILES ${ANTLR_LICENSE_NAME} DESTINATION ${LIB_PATH}/external COMPONENT Runtime)
    install(FILES ${ANTLRcBUILDLOCATION_LIBRARY}/${ANTLR3c_lib} DESTINATION ${LIB_PATH}/external COMPONENT Runtime)
endif()
