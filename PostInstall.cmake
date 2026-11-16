set(BIN_DIR "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/bin")
set(SCRIPT_DIR "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/scripts")
set(OUTPUT_FILE "${BIN_DIR}/zsh_completions.sh")

file(WRITE "${OUTPUT_FILE}" "# Generated zsh completion registrations\n")

file(GLOB EXECUTABLES "${BIN_DIR}/*" "${SCRIPT_DIR}/*")

foreach(EXE IN LISTS EXECUTABLES)
    if(NOT IS_DIRECTORY "${EXE}")
        execute_process(
            COMMAND test -x "${EXE}"
            RESULT_VARIABLE IS_EXECUTABLE
        )

        if("${IS_EXECUTABLE}" STREQUAL "0")
            get_filename_component(EXE_NAME "${EXE}" NAME)
            file(APPEND "${OUTPUT_FILE}"
                "compdef _gnu_generic ${EXE_NAME}\n"
            )
        endif()
    endif()
endforeach()

message(STATUS "Generated ${OUTPUT_FILE}")
