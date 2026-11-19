file(GLOB src_files ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp)
file(GLOB header_files ${CMAKE_CURRENT_SOURCE_DIR}/include/*.hpp)

list(APPEND ALL_DOC_HEADERS ${header_files})
list(APPEND ALL_DOC_SRC ${src_files})
set(ALL_DOC_HEADERS ${ALL_DOC_HEADERS} PARENT_SCOPE)
set(ALL_DOC_SRC ${ALL_DOC_SRC} PARENT_SCOPE)

add_library(GenScanCore ${src_files})
target_link_libraries(GenScanCore PUBLIC ${ROOT_LIBRARIES} GenScanUtility GenScanProcessors pugixml)
target_include_directories(GenScanCore PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
if( EXTERNAL_CORE_DEPS OR EXTERNAL_LOG_DEPS )
	add_dependencies(GenScanCore ${EXTERNAL_CORE_DEPS} ${EXTERNAL_LOG_DEPS})
endif()

if(GS_CORRELATOR_DEBUG)
	target_compile_definitions(GenScanCore PUBLIC CORRELATOR_DEBUG)
endif()

install(DIRECTORY include DESTINATION ${CMAKE_INSTALL_PREFIX})
install(TARGETS GenScanCore DESTINATION ${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_LIBDIR})
