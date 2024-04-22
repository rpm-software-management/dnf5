set(SYSTEMD_UNIT_DIR /usr/lib/systemd/system)
set(DBUS_SHARE_DIR /usr/share/dbus-1)
set(DBUS_CONFIG_DIR ${DBUS_SHARE_DIR}/system.d)

install (
    FILES "dnf5daemon-server.service"
    DESTINATION ${SYSTEMD_UNIT_DIR}
)

install (
    FILES "org.rpm.dnf.v0.service"
    DESTINATION ${DBUS_SHARE_DIR}/system-services
)

install (
    FILES "org.rpm.dnf.v0.conf"
    DESTINATION ${DBUS_CONFIG_DIR}
)

install(
    DIRECTORY interfaces/
    DESTINATION ${DBUS_SHARE_DIR}/interfaces
    FILES_MATCHING PATTERN "org.rpm.dnf.v0.*.xml"
)

add_subdirectory(interfaces)
