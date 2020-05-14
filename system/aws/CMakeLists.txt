if(USE_AWS)

    project(aws)

    set(CMAKE_CXX_FLAGS "-Wno-error=implicit-fallthrough -Wno-error=int-in-bool-context -Wno-error=format-nonliteral")
    set(BUILD_SHARED_LIBS ON)
    add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/aws-c-common EXCLUDE_FROM_ALL)
    list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/aws-c-common/cmake")
    add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/aws-checksums EXCLUDE_FROM_ALL)
    add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/aws-c-event-stream EXCLUDE_FROM_ALL)
    
    #set(AWS_DEPS_INSTALL_DIR ${CMAKE_CURRENT_BINARY_DIR}/.deps/install CACHE STRING "A string describes the path where 3rd-party dependencies will be or have been installed")
    set(BUILD_ONLY "sqs;s3" CACHE STRING "semi colon seperated component list")
    set(BUILD_DEPS false CACHE BOOL "Build third-party dependencies")
    add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/aws-sdk-cpp EXCLUDE_FROM_ALL)

    # force build for installation with platform even if underlying library is only used for plugin (ex: sqs)
    add_custom_target(aws-generation ALL
        DEPENDS
            aws-cpp-sdk-core
            aws-cpp-sdk-sqs
            aws-cpp-sdk-s3
            )

    if(NOT PLUGIN)
        install(TARGETS
            aws-cpp-sdk-core
            aws-cpp-sdk-sqs
            aws-cpp-sdk-s3
            aws-c-event-stream
            aws-c-common
            aws-checksums
            DESTINATION lib)
    endif(NOT PLUGIN)
endif()
