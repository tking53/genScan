# Set the output directory to the install path
set(DOXYGEN_OUTPUT_DIR "${CMAKE_INSTALL_PREFIX}/docs")
set(DOXYGEN_IN ${CMAKE_CURRENT_SOURCE_DIR}/Doxyfile.in)
set(DOXYGEN_OUT ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile)

option(BUILD_DOCS_LATEX "Build latex documentation YES/NO" "NO")
set_property(CACHE BUILD_DOCS_LATEX PROPERTY STRINGS NO YES)
mark_as_advanced(BUILD_DOCS_LATEX)

option(BUILD_DOCS_QUIET "Build latex documentation YES/NO" "YES")
set_property(CACHE BUILD_DOCS_QUIET PROPERTY STRINGS NO YES)
mark_as_advanced(BUILD_DOCS_QUIET)

list(REMOVE_DUPLICATES ALL_DOC_SRC)
string(REPLACE ";" " " ALL_DOC_SRC "${ALL_DOC_SRC}")
list(REMOVE_DUPLICATES ALL_DOC_HEADERS)
string(REPLACE ";" " " ALL_DOC_HEADERS "${ALL_DOC_HEADERS}")

# Configure Doxyfile with CMake
configure_file(${DOXYGEN_IN} ${DOXYGEN_OUT} @ONLY)

add_custom_target(doc_doxygen
    COMMAND ${DOXYGEN_EXECUTABLE} ${DOXYGEN_OUT}
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
    COMMENT "Generating API documentation with Doxygen"
    VERBATIM
)

# Optional: install the generated docs
install(CODE "execute_process(COMMAND ${CMAKE_COMMAND} --build . --target doc_doxygen WORKING_DIRECTORY ${CMAKE_BINARY_DIR})")
install(DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/html DESTINATION share/doc)
