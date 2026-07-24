Name: watermelon
Version: 3.0
Release: 1.fc29
Summary: Made up package

License: GPLv3+
Url: None

Obsoletes: %{name}-dnf-plugin < %{version}
Provides: %{name}-dnf-plugin = %{version}-%{release}

%description
watermelon description

%install
mkdir -p %{buildroot}/dnf-plugins/
cat >> %{buildroot}/dnf-plugins/watermelon-dnf-plugin.py <<HERE
import dnf

class Watermelon(dnf.Plugin):
    name = "watermelon"

    def transaction(self):
        print("back in the main package watermelon-dnf-plugin transaction()")
HERE

%files
/dnf-plugins/%{name}-dnf-plugin.py

%changelog
