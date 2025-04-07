file(GLOB script_files ${CMAKE_CURRENT_SOURCE_DIR}/*.cxx)
install(FILES ${script_files} DESTINATION scripts)

configure_file("${CMAKE_CURRENT_SOURCE_DIR}/ParScan.py" ParScan @ONLY)
install(PROGRAMS ${CMAKE_BINARY_DIR}/scripts/ParScan DESTINATION scripts)
