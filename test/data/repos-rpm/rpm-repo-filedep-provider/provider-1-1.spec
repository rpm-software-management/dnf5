Name:           provider
Epoch:          0
Version:        1
Release:        1
Vendor:         dnf5-test

License:        Public Domain
URL:            http://example.com/

Summary:        A dummy package that owns a file outside the standard filtered directories
BuildArch:      noarch

%description
A dummy package that owns a file outside the paths createrepo_c treats as "primary"
(cr_is_primary() in createrepo_c promotes any path containing "bin/", "/etc/", or
"/usr/lib/sendmail" into primary.xml - this path deliberately avoids all of those), so
the file only shows up in filelists.xml and not in primary.xml.

%install
mkdir -p %{buildroot}/opt/dnf5-test
touch %{buildroot}/opt/dnf5-test/tool

%files
/opt/dnf5-test/tool

%changelog
