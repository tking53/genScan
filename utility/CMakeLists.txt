file(GLOB src_files ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp)
file(GLOB header_files ${CMAKE_CURRENT_SOURCE_DIR}/include/*.hpp)

list(APPEND ALL_DOC_HEADERS ${header_files})
list(APPEND ALL_DOC_SRC ${src_files})
set(ALL_DOC_HEADERS ${ALL_DOC_HEADERS} PARENT_SCOPE)
set(ALL_DOC_SRC ${ALL_DOC_SRC} PARENT_SCOPE)

add_library(GenScanUtility ${src_files})
target_include_directories(GenScanUtility PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)

install(DIRECTORY include DESTINATION ${CMAKE_INSTALL_PREFIX})
install(TARGETS GenScanUtility DESTINATION ${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_LIBDIR})
