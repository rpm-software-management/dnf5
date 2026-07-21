@xfail
# repository-packages upgrade is missing: https://github.com/rpm-software-management/dnf5/issues/957
Feature: repo-packages upgrade


Scenario: upgrade packages from not enabled repo
Given I use repository "dnf-ci-fedora"
 When I execute dnf with args "install glibc"
 Then the exit code is 0
  And Transaction is following
      | Action        | Package                                   |
      | install       | glibc-0:2.28-9.fc29.x86_64                |
      | install-dep   | basesystem-0:11-6.fc29.noarch             |
      | install-dep   | filesystem-0:3.9-2.fc29.x86_64            |
      | install-dep   | setup-0:2.12.1-1.fc29.noarch              |
      | install-dep   | glibc-common-0:2.28-9.fc29.x86_64         |
      | install-dep   | glibc-all-langpacks-0:2.28-9.fc29.x86_64  |
  Given I use repository "dnf-ci-fedora-updates" with configuration
      | key     | value |
      | enabled | 0     |
 When I execute dnf with args "repository-packages dnf-ci-fedora-updates upgrade"
 Then the exit code is 0
  And Transaction is following
      | Action        | Package                                   |
      | upgrade       | glibc-0:2.28-26.fc29.x86_64               |
      | upgrade       | glibc-common-0:2.28-26.fc29.x86_64        |
      | upgrade       | glibc-all-langpacks-0:2.28-26.fc29.x86_64 |


Scenario: upgrade packages from enabled repo
Given I use repository "dnf-ci-fedora"
 When I execute dnf with args "install glibc"
 Then the exit code is 0
  And Transaction is following
      | Action        | Package                                   |
      | install       | glibc-0:2.28-9.fc29.x86_64                |
      | install-dep   | basesystem-0:11-6.fc29.noarch             |
      | install-dep   | filesystem-0:3.9-2.fc29.x86_64            |
      | install-dep   | setup-0:2.12.1-1.fc29.noarch              |
      | install-dep   | glibc-common-0:2.28-9.fc29.x86_64         |
      | install-dep   | glibc-all-langpacks-0:2.28-9.fc29.x86_64  |
Given I use repository "dnf-ci-fedora-updates"
 When I execute dnf with args "repository-packages dnf-ci-fedora-updates upgrade"
 Then the exit code is 0
  And Transaction is following
      | Action        | Package                                   |
      | upgrade       | glibc-0:2.28-26.fc29.x86_64               |
      | upgrade       | glibc-common-0:2.28-26.fc29.x86_64        |
      | upgrade       | glibc-all-langpacks-0:2.28-26.fc29.x86_64 |


# For: https://github.com/openSUSE/libsolv/issues/287
# We have two nearly identical vagare packages. One is in simple-base and one in repository-packages repo.
# The packages have the same NEVRA but different Vendor -> this shoudn't lead to a reinstall.
# (The packages likely also have a different buildtime but we cannot rely on that because
# if we are fast enough running the build.sh they might have the same buildtime)
Scenario: repository-packages upgrade doesn't reinstall a pkg if it has same nevra but different vendor or build-time
Given I use repository "simple-base"
  And I execute dnf with args "install vagare"
  And I use repository "repository-packages"
 When I execute dnf with args "repository-packages repository-packages upgrade"
 Then the exit code is 0
  And Transaction is empty


Scenario: repository-packages upgrade with --nobest upgrades packages that can be upgraded
Given I use repository "repository-packages"
  And I execute dnf with args "install A-1 B-1"
  And I use repository "repository-packages-upgrades"
 When I execute dnf with args "repository-packages repository-packages-upgrades upgrade --nobest"
 Then the exit code is 0
  Then Transaction is following
      | Action        | Package             |
      | upgrade       | A-0:2-2.fc29.x86_64 |
      | upgrade       | B-0:2-2.fc29.x86_64 |
