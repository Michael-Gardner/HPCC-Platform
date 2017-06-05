################################################################################
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
#
# File  : functions.cmake
# Contains:
#   MACRO_ENSURE_OUT_OF_SOURCE_BUILD
#   FETCH_GIT_TAG
#   HPCC_ADD_EXECUTABLE
#   HPCC_ADD_LIBRARY
#   PARSE_ARGUMENTS
#   HPCC_ADD_SUBDIRECTORY
#   LOG_PLUGIN
#   ADD_PLUIN
#   LIST_TO_STRING
#   STRING_TO_LIST
#   SET_DEPENDENCIES
#   SIGN_MODULES
#
################################################################################
# Description:
# ------------
# declares various cmake functions and macros.
################################################################################


###############################################################################
## Ensure out of source build
###############################################################################
macro(MACRO_ENSURE_OUT_OF_SOURCE_BUILD _errorMessage)
    STRING(COMPARE EQUAL "${CMAKE_SOURCE_DIR}" "${CMAKE_BINARY_DIR}" insource)
    IF (insource)
        MESSAGE(FATAL_ERROR "${_errorMessage}")
    ENDIF(insource)
endmacro(MACRO_ENSURE_OUT_OF_SOURCE_BUILD)

###############################################################################
## Git tag macro
###############################################################################
macro(FETCH_GIT_TAG workdir edition result)
    execute_process(COMMAND "${GIT_COMMAND}" describe --tags --dirty --abbrev=6 --match ${edition}*
        WORKING_DIRECTORY "${workdir}"
        OUTPUT_VARIABLE ${result}
        ERROR_QUIET
        OUTPUT_STRIP_TRAILING_WHITESPACE)
    if ("${${result}}" STREQUAL "")
        execute_process(COMMAND "${GIT_COMMAND}" describe --always --tags --all --abbrev=6 --dirty --long
            WORKING_DIRECTORY "${workdir}"
            OUTPUT_VARIABLE ${result}
            ERROR_QUIET
            OUTPUT_STRIP_TRAILING_WHITESPACE)
    endif()
endmacro()

###############################################################################
## HPCC add executable wrapper
###############################################################################
macro(HPCC_ADD_EXECUTABLE target)
    add_executable(${target} ${ARGN})
endmacro(HPCC_ADD_EXECUTABLE target)

###############################################################################
## HPCC add library wrapper
###############################################################################
macro(HPCC_ADD_LIBRARY target)
    add_library(${target} ${ARGN})
endmacro(HPCC_ADD_LIBRARY target)

###############################################################################
## This Macro is provided as Public domain from
## http://www.cmake.org/Wiki/CMakeMacroParseArguments
###############################################################################
macro(PARSE_ARGUMENTS prefix arg_names option_names)
    SET(DEFAULT_ARGS)
    FOREACH(arg_name ${arg_names})
        SET(${prefix}_${arg_name})
    ENDFOREACH(arg_name)
    FOREACH(option ${option_names})
        SET(${prefix}_${option} FALSE)
    ENDFOREACH(option)

    SET(current_arg_name DEFAULT_ARGS)
    SET(current_arg_list)
    FOREACH(arg ${ARGN})
        SET(larg_names ${arg_names})
        LIST(FIND larg_names "${arg}" is_arg_name)
        IF (is_arg_name GREATER -1)
            SET(${prefix}_${current_arg_name} ${current_arg_list})
            SET(current_arg_name ${arg})
            SET(current_arg_list)
        ELSE (is_arg_name GREATER -1)
            SET(loption_names ${option_names})
            LIST(FIND loption_names "${arg}" is_option)
            IF (is_option GREATER -1)
                SET(${prefix}_${arg} TRUE)
            ELSE (is_option GREATER -1)
                SET(current_arg_list ${current_arg_list} ${arg})
            ENDIF (is_option GREATER -1)
        ENDIF (is_arg_name GREATER -1)
    ENDFOREACH(arg)
    SET(${prefix}_${current_arg_name} ${current_arg_list})
endmacro(PARSE_ARGUMENTS)

###############################################################################
## This macro allows for disabling a directory based on the value of a variable
##     passed to the macro.
## ex. HPCC_ADD_SUBDIRECORY(roxie ${CLIENTTOOLS_ONLY})
## This call will disable the roxie dir if -DCLIENTTOOLS_ONLY=ON is set at
##     config time.
###############################################################################
macro(HPCC_ADD_SUBDIRECTORY)
    set(adddir OFF)
    PARSE_ARGUMENTS(_HPCC_SUB "" "" ${ARGN})
    LIST(GET _HPCC_SUB_DEFAULT_ARGS 0 subdir)
    set(flags ${_HPCC_SUB_DEFAULT_ARGS})
    LIST(REMOVE_AT flags 0)
    LIST(LENGTH flags length)
    if(NOT length)
        set(adddir ON)
    else()
        foreach(f ${flags})
            if(${f})
                set(adddir ON)
            endif()
        endforeach()
    endif()
    if ( adddir )
        add_subdirectory(${subdir})
    endif()
endmacro(HPCC_ADD_SUBDIRECTORY)



###############################################################################
## Macro for Logging Plugin build in CMake
###############################################################################
macro(LOG_PLUGIN)
    PARSE_ARGUMENTS(pLOG
        "OPTION;MDEPS"
        ""
        ${ARGN})
    LIST(GET pLOG_DEFAULT_ARGS 0 PLUGIN_NAME)
    if(${pLOG_OPTION})
        message(STATUS "Building Plugin: ${PLUGIN_NAME}" )
    endif()
endmacro()


###############################################################################
## Macro for adding an optional plugin to the CMake build.
###############################################################################
macro(ADD_PLUGIN)
    PARSE_ARGUMENTS(PLUGIN
        "PACKAGES;MINVERSION;MAXVERSION"
        ""
        ${ARGN})
    LIST(GET PLUGIN_DEFAULT_ARGS 0 PLUGIN_NAME)
    string(TOUPPER ${PLUGIN_NAME} name)
    set(ALL_PLUGINS_FOUND 1)
    set(PLUGIN_MDEPS ${PLUGIN_NAME}_mdeps)
    set(${PLUGIN_MDEPS} "")

    foreach(package ${PLUGIN_PACKAGES})
        set(findvar ${package}_FOUND)
        string(TOUPPER ${findvar} PACKAGE_FOUND)
        if("${PLUGIN_MINVERSION}" STREQUAL "")
            find_package(${package})
        else()
            set(findvar ${package}_VERSION_STRING)
            string(TOUPPER ${findvar} PACKAGE_VERSION_STRING)
            find_package(${package} ${PLUGIN_MINVERSION} )
            if ("${${PACKAGE_VERSION_STRING}}" VERSION_GREATER "${PLUGIN_MAXVERSION}")
                set(${ALL_PLUGINS_FOUND} 0)
            endif()
        endif()
        if(NOT ${PACKAGE_FOUND})
            set(ALL_PLUGINS_FOUND 0)
            set(${PLUGIN_MDEPS} ${${PLUGIN_MDEPS}} ${package})
        endif()
    endforeach()
    set(MAKE_${name} ${ALL_PLUGINS_FOUND})
    LOG_PLUGIN(${PLUGIN_NAME} OPTION ${ALL_PLUGINS_FOUND} MDEPS ${${PLUGIN_MDEPS}})
    if(${ALL_PLUGINS_FOUND})
        set(bPLUGINS ${bPLUGINS} ${PLUGIN_NAME})
    else()
        set(nbPLUGINS ${nbPLUGINS} ${PLUGIN_NAME})
    endif()
endmacro()

###############################################################################
## Helper function to move list to string
###############################################################################
function(LIST_TO_STRING separator outvar)
    set ( tmp_str "" )
    list (LENGTH ARGN list_length)
    if ( ${list_length} LESS 2 )
        set ( tmp_str "${ARGN}" )
    else()
        math(EXPR last_index "${list_length} - 1")

        foreach( index RANGE ${last_index} )
            if ( ${index} GREATER 0 )
                list( GET ARGN ${index} element )
                set( tmp_str "${tmp_str}${separator}${element}")
            else()
                list( GET ARGN 0 element )
                set( tmp_str "${element}")
            endif()
        endforeach()
    endif()
    set ( ${outvar} "${tmp_str}" PARENT_SCOPE )
endfunction()

###############################################################################
## Helper function to separate string to list
###############################################################################
function(STRING_TO_LIST separator outvar stringvar)
    set( tmp_list "" )
    string(REPLACE "${separator}" ";" tmp_list ${stringvar})
    string(STRIP "${tmp_list}" tmp_list)
    set( ${outvar} "${tmp_list}" PARENT_SCOPE)
endfunction()

###############################################################################
## The following sets the dependency list for a package
###############################################################################
function(SET_DEPENDENCIES cpackvar)
    set(_tmp "")
    if(${cpackvar})
        STRING_TO_LIST(", " _tmp ${${cpackvar}})
    endif()
    foreach(element ${ARGN})
        list(APPEND _tmp ${element})
    endforeach()
    list(REMOVE_DUPLICATES _tmp)
    LIST_TO_STRING(", " _tmp "${_tmp}")
    set(${cpackvar} "${_tmp}" CACHE STRING "" FORCE)
    message(STATUS "Updated ${cpackvar} to ${${cpackvar}}")
endfunction()


###############################################################################
## Macro to sign modules with gpg key
###############################################################################
macro(SIGN_MODULE module)
    if(SIGN_MODULES)
        set(GPG_COMMAND_STR "gpg")
        if(DEFINED SIGN_MODULES_PASSPHRASE)
            set(GPG_COMMAND_STR "${GPG_COMMAND_STR} --passphrase ${SIGN_MODULES_PASSPHRASE}")
        endif()
        if(DEFINED SIGN_MODULES_KEYID)
            set(GPG_COMMAND_STR "${GPG_COMMAND_STR} --default-key ${SIGN_MODULES_KEYID}")
        endif()
        if("${GPG_VERSION}" VERSION_GREATER "2.1")
            set(GPG_COMMAND_STR "${GPG_COMMAND_STR} --pinentry-mode loopback")
        endif()
        set(GPG_COMMAND_STR "${GPG_COMMAND_STR} --batch --yes --no-tty --output ${CMAKE_CURRENT_BINARY_DIR}/${module} --clearsign ${module}")
        add_custom_command(
            OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/${module}
            COMMAND bash "-c" "${GPG_COMMAND_STR}"
            DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/${module}
            WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
            COMMENT "Adding signed ${module} to project"
            )
    else()
        add_custom_command(
            OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/${module}
            COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_SOURCE_DIR}/${module} ${CMAKE_CURRENT_BINARY_DIR}/${module}
            DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/${module}
            WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
            COMMENT "Adding unsigned ${module} to project"
            VERBATIM
            )
    endif()
    # Use custom target to cause build to fail if dependency file isn't generated by gpg or cp commands
    get_filename_component(module_without_extension ${module} NAME_WE)
    add_custom_target(
        ${module_without_extension}-ecl ALL
        DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/${module}
        )
    if(SIGN_MODULES)
        add_dependencies(${module_without_extension}-ecl export-stdlib-pubkey)
    endif()
endmacro()
