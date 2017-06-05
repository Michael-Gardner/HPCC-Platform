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
# Component: hql 
#
# Description:
# ------------
#    Cmake Input File for hql
################################################################################

project(hql) 

set(extrasources)
if(WIN32 AND ARCH64BIT)
    # 64bit windows builds do not support inline assembler
    # so the folding function needs to be defined in an asm file
    enable_language(ASM_MASM)
    set(extrasources hqlfoldasm.asm)
endif()

set(SRCS
    hqlatoms.cpp
    hqlattr.cpp
    hqlcollect.cpp
    hqldesc.cpp
    hqldsparam.cpp
    hqlerror.cpp
    hqlesp.cpp
    hqlexpr.cpp
    hqlfield.cpp
    hqlfold.cpp
    hqlgram2.cpp
    hqlmanifest.cpp
    hqlmeta.cpp
    hqlopt.cpp
    hqlparse.cpp
    hqlpmap.cpp
    hqlplugininfo.cpp
    hqlpregex.cpp
    hqlrepository.cpp
    hqlscope.cpp
    hqlstack.cpp
    hqlthql.cpp
    hqltrans.cpp
    hqlusage.cpp
    hqlutil.cpp
    hqlir.cpp
    hqlvalid.cpp
    hqlwuerr.cpp
    hqlxmldb.cpp
    reservedwords.cpp

    ${extrasources}

    hqlgram.y
    hqllex.l

    hqlerror.hpp
    hqlerrors.hpp
    hql.hpp
    hqlatoms.hpp
    hqlattr.hpp
    hqlcollect.hpp
    hqldesc.hpp
    hqlesp.hpp
    hqlexpr.hpp
    hqlfield.hpp
    hqlfold.hpp
    hqlgram.hpp
    hqlmeta.hpp
    hqlopt.hpp
    hqlopt.ipp
    hqlplugininfo.hpp
    hqlpmap.hpp
    hqlpregex.hpp
    hqlrepository.hpp
    hqlscope.hpp
    hqlstack.hpp
    hqlthor.hpp
    hqlthql.hpp
    hqlusage.hpp
    hqlutil.hpp
    hqlir.hpp
    hqlvalid.hpp
    hqlwuerr.hpp
    hqlxmldb.hpp
    reservedwords.hpp
    )

include_directories(
    ${CMAKE_CURRENT_BINARY_DIR}  # for generated header files
    ${CMAKE_BINARY_DIR}
    ${CMAKE_BINARY_DIR}/oss
    ./../../common/deftype 
    ./../../system/hrpc
    ./../../common/thorhelper
    ./../../rtl/eclrtl
    ./../../rtl/include
    ./../eclagent
    ./../../system/include
    ./../../common/workunit
    ./../../system/jlib
    ./../../ecl/hql
    ./../../testing/unittests
    ./../../system/security/zcrypt
    )

find_package(BISON 2.7.0 REQUIRED)
find_package(FLEX 2.5.35 REQUIRED)

BISON_TARGET(hql-parser hqlgram.y ${CMAKE_CURRENT_BINARY_DIR}/hqlgram.cpp
    DEFINES_FILE ${CMAKE_CURRENT_BINARY_DIR}/hqlgram.h)
FLEX_TARGET(hql-lexer hqllex.l ${CMAKE_CURRENT_BINARY_DIR}/hqllex.cpp
    COMPILE_FLAGS "--header-file=${CMAKE_CURRENT_BINARY_DIR}/hqllex.hpp")
ADD_FLEX_BISON_DEPENDENCY(hql-lexer hql-parser)

set_directory_properties(PROPERTIES
    ADDITIONAL_MAKE_CLEAN_FILES ${CMAKE_CURRENT_BINARY_DIR}/hqllex.hpp)

if(WIN32)
    set_source_files_properties(hqlgram.cpp PROPERTIES COMPILE_FLAGS "/Od")
endif()

add_definitions(-D_USRDLL -DHQL_EXPORTS -DHQLFOLD_EXPORTS -DHQLTRANS_EXPORTS)

add_library(hql SHARED ${SRCS} ${BISON_hql-parser_OUTPUT_SOURCE} ${FLEX_hql-lexer_OUTPUTS})
install(TARGETS hql RUNTIME DESTINATION ${EXEC_DIR} LIBRARY DESTINATION ${LIB_DIR} COMPONENT Runtime)
target_link_libraries(
    hql
    jlib
    nbcd
    eclrtl
    deftype
    ${CPPUNIT_LIBRARIES}
    )

if(USE_ZLIB)
    target_link_libraries(
        hql
        ${ZLIB_LIBRARIES}
        zcrypt
        )
endif()
