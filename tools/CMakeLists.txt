file(GLOB src_files ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp)
file(GLOB header_files ${CMAKE_CURRENT_SOURCE_DIR}/include/*.hpp)

list(APPEND ALL_DOC_HEADERS ${header_files})
list(APPEND ALL_DOC_SRC ${src_files})
set(ALL_DOC_HEADERS ${ALL_DOC_HEADERS} PARENT_SCOPE)
set(ALL_DOC_SRC ${ALL_DOC_SRC} PARENT_SCOPE)

add_executable(GenConfig src/configmain.cpp)
target_link_libraries(GenConfig PRIVATE pugixml GenScanUtility ${ROOT_LIBRARIES})
list(APPEND EXE_LIST GenConfig)

add_executable(GenDev src/devmain.cpp)
target_link_libraries(GenDev PRIVATE pugixml GenScanUtility ${ROOT_LIBRARIES})
list(APPEND EXE_LIST GenDev)

add_executable(GenTraceFit src/fitmain.cpp)
target_link_libraries(GenTraceFit PRIVATE Eigen3::Eigen pugixml GenScanFitters GenScanUtility ${ROOT_LIBRARIES})
list(APPEND EXE_LIST GenTraceFit)

add_executable(GenPeakFit src/peakfittermain.cpp)
target_link_libraries(GenPeakFit PRIVATE Eigen3::Eigen GenScanFitters GenScanUtility ${ROOT_LIBRARIES} yaml-cpp)
list(APPEND EXE_LIST GenPeakFit)

add_executable(GenCalRipper src/calibrationrippermain.cpp)
target_link_libraries(GenCalRipper PRIVATE pugixml GenScanFitters GenScanUtility ${ROOT_LIBRARIES} yaml-cpp)
list(APPEND EXE_LIST GenCalRipper)

add_executable(GenTrapFilter src/trapfiltermain.cpp)
target_link_libraries(GenTrapFilter PRIVATE pugixml GenScanUtility ${ROOT_LIBRARIES})
list(APPEND EXE_LIST GenTrapFilter)

add_executable(GenCMapRegexTest src/channelmapregexchecker.cpp)
target_link_libraries(GenCMapRegexTest PRIVATE pugixml GenScanUtility ${ROOT_LIBRARIES})
list(APPEND EXE_LIST GenCMapRegexTest)

add_executable(GenUIDRegexTest src/uidregexchecker.cpp)
target_link_libraries(GenUIDRegexTest PRIVATE pugixml GenScanUtility ${ROOT_LIBRARIES})
list(APPEND EXE_LIST GenUIDRegexTest)

#add_executable(GenPeakLocator src/locatepeaksmain.cpp)
#target_link_libraries(GenPeakLocator PRIVATE pugixml GenScanUtility ${ROOT_LIBRARIES})
#list(APPEND EXE_LIST GenPeakLocator)

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
			${BOOST_EXTERNAL_INSTALL_LOCATION}/lib/libboost_container.a
			${BOOST_EXTERNAL_INSTALL_LOCATION}/lib/libboost_program_options.a
			)
	endforeach()
endif()
if(CMAKE_BUILD_TYPE STREQUAL "Debug" OR CMAKE_BUILD_TYPE STREQUAL "RelWithDebInfo")
	foreach( EXE IN LISTS EXE_LIST )
		target_compile_definitions(${EXE} PRIVATE DEBUG_MODE)
	endforeach()
endif()

install(TARGETS ${EXE_LIST} DESTINATION bin)
