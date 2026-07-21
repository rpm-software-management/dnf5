Name:           Package-preun-ok
Epoch:          0
Version:        1.0
Release:        1

License:        Public Domain
URL:            None

Summary:        The made up package for scriptlets testing.

%description
Successfully runs preun scriptlet

%preun -p <lua>
print("preun scriptlet successfully done")

%files

%changelog
