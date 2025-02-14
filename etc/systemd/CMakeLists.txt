set(systemd_FILES
    dnf-makecache.service
    dnf-makecache.timer)

install(FILES ${systemd_FILES} DESTINATION ${SYSTEMD_DIR})
