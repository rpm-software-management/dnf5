Name:           dnf-ci-alfa
Epoch:          0
Version:        1.0
Release:        1

License:        Public Domain
URL:            None
BuildArch:      noarch

Summary:        Dummy package with file.

%description
Dummy package with file.

%install
mkdir -p %{buildroot}%{_sysconfdir}
touch %{buildroot}%{_sysconfdir}/dummy.conf

%files
%{_sysconfdir}/dummy.conf

%changelog
