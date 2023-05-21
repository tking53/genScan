file(GLOB src_files ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp)

add_library(UTILITY_LIB ${src_files})
target_include_directories(UTILITY_LIB PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
