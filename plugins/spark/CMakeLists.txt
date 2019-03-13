################################################################################
#    HPCC SYSTEMS software Copyright (C) 2018 HPCC Systems.
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


#########################################################
# Description:
# ------------
#    Spark with HPCC
#
#
#########################################################
cmake_minimum_required(VERSION 3.3)

project(spark-integration)

if(SPARK)
    ADD_PLUGIN(spark PACKAGES Java MINVERSION 1.8.0)

    file(STRINGS ${CMAKE_CURRENT_SOURCE_DIR}/spark-hadoop/RELEASE SPARK_RELEASE_FULL)
    list(GET SPARK_RELEASE_FULL 0 SPARK_RELEASE)
    list(GET SPARK_RELEASE_FULL 1 SPARK_BUILD_FLAGS)
    message(STATUS "${SPARK_RELEASE}")
    message(STATUS "${SPARK_BUILD_FLAGS}")

    set(CENTRAL_REPO "https://repo1.maven.org/maven2")

    if(NOT SPARK_HPCC_JAR)
        if(NOT SPARK_HPCC_VERSION)
            set(SPARK_HPCC_VERSION "7.0.2")
        endif()
        file(DOWNLOAD
            ${CENTRAL_REPO}/org/hpccsystems/spark-hpcc/${SPARK_HPCC_VERSION}/spark-hpcc-${SPARK_HPCC_VERSION}.jar
            ${CMAKE_CURRENT_BINARY_DIR}/spark-hpcc-${SPARK_HPCC_VERSION}.jar
            INACTIVITY_TIMEOUT 30
            TIMEOUT 90)
        set(SPARK_HPCC_JAR ${CMAKE_CURRENT_BINARY_DIR}/spark-hpcc-${SPARK_HPCC_VERSION}.jar)
    endif()

    if(NOT WSCLIENT_JAR)    
        if(NOT WSCLIENT_VERSION)
            set(WSCLIENT_VERSION "7.0.2")
        endif()
        file(DOWNLOAD
            ${CENTRAL_REPO}/org/hpccsystems/wsclient/${WSCLIENT_VERSION}/wsclient-${WSCLIENT_VERSION}-jar-with-dependencies.jar
            ${CMAKE_CURRENT_BINARY_DIR}/wsclient-${WSCLIENT_VERSION}-jar-with-dependencies.jar
            INACTIVITY_TIMEOUT 30
            TIMEOUT 90)
        set(WSCLIENT_JAR ${CMAKE_CURRENT_BINARY_DIR}/wsclient-${WSCLIENT_VERSION}-jar-with-dependencies.jar)
    endif(NOT WSCLIENT_JAR)

    install(
        DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/spark-hadoop"
        USE_SOURCE_PERMISSIONS
        COMPONENT runtime
        DESTINATION "externals"
        )

    install(
        FILES 
            ${SPARK_HPCC_JAR}
            ${WSCLIENT_JAR}
        COMPONENT runtime
        DESTINATION "jars/spark/"
        )

    
    configure_file(spark-defaults.conf.in spark-defaults.conf @ONLY)
    configure_file(spark-env.sh.in spark-env.sh @ONLY)
    install(
        FILES
            ${CMAKE_CURRENT_BINARY_DIR}/spark-defaults.conf
            ${CMAKE_CURRENT_BINARY_DIR}/spark-env.sh
        COMPONENT runtime
        DESTINATION "externals/spark-hadoop/conf"
        )
    install(
        FILES
            ${CMAKE_CURRENT_SOURCE_DIR}/spark-env.install
        COMPONENT runtime
        DESTINATION "etc/init.d/install"
        )

    configure_file("${CMAKE_CURRENT_SOURCE_DIR}/sparkthor.sh.in" "${CMAKE_CURRENT_BINARY_DIR}/sparkthor.sh" @ONLY)
    configure_file("${CMAKE_CURRENT_SOURCE_DIR}/sparkthor-worker.sh.in" "${CMAKE_CURRENT_BINARY_DIR}/sparkthor-worker.sh" @ONLY)
    install(PROGRAMS 
        ${CMAKE_CURRENT_BINARY_DIR}/sparkthor.sh
        ${CMAKE_CURRENT_BINARY_DIR}/sparkthor-worker.sh
        DESTINATION sbin COMPONENT Runtime)

    configure_file(init_sparkthor.in init_sparkthor @ONLY)
    install(
        PROGRAMS ${CMAKE_CURRENT_BINARY_DIR}/init_sparkthor
        DESTINATION ${EXEC_DIR}
        COMPONENT Runtime)
    
    configure_file(sparkthor@instance.service.in sparkthor@.service @ONLY)
    install(FILES ${CMAKE_CURRENT_BINARY_DIR}/sparkthor@.service DESTINATION etc/systemd/system COMPONENT Systemd)

    if(PLATFORM)
        install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/sparkthor-service.install DESTINATION etc/init.d/install COMPONENT Systemd)
        install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/sparkthor-service.uninstall DESTINATION etc/init.d/uninstall COMPONENT Systemd)
    endif(PLATFORM)
endif(SPARK)
