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
# Component: esdlcomp
# Description:
# ------------
#    Cmake Input File for esdlcomp
################################################################################

project(esdlcomp)

find_package(BISON REQUIRED)
find_package(FLEX REQUIRED)
BISON_TARGET(esdlcomp-parser esdlgram.y ${CMAKE_CURRENT_BINARY_DIR}/esdlgram.cpp)
FLEX_TARGET(esdlcomp-lexer esdllex.l ${CMAKE_CURRENT_BINARY_DIR}/hidllex.cpp)
ADD_FLEX_BISON_DEPENDENCY(esdlcomp-lexer esdlcomp-parser)

set(SRCS
    ${BISON_esdlcomp-parser_OUTPUTS}
    ${FLEX_esdlcomp-lexer_OUTPUTS}
    ${CMAKE_CURRENT_SOURCE_DIR}/esdlcomp.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/esdl_utils.cpp
    )

include_directories(
    ${CMAKE_CURRENT_BINARY_DIR}   # for generated .hpp file
    ${HPCC_SOURCE_DIR}/system/include
    ${HPCC_SOURCE_DIR}/system/jlib
    ${CMAKE_CURRENT_SOURCE_DIR}
    )

ADD_DEFINITIONS( -D_CONSOLE -DESDLCOMP_EXPORTS )
if (CMAKE_COMPILER_IS_GNUCC OR CMAKE_COMPILER_IS_CLANG)
    ADD_DEFINITIONS( -O0 )
    set_source_files_properties(${CMAKE_CURRENT_BINARY_DIR}/esdllex.cpp PROPERTIES COMPILE_FLAGS "-Wno-sign-compare -Wno-unused-function -Wno-unneeded-internal-declaration")
    set_source_files_properties(esdlcomp.cpp PROPERTIES COMPILE_FLAGS "-Wno-unused-function")
endif ()

add_library(esdlcomp SHARED ${SRCS})

message(STATUS "Installing esdlcomp")
install(TARGETS esdlcomp
    RUNTIME DESTINATION bin
    LIBRARY DESTINATION lib
    COMPONENT tools)

target_link_libraries(esdlcomp jlib)
