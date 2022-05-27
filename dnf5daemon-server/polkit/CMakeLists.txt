SET(POLKIT_DIR /usr/share/polkit-1/actions)

install (
    FILES "org.rpm.dnf.v0.policy"
    DESTINATION ${POLKIT_DIR}
)
