configure_file("start_backupnode.in" "start_backupnode" @ONLY)
FOREACH( iFILES
    ${CMAKE_CURRENT_BINARY_DIR}/start_backupnode 
    ${CMAKE_CURRENT_SOURCE_DIR}/get_thor_options
    ${CMAKE_CURRENT_SOURCE_DIR}/multiscp
    ${CMAKE_CURRENT_SOURCE_DIR}/multissh
    ${CMAKE_CURRENT_SOURCE_DIR}/scpslaves
    ${CMAKE_CURRENT_SOURCE_DIR}/setup_nfs
    ${CMAKE_CURRENT_SOURCE_DIR}/setup_one_nfs
    ${CMAKE_CURRENT_SOURCE_DIR}/sshslaves
)
    Install ( PROGRAMS ${iFILES} DESTINATION ${BIN_DIR} COMPONENT Runtime )
ENDFOREACH ( iFILES )
