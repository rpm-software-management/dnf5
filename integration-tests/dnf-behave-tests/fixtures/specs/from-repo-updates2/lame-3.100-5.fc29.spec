Name:           lame
Version:        3.100
Release:        5%{?dist}
Summary:        Free MP3 audio compressor

License:        GPLv2+
URL:            http://lame.sourceforge.net/

Requires:       %{name}-libs = %{version}-%{release}

%description
LAME is an open source MP3 encoder whose quality and speed matches
commercial encoders. LAME handles MPEG1,2 and 2.5 layer III encoding
with both constant and variable bitrates.

%package        libs
Summary:        LAME MP3 encoding library

%description    libs
LAME MP3 encoding library.

%files

%files libs

%changelog
