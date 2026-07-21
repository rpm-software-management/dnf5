Name:           Package-posttrans-ok
Epoch:          0
Version:        1.0
Release:        1

License:        Public Domain
URL:            None

Summary:        The made up package for scriptlets testing.

%description
Successfully runs posttrans scriptlet

%posttrans -p <lua>
print("posttrans scriptlet successfully done")

%files

%changelog
