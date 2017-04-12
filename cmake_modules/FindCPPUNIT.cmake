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


# - Try to find the CppUnit unit testing library
# Once done this will define
#
#  CPPUNIT_FOUND - system has the CppUnit library
#  CPPUNIT_INCLUDE_DIR - the CppUnit include directory
#  CPPUNIT_LIBRARIES - The libraries needed to use CppUnit

IF (NOT CPPUNIT_FOUND)
    SET (cppunit_dll "cppunit")


      FIND_PATH (CPPUNIT_INCLUDE_DIR NAMES cppunit/TestFixture.h)
    FIND_LIBRARY (CPPUNIT_LIBRARIES NAMES ${cppunit_dll})

  include(FindPackageHandleStandardArgs)
  find_package_handle_standard_args(CppUnit DEFAULT_MSG
    CPPUNIT_LIBRARIES 
    CPPUNIT_INCLUDE_DIR
  )

  MARK_AS_ADVANCED(CPPUNIT_INCLUDE_DIR CPPUNIT_LIBRARIES)
ENDIF()
