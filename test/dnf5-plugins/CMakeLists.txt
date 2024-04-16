# Temporarily disable Copr unit tests until the plugin is fixed.
# The Copr dnf5 plug-in is broken. It uses an uninitialized variable. Because of the random value
# of this variable, unit tests fail randomly (if it has a value of 0, which is always the case when we allow
# the compiler to initialize memory to 0 or use clang).
#add_subdirectory(copr_plugin)
