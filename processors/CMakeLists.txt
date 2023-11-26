file(GLOB src_files ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp)

add_library(PROCESSOR_LIB ${src_files})
target_link_libraries(PROCESSOR_LIB PUBLIC ${ROOT_LIBRARIES} pugixml yaml-cppd jsoncpp)
target_include_directories(PROCESSOR_LIB PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
