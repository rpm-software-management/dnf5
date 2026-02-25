import libdnf5

# Create a new Base object.
base = libdnf5.base.Base()

# Set configuration for the tests
base_config = base.get_config()
base_config.cachedir = cachedir
base_config.installroot = installroot
# For unit tests only - Prevent loading plugins from the host system.
# In production code, plugins are typically left enabled (default behavior).
base_config.plugins = False

# Optionally, load configuration from the file defined in the current
# configuration and files in the drop-in directories.
base.load_config()

# Load variables and do other initialization based on the configuration.
base.setup()
