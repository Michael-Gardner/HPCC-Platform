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

include(UseJava)
find_jar(ANTLR4_RUNTIME_JAR
    NAMES antlr4-runtime antlr-runtime
    PATHS /usr/share/java
    VERSIONS 4.6 4.7 4.8
    )
include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(
    ANTLR4-RUNTIME
    DEFAULT_MSG
    ANTLR4_RUNTIME_JAR
    )

mark_as_advanced(ANTLR4_RUNTIME_JAR)
