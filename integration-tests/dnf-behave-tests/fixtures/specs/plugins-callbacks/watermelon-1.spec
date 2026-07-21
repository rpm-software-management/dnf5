Name: watermelon
Version: 1.0
Release: 1.fc29
Summary: Made up package

License: GPLv3+
Url: None

%description
watermelon description

%install
mkdir -p %{buildroot}/dnf-plugins/
cat >> %{buildroot}/dnf-plugins/watermelon-dnf-plugin.py <<HERE
import dnf

class Watermelon(dnf.Plugin):
    name = "watermelon"

    def transaction(self):
        print("original watermelon-dnf-plugin transaction()")
HERE

%files
/dnf-plugins/%{name}-dnf-plugin.py

%changelog
