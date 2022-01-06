auto repo_sack = base.get_repo_sack();

// Create repositories from system configuration files.
repo_sack->create_repos_from_system_configuration();

// If out of date, downloads fresh repositories' metadata and loads the
// repositories into memory.
//
// `true` as first argument says to also load the @System repository (the
// packages installed on the system, loaded from the rpmdb).
repo_sack->update_and_load_enabled_repos(true);
