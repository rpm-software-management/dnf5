``--cves=CVE_ID,...``
    | Include content contained in advisories that fix a ticket of the given CVE (Common Vulnerabilities and Exposures) ID.
    | This is a list option.
    | Expected values are string IDs in CVE format, e.g. `CVE-2201-0123`.
    | Any transaction command (install, upgrade) will fail with an error if there is no advisory fixing the given ticket; this can be bypassed by using the --skip-unavailable switch.
