Name: flac
Version: 1.3.9
Release: 1%{?dist}
Summary: An encoder/decoder for the Free Lossless Audio Codec

License: BSD and GPLv2+ and GFDL
URL: http://www.xiph.org/flac/

%description
FLAC stands for Free Lossless Audio Codec. Grossly oversimplified, FLAC
is similar to Ogg Vorbis, but lossless. The FLAC project consists of
the stream format, reference encoders and decoders in library form,
flac, a command-line program to encode and decode FLAC files, metaflac,
a command-line metadata editor for FLAC files and input plugins for
various music players.

This package contains the command-line tools and documentation.

%package libs
Summary: Libraries for the Free Lossless Audio Codec

%description libs
FLAC stands for Free Lossless Audio Codec. Grossly oversimplified, FLAC
is similar to Ogg Vorbis, but lossless. The FLAC project consists of
the stream format, reference encoders and decoders in library form,
flac, a command-line program to encode and decode FLAC files, metaflac,
a command-line metadata editor for FLAC files and input plugins for
various music players.
This package contains the FLAC libraries.

%files

%files libs

%changelog
