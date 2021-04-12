// create a reference to the Base's repo_sack for better code readability
auto & repo_sack = base.get_rpm_repo_sack();

// create a new repo with a given repoid
// the repo is a weak pointer to an object owned by the repo_sack
std::string repoid = "example";
auto repo = repo_sack.new_repo(repoid);

// configure the repo
// setting at least one of the baseurl, mirrorlist or metalink options is mandatory
//
// baseurl examples:
// * /absolute/path/
// * file:///absolute/path/url/
// * https://example.com/url/
repo->get_config().baseurl().set(libdnf::Option::Priority::RUNTIME, baseurl);

// download repodata to a local cache
repo->load();

// create a reference to the Base's rpm_sack for better code readability
auto & rpm_sack = base.get_rpm_solv_sack();

// load cached repodata to rpm sack (xml files are converted to solv/solvx at this point)
rpm_sack.load_repo(*repo.get(),
    libdnf::rpm::SolvSack::LoadRepoFlags::USE_FILELISTS |
    libdnf::rpm::SolvSack::LoadRepoFlags::USE_OTHER |
    libdnf::rpm::SolvSack::LoadRepoFlags::USE_PRESTO |
    libdnf::rpm::SolvSack::LoadRepoFlags::USE_UPDATEINFO
);
