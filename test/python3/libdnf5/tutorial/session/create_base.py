import libdnf5

# Create a new Base object
base = libdnf5.base.Base()

# Optionally set installroot.
#
# Installroot is set to '/' when we're working with the system, but we can set
# it to a different location. The Base instance will then work with the
# installroot directory tree as its root for the rest of its lifetime.
base_config = base.get_config()
base_config.installroot = installroot

# Optionally prevent of loading of plugins. Plugins are loaded by default from the host
base_config.plugins = False

# Optionally load configuration from the config files.
#
# The Base's config is initialized with default values, one of which is
# "config_file_path". This contains the default path to the config file
# ("/etc/dnf/dnf.conf"). If the file does not exist the distribution config file
# is loaded. Function also loads configuration files from distribution and
# user ("/etc/dnf/libdnf5.conf.d") drop-in directories.
# Optionally set a custom value to "config_file_path" before calling this method
# to load configuration from a another configuration file.
base.load_config()

# Optionally you can set and get vars
# vars = base.get_vars().get()
# vars.set('releasever', '33')
#
# Its value can be printed by get_value method
# print(vars.get_value('releasever'))
#
# There are plans in the future to support the methods get() or get_priority()

# Optionally set cachedir.
#
# By default, system cachedir or user cachedir is used, but we can set it to a
# different location. This is useful for tests.
base_config.cachedir = cachedir

# Load vars and do other initialization (of libsolv pool, etc.) based on the
# configuration.  Locks the installroot and varsdir configuration values so
# that they can't be changed.
base.setup()
