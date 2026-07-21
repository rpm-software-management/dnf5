%undefine _debuginfo_subpackages

Name:           nodejs
Epoch:          1
Version:        8.14.0
Release:        1

License:        MIT and ASL 2.0 and ISC and BSD
URL:            http://nodejs.org/

Summary:        JavaScript runtime

Provides:       nodejs = 1:8.14.0-1
Provides:       nodejs(x86-64) = 1:8.14.0-1
Provides:       bundled(c-ares) = 1.10.1
Provides:       bundled(icu) = 60.1
Provides:       bundled(v8) = 6.2.414.54
Provides:       nodejs(abi) = 8.11
Provides:       nodejs(abi8) = 8.11
Provides:       nodejs(engine) = 8.14.0
Provides:       nodejs(v8-abi) = 6.2
Provides:       nodejs(v8-abi6) = 6.2
Provides:       nodejs-punycode = 2.0.0
Provides:       npm(punycode) = 2.0.0

Requires:       rtld(GNU_HASH)

Conflicts:      node <= 0.3.2-12

Recommends:     npm = 1:5.6.0-1.8.14.0.1.module_2030+42747d41

%description
Node.js is a platform built on Chrome's JavaScript runtime
for easily building fast, scalable network applications.
Node.js uses an event-driven, non-blocking I/O model that
makes it lightweight and efficient, perfect for data-intensive
real-time applications that run across distributed devices.

%files

%changelog
