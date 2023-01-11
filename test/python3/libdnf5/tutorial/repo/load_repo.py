repo_sack = base.get_repo_sack()

# Create a new repo with the given id.
#
# The repo is a weak pointer to an object owned by the repo sack.
repo = repo_sack.create_repo("rpm-repo1")

# Configure the repo.
#
# Setting at least one of the baseurl, mirrorlist or metalink options is
# mandatory.
#
# baseurl examples:
# * /absolute/path/
# * file:///absolute/path/url/
# * https://example.com/url/
base_config = repo.get_config()
base_config.baseurl().set(baseurl);

# If out of date, downloads fresh metadata of all available repositories and
# loads the repositories into memory.
#
# `False` as first argument says not to load the @System repository (the
# packages installed on the system).
repo_sack.update_and_load_enabled_repos(False)
