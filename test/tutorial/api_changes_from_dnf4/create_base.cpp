/* includes; won't compile in tests, in the docs we leave out the comment lines to show them
#include <libdnf5/base/base.hpp>
*/

// Create a new Base object.
libdnf5::Base base;

// Set configuration for the tests
auto & base_config = base.get_config();
base_config.get_cachedir_option().set(cachedir);
base_config.get_installroot_option().set(installroot);
// For unit tests only - Prevent loading plugins from the host system.
// In production code, plugins are typically left enabled (default behavior).
base_config.get_plugins_option().set(false);

// Optionally, load configuration from the file defined in the current
// configuration and files in the drop-in directories.
base.load_config();

// Load variables and do other initialization based on the configuration.
base.setup();
