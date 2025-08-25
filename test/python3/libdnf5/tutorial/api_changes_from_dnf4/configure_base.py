# Get or set configuration options.
config = base.get_config()
config.skip_unavailable = True
print(config.skip_unavailable)

# Get or set values of variables.
vars = base.get_vars()
vars.set("releasever", "42")
print(vars.get_value("releasever"))
