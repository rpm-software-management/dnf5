configure_file(${CMAKE_CURRENT_SOURCE_DIR}/Doxyfile.in ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile)
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/conf.py.in ${CMAKE_CURRENT_BINARY_DIR}/conf.py)


add_custom_target(doc)


add_subdirectory(sphinx)


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

    if(WITH_PLUGIN_ACTIONS)
        install(FILES ${CMAKE_CURRENT_BINARY_DIR}/man/libdnf5-actions.8 DESTINATION share/man/man8)
    endif()

    if(WITH_PLUGIN_EXPIRED_PGP_KEYS)
        install(FILES ${CMAKE_CURRENT_BINARY_DIR}/man/libdnf5-expired-pgp-keys.8 DESTINATION share/man/man8)
    endif()

    if(WITH_PLUGIN_MANIFEST)
        install(FILES ${CMAKE_CURRENT_BINARY_DIR}/man/dnf5-manifest.8 DESTINATION share/man/man8)
    endif()

    if(WITH_DNF5DAEMON_CLIENT)
        install(FILES ${CMAKE_CURRENT_BINARY_DIR}/man/dnf5daemon-client.8 DESTINATION share/man/man8)
    endif()

    if(WITH_DNF5DAEMON_SERVER)
        install(FILES ${CMAKE_CURRENT_BINARY_DIR}/man/dnf5daemon-server.8 DESTINATION share/man/man8)
        install(FILES ${CMAKE_CURRENT_BINARY_DIR}/man/dnf5daemon-dbus-api.8 DESTINATION share/man/man8)
    endif()

    if(WITH_PLUGIN_LOCAL)
        install(FILES ${CMAKE_CURRENT_BINARY_DIR}/man/libdnf5-local.8 DESTINATION share/man/man8)
    endif()

    if(WITH_DNF5)
        install(FILES ${CMAKE_CURRENT_BINARY_DIR}/man/dnf5.8 DESTINATION share/man/man8)
        install(FILES ${CMAKE_CURRENT_BINARY_DIR}/man/dnf5.conf.5 DESTINATION share/man/man5)
        install(FILES ${CMAKE_CURRENT_BINARY_DIR}/man/dnf5.conf-vendorpolicy.5 DESTINATION share/man/man5)
        install(FILES ${CMAKE_CURRENT_BINARY_DIR}/man/dnf5.conf-vendorpolicy-v1_0.5 DESTINATION share/man/man5)
        install(FILES ${CMAKE_CURRENT_BINARY_DIR}/man/dnf5.conf-vendorpolicy-v1_1.5 DESTINATION share/man/man5)
        install(FILES ${CMAKE_CURRENT_BINARY_DIR}/man/dnf5.conf-todo.5 DESTINATION share/man/man5)
        install(FILES ${CMAKE_CURRENT_BINARY_DIR}/man/dnf5.conf-deprecated.5 DESTINATION share/man/man5)
        install(FILES ${CMAKE_CURRENT_BINARY_DIR}/man/dnf5-advisory.8 DESTINATION share/man/man8)
        install(FILES ${CMAKE_CURRENT_BINARY_DIR}/man/dnf5-autoremove.8 DESTINATION share/man/man8)
        install(FILES ${CMAKE_CURRENT_BINARY_DIR}/man/dnf5-check.8 DESTINATION share/man/man8)
        install(FILES ${CMAKE_CURRENT_BINARY_DIR}/man/dnf5-check-upgrade.8 DESTINATION share/man/man8)
        install(FILES ${CMAKE_CURRENT_BINARY_DIR}/man/dnf5-clean.8 DESTINATION share/man/man8)
        install(FILES ${CMAKE_CURRENT_BINARY_DIR}/man/dnf5-debuginfo-install.8 DESTINATION share/man/man8)
        install(FILES ${CMAKE_CURRENT_BINARY_DIR}/man/dnf5-distro-sync.8 DESTINATION share/man/man8)
        install(FILES ${CMAKE_CURRENT_BINARY_DIR}/man/dnf5-do.8 DESTINATION share/man/man8)
        install(FILES ${CMAKE_CURRENT_BINARY_DIR}/man/dnf5-downgrade.8 DESTINATION share/man/man8)
        install(FILES ${CMAKE_CURRENT_BINARY_DIR}/man/dnf5-download.8 DESTINATION share/man/man8)
        install(FILES ${CMAKE_CURRENT_BINARY_DIR}/man/dnf5-environment.8 DESTINATION share/man/man8)
        install(FILES ${CMAKE_CURRENT_BINARY_DIR}/man/dnf5-group.8 DESTINATION share/man/man8)
        install(FILES ${CMAKE_CURRENT_BINARY_DIR}/man/dnf5-history.8 DESTINATION share/man/man8)
        install(FILES ${CMAKE_CURRENT_BINARY_DIR}/man/dnf5-info.8 DESTINATION share/man/man8)
        install(FILES ${CMAKE_CURRENT_BINARY_DIR}/man/dnf5-install.8 DESTINATION share/man/man8)
        install(FILES ${CMAKE_CURRENT_BINARY_DIR}/man/dnf5-leaves.8 DESTINATION share/man/man8)
        install(FILES ${CMAKE_CURRENT_BINARY_DIR}/man/dnf5-list.8 DESTINATION share/man/man8)
        install(FILES ${CMAKE_CURRENT_BINARY_DIR}/man/dnf5-makecache.8 DESTINATION share/man/man8)
        install(FILES ${CMAKE_CURRENT_BINARY_DIR}/man/dnf5-mark.8 DESTINATION share/man/man8)
        install(FILES ${CMAKE_CURRENT_BINARY_DIR}/man/dnf5-module.8 DESTINATION share/man/man8)
        install(FILES ${CMAKE_CURRENT_BINARY_DIR}/man/dnf5-offline.8 DESTINATION share/man/man8)
        install(FILES ${CMAKE_CURRENT_BINARY_DIR}/man/dnf5-provides.8 DESTINATION share/man/man8)
        install(FILES ${CMAKE_CURRENT_BINARY_DIR}/man/dnf5-reinstall.8 DESTINATION share/man/man8)
        install(FILES ${CMAKE_CURRENT_BINARY_DIR}/man/dnf5-remove.8 DESTINATION share/man/man8)
        install(FILES ${CMAKE_CURRENT_BINARY_DIR}/man/dnf5-replay.8 DESTINATION share/man/man8)
        install(FILES ${CMAKE_CURRENT_BINARY_DIR}/man/dnf5-repo.8 DESTINATION share/man/man8)
        install(FILES ${CMAKE_CURRENT_BINARY_DIR}/man/dnf5-repoquery.8 DESTINATION share/man/man8)
        install(FILES ${CMAKE_CURRENT_BINARY_DIR}/man/dnf5-search.8 DESTINATION share/man/man8)
        install(FILES ${CMAKE_CURRENT_BINARY_DIR}/man/dnf5-swap.8 DESTINATION share/man/man8)
        install(FILES ${CMAKE_CURRENT_BINARY_DIR}/man/dnf5-system-upgrade.8 DESTINATION share/man/man8)
        install(FILES ${CMAKE_CURRENT_BINARY_DIR}/man/dnf5-upgrade.8 DESTINATION share/man/man8)
        install(FILES ${CMAKE_CURRENT_BINARY_DIR}/man/dnf5-versionlock.8 DESTINATION share/man/man8)
        install(FILES ${CMAKE_CURRENT_BINARY_DIR}/man/dnf5-aliases.7 DESTINATION share/man/man7)
        install(FILES ${CMAKE_CURRENT_BINARY_DIR}/man/dnf5-caching.7 DESTINATION share/man/man7)
        install(FILES ${CMAKE_CURRENT_BINARY_DIR}/man/dnf5-comps.7 DESTINATION share/man/man7)
        install(FILES ${CMAKE_CURRENT_BINARY_DIR}/man/dnf5-filtering.7 DESTINATION share/man/man7)
        install(FILES ${CMAKE_CURRENT_BINARY_DIR}/man/dnf5-forcearch.7 DESTINATION share/man/man7)
        install(FILES ${CMAKE_CURRENT_BINARY_DIR}/man/dnf5-installroot.7 DESTINATION share/man/man7)
         install(FILES ${CMAKE_CURRENT_BINARY_DIR}/man/dnf5-modularity.7 DESTINATION share/man/man7)
        install(FILES ${CMAKE_CURRENT_BINARY_DIR}/man/dnf5-specs.7 DESTINATION share/man/man7)
        install(FILES ${CMAKE_CURRENT_BINARY_DIR}/man/dnf5-system-state.7 DESTINATION share/man/man7)
        install(FILES ${CMAKE_CURRENT_BINARY_DIR}/man/dnf5-changes-from-dnf4.7 DESTINATION share/man/man7)

        # plugin commands
        install(FILES ${CMAKE_CURRENT_BINARY_DIR}/man/dnf5-automatic.8 DESTINATION share/man/man8)
        install(FILES ${CMAKE_CURRENT_BINARY_DIR}/man/dnf5-builddep.8 DESTINATION share/man/man8)
        install(FILES ${CMAKE_CURRENT_BINARY_DIR}/man/dnf5-changelog.8 DESTINATION share/man/man8)
        install(FILES ${CMAKE_CURRENT_BINARY_DIR}/man/dnf5-config-manager.8 DESTINATION share/man/man8)
        install(FILES ${CMAKE_CURRENT_BINARY_DIR}/man/dnf5-copr.8 DESTINATION share/man/man8)
        install(FILES ${CMAKE_CURRENT_BINARY_DIR}/man/dnf5-obs.8 DESTINATION share/man/man8)
        install(FILES ${CMAKE_CURRENT_BINARY_DIR}/man/dnf5-needs-restarting.8 DESTINATION share/man/man8)
        install(FILES ${CMAKE_CURRENT_BINARY_DIR}/man/dnf5-repoclosure.8 DESTINATION share/man/man8)
        install(FILES ${CMAKE_CURRENT_BINARY_DIR}/man/dnf5-repomanage.8 DESTINATION share/man/man8)
        install(FILES ${CMAKE_CURRENT_BINARY_DIR}/man/dnf5-reposync.8 DESTINATION share/man/man8)
    endif()

endif()
