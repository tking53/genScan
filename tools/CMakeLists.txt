file(GLOB src_files ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp)
file(GLOB header_files ${CMAKE_CURRENT_SOURCE_DIR}/include/*.hpp)

list(APPEND ALL_DOC_HEADERS ${header_files})
list(APPEND ALL_DOC_SRC ${src_files})
set(ALL_DOC_HEADERS ${ALL_DOC_HEADERS} PARENT_SCOPE)
set(ALL_DOC_SRC ${ALL_DOC_SRC} PARENT_SCOPE)

add_executable(GenConfig src/configmain.cpp)
add_dependencies(GenConfig ${EXTERNAL_DEPS})
target_link_libraries(GenConfig PRIVATE pugixml GenScanUtility ${ROOT_LIBRARIES})
list(APPEND EXE_LIST GenConfig)

add_executable(GenDev src/devmain.cpp)
add_dependencies(GenDev ${EXTERNAL_DEPS})
target_link_libraries(GenDev PRIVATE pugixml GenScanUtility ${ROOT_LIBRARIES})
list(APPEND EXE_LIST GenDev)

add_executable(GenTraceFit src/fitmain.cpp)
add_dependencies(GenTraceFit ${EXTERNAL_DEPS})
target_link_libraries(GenTraceFit PRIVATE Eigen3::Eigen pugixml GenScanFitters GenScanUtility ${ROOT_LIBRARIES})
list(APPEND EXE_LIST GenTraceFit)

add_executable(GenPeakFit src/peakfittermain.cpp)
add_dependencies(GenPeakFit ${EXTERNAL_DEPS})
target_link_libraries(GenPeakFit PRIVATE Eigen3::Eigen GenScanFitters GenScanUtility ${ROOT_LIBRARIES})
list(APPEND EXE_LIST GenPeakFit)

add_executable(GenTrapFilter src/trapfiltermain.cpp)
add_dependencies(GenTrapFilter ${EXTERNAL_DEPS})
target_link_libraries(GenTrapFilter PRIVATE pugixml GenScanUtility ${ROOT_LIBRARIES})
list(APPEND EXE_LIST GenTrapFilter)

if( USE_SYSTEM_YAMLCPP )
	target_link_libraries(GenPeakFit PUBLIC yaml-cppd)
else()
	target_link_libraries(GenPeakFit PUBLIC yaml-cpp)
endif()
if( USE_SYSTEM_FMT )
	target_link_libraries(GenConfig PRIVATE fmt::fmt)
	target_link_libraries(GenDev PRIVATE fmt::fmt)
	target_link_libraries(GenTraceFit PRIVATE fmt::fmt)
	target_link_libraries(GenTrapFilter PRIVATE fmt::fmt)
	target_link_libraries(GenPeakFit PRIVATE fmt::fmt)
endif()
if( USE_SYSTEM_SPDLOG )
	target_link_libraries(GenConfig PRIVATE spdlog::spdlog_header_only)
	target_link_libraries(GenDev PRIVATE spdlog::spdlog_header_only)
	target_link_libraries(GenTraceFit PRIVATE spdlog::spdlog_header_only)
	target_link_libraries(GenTrapFilter PRIVATE spdlog::spdlog_header_only)
	target_link_libraries(GenPeakFit PRIVATE spdlog::spdlog_header_only)
endif()
if( USE_SYSTEM_BOOST )
	target_link_libraries(GenConfig PRIVATE Boost::container Boost::program_options )
	target_link_libraries(GenDev PRIVATE Boost::container Boost::program_options )
	target_link_libraries(GenTraceFit PRIVATE Boost::container Boost::program_options )
	target_link_libraries(GenTrapFilter PRIVATE Boost::container Boost::program_options )
	target_link_libraries(GenPeakFit PRIVATE Boost::container Boost::program_options )
else()
	target_link_libraries(GenDev PRIVATE 
		${BOOST_EXTERNAL_INSTALL_LOCATION}/lib/libboost_container.a
		${BOOST_EXTERNAL_INSTALL_LOCATION}/lib/libboost_program_options.a
		)
	target_link_libraries(GenTraceFit PRIVATE 
		${BOOST_EXTERNAL_INSTALL_LOCATION}/lib/libboost_container.a
		${BOOST_EXTERNAL_INSTALL_LOCATION}/lib/libboost_program_options.a
		)
	target_link_libraries(GenTrapFilter PRIVATE 
		${BOOST_EXTERNAL_INSTALL_LOCATION}/lib/libboost_container.a
		${BOOST_EXTERNAL_INSTALL_LOCATION}/lib/libboost_program_options.a
		)
	target_link_libraries(GenPeakFit PRIVATE 
		${BOOST_EXTERNAL_INSTALL_LOCATION}/lib/libboost_container.a
		${BOOST_EXTERNAL_INSTALL_LOCATION}/lib/libboost_program_options.a
		)
endif()
if(CMAKE_BUILD_TYPE STREQUAL "Debug" OR CMAKE_BUILD_TYPE STREQUAL "RelWithDebInfo")
	target_compile_definitions(GenConfig PRIVATE DEBUG_MODE)
	target_compile_definitions(GenDev PRIVATE DEBUG_MODE)
	target_compile_definitions(GenTraceFit PRIVATE DEBUG_MODE)
	target_compile_definitions(GenTrapFilter PRIVATE DEBUG_MODE)
endif()

install(TARGETS ${EXE_LIST} DESTINATION bin)
