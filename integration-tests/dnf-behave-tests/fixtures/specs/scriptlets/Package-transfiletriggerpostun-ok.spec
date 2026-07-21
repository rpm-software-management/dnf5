Name:           Package-transfiletriggerpostun-ok
Epoch:          0
Version:        1.0
Release:        1

License:        Public Domain
URL:            None

Summary:        The made up package for scriptlets testing.

%description
Successfully runs transfiletriggerpostun scriptlet after transaction uninstalls Package-install-file

%transfiletriggerpostun -p <lua> -- /usr/lib/Triggers
print("transfiletriggerpostun scriptlet (Package-transfiletriggerpostun-ok) for Package-install-file transaction uninstall successfully done")

%files

%changelog
