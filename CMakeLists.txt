# @authors S.V. Paulauskas, K. Smith, and C. Thronsberry

cmake_minimum_required(VERSION 3.1)

project(GENSCAN C CXX )

#Set the C++ standard to c++11.
set(CMAKE_CXX_STANDARD 17)

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

#Define the default build type to be Release
# if (NOT CMAKE_BUILD_TYPE)
#     set(CMAKE_BUILD_TYPE Release CACHE STRING
#             "Build type, options are: None Debug Release RelWithDebInfo MinSizeRel." FORCE)
#     message(STATUS "Build type not defined, using default: ${CMAKE_BUILD_TYPE}")
# endif (NOT CMAKE_BUILD_TYPE)
# message(STATUS "Build type: ${CMAKE_BUILD_TYPE}")

#Add additional CXX flags if we use the Debug option
# if (CMAKE_BUILD_TYPE MATCHES "Debug")
set(CMAKE_BUILD_TYPE Debug CACHE STRING "Build type, options are: None Debug Release RelWithDebInfo MinSizeRel." FORCE)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pg")
message(STATUS "Build type: ${CMAKE_BUILD_TYPE}")
# endif (CMAKE_BUILD_TYPE MATCHES "Debug")

#------------------------------------------------------------------------------
#We are going to include this additional module here since it could be useful
#when setting all of the following options.
include(CMakeDependentOption)

#Install options
# option(PAASS_BUILD_ACQ "Build and install Acquisition software" ON)
# option(PAASS_BUILD_SCAN_UTILITIES "Build and install the Analysis Utilities" ON)
# option(PAASS_BUILD_SETUP "Include the older setup programs in installation" OFF)
# option(PAASS_BUILD_SHARED_LIBS "Install only scan libraries" ON)
# option(PAASS_BUILD_TESTS "Builds programs designed to test the package. Including UnitTest++ test." OFF)
# option(PAASS_BUILD_UTKSCAN "Build utkscan" OFF)
# option(PAASS_USE_DAMM "Use DAMM for MCA" ON)
# option(PAASS_USE_NCURSES "Use ncurses for terminal" ON)
# option(PAASS_USE_ROOT "Use ROOT (Currently REQUIRED!!)" ON)

# mark_as_advanced(PAASS_USE_NCURSES)
# mark_as_advanced(PAASS_USE_DAMM)
# mark_as_advanced(PAASS_USE_ROOT)


set(CMAKE_EXPORT_COMPILE_COMMANDS ON CACHE BOOL "Generates compile_commands.json, needed for some IDEs among other things" FORCE)
#option(GENSCAN_EXPORT_COMPILE_COMMANDS "CMAKE_EXPORT_COMPILE_COMMANDS WRAPPER" ON)
#mark_as_advanced(GENSCAN_EXPORT_COMPILE_COMMANDS)
#
##Generate the compile_commands.json file which is used by VSCode (among others) to generate the clang autocomplete
##Needs the PAASS_EXPORT_COMPILE_COMMANDS wrapper because of the "FORCE" which is required to even set the var from CMakeList.txt,
##while maintaining the ability to toggle it on/off without wiping the build dir
#if(GENSCAN_EXPORT_COMPILE_COMMANDS)
#    set(CMAKE_EXPORT_COMPILE_COMMANDS ON CACHE BOOL "Generates compile_commands.json, needed for some IDEs among other things" FORCE)
#else ()
#    set(CMAKE_EXPORT_COMPILE_COMMANDS OFF CACHE BOOL "Generates compile_commands.json, needed for some IDEs among other things" FORCE)   
#endif (GENSCAN_EXPORT_COMPILE_COMMANDS)

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

find_package(Pugixml REQUIRED)
option(USE_SPDLOG "Use either spdlog or messenger class" ON)

find_package(fmt REQUIRED)
find_package(spdlog REQUIRED)
add_compile_definitions(USE_SPDLOG)

add_subdirectory(utility)
add_subdirectory(core)

add_executable(GenScanor scanmain.cpp)

#include_directories(${PROJECT_SOURCE_DIR}/utility/include)
target_link_libraries(GenScanor PRIVATE CORE_LIB UTILITY_LIB spdlog::spdlog_header_only fmt::fmt)
