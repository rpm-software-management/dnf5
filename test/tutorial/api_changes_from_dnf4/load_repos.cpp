auto repo_sack = base.get_repo_sack();

// Optionally, read repositories from system configuration files.
repo_sack->create_repos_from_system_configuration();

// Optionally, create and configure a new repository.
auto repo = repo_sack->create_repo("my_new_repo_id");
repo->get_config().get_baseurl_option().set(baseurl);

// Load repositores. To limit which repositories are loaded, pass
// a repository type (e.g. libdnf5.repo.Repo.Type_SYSTEM).
repo_sack->load_repos();
