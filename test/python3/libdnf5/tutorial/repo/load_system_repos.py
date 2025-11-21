repo_sack = base.get_repo_sack()

# Create repositories from system configuration files.
repo_sack.create_repos_from_system_configuration()

# Acquire a write lock on the system repository before loading it
base.lock_system_repo(libdnf5.utils.LockAccessType_WRITE, libdnf5.utils.LockBlockingType_BLOCKING)

# If out of date, downloads fresh repositories' metadata and loads the
# repositories into memory.
#
# load_repos() without any arguments says to load both the @System
# repository (the packages installed on the system, loaded from the rpmdb)
# and the available repositories.
repo_sack.load_repos()
