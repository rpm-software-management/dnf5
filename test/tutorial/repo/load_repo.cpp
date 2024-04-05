auto repo_sack = base.get_repo_sack();

// Create a new repo with the given id.
//
// The repo is a weak pointer to an object owned by the repo sack.
auto repo = repo_sack->create_repo("my_new_repo_id");

// Configure the repo.
//
// Setting at least one of the baseurl, mirrorlist or metalink options is
// mandatory.
//
// baseurl examples:
// * /absolute/path/
// * file:///absolute/path/url/
// * https://example.com/url/
repo->get_config().get_baseurl_option().set(baseurl);

// If out of date, downloads fresh metadata of all available repositories and
// loads the repositories into memory.
//
// `libdnf5.repo.Repo.Type_AVAILABLE` as first argument says to load only the available
// repositories (repository SYSTEM, that contains installed pacakges, is not loaded).
repo_sack->load_repos(libdnf5::repo::Repo::Type::AVAILABLE);
