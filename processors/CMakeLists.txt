file(GLOB src_files ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp)

add_library(PROCESSOR_LIB ${src_files})
if( USE_SYSTEM_YAMLCPP )
target_link_libraries(PROCESSOR_LIB PUBLIC ${ROOT_LIBRARIES} pugixml yaml-cppd jsoncpp)
else()
target_link_libraries(PROCESSOR_LIB PUBLIC ${ROOT_LIBRARIES} pugixml yaml-cpp jsoncpp)
endif()
target_include_directories(PROCESSOR_LIB PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
add_dependencies(PROCESSOR_LIB ${EXTERNAL_PROCESSOR_DEPS} ${EXTERNAL_LOG_DEPS})

if(GS_PROCESSOR_DEBUG)
	target_compile_definitions(PROCESSOR_LIB PUBLIC PROCESSOR_DEBUG)
endif()
