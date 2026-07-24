Summary: It
Name: it
Version: 1.0
Release: 1
License: Commercial
Group: Development/Libraries
AutoReqProv: no

%description
It.

%prep

%build

%install
cd $RPM_BUILD_ROOT
mkdir -p    usr/opt/it
echo "it" > usr/opt/it/it

%clean
cd $RPM_BUILD_ROOT
rm -rf usr

%files
%dir /usr/opt/it/
     /usr/opt/it/it
