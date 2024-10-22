if(NOT WITH_RUBY)
    return()
endif()


message("Building bindings for ruby")


find_package(Ruby REQUIRED)
include_directories(${RUBY_INCLUDE_DIRS})


function(add_ruby_module LIBRARY_NAME MODULE_NAME)
    set(TARGET_NAME "ruby_${MODULE_NAME}")
    set_source_files_properties(../../${LIBRARY_NAME}/${MODULE_NAME}.i PROPERTIES CPLUSPLUS ON)
    set_property(SOURCE ../../${LIBRARY_NAME}/${MODULE_NAME}.i PROPERTY SWIG_MODULE_NAME ${MODULE_NAME})
    set(CMAKE_SWIG_FLAGS ${CMAKE_SWIG_FLAGS}
        -module ${MODULE_NAME}
    )
    swig_add_library(${TARGET_NAME} LANGUAGE ruby SOURCES ../../${LIBRARY_NAME}/${MODULE_NAME}.i)
    set_property(TARGET ${TARGET_NAME} PROPERTY OUTPUT_NAME ${MODULE_NAME})
    target_compile_options(${TARGET_NAME} PUBLIC ${SWIG_COMPILE_OPTIONS})

    string(REPLACE "_" "-" C_LIBRARY_NAME ${LIBRARY_NAME})
    target_link_libraries(${TARGET_NAME} PRIVATE ${C_LIBRARY_NAME})
    target_link_libraries(${TARGET_NAME} PRIVATE ${RUBY_LIBRARY})

    install(TARGETS ${TARGET_NAME} LIBRARY DESTINATION "${RUBY_VENDORARCH_DIR}/${LIBRARY_NAME}")
endfunction()


add_subdirectory(libdnf5)
add_subdirectory(libdnf5_cli)
