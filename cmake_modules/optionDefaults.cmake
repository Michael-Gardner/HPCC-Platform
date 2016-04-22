
if(NOT PREFIX)
  if(WIN32)
    set(prog86key "ProgramFiles(x86)")
    if("$ENV{${prog86key}}" STREQUAL "")
      set(PREFIX "$ENV{ProgramFiles}")
    else()
      set(PREFIX "$ENV{${prog86key}}")
    endif()
    string(REGEX REPLACE "\\\\" "/" PREFIX ${PREFIX})    
  else(WIN32)
    set(PREFIX "/usr")
  endif (WIN32)    
endif()

#replace EXEC_PREFIX with RUNTIME_PREFIX
if(NOT RUNTIME_PREFIX)
  set(RUNTIME_PREFIX "/var")
endif()

if(NOT CONFIG_PREFIX)
  set(CONFIG_PREFIX "/etc")
endif()

#replace HOME_DIR with HOME_PREFIX like other absolutes
if(NOT HOME_PREFIX)
  set(HOME_PREFIX "/home")
endif()

if(NOT HPCC_PROJECT_NAME)
  set(HPCC_PROJECT_NAME "hpccsystems")
endif()

if(NOT LIB_DIR)
  set(LIB_DIR "lib")
endif()

if(NOT BIN_DIR)
  set(BIN_DIR "bin")
endif()

if(NOT SBIN_DIR)
  set(SBIN_DIR "sbin")
endif()

if(NOT COMPONENTFILES_DIR)
  set(COMPONENTFILES_DIR "componentfiles")
endif()

if(NOT SHARE_DIR)
  set(SHARE_DIR "share")
endif()

if(NOT PLUGINS_DIR)
  set(PLUGINS_DIR "${HPCC_PROJECT_NAME}-plugins")
endif()

if(NOT CONFIG_SOURCE_DIR)
  set(CONFIG_SOURCE_DIR "source")
endif()

if(NOT RUNTIME_DIR)
  set(RUNTIME_DIR "lib")
endif()

if(NOT LOCK_DIR)
  set(LOCK_DIR "lock")
endif()

if(NOT PID_DIR)
  set(PID_DIR "run")
endif()

if(NOT LOG_DIR)
  set(LOG_DIR "log")
endif()

if(NOT RUNTIME_USER)
  set(RUNTIME_USER "hpcc")
endif()

if(NOT RUNTIME_GROUP)
  set(RUNTIME_GROUP "hpcc")
endif()

if(NOT ENV_XML_FILE)
  set(ENV_XML_FILE "environment.xml")
endif()

if(NOT ENV_CONF_FILE)
  set(ENV_CONF_FILE "environment.conf")
endif()

set(CONFIG_PATH "${CONFIG_PREFIX}/${HPCC_PROJECT_NAME}")
set(RUNTIME_PATH "${RUNTIME_PREFIX}/${RUNTIME_DIR}/${HPCC_PROJECT_NAME}")
set(LOG_PATH "${RUNTIME_PREFIX}/${LOG_DIR}/${HPCC_PROJECT_NAME}")
set(LOCK_PATH "${RUNTIME_PREFIX}/${LOCK_DIR}/${HPCC_PROJECT_NAME}")
set(PID_PATH "${RUNTIME_PREFIX}/${PID_DIR}/${HPCC_PROJECT_NAME}")
set(SHARE_PATH "${PREFIX}/${SHARE_DIR}/${HPCC_PROJECT_NAME}")
set(CONFIG_SOURCE_PATH "${CONFIG_PATH}/${CONFIG_SOURCE_DIR}")
set(COMPONENTFILES_PATH "${SHARE_PATH}/${COMPONENTFILES_DIR}")
set(LIB_PATH "${PREFIX}/${LIB_DIR}")
set(PLUGINS_PATH "${LIB_PATH}/${PLUGINS_DIR}")
set(BIN_PATH "${PREFIX}/${BIN_DIR}")
set(SBIN_PATH "${PREFIX}/${SBIN_DIR}")
