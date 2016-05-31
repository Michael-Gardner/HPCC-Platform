
if(NOT PREFIX)
    if(WIN32)
        set(prog86key "ProgramFiles(x86)")
        if ("$ENV{${prog86key}}" STREQUAL "")
            set(PREFIX "$ENV{ProgramFiles}")
        else()
            set(PREFIX "$ENV{${prog86key}}")
        endif()
        string(REGEX REPLACE "\\\\" "/" PREFIX ${PREFIX})    
    else(WIN32)
        set(PREFIX "opt")
    endif(WIN32)    
endif()

if(NOT EXEC_PREFIX)
    set(EXEC_PREFIX "var")
endif()

if(NOT DIR_NAME)
    set(DIR_NAME "HPCCSystems")
endif()

if(NOT ARCHIVE_DIR)
    set(ARCHIVE_DIR "lib")
endif()

if(NOT HOME_DIR)
    set(HOME_DIR "/home")
endif()

if(NOT RUNTIME_USER)
    set(RUNTIME_USER "hpcc")
endif()

if(NOT RUNTIME_GROUP)
    set(RUNTIME_GROUP "hpcc")
endif()

# relative locations that get appended to $ENV{DESTDIR}CMAKE_INSTALL_PREFIX
if(NOT INSTALL_PATH)
    set( INSTALL_PATH "${PREFIX}/${DIR_NAME}" )
endif()

if(NOT CONFIG_DIR)
    set( CONFIG_DIR "etc/${DIR_NAME}" )
endif()

if(NOT LIB_DIR)
    set( LIB_DIR "${INSTALL_PATH}/lib" )
endif()

if(NOT BIN_DIR)
    set( BIN_DIR "${INSTALL_PATH}/bin" )
endif()

if(NOT SBIN_DIR)
    set( SBIN_DIR "${INSTALL_PATH}/sbin" )
endif()

if(NOT COMP_DIR)
    set( COMP_DIR "${INSTALL_PATH}/componentfiles" )
endif()

if(NOT PLUGIN_DIR)
    set( PLUGIN_DIR "${INSTALL_PATH}/plugins" )
endif()

if(NOT TEST_DIR)
    set( TEST_DIR "${INSTALL_PATH}/testing" )
endif()

if(NOT FILEHOOK_DIR)
    set( FILEHOOK_DIR "${INSTALL_PATH}/filehooks" )
endif()

if(NOT SHARE_DIR)
    set( SHARE_DIR "${INSTALL_PATH}/share" )
endif()

if(NOT CLASS_DIR)
    set( CLASS_DIR "${INSTALL_PATH}/classes" )
endif()
