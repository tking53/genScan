file(GLOB src_files ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp)
file(GLOB header_files ${CMAKE_CURRENT_SOURCE_DIR}/include/*.hpp)

list(APPEND ALL_DOC_HEADERS ${header_files})
list(APPEND ALL_DOC_SRC ${src_files})
set(ALL_DOC_HEADERS ${ALL_DOC_HEADERS} PARENT_SCOPE)
set(ALL_DOC_SRC ${ALL_DOC_SRC} PARENT_SCOPE)

add_library(GenScanTranslators ${src_files})
target_link_libraries(GenScanTranslators PUBLIC GenScanCore ${ROOT_LIBRARIES})
target_include_directories(GenScanTranslators PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
if( EXTERNAL_TRANSLATOR_DEPS OR EXTERNAL_LOG_DEPS )
	add_dependencies(GenScanTranslators ${EXTERNAL_TRANSLATOR_DEPS} ${EXTERNAL_LOG_DEPS})
endif()

if(GS_TRANSLATOR_DEBUG)
	target_compile_definitions(GenScanTranslators PUBLIC TRANSLATOR_DEBUG)
endif()

install(DIRECTORY include DESTINATION ${CMAKE_INSTALL_PREFIX})
install(TARGETS GenScanTranslators DESTINATION ${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_LIBDIR})
