file(GLOB src_files ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp)

add_library(GenScanUtility ${src_files})
target_include_directories(GenScanUtility PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)

install(DIRECTORY include DESTINATION ${CMAKE_INSTALL_PREFIX})
install(TARGETS GenScanUtility DESTINATION ${CMAKE_INSTALL_PREFIX}/lib)
