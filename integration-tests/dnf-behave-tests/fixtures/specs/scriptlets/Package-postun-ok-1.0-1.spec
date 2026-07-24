Name:           Package-postun-ok
Epoch:          0
Version:        1.0
Release:        1

License:        Public Domain
URL:            None

Summary:        The made up package for scriptlets testing.

%description
Successfully runs postun scriptlet

%postun -p <lua>
print("postun scriptlet successfully done")

%files

%changelog
