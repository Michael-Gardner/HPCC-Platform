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
# Component: hidl 
# Description:
# ------------
#    Cmake Input File for hidl
################################################################################

project(hidl) 

find_package(BISON REQUIRED)
find_package(FLEX REQUIRED)
BISON_TARGET(hidl-parser hidlgram.y ${CMAKE_CURRENT_BINARY_DIR}/hidlgram.cpp)
FLEX_TARGET(hidl-lexer hidllex.l ${CMAKE_CURRENT_BINARY_DIR}/hidllex.cpp)
ADD_FLEX_BISON_DEPENDENCY(hidl-lexer hidl-parser)

set(SRCS 
    ${BISON_hidl-parser_OUTPUTS}
    ${FLEX_hidl-lexer_OUTPUTS}
    hidl_esp_ng.cpp 
    hidl_utils.cpp 
    hidlcomp.cpp 
    main.cpp 
    )

include_directories ( 
         ${CMAKE_CURRENT_BINARY_DIR}   # for generated .h file
         ${HPCC_SOURCE_DIR}/system/include
         ${CMAKE_CURRENT_SOURCE_DIR}
    )

set_source_files_properties(${CMAKE_CURRENT_BINARY_DIR}/hidlgram.cpp
    PROPERTIES COMPILE_FLAGS -fno-strict-aliasing)

add_definitions(-D_CONSOLE)
add_executable(hidl ${SRCS})
#install(TARGETS hidl RUNTIME DESTINATION ${EXEC_DIR} COMPONENT tools)
