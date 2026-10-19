file(GLOB script_files ${CMAKE_CURRENT_SOURCE_DIR}/*.cxx)
install(FILES ${script_files} DESTINATION scripts)

configure_file("${CMAKE_CURRENT_SOURCE_DIR}/ParScan.py" ParScan @ONLY)
install(PROGRAMS ${CMAKE_BINARY_DIR}/scripts/ParScan DESTINATION scripts)

configure_file("${CMAKE_CURRENT_SOURCE_DIR}/BulkFDSiScan.py" BulkFDSiScan @ONLY)
install(PROGRAMS ${CMAKE_BINARY_DIR}/scripts/BulkFDSiScan DESTINATION scripts)

configure_file("${CMAKE_CURRENT_SOURCE_DIR}/CalibrateMTASPMTs.py" CalibrateMTASPMTs @ONLY)
install(PROGRAMS ${CMAKE_BINARY_DIR}/scripts/CalibrateMTASPMTs DESTINATION scripts)

configure_file("${CMAKE_CURRENT_SOURCE_DIR}/CalibrateMTASSums.py" CalibrateMTASSums @ONLY)
install(PROGRAMS ${CMAKE_BINARY_DIR}/scripts/CalibrateMTASSums DESTINATION scripts)

configure_file("${CMAKE_CURRENT_SOURCE_DIR}/GainMatchMTAS.py" GainMatchMTAS @ONLY)
install(PROGRAMS ${CMAKE_BINARY_DIR}/scripts/GainMatchMTAS DESTINATION scripts)
