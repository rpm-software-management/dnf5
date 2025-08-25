repo_sack = base.get_repo_sack()

# Optionally, read repositories from system configuration files.
repo_sack.create_repos_from_system_configuration()

# Optionally, create and configure a new repository.
repo = repo_sack.create_repo("rpm-repeo1")
repo.get_config().baseurl = baseurl

# Load repositories. To limit which repositories are loaded, pass
# a repository type (e.g. libdnf5.repo.Repo.Type_SYSTEM).
repo_sack.load_repos()
