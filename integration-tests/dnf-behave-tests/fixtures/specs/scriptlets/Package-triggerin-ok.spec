Name:           Package-triggerin-ok
Epoch:          0
Version:        1.0
Release:        1

License:        Public Domain
URL:            None

Summary:        The made up package for scriptlets testing.

%description
Successfully runs triggering scriptlet on Package-install-file install/update

%triggerin -p <lua> -- Package-install-file
print("triggerin scriptlet (Package-triggerin-ok) for Package-install-file install/update successfully done")

%files

%changelog
