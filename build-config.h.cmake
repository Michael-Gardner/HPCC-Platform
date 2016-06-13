#ifndef ABS_EXEC_PREFIX
    #cmakedefine ABS_EXEC_PREFIX "${ABS_EXEC_PREFIX}"
#endif

#ifndef ABS_CONF_PREFIX
    #cmakedefine ABS_CONF_PREFIX "${ABS_CONF_PREFIX}"
#endif

#ifndef DIR_NAME
    #cmakedefine DIR_NAME "${DIR_NAME}"
#endif

#ifndef ABS_INSTALL_PATH
    #define ABS_INSTALL_PATH "${ABS_INSTALL_PATH}"
#endif

#ifndef ABS_COMP_PATH
    #cmakedefine ABS_COMP_PATH "${ABS_COMP_PATH}"
#endif

#ifndef ABS_CONF_PATH
    #define ABS_CONF_PATH "${ABS_CONF_PATH}"
#endif

#ifndef ABS_CONF_SOURCE_PATH
    #cmakedefine ABS_CONF_SOURCE_PATH "${ABS_CONF_SOURCE_PATH}"
#endif

#ifndef ABS_CLASS_PATH
    #cmakedefine ABS_CLASS_PATH "${ABS_CLASS_PATH}"
#endif

#ifndef ABS_SBIN_PATH    
    #cmakedefine ABS_SBIN_PATH "${ABS_SBIN_PATH}"
#endif

#ifndef ABS_RUNTIME_PATH
    #cmakedefine ABS_RUNTIME_PATH "${ABS_RUNTIME_PATH}"
#endif

#ifndef ENV_XML_FILE
    #cmakedefine ENV_XML_FILE "${ENV_XML_FILE}"
#endif

#ifndef ENV_CONF_FILE
    #cmakedefine ENV_CONF_FILE "${ENV_CONF_FILE}"
#endif

#ifndef BUILD_TAG
    #cmakedefine BUILD_TAG "${BUILD_TAG}"
#endif

#ifndef BUILD_VERSION_MAJOR
    #define BUILD_VERSION_MAJOR ${HPCC_MAJOR}
#endif

#ifndef BUILD_VERSION_MINOR
    #define BUILD_VERSION_MINOR ${HPCC_MINOR}
#endif

#ifndef BUILD_VERSION_POINT
    #define BUILD_VERSION_POINT ${HPCC_POINT}
#endif

#ifndef BASE_BUILD_TAG
    #cmakedefine BASE_BUILD_TAG "${BASE_BUILD_TAG}"
#endif

#ifndef BUILD_LEVEL
    #cmakedefine BUILD_LEVEL "${BUILD_LEVEL}"
#endif

#ifndef USE_RESOURCE
    #cmakedefine USE_RESOURCE "${USE_RESOURCE}"
#endif
