if(NOT WITH_PYTHON3)
    return()
endif()


message("Building bindings for python3")


find_package(Python3 REQUIRED COMPONENTS Interpreter Development)
include_directories(${Python3_INCLUDE_DIRS})


function(add_python3_module LIBRARY_NAME MODULE_NAME)
    set(TARGET_NAME "python3_${MODULE_NAME}")
    list(APPEND SWIG_COMPILE_OPTIONS
        -Wno-redundant-decls
    )
    # Currently the SWIG_PYTHON_SILENT_MEMLEAK controls only whether message:
    # "swig/python detected a memory leak of type '%s', no destructor found." is printed, used here:
    # https://github.com/swig/swig/blob/33f6a2d0b2c3d90b928f56ddfa599afe87903f76/Lib/python/pyrun.swg#L776
    # We want to suppress this message due to a bug in Python part of swig.
    # The core of the issue is that Python Swig has a global list of types used in all swig modules. This is
    # needed because the modules are interlinked eg. libdnf5::Base is used in all of them. During finalization
    # of the Python runtime unused modules are cleaned up before garbage collector runs and cleans global objects.
    # This can result in a situation where for example libdnf5.advisory module is destroyed, removing its types
    # from the global swig types list including libdnf5.base.Base, and only after that a global libdnf5.base.Base
    # is garbage collected which is now missing type information -> no destructor can be called -> swig wants to
    # print the message
    # There is an issue reported on SWIG with the same root cause: https://github.com/swig/swig/issues/2037 it
    # also contains more details.
    list(APPEND SWIG_COMPILE_OPTIONS
        -DSWIG_PYTHON_SILENT_MEMLEAK
    )
    list(APPEND CMAKE_SWIG_FLAGS
        -doxygen
    )
    set_source_files_properties(../../${LIBRARY_NAME}/${MODULE_NAME}.i PROPERTIES CPLUSPLUS ON)
    set_source_files_properties(../../${LIBRARY_NAME}/${MODULE_NAME}.i PROPERTIES SWIG_FLAGS "-relativeimport")
    swig_add_library(${TARGET_NAME} LANGUAGE python SOURCES ../../${LIBRARY_NAME}/${MODULE_NAME}.i)
    set_property(TARGET ${TARGET_NAME} PROPERTY OUTPUT_NAME ${MODULE_NAME})
    target_compile_options(${TARGET_NAME} PUBLIC ${SWIG_COMPILE_OPTIONS})

    string(REPLACE "_" "-" C_LIBRARY_NAME ${LIBRARY_NAME})
    target_link_libraries(${TARGET_NAME} PRIVATE ${C_LIBRARY_NAME})
    target_link_libraries(${TARGET_NAME} PRIVATE ${Python3_LIBRARIES})

    # generate a dist-info for the library (redundantly created for multi-module packages, oh well...)
    set(DISTINFO_PATH "${CMAKE_CURRENT_BINARY_DIR}/${LIBRARY_NAME}-${CMAKE_PROJECT_VERSION}.dist-info")
    set(METADATA_FILE "${DISTINFO_PATH}/METADATA")
    file(MAKE_DIRECTORY ${DISTINFO_PATH})
    file(WRITE ${METADATA_FILE} "Metadata-Version: 2.1\n")
    file(APPEND ${METADATA_FILE} "Name: ${LIBRARY_NAME}\n")
    file(APPEND ${METADATA_FILE} "Version: ${CMAKE_PROJECT_VERSION}\n")

    install(TARGETS ${TARGET_NAME} LIBRARY DESTINATION ${Python3_SITEARCH}/${LIBRARY_NAME})
    install(FILES ${CMAKE_CURRENT_BINARY_DIR}/${MODULE_NAME}.py DESTINATION ${Python3_SITEARCH}/${LIBRARY_NAME})
    install(DIRECTORY ${DISTINFO_PATH} DESTINATION ${Python3_SITEARCH})
endfunction()


add_subdirectory(libdnf5)
add_subdirectory(libdnf5_cli)
