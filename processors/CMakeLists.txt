file(GLOB src_files ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp)
file(GLOB header_files ${CMAKE_CURRENT_SOURCE_DIR}/include/*.hpp)

list(APPEND ALL_DOC_HEADERS ${header_files})
list(APPEND ALL_DOC_SRC ${src_files})
set(ALL_DOC_HEADERS ${ALL_DOC_HEADERS} PARENT_SCOPE)
set(ALL_DOC_SRC ${ALL_DOC_SRC} PARENT_SCOPE)

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)

set(ROOT_CONTAINERS 
	"RVec.hxx"
	)

#NEED to manually specify the root collections
message(STATUS "Searching for ${ROOT_CONTAINERS} in ${ROOT_INCLUDE_DIR} and ${ROOT_INCLUDE_DIR}/ROOT to build the dictionaries for RDataFrames")

set(ROOT_CONTAINER_PATHS)

foreach(file IN LISTS ROOT_CONTAINERS)
	string(MAKE_C_IDENTIFIER "FOUND_${file}" _var)
	find_file(${_var}
		NAMES ${file}
		PATHS
		${ROOT_INCLUDE_DIR}
		${ROOT_INCLUDE_DIR}/ROOT
		NO_DEFAULT_PATH
	)

	if(${_var})
		message(STATUS "Found ${${_var}}")
		list(APPEND ROOT_CONTAINER_PATHS "${${_var}}")
	else()
		message(FATAL_ERROR "Unable to find ${file} in ${ROOT_INCLUDE_DIR}") 
	endif()

	# Clean up to avoid cache pollution
	unset(${_var} CACHE)
endforeach()

ROOT_GENERATE_DICTIONARY(RootDevStruct_root 
	RootDevStruct.hpp ${ROOT_CONTAINER_PATHS}
	LINKDEF include/RootDevStructLinkDef.hpp )
add_library(RootDevStruct SHARED RootDevStruct_root.cxx RootDevStruct_root)
target_link_libraries(RootDevStruct ${ROOT_LIBRARIES})
list(APPEND PROCESSOR_STRUCT_LIBS RootDevStruct)
list(APPEND PROCESSOR_STRUCT_PCM ${CMAKE_CURRENT_BINARY_DIR}/libRootDevStruct_root_rdict.pcm)
list(APPEND PROCESSOR_STRUCT_RMAP ${CMAKE_CURRENT_BINARY_DIR}/libRootDevStruct_root.rootmap)

ROOT_GENERATE_DICTIONARY(MtasStruct_root 
	MtasStruct.hpp ${ROOT_CONTAINER_PATHS}
	LINKDEF include/MtasStructLinkDef.hpp )
add_library(MtasStruct SHARED MtasStruct_root.cxx MtasStruct_root)
target_link_libraries(MtasStruct ${ROOT_LIBRARIES})
list(APPEND PROCESSOR_STRUCT_LIBS MtasStruct)
list(APPEND PROCESSOR_STRUCT_PCM ${CMAKE_CURRENT_BINARY_DIR}/libMtasStruct_root_rdict.pcm)
list(APPEND PROCESSOR_STRUCT_RMAP ${CMAKE_CURRENT_BINARY_DIR}/libMtasStruct_root.rootmap)

ROOT_GENERATE_DICTIONARY(MusesStruct_root 
	MusesStruct.hpp ${ROOT_CONTAINER_PATHS}
	LINKDEF include/MusesStructLinkDef.hpp )
add_library(MusesStruct SHARED MusesStruct_root.cxx MusesStruct_root)
target_link_libraries(MusesStruct ${ROOT_LIBRARIES})
list(APPEND PROCESSOR_STRUCT_LIBS MusesStruct)
list(APPEND PROCESSOR_STRUCT_PCM ${CMAKE_CURRENT_BINARY_DIR}/libMusesStruct_root_rdict.pcm)
list(APPEND PROCESSOR_STRUCT_RMAP ${CMAKE_CURRENT_BINARY_DIR}/libMusesStruct_root.rootmap)

ROOT_GENERATE_DICTIONARY(GroverStruct_root 
	GroverStruct.hpp ${ROOT_CONTAINER_PATHS}
	LINKDEF include/GroverStructLinkDef.hpp )
add_library(GroverStruct SHARED GroverStruct_root.cxx GroverStruct_root)
target_link_libraries(GroverStruct ${ROOT_LIBRARIES})
list(APPEND PROCESSOR_STRUCT_LIBS GroverStruct)
list(APPEND PROCESSOR_STRUCT_PCM ${CMAKE_CURRENT_BINARY_DIR}/libGroverStruct_root_rdict.pcm)
list(APPEND PROCESSOR_STRUCT_RMAP ${CMAKE_CURRENT_BINARY_DIR}/libGroverStruct_root.rootmap)

ROOT_GENERATE_DICTIONARY(VetoStruct_root 
	VetoStruct.hpp ${ROOT_CONTAINER_PATHS}
	LINKDEF include/VetoStructLinkDef.hpp )
add_library(VetoStruct SHARED VetoStruct_root.cxx VetoStruct_root)
target_link_libraries(VetoStruct ${ROOT_LIBRARIES})
list(APPEND PROCESSOR_STRUCT_LIBS VetoStruct)
list(APPEND PROCESSOR_STRUCT_PCM ${CMAKE_CURRENT_BINARY_DIR}/libVetoStruct_root_rdict.pcm)
list(APPEND PROCESSOR_STRUCT_RMAP ${CMAKE_CURRENT_BINARY_DIR}/libVetoStruct_root.rootmap)

ROOT_GENERATE_DICTIONARY(MtasImplantStruct_root 
	MtasImplantStruct.hpp ${ROOT_CONTAINER_PATHS} 
	LINKDEF include/MtasImplantStructLinkDef.hpp )
add_library(MtasImplantStruct SHARED MtasImplantStruct_root.cxx MtasImplantStruct_root)
target_link_libraries(MtasImplantStruct ${ROOT_LIBRARIES})
list(APPEND PROCESSOR_STRUCT_LIBS MtasImplantStruct)
list(APPEND PROCESSOR_STRUCT_PCM ${CMAKE_CURRENT_BINARY_DIR}/libMtasImplantStruct_root_rdict.pcm)
list(APPEND PROCESSOR_STRUCT_RMAP ${CMAKE_CURRENT_BINARY_DIR}/libMtasImplantStruct_root.rootmap)

ROOT_GENERATE_DICTIONARY(BSMStruct_root 
	BSMStruct.hpp ${ROOT_CONTAINER_PATHS} 
	LINKDEF include/BSMStructLinkDef.hpp )
add_library(BSMStruct SHARED BSMStruct_root.cxx BSMStruct_root)
target_link_libraries(BSMStruct ${ROOT_LIBRARIES})
list(APPEND PROCESSOR_STRUCT_LIBS BSMStruct)
list(APPEND PROCESSOR_STRUCT_PCM ${CMAKE_CURRENT_BINARY_DIR}/libBSMStruct_root_rdict.pcm)
list(APPEND PROCESSOR_STRUCT_RMAP ${CMAKE_CURRENT_BINARY_DIR}/libBSMStruct_root.rootmap)

ROOT_GENERATE_DICTIONARY(PidStruct_root 
	PidStruct.hpp ${ROOT_CONTAINER_PATHS} 
	LINKDEF include/PidStructLinkDef.hpp )
add_library(PidStruct SHARED PidStruct_root.cxx PidStruct_root)
target_link_libraries(PidStruct ${ROOT_LIBRARIES})
list(APPEND PROCESSOR_STRUCT_LIBS PidStruct)
list(APPEND PROCESSOR_STRUCT_PCM ${CMAKE_CURRENT_BINARY_DIR}/libPidStruct_root_rdict.pcm)
list(APPEND PROCESSOR_STRUCT_RMAP ${CMAKE_CURRENT_BINARY_DIR}/libPidStruct_root.rootmap)

ROOT_GENERATE_DICTIONARY(PSPMTStruct_root 
	PSPMTStruct.hpp ${ROOT_CONTAINER_PATHS} 
	LINKDEF include/PSPMTStructLinkDef.hpp )
add_library(PSPMTStruct SHARED PSPMTStruct_root.cxx PSPMTStruct_root)
target_link_libraries(PSPMTStruct ${ROOT_LIBRARIES})
add_dependencies(PSPMTStruct ${EXTERNAL_PROCESSOR_DEPS} ${EXTERNAL_LOG_DEPS})
list(APPEND PROCESSOR_STRUCT_LIBS PSPMTStruct)
list(APPEND PROCESSOR_STRUCT_PCM ${CMAKE_CURRENT_BINARY_DIR}/libPSPMTStruct_root_rdict.pcm)
list(APPEND PROCESSOR_STRUCT_RMAP ${CMAKE_CURRENT_BINARY_DIR}/libPSPMTStruct_root.rootmap)

add_library(GenScanProcessors ${src_files})
target_link_libraries(GenScanProcessors PUBLIC ${ROOT_LIBRARIES} GenScanFitters GenScanUtility pugixml)
target_include_directories(GenScanProcessors PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)

if( EXTERNAL_PROCESSOR_DEPS OR EXTERNAL_LOG_DEPS )
	foreach( CURRLIB ${PROCESSOR_STRUCT_LIBS} )
		add_dependencies(${CURRLIB} ${EXTERNAL_PROCESSOR_DEPS} ${EXTERNAL_LOG_DEPS})
	endforeach()
	add_dependencies(GenScanProcessors ${EXTERNAL_PROCESSOR_DEPS} ${EXTERNAL_LOG_DEPS})
endif()


if(GS_PROCESSOR_DEBUG)
	target_compile_definitions(GenScanProcessors PUBLIC PROCESSOR_DEBUG)
endif()

if(GS_EVENTSUMMARY_DEBUG)
	target_compile_definitions(GenScanProcessors PUBLIC EVENTSUMMARY_DEBUG)
endif()

install(DIRECTORY include DESTINATION ${CMAKE_INSTALL_PREFIX})
install(TARGETS GenScanProcessors DESTINATION ${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_LIBDIR})
install(TARGETS ${PROCESSOR_STRUCT_LIBS} DESTINATION ${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_LIBDIR})
install(FILES ${PROCESSOR_STRUCT_PCM} DESTINATION ${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_LIBDIR})
install(FILES ${PROCESSOR_STRUCT_RMAP} DESTINATION ${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_LIBDIR})
foreach( CURRLIB ${PROCESSOR_STRUCT_LIBS} )
	if( ${CMAKE_SYSTEM_NAME} MATCHES "Darwin" )
		install(CODE 
			"execute_process(COMMAND ${CMAKE_COMMAND} -E create_symlink lib${CURRLIB}.dylib ${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_LIBDIR}/lib${CURRLIB}_root.dylib)"
			)
	else()
		install(CODE 
			"execute_process(COMMAND ${CMAKE_COMMAND} -E create_symlink lib${CURRLIB}.so ${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_LIBDIR}/lib${CURRLIB}_root.so)"
			)
	endif()
endforeach()
