/* includes; won't compile in tests, in the docs we leave out the comment lines to show them
#include <libdnf5/base/base.hpp>
*/

// Create a new Base object.
libdnf5::Base base;

// Optionally set installroot.
//
// Installroot is set to '/' when we're working with the system, but we can set
// it to a different location. The Base instance will then work with the
// installroot directory tree as its root for the rest of its lifetime.
base.get_config().get_installroot_option().set(installroot);

// Optionally - Prevent loading libdnf5 plugins.
// Plugins are loaded by default from the host
base.get_config().get_plugins_option().set(false);

// Optionally load configuration from the config files.
//
// The Base's config is initialized with default values, one of which is
// "config_file_path". This contains the default path to the config file
// ("/etc/dnf/dnf.conf"). If the file does not exist the distribution config file
// is loaded. Function also loads configuration files from distribution and
// user ("/etc/dnf/libdnf5.conf.d") drop-in directories.
// Optionally set a custom value to "config_file_path" before calling this method
// to load configuration from a another configuration file.
base.load_config();

// Load vars and do other initialization (of libsolv pool, etc.) based on the
// configuration.  Locks the installroot and varsdir configuration values so
// that they can't be changed.
base.setup();
