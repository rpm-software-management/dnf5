if(NOT WITH_GO)
    return()
endif()


message("Building bindings for go")


function(add_go_module LIBRARY_NAME MODULE_NAME)
    set(TARGET_NAME "go_${MODULE_NAME}")
    set_source_files_properties(../../${LIBRARY_NAME}/${MODULE_NAME}.i PROPERTIES CPLUSPLUS ON)
    set(CMAKE_SWIG_FLAGS -module ${TARGET_NAME} -cgo -intgosize 64 -package libdnf)
    swig_add_library(${TARGET_NAME} LANGUAGE go SOURCES ../../${LIBRARY_NAME}/${MODULE_NAME}.i)
    set_property(TARGET ${TARGET_NAME} PROPERTY OUTPUT_NAME ${MODULE_NAME})
    target_compile_options(${TARGET_NAME} PUBLIC ${SWIG_COMPILE_OPTIONS})

    string(REPLACE "_" "-" C_LIBRARY_NAME ${LIBRARY_NAME})
    swig_link_libraries(${TARGET_NAME} ${C_LIBRARY_NAME})
endfunction()


add_subdirectory(libdnf)
add_subdirectory(libdnf_cli)
