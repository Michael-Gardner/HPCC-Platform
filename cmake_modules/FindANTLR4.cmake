################################################################################
# Copyright (C) 2013 HPCC Systems.
#
# All rights reserved. This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU Affero General Public License as
# published by the Free Software Foundation, either version 3 of the
# License, or (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
# GNU Affero General Public License for more details.
#
# You should have received a copy of the GNU Affero General Public License
# along with this program. If not, see <http://www.gnu.org/licenses/>.
################################################################################
#
# - Attempt to find the ANTLR jar needed to build and run ANTLR Lexers and Parsers
# Once done this will define
#
# ANTLR_FOUND - ANTLR found in local system
# ANTLR_BUILDTIME_JAR - The jar needed to build/generate ANTLR Lexers and Parsers

find_package(Java REQUIRED)

file(DOWNLOAD http://www.antlr.org/download/antlr-4.7.1-complete.jar ${CMAKE_BINARY_DIR}/antlr-4.7.1-complete.jar)

add_custom_target(antlr4-complete
    DEPENDS ${CMAKE_BINARY_DIR}/antlr-4.7.1-complete.jar
    COMMENT "Fetch antlr-4.7.1-complete.jar")

include(UseJava)
find_jar(ANTLR4_JAR
    NAMES antlr-4.7.1-complete
    PATHS ${CMAKE_BINARY_DIR}
    )
include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(
    ANTLR4
    DEFAULT_MSG
    ANTLR4_JAR
    )
mark_as_advanced(ANTLR4_JAR)

function(ANTLR4_TARGET)
    # argument setup
    set(options GENERATE_VISITOR)
    set(oneValueArgs GRAMMAR_PREFIX LANGUAGE DESTINATION)
    set(multiValueArgs GRAMMAR_FILES)
    cmake_parse_arguments(antlr "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})
    list(GET ARGN 0 target_name)
    # argument handling
    if(antlr_LANGUAGE)
        set(antlr_options ${antlr_options} -Dlanguage=${antlr_LANGUAGE})
    endif()
    if(antlr_GENERATE_VISITOR)
        set(antlr_options ${antlr_options} -visitor)
    endif()
    if(antlr_DESTINATION)
        # normalize path
        string(REGEX REPLACE "[/]$" "" antlr_DESTINATION "${antlr_DESTINATION}")
        set(antlr_options ${antlr_options} -o ${antlr_DESTINATION})
    endif()

    set(generated_sources
        ${antlr_DESTINATION}/${antlr_GRAMMAR_PREFIX}Lexer.cpp
        ${antlr_DESTINATION}/${antlr_GRAMMAR_PREFIX}Parser.cpp
        ${antlr_DESTINATION}/${antlr_GRAMMAR_PREFIX}ParserBaseListener.cpp
        ${antlr_DESTINATION}/${antlr_GRAMMAR_PREFIX}ParserListener.cpp
        )
    if(antlr_GENERATE_VISITOR)
        set(generated_sources
            ${generated_sources}
            ${antlr_DESTINATION}/${antlr_GRAMMAR_PREFIX}ParserBaseVisitor.cpp
            ${antlr_DESTINATION}/${antlr_GRAMMAR_PREFIX}ParserVisitor.cpp
            )
    endif()
    set(generated_headers
        ${antlr_DESTINATION}/${antlr_GRAMMAR_PREFIX}Lexer.h
        ${antlr_DESTINATION}/${antlr_GRAMMAR_PREFIX}Parser.h
        ${antlr_DESTINATION}/${antlr_GRAMMAR_PREFIX}ParserBaseListener.h
        ${antlr_DESTINATION}/${antlr_GRAMMAR_PREFIX}ParserListener.h
        )
    if(antlr_GENERATE_VISITOR)
        set(generated_headers
            ${generated_headers}
            ${antlr_DESTINATION}/${antlr_GRAMMAR_PREFIX}ParserBaseVisitor.h
            ${antlr_DESTINATION}/${antlr_GRAMMAR_PREFIX}ParserVisitor.h
            )
    endif()
    set(generated_misc
        ${antlr_DESTINATION}/${antlr_GRAMMAR_PREFIX}Parser.tokens
        ${antlr_DESTINATION}/${antlr_GRAMMAR_PREFIX}Lexer.tokens
        )

    add_custom_command(OUTPUT ${generated_sources} ${generated_headers} ${generated_misc}
        COMMAND ${Java_JAVA_EXECUTABLE} -classpath ${ANTLR4_JAR} org.antlr.v4.Tool ${antlr_options} ${antlr_GRAMMAR_FILES}
        COMMENT "Generating ANTLR4 Lexer, Parser, and Listeners from grammars"
        DEPENDS antlr4-complete ${antlr_GRAMMAR_FILES}
        VERBATIM
        )
    add_custom_target("${target_name}"
        DEPENDS ${generated_sources} ${generated_headers} ${generated_misc}
        )
    set_source_files_properties(
        ${generated_sources}
        ${generated_headers}
        ${generated_misc}
        PROPERTIES GENERATED TRUE
        )
    if(antlr_DESTINATION)
        set_property(DIRECTORY PROPERTY INCLUDE_DIRECTORIES ${antlr_DESTINATION})
    endif()
    set_property(DIRECTORY PROPERTY
        ADDITIONAL_MAKE_CLEAN_FILES
            ${generated_sources}
            ${generated_headers}
            ${generated_misc}
        )
    set(ANTLR4_${target_name}_OUTPUTS ${generated_sources} ${generated_headers} ${generated_misc} PARENT_SCOPE)
    set(ANTLR4_${target_name}_SOURCES ${generated_sources} PARENT_SCOPE)
    set(ANTLR4_${target_name}_HEADERS ${generated_headers} PARENT_SCOPE)
    set(ANTLR4_${target_name}_MISC    ${generated_misc}    PARENT_SCOPE)
endfunction(ANTLR4_TARGET)
