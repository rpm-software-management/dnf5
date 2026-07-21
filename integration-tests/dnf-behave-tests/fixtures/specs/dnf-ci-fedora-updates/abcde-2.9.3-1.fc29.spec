Name:           abcde
Version:        2.9.3
Release:        1%{?dist}
Summary:        A Better CD Encoder

# cddb-tool is Public Domain, otherwise GPLv2+
License:        GPLv2+ and Public Domain
URL:            https://abcde.einval.com/

BuildArch:      noarch
Requires:       wget
Recommends:     flac
Suggests:       lame

%description
abcde is a front end command line utility (actually, a shell script)
that grabs audio tracks off a CD, encodes them to various formats, and
tags them, all in one go.

%files

%changelog
