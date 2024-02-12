# @authors S.V. Paulauskas, K. Smith, and C. Thronsberry

cmake_minimum_required(VERSION 3.1)

project(GENSCAN C CXX )

#Set the C++ standard to c++20.
if(NOT CMAKE_CXX_STANDARD)
	set(CMAKE_CXX_STANDARD 17)
	set(CMAKE_CXX_STANDARD_REQUIRED ON)
endif()


#Use rpath on Mac OS
set(CMAKE_MACOSX_RPATH TRUE)

if (CMAKE_COMPILER_IS_GNUCXX)
    #Make compiler messages nice with colored tags.
    if (${CMAKE_CXX_COMPILER_VERSION} VERSION_GREATER "4.9")
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fdiagnostics-color=auto")
    endif ()

    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -pedantic -fPIC")
endif ()

#if user does not specify prefix we assign it to the install directory
#@TODO I do not like the fact that it makes the install directory before I'm
#ready for it. There are cases where I do not want to actually install just
#build. This configuration adds additional junk to my directory that I may
#not actually want or need.
if (CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
    message(STATUS "Install Prefix not specified.")
    file(MAKE_DIRECTORY install)
    get_filename_component(INSTALL_DIR ${CMAKE_BINARY_DIR}/../install REALPATH)
    set(CMAKE_INSTALL_PREFIX ${INSTALL_DIR} CACHE PATH "Install Prefix" FORCE)
endif ()
message(STATUS "Installing to ${CMAKE_INSTALL_PREFIX}")

option(CORRELATOR_DEBUG "Enable debugging the correlator, output will be very slow" OFF)
set_property(CACHE CORRELATOR_DEBUG PROPERTY ADVANCED OFF)

#Add additional CXX flags if we use the Debug option
set(CMAKE_BUILD_TYPE Debug CACHE STRING "Build type, options are: None Debug Release RelWithDebInfo MinSizeRel." FORCE)
if(NOT CMAKE_BUILD_TYPE)
	set(CMAKE_BUILD_TYPE "Release" CACHE STRING "Choose the type of build (Debug, Release, RelWithDebInfo, MinSizeRel)" FORCE)
	message(STATUS "Build type: ${CMAKE_BUILD_TYPE}")
endif()

if(CMAKE_BUILD_TYPE STREQUAL "Debug" OR CMAKE_BUILD_TYPE STREQUAL "RelWithDebInfo" ) 
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pg -Wall -Wextra")
endif()
# endif (CMAKE_BUILD_TYPE MATCHES "Debug")

#------------------------------------------------------------------------------
#We are going to include this additional module here since it could be useful
#when setting all of the following options.
include(CMakeDependentOption)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON CACHE BOOL "Generates compile_commands.json, needed for some IDEs among other things" FORCE)
#------------------------------------------------------------------------------

#------------------------------------------------------------------------------
#Find packages needed for the software.
#Load additional find_package scripts.
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/Cmake_modules")

#Find ROOT if USE_ROOT was set.
find_package(ROOT REQUIRED COMPONENTS TreePlayer)
set(ROOTSYS $ENV{ROOTSYS} CACHE STRING "ROOT directory.")
find_program(ROOT_CONFIG_EXECUTABLE root-config PATHS ${ROOTSYS}/bin)
find_program(ROOTCINT_EXECUTABLE rootcint PATHS $ENV{ROOTSYS}/bin)
find_program(GENREFLEX_EXECUTABLE genreflex PATHS $ENV{ROOTSYS}/bin)
if (ROOT_CONFIG_EXECUTABLE)
    execute_process(
        COMMAND ${ROOT_CONFIG_EXECUTABLE} --prefix
        OUTPUT_VARIABLE ROOTSYS
        OUTPUT_STRIP_TRAILING_WHITESPACE)

    execute_process(
        COMMAND ${ROOT_CONFIG_EXECUTABLE} --version
        OUTPUT_VARIABLE ROOT_VERSION
        OUTPUT_STRIP_TRAILING_WHITESPACE)

    execute_process(
        COMMAND ${ROOT_CONFIG_EXECUTABLE} --incdir
        OUTPUT_VARIABLE ROOT_INCLUDE_DIR
        OUTPUT_STRIP_TRAILING_WHITESPACE)

    execute_process(
        COMMAND ${ROOT_CONFIG_EXECUTABLE} --libs
        OUTPUT_VARIABLE ROOT_LIBRARIES
        OUTPUT_STRIP_TRAILING_WHITESPACE)

    execute_process(
        COMMAND ${ROOT_CONFIG_EXECUTABLE} --glibs
        OUTPUT_VARIABLE ROOT_GUI_LIBRARIES
        OUTPUT_STRIP_TRAILING_WHITESPACE)

    set(ROOT_LIBRARY_DIR ${ROOTSYS}/lib)

    execute_process(
        COMMAND ${ROOT_CONFIG_EXECUTABLE} --has-minuit2
        OUTPUT_VARIABLE ROOT_HAS_MINUIT2
        OUTPUT_STRIP_TRAILING_WHITESPACE)

    execute_process(
        COMMAND ${ROOT_CONFIG_EXECUTABLE} --has-cxx17
        OUTPUT_VARIABLE ROOT_HAS_CXX17)

    execute_process(
        COMMAND ${ROOT_CONFIG_EXECUTABLE} --has-cxx14
        OUTPUT_VARIABLE ROOT_HAS_CXX14)

    execute_process(
        COMMAND ${ROOT_CONFIG_EXECUTABLE} --has-cxx11
        OUTPUT_VARIABLE ROOT_HAS_CXX11)

endif ()

if (${ROOT_VERSION} VERSION_LESS "6.10")
    message(FATAL_ERROR "Minimum ROOT version is 6.10.00.")
endif (${ROOT_VERSION} VERSION_LESS "6.10")
mark_as_advanced(FORCE GENREFLEX_EXECUTABLE ROOTCINT_EXECUTABLE ROOT_CONFIG_EXECUTABLE)
include_directories(${ROOT_INCLUDE_DIR})
link_directories(${ROOT_LIBRARY_DIR})
#------------------------------------------------------------------------------

include(ExternalProject)

set(_LOG_OPTIONS 
	LOG_DOWNLOAD TRUE
	LOG_UPDATE TRUE
	LOG_TEST FALSE
	GIT_PROGRESS TRUE
	LOG_CONFIGURE TRUE
	LOG_BUILD TRUE
	LOG_INSTALL TRUE
	LOG_PATCH TRUE
	LOG_OUTPUT_ON_FAILURE TRUE
	)

option(USE_SYSTEM_PUGIXML "Use the system installed pugixml, if not then we will download one and use that one" OFF)
if( USE_SYSTEM_PUGIXML )
	find_package(Pugixml REQUIRED)
else()
	set(PUGI_EXTERNAL_INSTALL_LOCATION ${CMAKE_BINARY_DIR}/external/pugi)
	set(PUGI_VERSION v1.13 CACHE STRING "Version of pugixml you want to get from git" )
	mark_as_advanced(PUGI_VERSION)
	ExternalProject_Add(PugixmlProject
		GIT_REPOSITORY https://github.com/zeux/pugixml
		GIT_TAG ${PUGI_VERSION}
		CMAKE_ARGS -DCMAKE_INSTALL_PREFIX=${PUGI_EXTERNAL_INSTALL_LOCATION} -DCMAKE_VERBOSE_MAKEFILE=OFF -DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE}
		BUILD_COMMAND make -s
		INSTALL_COMMAND make -s install
		${_LOG_OPTIONS}
		)
	include_directories(${PUGI_EXTERNAL_INSTALL_LOCATION}/include)
	link_directories(${PUGI_EXTERNAL_INSTALL_LOCATION}/lib)
endif()
	
option(USE_SYSTEM_YAMLCPP "Use the system installed yaml-cpp, if not then we will download one and use that one" OFF)
if( USE_SYSTEM_YAMLCPP )
	find_package(yaml-cpp REQUIRED)
else()
	set(YAML_EXTERNAL_INSTALL_LOCATION ${CMAKE_BINARY_DIR}/external/yaml-cpp)
	set(YAML_VERSION 0.8.0 CACHE STRING "Version of yaml-cpp you want to get from git" )
	mark_as_advanced(YAML_VERSION)
	ExternalProject_Add(yaml-cppProject
		GIT_REPOSITORY https://github.com/jbeder/yaml-cpp
		GIT_TAG ${YAML_VERSION}
		CMAKE_ARGS -DCMAKE_INSTALL_PREFIX=${YAML_EXTERNAL_INSTALL_LOCATION} -DCMAKE_VERBOSE_MAKEFILE=OFF -DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE} -DYAML_CPP_BUILD_CONTRIB=OFF -DYAML_CPP_BUILD_TESTS=OFF -DYAML_CPP_BUILD_TOOLS=OFF
		BUILD_COMMAND make -s
		INSTALL_COMMAND make -s install
		${_LOG_OPTIONS}
		)
	include_directories(${YAML_EXTERNAL_INSTALL_LOCATION}/include)
	link_directories(${YAML_EXTERNAL_INSTALL_LOCATION}/lib)
endif()
	
option(USE_SYSTEM_JSONCPP "Use the system installed jsoncpp, if not then we will download one and use that one" OFF)
if( USE_SYSTEM_JSONCPP )
	find_package(jsoncpp REQUIRED)
else()
	set(JSON_EXTERNAL_INSTALL_LOCATION ${CMAKE_BINARY_DIR}/external/jsoncpp)
	set(JSON_VERSION 1.9.5 CACHE STRING "Version of jsoncpp you want to get from git" )
	mark_as_advanced(JSON_VERSION)
	ExternalProject_Add(jsoncppProject
		GIT_REPOSITORY https://github.com/open-source-parsers/jsoncpp
		GIT_TAG ${JSON_VERSION}
		CMAKE_ARGS -DCMAKE_INSTALL_PREFIX=${JSON_EXTERNAL_INSTALL_LOCATION} -DCMAKE_VERBOSE_MAKEFILE=OFF -DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE} -DJSONCPP_WITH_TESTS=OFF
		BUILD_COMMAND make -s
		INSTALL_COMMAND make -s install
		${_LOG_OPTIONS}
		)
	include_directories(${JSON_EXTERNAL_INSTALL_LOCATION}/include)
	link_directories(${JSON_EXTERNAL_INSTALL_LOCATION}/lib)
endif()
	
option(USE_SYSTEM_FMT "Use the system fmt library, if not then we will download and use that one" OFF)
if( USE_SYSTEM_FMT )
	find_package(fmt REQUIRED)
	set(_FMT_NAME fmt)
else()
	set(FMT_EXTERNAL_INSTALL_LOCATION ${CMAKE_BINARY_DIR}/external/fmt)
	set(FMT_VERSION 8.1.1 CACHE STRING "Version of fmt you want to get from git" )
	mark_as_advanced(FMT_VERSION)
	set(_FMT_NAME fmtProject)
	ExternalProject_Add(fmtProject
		GIT_REPOSITORY https://github.com/fmtlib/fmt
		GIT_TAG ${FMT_VERSION}
		CMAKE_ARGS -DCMAKE_INSTALL_PREFIX=${FMT_EXTERNAL_INSTALL_LOCATION} -DCMAKE_VERBOSE_MAKEFILE=OFF -DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE}
		BUILD_COMMAND make -s
		INSTALL_COMMAND make -s install
		${_LOG_OPTIONS}
		)
	include_directories(${FMT_EXTERNAL_INSTALL_LOCATION}/include)
	link_directories(${FMT_EXTERNAL_INSTALL_LOCATION}/lib)
endif()

option(USE_SYSTEM_SPDLOG "Use either spdlog or grab it from github" OFF)
if( USE_SYSTEM_SPDLOG )
	find_package(spdlog REQUIRED)
else()
	set(SPDLOG_EXTERNAL_INSTALL_LOCATION ${CMAKE_BINARY_DIR}/external/spdlog)
	set(SPDLOG_VERSION v1.9.2 CACHE STRING "Version of spdlog you want to get from git" )
	mark_as_advanced(SPDLOG_VERSION)
	ExternalProject_Add(spdlogProject
		GIT_REPOSITORY https://github.com/gabime/spdlog
		GIT_TAG ${SPDLOG_VERSION}
		CMAKE_ARGS -DCMAKE_INSTALL_PREFIX=${SPDLOG_EXTERNAL_INSTALL_LOCATION} -DCMAKE_VERBOSE_MAKEFILE=OFF -DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE}
		BUILD_COMMAND make -s
		INSTALL_COMMAND make -s install
		${_LOG_OPTIONS}
		DEPENDS ${_FMT_NAME}
		)
	include_directories(${SPDLOG_EXTERNAL_INSTALL_LOCATION}/include)
	link_directories(${SPDLOG_EXTERNAL_INSTALL_LOCATION}/lib)
endif()

option(USE_SYSTEM_BOOST "Use either system boost or grab one from github" OFF)
if( USE_SYSTEM_BOOST )
	find_package(Boost COMPONENTS container program_options REQUIRED)
	if(Boost_FOUND)
		include_directories(${Boost_INCLUDE_DIRS})
		message(STATUS "Boost found: ${Boost_VERSION}")
	else()
		message(FATAL_ERROR "System Boost not found.")
	endif()
else()
	set(BOOST_EXTERNAL_INSTALL_LOCATION ${CMAKE_BINARY_DIR}/external/boost)
	set(BOOST_VERSION boost-1.84.0 CACHE STRING "Version of boost you want to get from git" )
	mark_as_advanced(BOOST_VERSION)
	ExternalProject_Add(boostProject
		GIT_REPOSITORY https://github.com/boostorg/boost
		GIT_TAG ${BOOST_VERSION}
		CMAKE_ARGS -DCMAKE_INSTALL_PREFIX=${BOOST_EXTERNAL_INSTALL_LOCATION} -DCMAKE_VERBOSE_MAKEFILE=OFF -DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE}
		BUILD_COMMAND make -s
		INSTALL_COMMAND make -s install
		${_LOG_OPTIONS}
		DEPENDS ${_FMT_NAME}
		)
	include_directories(${BOOST_EXTERNAL_INSTALL_LOCATION}/include)
	link_directories(${BOOST_EXTERNAL_INSTALL_LOCATION}/lib)
endif()

add_subdirectory(utility)
add_subdirectory(processors)
add_subdirectory(core)
add_subdirectory(translators)

target_include_directories(PROCESSOR_LIB PUBLIC core/include)

add_executable(GenScanor scanmain.cpp)

#include_directories(${PROJECT_SOURCE_DIR}/utility/include)
target_link_libraries(GenScanor PRIVATE TRANSLATOR_LIB PROCESSOR_LIB CORE_LIB UTILITY_LIB )
if( USE_SYSTEM_FMT )
	target_link_libraries(GenScanor PRIVATE fmt::fmt)
endif()
if( USE_SYSTEM_SPDLOG )
	target_link_libraries(GenScanor PRIVATE spdlog::spdlog_header_only)
endif()
if( USE_SYSTEM_BOOST )
	target_link_libraries(GenScanor PRIVATE Boost::container Boost::program_options)
else()
	target_link_libraries(GenScanor PRIVATE 
		${BOOST_EXTERNAL_INSTALL_LOCATION}/lib/libboost_container.a
		${BOOST_EXTERNAL_INSTALL_LOCATION}/lib/libboost_program_options.a
		)
endif()
if(CMAKE_BUILD_TYPE STREQUAL "Debug" OR CMAKE_BUILD_TYPE STREQUAL "RelWithDebInfo")
	target_compile_definitions(GenScanor PRIVATE DEBUG_MODE)
endif()

add_executable(GenConfig configmain.cpp)
target_link_libraries(GenConfig PRIVATE UTILITY_LIB)
if(CMAKE_BUILD_TYPE STREQUAL "Debug" OR CMAKE_BUILD_TYPE STREQUAL "RelWithDebInfo")
	target_compile_definitions(GenConfig PRIVATE DEBUG_MODE)
endif()

