file(GLOB src_files ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp)

add_library(FITTERS_LIB ${src_files})
target_link_libraries(FITTERS_LIB Eigen3::Eigen )
target_include_directories(FITTERS_LIB PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
