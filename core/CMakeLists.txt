file(GLOB src_files ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp)

add_library(CORE_LIB ${src_files})
target_link_libraries(CORE_LIB PUBLIC ${ROOT_LIBRARIES} UTILITY_LIB pugixml yaml-cpp jsoncpp)
target_include_directories(CORE_LIB PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
