file(GLOB src_files ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp)
file(GLOB header_files ${CMAKE_CURRENT_SOURCE_DIR}/include/*.hpp)

list(APPEND ALL_DOC_HEADERS ${header_files})
list(APPEND ALL_DOC_SRC ${src_files})
set(ALL_DOC_HEADERS ${ALL_DOC_HEADERS} PARENT_SCOPE)
set(ALL_DOC_SRC ${ALL_DOC_SRC} PARENT_SCOPE)

add_executable(GenTraceFit src/fitmain.cpp)
target_link_libraries(GenTraceFit PRIVATE Eigen3::Eigen pugixml GenScanFitters GenScanUtility ${ROOT_LIBRARIES})
list(APPEND EXE_LIST GenTraceFit)

add_executable(GenSplineFit src/makimamain.cpp)
target_link_libraries(GenSplineFit PRIVATE GenScanFitters GenScanUtility ${ROOT_LIBRARIES})
list(APPEND EXE_LIST GenSplineFit)

add_executable(GenPeakFit src/peakfittermain.cpp)
target_link_libraries(GenPeakFit PRIVATE Eigen3::Eigen GenScanFitters GenScanUtility ${ROOT_LIBRARIES} yaml-cpp::yaml-cpp)
list(APPEND EXE_LIST GenPeakFit)

add_executable(GenKMeansCluster src/kmeansclustermain.cpp)
target_link_libraries(GenKMeansCluster PRIVATE GenScanProcessors)
list(APPEND EXE_LIST GenKMeansCluster)

add_executable(GenTrapFilter src/trapfiltermain.cpp)
target_link_libraries(GenTrapFilter PRIVATE pugixml GenScanUtility ${ROOT_LIBRARIES})
list(APPEND EXE_LIST GenTrapFilter)

add_executable(GenMCHalfLifeFitter src/mcmchalflifefitter.cpp)
target_link_libraries(GenMCHalfLifeFitter PRIVATE GenScanUtility ${ROOT_LIBRARIES} yaml-cpp::yaml-cpp)
list(APPEND EXE_LIST GenMCHalfLifeFitter)

add_executable(GenPeakLocator src/locatepeaksmain.cpp)
target_link_libraries(GenPeakLocator PRIVATE GenScanUtility ${ROOT_LIBRARIES} yaml-cpp::yaml-cpp)
list(APPEND EXE_LIST GenPeakLocator)

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
