file(GLOB src_files ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp)

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)

ROOT_GENERATE_DICTIONARY(RootDevStruct_root RootDevStruct.hpp LINKDEF include/RootDevStructLinkDef.hpp )
add_library(RootDevStruct SHARED RootDevStruct_root.cxx RootDevStruct_root)
target_link_libraries(RootDevStruct ${ROOT_LIBRARIES})
add_dependencies(RootDevStruct ${EXTERNAL_PROCESSOR_DEPS} ${EXTERNAL_LOG_DEPS})

ROOT_GENERATE_DICTIONARY(MtasStruct_root MtasStruct.hpp LINKDEF include/MtasStructLinkDef.hpp )
add_library(MtasStruct SHARED MtasStruct_root.cxx MtasStruct_root)
target_link_libraries(MtasStruct ${ROOT_LIBRARIES})
add_dependencies(MtasStruct ${EXTERNAL_PROCESSOR_DEPS} ${EXTERNAL_LOG_DEPS})

add_library(PROCESSOR_LIB ${src_files})
if( USE_SYSTEM_YAMLCPP )
target_link_libraries(PROCESSOR_LIB PUBLIC ${ROOT_LIBRARIES} pugixml yaml-cppd jsoncpp)
else()
target_link_libraries(PROCESSOR_LIB PUBLIC ${ROOT_LIBRARIES} pugixml yaml-cpp jsoncpp)
endif()
target_include_directories(PROCESSOR_LIB PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
add_dependencies(PROCESSOR_LIB ${EXTERNAL_PROCESSOR_DEPS} ${EXTERNAL_LOG_DEPS})

if(GS_PROCESSOR_DEBUG)
	target_compile_definitions(PROCESSOR_LIB PUBLIC PROCESSOR_DEBUG)
endif()
