``--from-vendor=VENDOR,...``
    | List of vendors separated by commas or spaces. Globs are supported (e.g., ``--from-vendor='Fedora\ project, vendorA, vendorB*, vendorC'``).
    | Packages (or their provides) explicitly specified on the command line will only be looked up from the specified vendors.
    | The vendor change check is **bypassed** for these packages.
    | The vendor is ignored or vendor change policies (if ``allow_vendor_change=0``) will still be used for packages that satisfy dependencies.
