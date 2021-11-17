// create a reference to the Base's repo_sack for better code readability
auto repo_sack = base.get_repo_sack();

// create a new repo with a given repoid
// the repo is a weak pointer to an object owned by the repo_sack
std::string repoid = "example";
auto repo = repo_sack->new_repo(repoid);

// configure the repo
// setting at least one of the baseurl, mirrorlist or metalink options is mandatory
//
// baseurl examples:
// * /absolute/path/
// * file:///absolute/path/url/
// * https://example.com/url/
repo->get_config().baseurl().set(libdnf::Option::Priority::RUNTIME, baseurl);

// download repodata if not fresh, read metadata cache
repo->fetch_metadata();

// create a reference to the Base's rpm_sack for better code readability
auto & rpm_sack = *base.get_rpm_package_sack();

// load cached repodata to rpm sack (xml files are converted to solv/solvx at this point)
rpm_sack.load_repo(*repo.get());
