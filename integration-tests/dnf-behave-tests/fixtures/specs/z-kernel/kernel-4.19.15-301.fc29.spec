Name:           kernel
Epoch:          0
Version:        4.19.15
Release:        301.fc29

License:        GPLv2 and Redistributable, no modification permitted
URL:            https://www.kernel.org/

Summary:        The Linux kernel

Provides:       kernel = 4.19.15-301.fc29
Provides:       kernel(x86-64) = 4.19.15-301.fc29

Requires:       kernel-modules-uname-r = 4.19.15-301.fc29.x86_64
Requires:       kernel-core-uname-r = 4.19.15-301.fc29.x86_64

%description
The kernel meta package

%package core
Summary:        The Linux kernel

Provides:       installonlypkg(kernel)
Provides:       kernel = 4.19.15-301.fc29
Provides:       kernel-uname-r = 4.19.15-301.fc29.x86_64
Provides:       kernel-core-uname-r = 4.19.15-301.fc29.x86_64
Provides:       kernel-core = 4.19.15-301.fc29
Provides:       kernel-core(x86-64) = 4.19.15-301.fc29
Provides:       kernel-x86_64 = 4.19.15-301.fc29

%description core
The kernel package contains the Linux kernel (vmlinuz), the core of any
Linux operating system.  The kernel handles the basic functions
of the operating system: memory allocation, process allocation, device
input and output, etc.

%package modules
Summary:        kernel modules to match the core kernel

Provides:       installonlypkg(kernel-module)
Provides:       kernel-modules-uname-r = 4.19.15-301.fc29.x86_64
Provides:       kernel-modules = 4.19.15-301.fc29
Provides:       kernel-modules(x86-64) = 4.19.15-301.fc29
Provides:       kernel-modules-x86_64 = 4.19.15-301.fc29

Requires:       kernel-uname-r = 4.19.15-301.fc29.x86_64

%description modules
This package provides commonly used kernel modules for the core kernel package.

%files

%files core

%files modules

%changelog
