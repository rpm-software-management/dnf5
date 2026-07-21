@xfail
# repository-packages check-update is missing: https://github.com/rpm-software-management/dnf5/issues/958
Feature: repo-packages check-update


Scenario: check for updates - not available
Given I use repository "dnf-ci-fedora"
 When I execute dnf with args "install basesystem"
 Then the exit code is 0
  And Transaction is following
      | Action        | Package                                   |
      | install       | basesystem-0:11-6.fc29.noarch             |
      | install-dep   | filesystem-0:3.9-2.fc29.x86_64            |
      | install-dep   | setup-0:2.12.1-1.fc29.noarch              |
 When I execute dnf with args "-q repository-packages dnf-ci-fedora check-update"
 Then stdout is empty
 Then the exit code is 0


Scenario: check for updates in not enabled repo - available
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
 When I execute dnf with args "repository-packages dnf-ci-fedora-updates check-update"
 Then the exit code is 100
 Then stdout contains "glibc.x86_64\s+2.28-26.fc29\s+dnf-ci-fedora-updates"
 Then stdout contains "glibc-common.x86_64\s+2.28-26.fc29\s+dnf-ci-fedora-updates"
 Then stdout contains "glibc-all-langpacks.x86_64\s+2.28-26.fc29\s+dnf-ci-fedora-updates"


Scenario: check for updates in enabled repo - available
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
 When I execute dnf with args "repository-packages dnf-ci-fedora-updates check-update"
 Then the exit code is 100
 Then stdout contains "glibc.x86_64\s+2.28-26.fc29\s+dnf-ci-fedora-updates"
 Then stdout contains "glibc-common.x86_64\s+2.28-26.fc29\s+dnf-ci-fedora-updates"
 Then stdout contains "glibc-all-langpacks.x86_64\s+2.28-26.fc29\s+dnf-ci-fedora-updates"
