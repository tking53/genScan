file(GLOB src_files ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp)

add_library(PARSER_LIB ${src_files})
target_link_libraries(PARSER_LIB PUBLIC ${ROOT_LIBRARIES})
target_include_directories(PARSER_LIB PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
