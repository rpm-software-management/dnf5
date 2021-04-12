// create a new Base object
libdnf::Base base;

// create a reference to the Base's config for better code readability
auto & conf = base.get_config();

// set installroot
//
// installroot is set to '/' when we're working with the system
// but we can set it to a different location if we don't need to read or modify system packages
// of if we want to install content in a different installroot
conf.installroot().set(libdnf::Option::Priority::RUNTIME, installroot);

// TODO(dmach): we shouldn't be forcing API users to always set the cache dir
conf.cachedir().set(libdnf::Option::Priority::RUNTIME, installroot + "/var/cache/dnf");
