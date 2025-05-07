SET(POLKIT_DIR /usr/share/polkit-1)

install (
    FILES "org.rpm.dnf.v0.policy"
    DESTINATION ${POLKIT_DIR}/actions
)

install (
    FILES "org.rpm.dnf.v0.rules"
    DESTINATION ${POLKIT_DIR}/rules.d
)
