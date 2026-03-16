file(GLOB src_files ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp)
file(GLOB header_files ${CMAKE_CURRENT_SOURCE_DIR}/include/*.hpp)

list(APPEND ALL_DOC_HEADERS ${header_files})
list(APPEND ALL_DOC_SRC ${src_files})
set(ALL_DOC_HEADERS ${ALL_DOC_HEADERS} PARENT_SCOPE)
set(ALL_DOC_SRC ${ALL_DOC_SRC} PARENT_SCOPE)

add_executable(GenConfig src/configmain.cpp)
target_link_libraries(GenConfig PRIVATE pugixml GenScanUtility ${ROOT_LIBRARIES})
list(APPEND EXE_LIST GenConfig)

add_executable(GenCalRipper src/calibrationrippermain.cpp)
target_link_libraries(GenCalRipper PRIVATE pugixml GenScanFitters GenScanUtility ${ROOT_LIBRARIES} yaml-cpp::yaml-cpp)
list(APPEND EXE_LIST GenCalRipper)

add_executable(GenTofShifter src/tofshiftermain.cpp)
target_link_libraries(GenTofShifter PRIVATE pugixml GenScanUtility ${ROOT_LIBRARIES} yaml-cpp::yaml-cpp)
list(APPEND EXE_LIST GenTofShifter)

add_executable(GenProcessorInserter src/processorinserter.cpp)
target_link_libraries(GenProcessorInserter PRIVATE pugixml GenScanUtility ${ROOT_LIBRARIES})
list(APPEND EXE_LIST GenProcessorInserter)

add_executable(GenProcessorDump src/dumpprocessormain.cpp)
target_link_libraries(GenProcessorDump PRIVATE pugixml GenScanUtility ${ROOT_LIBRARIES} )
list(APPEND EXE_LIST GenProcessorDump)

add_executable(GenCMapRegexTest src/channelmapregexchecker.cpp)
target_link_libraries(GenCMapRegexTest PRIVATE pugixml GenScanUtility ${ROOT_LIBRARIES})
list(APPEND EXE_LIST GenCMapRegexTest)

add_executable(GenUIDRegexTest src/uidregexchecker.cpp)
target_link_libraries(GenUIDRegexTest PRIVATE pugixml GenScanUtility ${ROOT_LIBRARIES})
list(APPEND EXE_LIST GenUIDRegexTest)

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
install(DIRECTORY include DESTINATION ${CMAKE_INSTALL_PREFIX})
