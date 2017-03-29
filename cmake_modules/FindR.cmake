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

# - Try to find the R library
# Once done this will define
#
#  R_FOUND - system has the R library
#  R_INCLUDE_DIR - the R include directory(s)
#  R_LIBRARIES - The libraries needed to use R

if(NOT R_FOUND)
    if(WIN32)
        set(R_lib "libR")
        set(Rcpp_lib "libRcpp")
        set(RInside_lib "libRInside")
    else()
        set(R_lib "R")
        set(Rcpp_lib "Rcpp")
        set(RInside_lib "RInside")
    endif()

    find_path(R_INCLUDE_DIR R.h PATHS /usr/lib /usr/lib64 /usr/share /usr/local/lib /usr/local/lib64 PATH_SUFFIXES R/include)
    find_path(RCPP_INCLUDE_DIR Rcpp.h PATHS /usr/lib /usr/lib64 /usr/share /usr/local/lib /usr/local/lib64 PATH_SUFFIXES R/library/Rcpp/include/ R/site-library/Rcpp/include/)
    find_path(RINSIDE_INCLUDE_DIR RInside.h PATHS /usr/lib /usr/lib64 /usr/share /usr/local/lib /usr/local/lib64 PATH_SUFFIXES R/library/RInside/include  R/site-library/RInside/include)

    find_library(R_LIBRARY NAMES ${R_lib}  PATHS /usr/lib /usr/share /usr/lib64 /usr/local/lib /usr/local/lib64 PATH_SUFFIXES R/lib)
    find_library(RCPP_LIBRARY NAMES ${Rcpp_lib} PATHS /usr/lib /usr/share /usr/lib64 /usr/local/lib /usr/local/lib64 PATH_SUFFIXES R/library/Rcpp/lib/ R/site-library/Rcpp/lib/)
    find_library(RINSIDE_LIBRARY NAMES ${RInside_lib} PATHS /usr/lib /usr/share /usr/lib64 /usr/local/lib /usr/local/lib64 PATH_SUFFIXES R/library/RInside/lib/ R/site-library/RInside/lib/)

    if(RCPP_LIBRARY STREQUAL "RCPP_LIBRARY-NOTFOUND")
        set(RCPP_LIBRARY "")    # Newer versions of Rcpp are header-only, with no associated library.
    endif()

    #Rcpp/config.h
    #define RCPP_VERSION Rcpp_Version(0,12,3)
    file(STRINGS "${RCPP_INCLUDE_DIR}/Rcpp/config.h" version_string REGEX "#define RCPP_VERSION Rcpp_Version\\(")
    #major
    string(REGEX REPLACE "#define RCPP_VERSION Rcpp_Version\\(" "" major "${version_string}")
    string(REGEX REPLACE ",[0-9]+,[0-9]+\\)" "" major "${major}")
    #minor
    string(REGEX REPLACE "#define RCPP_VERSION Rcpp_Version\\([0-9]+," "" minor "${version_string}")
    string(REGEX REPLACE ",[0-9]+\\)" "" minor "${minor}")
    #patch
    string(REGEX REPLACE "#define RCPP_VERSION Rcpp_Version\\([0-9]+,[0-9]+," "" patch "${version_string}")
    string(REGEX REPLACE "\\)" "" patch "${patch}")
    set(RCPP_VERSION_STRING "${major}.${minor}.${patch}")

    set(R_INCLUDE_DIRS ${R_INCLUDE_DIR} ${RINSIDE_INCLUDE_DIR} ${RCPP_INCLUDE_DIR})
    set(R_LIBRARIES ${R_LIBRARY} ${RINSIDE_LIBRARY} ${RCPP_LIBRARY})

    include(FindPackageHandleStandardArgs)
    find_package_handle_standard_args(R
        REQUIRED_VARS R_LIBRARY
                      RINSIDE_LIBRARY
                      R_INCLUDE_DIR
                      RCPP_INCLUDE_DIR
                      RINSIDE_INCLUDE_DIR
                      R_LIBRARIES
                      R_INCLUDE_DIRS
        VERSION_VAR RCPP_VERSION_STRING
        )

    mark_as_advanced(R_INCLUDE_DIRS R_LIBRARIES RINSIDE_LIBRARY)
endif()
