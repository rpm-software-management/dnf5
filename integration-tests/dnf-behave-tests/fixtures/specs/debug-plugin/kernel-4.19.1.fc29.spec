Name:           kernel
Epoch:          0
Version:        4.19.1
Release:        fc29

License:        GPLv2 and Redistributable, no modification permitted
URL:            https://www.kernel.org/

Summary:        The Linux kernel

Provides:       kernel = 4.19.1-fc29
Provides:       kernel(x86-64) = 4.19.1-fc29
Provides:       installonlypkg(kernel)

%description
The kernel meta package

%files
%ghost /boot/vmlinuz-4.19.1-fc29.x86_64

%changelog
