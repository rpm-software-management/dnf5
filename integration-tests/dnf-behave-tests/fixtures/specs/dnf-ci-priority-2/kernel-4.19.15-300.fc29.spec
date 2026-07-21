Name:           kernel
Epoch:          0
Version:        4.19.15
Release:        300.fc29

License:        GPLv2 and Redistributable, no modification permitted
URL:            https://www.kernel.org/

Summary:        The Linux kernel

Provides:       kernel = 4.19.15-300.fc29
Provides:       kernel(x86-64) = 4.19.15-300.fc29

Requires:       kernel-core-uname-r = 4.19.15-300.fc29.x86_64

%description
The kernel meta package

%package core
Summary:        The Linux kernel

Provides:       installonlypkg(kernel)
Provides:       kernel = 4.19.15-300.fc29
Provides:       kernel-core-uname-r = 4.19.15-300.fc29.x86_64
Provides:       kernel-core = 4.19.15-300.fc29
Provides:       kernel-core(x86-64) = 4.19.15-300.fc29
Provides:       kernel-x86_64 = 4.19.15-300.fc29

%description core
The kernel package contains the Linux kernel (vmlinuz), the core of any
Linux operating system.  The kernel handles the basic functions
of the operating system: memory allocation, process allocation, device
input and output, etc.

%files

%files core

%changelog
