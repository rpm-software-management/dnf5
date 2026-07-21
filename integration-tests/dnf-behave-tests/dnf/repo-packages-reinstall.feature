@xfail
# repository-packages reinstall-old is missing: https://github.com/rpm-software-management/dnf5/issues/961
Feature: repository-packages reinstall-old


Scenario: reinstall-old packages from repository
Given I use repository "dnf-ci-fedora"
 When I execute dnf with args "install glibc"
 Then the exit code is 0
  And Transaction is following
      | Action        | Package                                  |
      | install       | glibc-0:2.28-9.fc29.x86_64               |
      | install-dep   | basesystem-0:11-6.fc29.noarch            |
      | install-dep   | filesystem-0:3.9-2.fc29.x86_64           |
      | install-dep   | setup-0:2.12.1-1.fc29.noarch             |
      | install-dep   | glibc-common-0:2.28-9.fc29.x86_64        |
      | install-dep   | glibc-all-langpacks-0:2.28-9.fc29.x86_64 |
 When I execute dnf with args "repo-packages dnf-ci-fedora reinstall-old"
 Then the exit code is 0
  And Transaction is following
      | Action        | Package                                  |
      | reinstall     | basesystem-0:11-6.fc29.noarch            |
      | reinstall     | filesystem-0:3.9-2.fc29.x86_64           |
      | reinstall     | setup-0:2.12.1-1.fc29.noarch             |
      | reinstall     | glibc-0:2.28-9.fc29.x86_64               |
      | reinstall     | glibc-common-0:2.28-9.fc29.x86_64        |
      | reinstall     | glibc-all-langpacks-0:2.28-9.fc29.x86_64 |


Scenario: fail reinstall-old packages from non existing repository
Given I use repository "dnf-ci-fedora"
 When I execute dnf with args "install glibc"
 Then the exit code is 0
  And Transaction is following
      | Action        | Package                                  |
      | install       | glibc-0:2.28-9.fc29.x86_64               |
      | install-dep   | basesystem-0:11-6.fc29.noarch            |
      | install-dep   | filesystem-0:3.9-2.fc29.x86_64           |
      | install-dep   | setup-0:2.12.1-1.fc29.noarch             |
      | install-dep   | glibc-common-0:2.28-9.fc29.x86_64        |
      | install-dep   | glibc-all-langpacks-0:2.28-9.fc29.x86_64 |
Given I drop repository "dnf-ci-fedora"
 When I execute dnf with args "repo-packages dnf-ci-fedora reinstall-old"
 Then the exit code is 1
