if(APPLE)
   # If APPLE requires the same scripts  if/elseif  will be removed
elseif(UNIX)
  configure_file(${CMAKE_CURRENT_SOURCE_DIR}/postinst.in postinst @ONLY)
  configure_file(${CMAKE_CURRENT_SOURCE_DIR}/prerm.in prerm @ONLY)
  configure_file(${CMAKE_CURRENT_SOURCE_DIR}/postrm.in postrm @ONLY)
endif(APPLE)

install(PROGRAMS post_install.sh DESTINATION ${SHARE_PATH}/install COMPONENT Runtime)
