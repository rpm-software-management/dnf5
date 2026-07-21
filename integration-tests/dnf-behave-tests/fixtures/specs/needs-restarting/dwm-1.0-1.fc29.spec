BuildArch:     x86_64
Name:          dwm
Version:       1.0
Release:       1
License:       MIT
Group:         User Interface/Desktops
Summary:       Dynamic window manager for X
Distribution:  Fedora Project
DistTag:       module(dwm:6.0:20180813144159:6c81f848)
URL:           http://dwm.suckless.org/
Vendor:        Fedora Project
Packager:      Fedora Project




Provides:      dwm = 6.0-1.module_1997+c375c79c
Provides:      dwm(x86-64) = 6.0-1.module_1997+c375c79c
#suggest
#enhance
%description
Test package for needs-restarting plugins

%install
mkdir -p %{buildroot}/etc/dnf/plugins/needs-restarting.d/
echo "dwm" > %{buildroot}/etc/dnf/plugins/needs-restarting.d/dwm.conf

%files
%dir /etc/dnf/plugins/needs-restarting.d
/etc/dnf/plugins/needs-restarting.d/dwm.conf
