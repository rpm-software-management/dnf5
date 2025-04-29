``--advisories=ADVISORY_NAME,...``
    | Include content contained in advisories with specified name.
    | This is a list option.
    | Expected values are advisory IDs, e.g. `FEDORA-2201-123`.
    | Any transaction command (install, upgrade) will fail with an error if there is no existing advisory in the list; this can be bypassed by using the --skip-unavailable switch.
