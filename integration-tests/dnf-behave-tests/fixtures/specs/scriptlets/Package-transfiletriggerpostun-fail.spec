Name:           Package-transfiletriggerpostun-fail
Epoch:          0
Version:        1.0
Release:        1

License:        Public Domain
URL:            None

Summary:        The made up package for scriptlets testing.

%description
Fails to run transfiletriggerpostun scriptlet on finish of transaction Package-install-file uninstall

%transfiletriggerpostun -p <lua> -- /usr/lib/Triggers
error("transfiletriggerpostun scriptlet (Package-transfiletriggerpostun-fail) for uninstall transaction of Package-install-file is failing", 1)

%files

%changelog
