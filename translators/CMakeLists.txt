file(GLOB src_files ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp)

add_library(GenScanTranslators ${src_files})
target_link_libraries(GenScanTranslators PUBLIC GenScanCore ${ROOT_LIBRARIES})
target_include_directories(GenScanTranslators PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
add_dependencies(GenScanTranslators ${EXTERNAL_TRANSLATOR_DEPS} ${EXTERNAL_LOG_DEPS})

if(GS_TRANSLATOR_DEBUG)
	target_compile_definitions(GenScanTranslators PUBLIC TRANSLATOR_DEBUG)
endif()

install(DIRECTORY include DESTINATION ${CMAKE_INSTALL_PREFIX})
install(TARGETS GenScanTranslators DESTINATION ${CMAKE_INSTALL_PREFIX}/lib)
