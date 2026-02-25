/* includes; won't compile in tests, in the docs we leave out the comment lines to show them
#include <libdnf5/base/base.hpp>
*/

// Create a new Base object.
libdnf5::Base base;

// Optionally load configuration from the config files.
base.load_config();

// For unit tests only - Prevent loading plugins from the host system.
// In production code, plugins are typically left enabled (default behavior).
base.get_config().get_plugins_option().set(false);

// Override the detected system architecture, similar to how the
// `--forcearch=aarch64` switch works in the dnf5 command line tool.
base.get_vars()->set("arch", "aarch64");

// This is sufficient for loading repositories and querying packages using the
// `aarch64` architecture.
// However, if you also want to run a transaction (e.g., you want to modify a
// foreign system from "outside" using `installroot`), you need to set the
// `ignorearch` option to instruct RPM to permit packages that are incompatible
// with the system architecture.
base.get_config().get_ignorearch_option().set(true);

// The base.setup() call configures the architecture for the solver, so the
// `arch` variable needs to be set beforehand.
base.setup();
