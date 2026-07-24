Name: dnf-plugin-remove-dummy
Version: 1
Release: 1
Summary: Plugin that removes the pkg dummy

License: GPLv3+
Url: None

%description
This package adds a plugin that removes the package dummy
after the transaction gets resolved

%install
mkdir -p %{buildroot}/dnf-plugins/
cat >> %{buildroot}/dnf-plugins/dnf-plugin-remove-dummy.py <<EOF
import dnf
import os

class RemoveDummy(dnf.Plugin):
    name = "remove-dummy"

    def resolved(self):
        print("RESOLVED HOOK: removing dummy")
        os.system("rpm -e dummy")
EOF

%files
/dnf-plugins/%{name}.py

%changelog
