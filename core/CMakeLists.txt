file(GLOB src_files ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp)

add_library(CORE_LIB ${src_files})
if( USE_SYSTEM_YAMLCPP )
	target_link_libraries(CORE_LIB PUBLIC ${ROOT_LIBRARIES} UTILITY_LIB PROCESSOR_LIB pugixml yaml-cppd jsoncpp)
else()
	target_link_libraries(CORE_LIB PUBLIC ${ROOT_LIBRARIES} UTILITY_LIB PROCESSOR_LIB pugixml yaml-cpp jsoncpp)
endif()
target_include_directories(CORE_LIB PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
add_dependencies(CORE_LIB ${EXTERNAL_CORE_DEPS} ${EXTERNAL_LOG_DEPS})

if(GS_CORRELATOR_DEBUG)
	target_compile_definitions(CORE_LIB PUBLIC CORRELATOR_DEBUG)
endif()
