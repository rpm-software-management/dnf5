// Get or set configuration options.
auto & config = base.get_config();
config.get_skip_unavailable_option().set(true);
std::cout << config.get_skip_unavailable_option().get_value() << std::endl;

// Get or set a value of a variable, e.g.:
auto vars = base.get_vars();
vars->set("releasever", "42");
std::cout << vars->get_value("releasever") << std::endl;
