configure_file(${CMAKE_CURRENT_SOURCE_DIR}/Doxyfile.in ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile)
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/conf.py.in ${CMAKE_CURRENT_BINARY_DIR}/conf.py)


add_custom_target(doc)


add_custom_target(doxygen
    doxygen ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile
    COMMENT "Building doxygen xml files"
)


if(WITH_HTML)
    add_custom_target(doc-html
        sphinx-build -c ${CMAKE_CURRENT_BINARY_DIR} -b html
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${CMAKE_CURRENT_BINARY_DIR}/html
        COMMENT "Building html documentation"
    )
    add_dependencies(doc doc-html)
    add_dependencies(doc-html doxygen)

    #install(FILES ${HTML_FILES} )
endif()


if(WITH_MAN)
    add_custom_target(doc-man
        sphinx-build -c ${CMAKE_CURRENT_BINARY_DIR} -b man
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${CMAKE_CURRENT_BINARY_DIR}/man
        COMMENT "Building man documentation"
    )
    add_dependencies(doc doc-man)

    if(WITH_DNFDAEMON_CLIENT)
        install(FILES ${CMAKE_CURRENT_BINARY_DIR}/man/dnfdaemon-client.8 DESTINATION share/man/man8)
    endif()

    if(WITH_DNFDAEMON_SERVER)
        install(FILES ${CMAKE_CURRENT_BINARY_DIR}/man/dnfdaemon-server.8 DESTINATION share/man/man8)
    endif()

    if(WITH_MICRODNF)
        install(FILES ${CMAKE_CURRENT_BINARY_DIR}/man/microdnf.8 DESTINATION share/man/man8)
    endif()

endif()
