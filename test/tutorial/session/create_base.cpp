/* includes; won't compile in tests, in the docs we leave out the comment lines to show them
#include <libdnf/base/base.hpp>
*/

// Create a new Base object.
libdnf::Base base;

// Optionally load configuration from the config file.
//
// The Base's config is initialized with default values, one of which is
// `config_file_path()`. This contains the default path to the config file
// ("/etc/dnf/dnf.conf"). Set a custom value and call the below method to load
// configuration from a different location.
base.load_config_from_file();

// Optionally set installroot.
//
// Installroot is set to '/' when we're working with the system, but we can set
// it to a different location. The Base instance will then work with the
// installroot directory tree as its root for the rest of its lifetime.
base.get_config().installroot().set(installroot);

// Load vars and do other initialization (of libsolv pool, etc.) based on the
// configuration.  Locks the installroot and varsdir configuration values so
// that they can't be changed.
base.setup();
