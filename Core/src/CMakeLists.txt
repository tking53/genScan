#utkscan will be more verbose in its output
if (GENSCAN_VERBOSE)
    add_definitions(-D VERBOSE)
endif (GENSCAN_VERBOSE)

#------------------------------------------------------------------------------

##added by T.T. King for Vector Root Libs
#cmake_policy(SET CMP0068 OLD)
#SET(CMAKE_SKIP_BUILD_RPATH  FALSE)
#SET(CMAKE_SKIP_INSTALL_RPATH FALSE)
#SET(CMAKE_BUILD_WITH_INSTALL_RPATH TRUE)

#SET(CMAKE_INSTALL_RPATH_USE_LINK_PATH TRUE)
#SET(CMAKE_INSTALL_RPATH "${CMAKE_INSTALL_PREFIX}/lib")

#ROOT_GENERATE_DICTIONARY(PaassRootStruct_root PaassRootStruct.hpp LINKDEF core/include/PaassRootLinkDef.hpp )
#add_library(PaassRootStruct SHARED PaassRootStruct_root.cxx PaassRootStruct_root)
#target_link_libraries(PaassRootStruct ${ROOT_LIBRARIES})  

#End T.T. King Additions
#------------------------------------------------------------------------------

set(EXE_NAME scan_all_things)
set(EXE_SOURCES genscan.cpp CompassROOTUnpacker.cpp Unpacker.cpp ArgParser.cpp GeneralConfigData.cpp ConfigXMLparser.cpp Messenger.cpp )
add_executable(${EXE_NAME} scanmain.cpp  ${EXE_SOURCES}) 
target_include_directories(${EXE_NAME} PUBLIC )
#need spdlog and fmt for pretty logging and printing because fuck the original messenger class
target_link_libraries(${EXE_NAME} pugixml)
if( USE_SPDLOG )
	target_link_libraries(${EXE_NAME}  spdlog::spdlog_header_only fmt::fmt )
endif()



# target_link_libraries(${SCAN_NAME} ${LIBS} PaassRootStruct PaassScanStatic ResourceStatic PaassCoreStatic PugixmlStatic PaassResourceStatic )
# target_link_libraries(${SCAN_NAME} ${ROOT_LIBRARIES} PaassRootStruct)

#------------------------------------------------------------------------------

install(TARGETS ${EXE_NAME} DESTINATION bin)
# install(DIRECTORY share/utkscan DESTINATION share)
# install(TARGETS PaassRootStruct DESTINATION lib)
# install(CODE "execute_process(COMMAND ${CMAKE_COMMAND} -E create_symlink ${CMAKE_INSTALL_PREFIX}/lib/libPaassRootStruct_root_rdict.pcm ${CMAKE_INSTALL_PREFIX}/bin/libPaassRootStruct_root_rdict.pcm)")

#install(FILES ${CMAKE_SOURCE_DIR}/Analysis/Utkscan/core/include/PaassRootStruct.hpp DESTINATION include/)
#install(FILES ${CMAKE_SOURCE_DIR}/Analysis/Utkscan/processors/include/GSaddback.hpp DESTINATION include/)
##Install the ROOT 6 PCM.
#if (${ROOT_VERSION} VERSION_GREATER "6.0")
#    install( FILES ${CMAKE_CURRENT_BINARY_DIR}/libPaassRootStruct_root_rdict.pcm DESTINATION lib/)
#endif (${ROOT_VERSION} VERSION_GREATER "6.0")
