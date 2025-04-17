file(GLOB src_files ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp)
file(GLOB header_files ${CMAKE_CURRENT_SOURCE_DIR}/include/*.hpp)

list(APPEND ALL_DOC_HEADERS ${header_files})
list(APPEND ALL_DOC_SRC ${src_files})
set(ALL_DOC_HEADERS ${ALL_DOC_HEADERS} PARENT_SCOPE)
set(ALL_DOC_SRC ${ALL_DOC_SRC} PARENT_SCOPE)

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)

#file(GLOB linkdef_files "${CMAKE_CURRENT_SOURCE_DIR}/include/*LinkDef.hpp")
#
#foreach(ldef ${linkdef_files})
#	get_filename_component(barename ${ldef} NAME_WE)
#	string(REGEX MATCH "^(.*)LinkDef*$" dummy ${barename})
#	set(LibName ${CMAKE_MATCH_1})
#	message(STATUS "Found LinkDef ${ldef} ${LibName}")
#
#	set(RootName "${LibName}_root")
#	set(HeaderName "include/${LibName}.hpp")
#	set(CxxName "${RootName}.cxx")
#	set(LinkName "include/${barename}.hpp")
#	message(STATUS "Attempting to generate dictionary for ${barename} using ${RootName} ${HeaderName} ${LinkName}")
#	#ROOT_GENERATE_DICTIONARY(${RootName} ${HeaderName} LINKDEF ${LinkName})
#	add_custom_command( OUTPUT ${CxxName} 
#		COMMAND ${CMAKE_COMMAND} ROOT_GENERATE_DICTIONARY ${RootName} ${HeaderName} LINKDEF ${LinkName}
#		DEPENDS ${ldef} 
#		)
#	add_library(${LibName} SHARED ${CxxName} ${RootName})
#	target_link_libraries(${LibName} ${ROOT_LIBRARIES})
#	add_dependencies(${LibName} ${EXTERNAL_PROCESSOR_DEPS} ${EXTERNAL_LOG_DEPS})
#	list(APPEND PROCESSOR_STRUCT_LIBS ${LibName})
#	list(APPEND PROCESSOR_STRUCTS "lib${LibName}.so")
#	list(APPEND PROCESSOR_STRUCT_PCM "${CMAKE_CURRENT_BINARY_DIR}/lib${RootName}_rdict.pcm")
#	list(APPEND PROCESSOR_STRUCT_RMAP "${CMAKE_CURRENT_BINARY_DIR}/lib${RootName}.rootmap")
#endforeach()

ROOT_GENERATE_DICTIONARY(RootDevStruct_root RootDevStruct.hpp LINKDEF include/RootDevStructLinkDef.hpp )
add_library(RootDevStruct SHARED RootDevStruct_root.cxx RootDevStruct_root)
target_link_libraries(RootDevStruct ${ROOT_LIBRARIES})
list(APPEND PROCESSOR_STRUCT_LIBS RootDevStruct)
list(APPEND PROCESSOR_STRUCT_PCM ${CMAKE_CURRENT_BINARY_DIR}/libRootDevStruct_root_rdict.pcm)
list(APPEND PROCESSOR_STRUCT_RMAP ${CMAKE_CURRENT_BINARY_DIR}/libRootDevStruct_root.rootmap)

ROOT_GENERATE_DICTIONARY(MtasStruct_root MtasStruct.hpp LINKDEF include/MtasStructLinkDef.hpp )
add_library(MtasStruct SHARED MtasStruct_root.cxx MtasStruct_root)
target_link_libraries(MtasStruct ${ROOT_LIBRARIES})
list(APPEND PROCESSOR_STRUCT_LIBS MtasStruct)
list(APPEND PROCESSOR_STRUCT_PCM ${CMAKE_CURRENT_BINARY_DIR}/libMtasStruct_root_rdict.pcm)
list(APPEND PROCESSOR_STRUCT_RMAP ${CMAKE_CURRENT_BINARY_DIR}/libMtasStruct_root.rootmap)

ROOT_GENERATE_DICTIONARY(MtasImplantStruct_root MtasImplantStruct.hpp LINKDEF include/MtasImplantStructLinkDef.hpp )
add_library(MtasImplantStruct SHARED MtasImplantStruct_root.cxx MtasImplantStruct_root)
target_link_libraries(MtasImplantStruct ${ROOT_LIBRARIES})
list(APPEND PROCESSOR_STRUCT_LIBS MtasImplantStruct)
list(APPEND PROCESSOR_STRUCT_PCM ${CMAKE_CURRENT_BINARY_DIR}/libMtasImplantStruct_root_rdict.pcm)
list(APPEND PROCESSOR_STRUCT_RMAP ${CMAKE_CURRENT_BINARY_DIR}/libMtasImplantStruct_root.rootmap)

ROOT_GENERATE_DICTIONARY(BSMTraceFitStruct_root BSMTraceFitStruct.hpp LINKDEF include/BSMTraceFitStructLinkDef.hpp )
add_library(BSMTraceFitStruct SHARED BSMTraceFitStruct_root.cxx BSMTraceFitStruct_root)
target_link_libraries(BSMTraceFitStruct ${ROOT_LIBRARIES})
list(APPEND PROCESSOR_STRUCT_LIBS BSMTraceFitStruct)
list(APPEND PROCESSOR_STRUCT_PCM ${CMAKE_CURRENT_BINARY_DIR}/libBSMTraceFitStruct_root_rdict.pcm)
list(APPEND PROCESSOR_STRUCT_RMAP ${CMAKE_CURRENT_BINARY_DIR}/libBSMTraceFitStruct_root.rootmap)

ROOT_GENERATE_DICTIONARY(PidStruct_root PidStruct.hpp LINKDEF include/PidStructLinkDef.hpp )
add_library(PidStruct SHARED PidStruct_root.cxx PidStruct_root)
target_link_libraries(PidStruct ${ROOT_LIBRARIES})
list(APPEND PROCESSOR_STRUCT_LIBS PidStruct)
list(APPEND PROCESSOR_STRUCT_PCM ${CMAKE_CURRENT_BINARY_DIR}/libPidStruct_root_rdict.pcm)
list(APPEND PROCESSOR_STRUCT_RMAP ${CMAKE_CURRENT_BINARY_DIR}/libPidStruct_root.rootmap)


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
install(TARGETS GenScanProcessors DESTINATION ${CMAKE_INSTALL_PREFIX}/lib)
install(TARGETS ${PROCESSOR_STRUCT_LIBS} DESTINATION ${CMAKE_INSTALL_PREFIX}/lib)
install(FILES ${PROCESSOR_STRUCT_PCM} DESTINATION ${CMAKE_INSTALL_PREFIX}/lib)
install(FILES ${PROCESSOR_STRUCT_RMAP} DESTINATION ${CMAKE_INSTALL_PREFIX}/lib)
foreach( CURRLIB ${PROCESSOR_STRUCT_LIBS} )
	if( ${CMAKE_SYSTEM_NAME} MATCHES "Darwin" )
		install(CODE 
			"execute_process(COMMAND ${CMAKE_COMMAND} -E create_symlink lib${CURRLIB}.dylib ${CMAKE_INSTALL_PREFIX}/lib/lib${CURRLIB}_root.dylib)"
			)
	else()
		install(CODE 
			"execute_process(COMMAND ${CMAKE_COMMAND} -E create_symlink lib${CURRLIB}.so ${CMAKE_INSTALL_PREFIX}/lib/lib${CURRLIB}_root.so)"
			)
	endif()
endforeach()
