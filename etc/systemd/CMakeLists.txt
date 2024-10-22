set(systemd_FILES
    dnf5-makecache.service
    dnf5-makecache.timer)

install(FILES ${systemd_FILES} DESTINATION ${SYSTEMD_DIR})
