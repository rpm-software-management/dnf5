Name: watermelon
Version: 2.0
Release: 1.fc29
Summary: Made up package

License: GPLv3+
Url: None

%description
watermelon description


%package dnf-plugin
Summary: watermelon dnf plugin
Requires: %{name} = %{version}-%{release}

%description dnf-plugin
watermelon dnf plugin description


%install
mkdir -p %{buildroot}/dnf-plugins/
cat >> %{buildroot}/dnf-plugins/watermelon-dnf-plugin.py <<HERE
import dnf

class Watermelon(dnf.Plugin):
    name = "watermelon"

    def transaction(self):
        print("moved to subpackage watermelon-dnf-plugin transaction()")
HERE

%files

%files dnf-plugin
/dnf-plugins/%{name}-dnf-plugin.py

%changelog
