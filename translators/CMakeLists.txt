file(GLOB src_files ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp)

add_library(TRANSLATOR_LIB ${src_files})
target_link_libraries(TRANSLATOR_LIB PUBLIC CORE_LIB ${ROOT_LIBRARIES})
target_include_directories(TRANSLATOR_LIB PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
#add_dependencies(TRANSLATOR_LIB ${EXTERNAL_TRANSLATOR_DEPS} ${EXTERNAL_LOG_DEPS})

if(GS_TRANSLATOR_DEBUG)
	target_compile_definitions(TRANSLATOR_LIB PUBLIC TRANSLATOR_DEBUG)
endif()
