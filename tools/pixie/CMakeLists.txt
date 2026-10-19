file(GLOB src_files ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp)
file(GLOB header_files ${CMAKE_CURRENT_SOURCE_DIR}/include/*.hpp)

list(APPEND ALL_DOC_HEADERS ${header_files})
list(APPEND ALL_DOC_SRC ${src_files})
set(ALL_DOC_HEADERS ${ALL_DOC_HEADERS} PARENT_SCOPE)
set(ALL_DOC_SRC ${ALL_DOC_SRC} PARENT_SCOPE)

add_executable(GenCSRATest src/csra_test.cpp)
list(APPEND EXE_LIST GenCSRATest)

add_executable(GenTrigConfigTest src/trigconfig_test.cpp)
list(APPEND EXE_LIST GenTrigConfigTest)

add_executable(GenModuleCSRBTest src/module_csrb_test.cpp)
list(APPEND EXE_LIST GenModuleCSRBTest)

add_executable(GenSet2Ascii src/set2root.cpp)
target_include_directories(GenSet2Ascii PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include)
list(APPEND EXE_LIST GenSet2Ascii)

add_executable(GenSet2Root src/set2root.cpp)
target_include_directories(GenSet2Root PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_compile_definitions(GenSet2Root PRIVATE USE_ROOT_OUTPUT)
target_link_libraries(GenSet2Root PRIVATE ${ROOT_LIBRARIES})
list(APPEND EXE_LIST GenSet2Root)

if( EXTERNAL_DEPS )
	foreach(CURREXE ${EXE_LIST})
		add_dependencies(${CURREXE} ${EXTERNAL_DEPS})	
	endforeach()
endif()

if( USE_SYSTEM_FMT )
	foreach( EXE IN LISTS EXE_LIST )
		target_link_libraries(${EXE} PRIVATE fmt::fmt)
	endforeach()
endif()
if( USE_SYSTEM_SPDLOG )
	foreach( EXE IN LISTS EXE_LIST )
		target_link_libraries(${EXE} PRIVATE spdlog::spdlog_header_only)
	endforeach()
endif()
if( USE_SYSTEM_BOOST )
	foreach( EXE IN LISTS EXE_LIST )
		target_link_libraries(${EXE} PRIVATE Boost::container Boost::program_options)
	endforeach()
else()
	foreach( EXE IN LISTS EXE_LIST )
		target_link_libraries(${EXE} PRIVATE 
			${BOOST_EXTERNAL_INSTALL_LOCATION}/${CMAKE_INSTALL_LIBDIR}/libboost_container.a
			${BOOST_EXTERNAL_INSTALL_LOCATION}/${CMAKE_INSTALL_LIBDIR}/libboost_program_options.a
			)
	endforeach()
endif()
if(CMAKE_BUILD_TYPE STREQUAL "Debug" OR CMAKE_BUILD_TYPE STREQUAL "RelWithDebInfo")
	foreach( EXE IN LISTS EXE_LIST )
		target_compile_definitions(${EXE} PRIVATE DEBUG_MODE)
	endforeach()
endif()

install(TARGETS ${EXE_LIST} DESTINATION bin)
install(DIRECTORY include DESTINATION ${CMAKE_INSTALL_PREFIX} PATTERN ".gitkeep" EXCLUDE)
