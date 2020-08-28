set(SYSCONFDIR /etc)
set(SYSTEMD_UNIT_DIR /usr/lib/systemd/system)
set(DBUS_SERVICES_DIR /usr/share/dbus-1/system-services)
set(DBUS_CONFIG_DIR ${SYSCONFDIR}/dbus-1/system.d)

install (
    FILES "dnfdaemon.service"
    DESTINATION ${SYSTEMD_UNIT_DIR}
)

install (
    FILES "org.rpm.dnf.v0.service"
    DESTINATION ${DBUS_SERVICES_DIR}
)

install (
    FILES "org.rpm.dnf.v0.conf"
    DESTINATION ${DBUS_CONFIG_DIR}
)
