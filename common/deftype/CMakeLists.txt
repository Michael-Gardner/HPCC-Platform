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


# Component: deftype 
#####################################################
# Description:
# ------------
#    Cmake Input File for deftype
#####################################################

project( deftype ) 

set (    SRCS 
         deffield.cpp 
         deftype.cpp 
         defvalue.cpp 
         
         deffield.hpp
         deftype.hpp
         defvalue.hpp
         
    )

include_directories ( 
         ./../../common/deftype 
         ./../../rtl/eclrtl 
         ./../../rtl/nbcd 
         ./../../system/include 
         ./../../system/jlib 
    )

ADD_DEFINITIONS( -D_USRDLL -DDEFTYPE_EXPORTS )

add_library( deftype SHARED ${SRCS} )
set_target_properties(deftype PROPERTIES
    CXX_STANDARD_REQUIRED ON
    CXX_STANDARD 11)
install ( TARGETS deftype RUNTIME DESTINATION ${EXEC_DIR} LIBRARY DESTINATION ${LIB_DIR} COMPONENT common-library)
target_link_libraries ( deftype 
         jlib 
         nbcd
         eclrtl 
    )

