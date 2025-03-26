file(GLOB src_files ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp)

add_library(GenScanCore ${src_files})
if( USE_SYSTEM_YAMLCPP )
	target_link_libraries(GenScanCore PUBLIC ${ROOT_LIBRARIES} GenScanUtility GenScanProcessors pugixml yaml-cppd jsoncpp)
else()
	target_link_libraries(GenScanCore PUBLIC ${ROOT_LIBRARIES} GenScanUtility GenScanProcessors pugixml yaml-cpp jsoncpp)
endif()
target_include_directories(GenScanCore PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
add_dependencies(GenScanCore ${EXTERNAL_CORE_DEPS} ${EXTERNAL_LOG_DEPS})

if(GS_CORRELATOR_DEBUG)
	target_compile_definitions(GenScanCore PUBLIC CORRELATOR_DEBUG)
endif()

install(DIRECTORY include DESTINATION ${CMAKE_INSTALL_PREFIX})
install(TARGETS GenScanCore DESTINATION ${CMAKE_INSTALL_PREFIX}/lib)
