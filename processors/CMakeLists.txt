file(GLOB src_files ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp)

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)

ROOT_GENERATE_DICTIONARY(RootDevStruct_root RootDevStruct.hpp LINKDEF include/RootDevStructLinkDef.hpp )
add_library(RootDevStruct SHARED RootDevStruct_root.cxx RootDevStruct_root)
target_link_libraries(RootDevStruct ${ROOT_LIBRARIES})
add_dependencies(RootDevStruct ${EXTERNAL_PROCESSOR_DEPS} ${EXTERNAL_LOG_DEPS})

if( ${CMAKE_SYSTEM_NAME} MATCHES "Darwin" )
	add_custom_command(TARGET RootDevStruct
		POST_BUILD
		COMMAND ${CMAKE_COMMAND} -E create_symlink
		$<TARGET_FILE_NAME:RootDevStruct>
		libRootDevStruct_root.dylib
		WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/processors
		COMMENT "Creating symlink for RootDevStruct"
		)
else()
	add_custom_command(TARGET RootDevStruct
		POST_BUILD
		COMMAND ${CMAKE_COMMAND} -E create_symlink
		$<TARGET_FILE_NAME:RootDevStruct>
		libRootDevStruct_root.so
		WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/processors
		COMMENT "Creating symlink for RootDevStruct"
		)
endif()

ROOT_GENERATE_DICTIONARY(MtasStruct_root MtasStruct.hpp LINKDEF include/MtasStructLinkDef.hpp )
add_library(MtasStruct SHARED MtasStruct_root.cxx MtasStruct_root)
target_link_libraries(MtasStruct ${ROOT_LIBRARIES})
add_dependencies(MtasStruct ${EXTERNAL_PROCESSOR_DEPS} ${EXTERNAL_LOG_DEPS})
if( ${CMAKE_SYSTEM_NAME} MATCHES "Darwin" )
	add_custom_command(TARGET MtasStruct
		POST_BUILD
		COMMAND ${CMAKE_COMMAND} -E create_symlink
		$<TARGET_FILE_NAME:MtasStruct>
		libMtasStruct_root.dylib
		WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/processors
		COMMENT "Creating symlink for MtasStruct"
		)
else()
	add_custom_command(TARGET MtasStruct
		POST_BUILD
		COMMAND ${CMAKE_COMMAND} -E create_symlink
		$<TARGET_FILE_NAME:MtasStruct>
		libMtasStruct_root.so
		WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/processors
		COMMENT "Creating symlink for MtasStruct"
		)
endif()

add_library(PROCESSOR_LIB ${src_files})
if( USE_SYSTEM_YAMLCPP )
	target_link_libraries(PROCESSOR_LIB PUBLIC ${ROOT_LIBRARIES} UTILITY_LIB pugixml yaml-cpp jsoncpp)
else()
	target_link_libraries(PROCESSOR_LIB PUBLIC ${ROOT_LIBRARIES} UTILITY_LIB pugixml yaml-cpp jsoncpp)
endif()
target_include_directories(PROCESSOR_LIB PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
add_dependencies(PROCESSOR_LIB ${EXTERNAL_PROCESSOR_DEPS} ${EXTERNAL_LOG_DEPS})

if(GS_PROCESSOR_DEBUG)
	target_compile_definitions(PROCESSOR_LIB PUBLIC PROCESSOR_DEBUG)
endif()

if(GS_EVENTSUMMARY_DEBUG)
	target_compile_definitions(PROCESSOR_LIB PUBLIC EVENTSUMMARY_DEBUG)
endif()
