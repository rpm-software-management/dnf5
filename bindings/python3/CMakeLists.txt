if(NOT WITH_PYTHON3)
    return()
endif()


message("Building bindings for python3")


find_package(Python3 REQUIRED COMPONENTS Interpreter Development)
include_directories(${Python3_INCLUDE_DIRS})


function(add_python3_module LIBRARY_NAME MODULE_NAME)
    set(TARGET_NAME "python3_${MODULE_NAME}")
    set(SWIG_COMPILE_OPTIONS ${SWIG_COMPILE_OPTIONS}
        -Wno-redundant-decls
    )
    set(CMAKE_SWIG_FLAGS ${CMAKE_SWIG_FLAGS}
        -doxygen
    )
    set_source_files_properties(../../${LIBRARY_NAME}/${MODULE_NAME}.i PROPERTIES CPLUSPLUS ON)
    set_source_files_properties(../../${LIBRARY_NAME}/${MODULE_NAME}.i PROPERTIES SWIG_FLAGS "-relativeimport")
    swig_add_library(${TARGET_NAME} LANGUAGE python SOURCES ../../${LIBRARY_NAME}/${MODULE_NAME}.i)
    set_property(TARGET ${TARGET_NAME} PROPERTY OUTPUT_NAME ${MODULE_NAME})
    target_compile_options(${TARGET_NAME} PUBLIC ${SWIG_COMPILE_OPTIONS})

    string(REPLACE "_" "-" C_LIBRARY_NAME ${LIBRARY_NAME})
    # c library is called just libdnf (libdnf_cli) -> strip the 5
    string(REPLACE "5" "" C_LIBRARY_NAME ${C_LIBRARY_NAME})
    swig_link_libraries(${TARGET_NAME} ${C_LIBRARY_NAME})
    swig_link_libraries(${TARGET_NAME} ${Python3_LIBRARIES})

    install(TARGETS ${TARGET_NAME} LIBRARY DESTINATION ${Python3_SITEARCH}/${LIBRARY_NAME})
    install(FILES ${CMAKE_CURRENT_BINARY_DIR}/${MODULE_NAME}.py DESTINATION ${Python3_SITEARCH}/${LIBRARY_NAME})
endfunction()


add_subdirectory(libdnf5)
add_subdirectory(libdnf5_cli)
