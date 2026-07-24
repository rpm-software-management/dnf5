Name:           meson
Epoch:          0
Version:        0.47.1
Release:        5.module_1993+7c0a4d1e

License:        ASL 2.0
URL:            http://mesonbuild.com/

Summary:        High productivity build system
BuildArch:      noarch

Provides:       meson = 0.47.1-5.module_1993+7c0a4d1e
Provides:       python3.7dist(meson) = 0.47.1
Provides:       python3dist(meson) = 0.47.1

Requires:       ninja-build

Obsoletes:      meson-gui < 0.31.0-3

%description
Meson is a build system designed to optimize programmer
productivity. It aims to do this by providing simple, out-of-the-box
support for modern software development tools and practices, such as
unit tests, coverage reports, Valgrind, CCache and the like.

%files

%changelog
