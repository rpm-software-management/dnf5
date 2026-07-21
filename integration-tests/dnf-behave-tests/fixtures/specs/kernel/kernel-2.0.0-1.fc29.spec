Name:           kernel
Epoch:          0
Version:        2.0.0
Release:        1.fc29

License:        GPLv2 and Redistributable, no modification permitted
URL:            https://www.kernel.org/

Summary:        The Linux kernel

Requires:       kernel-modules-uname-r = %{version}-%{release}.x86_64
Requires:       kernel-core-uname-r = %{version}-%{release}.x86_64

%description
The kernel meta package

%package core
Summary:        The Linux kernel

Provides:       installonlypkg(kernel)
Provides:       kernel-uname-r = %{version}-%{release}.x86_64
Provides:       kernel-core-uname-r = %{version}-%{release}.x86_64

%description core
The kernel package contains the Linux kernel (vmlinuz), the core of any
Linux operating system.  The kernel handles the basic functions
of the operating system: memory allocation, process allocation, device
input and output, etc.

%package modules
Summary:        kernel modules to match the core kernel

Provides:       installonlypkg(kernel-module)
Provides:       kernel-modules-uname-r = %{version}-%{release}.x86_64

Requires:       kernel-uname-r = %{version}-%{release}.x86_64

%description modules
This package provides commonly used kernel modules for the core kernel package.

%files

%files core
%ghost /boot/vmlinuz-%{version}-%{release}.x86_64

%files modules

%changelog
