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
add_subdirectory(base)
add_subdirectory(dafilesrv)
add_subdirectory(dalidiag)
add_subdirectory(dalistop)
add_subdirectory(datest)
if(USE_CPPUNIT)
    add_subdirectory(daunittest)
endif()
add_subdirectory(dfu)
add_subdirectory(dfuplus)
add_subdirectory(dfuxref)
add_subdirectory(dfuXRefLib)
add_subdirectory(ft)
add_subdirectory(daliadmin)
add_subdirectory(server)
add_subdirectory(treeview)
add_subdirectory(updtdalienv)
add_subdirectory(sasha)
