``--bzs=BUGZILLA_ID,...``
    | Include content contained in advisories that fix a ticket of the given Bugzilla ID.
    | This is a list option.
    | Expected values are numeric IDs, e.g. `123123`.
    | Any transaction command (install, upgrade) will fail with an error if there is no advisory fixing the given ticket; this can be bypassed by using the --skip-unavailable switch.
