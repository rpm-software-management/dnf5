# TIMER_PREFIX is a temporary option intended to support the parallel
# installation of dnf4 and dnf5. When dnf4 is fully deprecated, the
# commit that introduced this option can be reverted to simplify the
# build.
set(TIMER_PREFIX "dnf" CACHE STRING
    "Prefix for makecache timer: (default is dnf)"
)

file(RENAME dnf-makecache.service ${TIMER_PREFIX}-makecache.service)
file(RENAME dnf-makecache.timer ${TIMER_PREFIX}-makecache.timer)
set(systemd_FILES
    ${TIMER_PREFIX}-makecache.service
    ${TIMER_PREFIX}-makecache.timer)

install(FILES ${systemd_FILES} DESTINATION ${SYSTEMD_DIR})
