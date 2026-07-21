Name:           c1
Version:        1.0
Release:        1

License:        Public Domain
URL:            None

Provides:       c1-prov1 = 1.0

Requires:       (a1-prov1 if b1)
Requires:       (b1-prov2 >= 1.0 with b1-prov2 < 2.0)

Conflicts:      (d1-prov1 >= 1.0 with d1-prov1 < 2.0)

Recommends:     (b1 and a1)

Summary:        Rich deps package.

%description
Dummy.

%files

%changelog
