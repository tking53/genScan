file(GLOB src_files ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp)

add_library(GenScanFitters ${src_files})
target_link_libraries(GenScanFitters Eigen3::Eigen )
target_include_directories(GenScanFitters PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)

install(DIRECTORY include DESTINATION ${CMAKE_INSTALL_PREFIX})
install(TARGETS GenScanFitters DESTINATION ${CMAKE_INSTALL_PREFIX}/lib)
