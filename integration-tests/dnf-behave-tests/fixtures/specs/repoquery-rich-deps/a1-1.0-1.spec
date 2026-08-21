Name:           a1
Version:        1.0
Release:        1

License:        Public Domain
URL:            None

Provides:       a1-prov1

Conflicts:      ((b1 and x1) or c1)

Recommends:     (b1 < 2.0 if x1 >= 2.0 else c1)
Suggests:       ((b1 with b1-prov2 > 1.7) or (c1 <= 1.0 without c1-prov1 > 0.5))
Supplements:    ((b1 < 2.0 with b1-prov2 > 1.7) or (c1 > 1.0 without c1-prov1 > 0.5))
Enhances:       (b1 unless x1)

Summary:        Rich deps package.

%description
Dummy.

%files

%changelog
