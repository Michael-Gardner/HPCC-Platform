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
#####################################################
# Description:
# ------------
#    Cmake Input File for esdlcomp
#####################################################

project( esdlcomp )

find_package(FLEX REQUIRED)
find_package(BISON REQUIRED)

add_custom_command ( OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/esdlgram.cpp ${CMAKE_CURRENT_BINARY_DIR}/esdlgram.h
    COMMAND ${BISON_EXECUTABLE} --defines=${CMAKE_CURRENT_BINARY_DIR}/esdlgram.h --output=${CMAKE_CURRENT_BINARY_DIR}/esdlgram.cpp ${CMAKE_CURRENT_SOURCE_DIR}/esdlgram.y
    DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/esdlgram.y
)

add_custom_command ( OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/esdllex.cpp
    COMMAND ${FLEX_EXECUTABLE} --outfile=${CMAKE_CURRENT_BINARY_DIR}/esdllex.cpp --header-file=${CMAKE_CURRENT_BINARY_DIR}/esdllex.hpp ${CMAKE_CURRENT_SOURCE_DIR}/esdllex.l
    DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/esdllex.l
)

set ( SRCS
        ${CMAKE_CURRENT_BINARY_DIR}/esdlgram.cpp
        ${CMAKE_CURRENT_BINARY_DIR}/esdllex.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/esdlcomp.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/esdl_utils.cpp
    )

include_directories (
         ${CMAKE_CURRENT_BINARY_DIR}   # for generated .h file
         ./../../system/include
         ./../../system/jlib
         ${CMAKE_CURRENT_SOURCE_DIR}
    )

ADD_DEFINITIONS( -D_CONSOLE -DESDLCOMP_EXPORTS )
if (CMAKE_COMPILER_IS_GNUCC OR CMAKE_COMPILER_IS_CLANG)
    ADD_DEFINITIONS( -O0 )
    set_source_files_properties(${CMAKE_CURRENT_BINARY_DIR}/esdllex.cpp PROPERTIES COMPILE_FLAGS "-Wno-sign-compare -Wno-unused-function -Wno-unneeded-internal-declaration")
    set_source_files_properties(esdlcomp.cpp PROPERTIES COMPILE_FLAGS "-Wno-unused-function")
endif ()

ADD_LIBRARY ( esdlcomp SHARED ${SRCS} )

MESSAGE("----INSTALLING esdlcomp")
install ( TARGETS esdlcomp RUNTIME DESTINATION bin LIBRARY DESTINATION lib COMPONENT tools)

target_link_libraries ( esdlcomp
                   jlib )
