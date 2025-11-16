#black magic to get the commit hash so we can embed it into genscan itself
execute_process(
	COMMAND git describe --always --dirty --long --tags
	OUTPUT_VARIABLE GIT_COMMIT_HASH
	OUTPUT_STRIP_TRAILING_WHITESPACE
)

execute_process(
	COMMAND git describe --always --dirty --long --all
	OUTPUT_VARIABLE GIT_INFO
	OUTPUT_STRIP_TRAILING_WHITESPACE
)

configure_file("${CMAKE_CURRENT_SOURCE_DIR}/include/git_version.hpp.in" "${CMAKE_BINARY_DIR}/generated/include/git_version.hpp" @ONLY)

#get the compiler info
configure_file("${CMAKE_CURRENT_SOURCE_DIR}/include/compiler_version.hpp.in" "${CMAKE_BINARY_DIR}/generated/include/compiler_version.hpp" @ONLY)

#get the cmake info
configure_file("${CMAKE_CURRENT_SOURCE_DIR}/include/cmake_info.hpp.in" "${CMAKE_BINARY_DIR}/generated/include/cmake_info.hpp" @ONLY)

file(GLOB header_files ${CMAKE_CURRENT_SOURCE_DIR}/include/*.hpp)

list(APPEND ALL_DOC_HEADERS ${header_files})

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)
