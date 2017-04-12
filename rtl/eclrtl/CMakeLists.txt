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

# Component: eclrtl 

#####################################################
# Description:
# ------------
#    Cmake Input File for eclrtl
#####################################################


project( eclrtl ) 

find_package(Boost COMPONENTS regex)

set (    SRCS 
         eclrtl.cpp 
         eclregex.cpp
         rtlbcd.cpp 
         rtldistr.cpp 
         rtlds.cpp 
         rtldynfield.cpp 
         rtlint.cpp 
         rtlkey.cpp 
         rtlqstr.cpp 
         rtlrank.cpp 
         rtlfield.cpp 
         rtlread.cpp 
         rtlrecord.cpp
         rtltype.cpp 
         rtlxml.cpp
         
         eclinclude4.hpp
         eclrtl.hpp
         eclrtl_imp.hpp
         rtlbcd.hpp
         rtldistr.hpp
         rtlds_imp.hpp
         rtlembed.hpp
         rtlfield.hpp
         rtlkey2.hpp
         rtlkey.hpp
         rtlread_imp.hpp
         rtlrecord.hpp
         rtlsize.hpp
         rtltype.hpp
         rtlbcdtest.cpp 
    )

include_directories ( 
         ./../../rtl/include 
         ./../../rtl/nbcd 
         ./../../system/include 
         ./../../system/jlib 
         ./../../roxie/roxiemem
         ./../../testing/unittests
         ${Boost_INCLUDE_DIR}
    )

ADD_DEFINITIONS( -D_USRDLL -DECLRTL_EXPORTS )

if (BOOST_REGEX_FOUND)
    if (CMAKE_COMPILER_IS_GNUCXX OR CMAKE_COMPILER_IS_CLANGXX)
        set_source_files_properties(eclregex.cpp PROPERTIES COMPILE_FLAGS -std=c++98)
    endif ()

    if (WIN32)
    else ()
        ADD_DEFINITIONS( -DBOOST_DYN_LINK )
    endif ()
endif ()

ADD_LIBRARY( eclrtl SHARED ${SRCS} )
target_link_libraries ( eclrtl 
    ${Boost_REGEX_LIBRARY}
      jlib
      nbcd
      roxiemem
    )

if (USE_ICU)
    include_directories (
         ${ICU_INCLUDE_DIR}
        )
    target_link_libraries ( eclrtl
          ${ICU_LIBRARIES}
        )
endif ()



if (NOT PLUGIN)
    install ( TARGETS eclrtl RUNTIME DESTINATION ${EXEC_DIR} LIBRARY DESTINATION ${LIB_DIR} ARCHIVE DESTINATION componentfiles/cl/lib COMPONENT eclrtl)
  FOREACH( iFILES
    ${CMAKE_CURRENT_SOURCE_DIR}/eclinclude4.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/eclrtl.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/eclrtl_imp.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/rtlbcd.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/rtldistr.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/rtlds_imp.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/rtlfield.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/rtlkey.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/rtlrecord.hpp
  )
    install ( FILES ${iFILES} DESTINATION componentfiles/cl/include COMPONENT Runtime )
  ENDFOREACH ( iFILES )
endif()
