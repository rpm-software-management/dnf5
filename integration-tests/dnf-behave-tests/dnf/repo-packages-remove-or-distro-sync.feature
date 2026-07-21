@xfail
# repository-packages remove-or-distro-sync is missing: https://github.com/rpm-software-management/dnf5/issues/960
Feature: repo-packages remove-or-distro-sync


Scenario: Sync package to latest version in available repositories
  Given I use repository "dnf-ci-fedora"
   When I execute dnf with args "install glibc"
   Then the exit code is 0
    And Transaction is following
        | Action        | Package                               |
        | install       | glibc-0:2.28-9.fc29.x86_64            |
        | install-dep   | glibc-common-0:2.28-9.fc29.x86_64     |
        | install-dep   | glibc-all-langpacks-0:2.28-9.fc29.x86_64      |
        | install-dep   | basesystem-0:11-6.fc29.noarch         |
        | install-dep   | filesystem-0:3.9-2.fc29.x86_64        |
        | install-dep   | setup-0:2.12.1-1.fc29.noarch          |
  Given I use repository "dnf-ci-fedora-updates"
   When I execute dnf with args "repo-packages dnf-ci-fedora remove-or-distro-sync glibc"
   Then the exit code is 0
    And Transaction is following
        | Action        | Package                               |
        | upgrade       | glibc-0:2.28-26.fc29.x86_64           |
        | upgrade       | glibc-common-0:2.28-26.fc29.x86_64    |
        | upgrade       | glibc-all-langpacks-0:2.28-26.fc29.x86_64     |


Scenario: Remove repository package because it's not available in other repositories
  Given I use repository "dnf-ci-fedora"
   When I execute dnf with args "install setup"
   Then the exit code is 0
    And Transaction is following
        | Action        | Package                               |
        | install       | setup-0:2.12.1-1.fc29.noarch          |
  # following line is not necessary - the package isn't available in the repo anyway
  Given I use repository "dnf-ci-fedora-updates"
   When I execute dnf with args "repo-packages dnf-ci-fedora remove-or-distro-sync setup"
   Then the exit code is 0
    And Transaction is following
        | Action        | Package                               |
        | remove        | setup-0:2.12.1-1.fc29.noarch          |


Scenario: Remove and distro-sync packages from a repository
  Given I use repository "dnf-ci-fedora"
  Given I use repository "dnf-ci-fedora-updates"
   When I execute dnf with args "install glibc libzstd"
   Then the exit code is 0
    And Transaction is following
        | Action        | Package                               |
        | install       | libzstd-0:1.3.6-1.fc29.x86_64         |
        | install       | glibc-0:2.28-26.fc29.x86_64           |
        | install-dep   | glibc-common-0:2.28-26.fc29.x86_64    |
        | install-dep   | glibc-all-langpacks-0:2.28-26.fc29.x86_64     |
        | install-dep   | basesystem-0:11-6.fc29.noarch         |
        | install-dep   | filesystem-0:3.9-2.fc29.x86_64        |
        | install-dep   | setup-0:2.12.1-1.fc29.noarch          |
   When I execute dnf with args "repo-packages dnf-ci-fedora-updates remove-or-distro-sync"
   Then the exit code is 0
    And Transaction is following
        | Action        | Package                               |
        | downgrade     | glibc-0:2.28-9.fc29.x86_64            |
        | downgrade     | glibc-common-0:2.28-9.fc29.x86_64     |
        | downgrade     | glibc-all-langpacks-0:2.28-9.fc29.x86_64      |
        | remove        | libzstd-0:1.3.6-1.fc29.x86_64         |


Scenario: Fail on no package installed from the repo
  Given I use repository "dnf-ci-fedora"
   When I execute dnf with args "install filesystem"
   Then the exit code is 0
    And Transaction is following
        | Action        | Package                               |
        | install       | filesystem-0:3.9-2.fc29.x86_64        |
        | install-dep   | setup-0:2.12.1-1.fc29.noarch          |
   When I execute dnf with args "repo-packages dnf-ci-fedora-updates remove-or-distro-sync"
   Then the exit code is 1


Scenario: Fail on a non-existent package
  Given I use repository "dnf-ci-fedora"
   When I execute dnf with args "repo-packages dnf-ci-fedora remove-or-distro-sync pkg-does-not-exist"
   Then the exit code is 1
