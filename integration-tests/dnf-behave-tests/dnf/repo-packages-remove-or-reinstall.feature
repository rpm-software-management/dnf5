@xfail
# repository-packages remove-or-reinstall is missing: https://github.com/rpm-software-management/dnf5/issues/959
Feature: repo-packages remove-or-reinstall


Scenario: remove-or-reinstall all packages from repository
Given I use repository "dnf-ci-fedora"
Given I use repository "dnf-ci-fedora-updates"
 When I execute dnf with args "install CQRlib-devel libzstd"
 Then the exit code is 0
  And Transaction is following
      | Action        | Package                                   |
      | install       | CQRlib-devel-0:1.1.2-16.fc29.x86_64       |
      | install       | libzstd-0:1.3.6-1.fc29.x86_64             |
      | install-dep   | glibc-0:2.28-26.fc29.x86_64               |
      | install-dep   | glibc-common-0:2.28-26.fc29.x86_64        |
      | install-dep   | glibc-all-langpacks-0:2.28-26.fc29.x86_64 |
      | install-dep   | basesystem-0:11-6.fc29.noarch             |
      | install-dep   | filesystem-0:3.9-2.fc29.x86_64            |
      | install-dep   | setup-0:2.12.1-1.fc29.noarch              |
      | install-dep   | CQRlib-0:1.1.2-16.fc29.x86_64             |
Given I use repository "dnf-ci-fedora-updates-testing"
 When I execute dnf with args "repo-packages dnf-ci-fedora-updates remove-or-reinstall"
 Then the exit code is 0
  And Transaction is following
      | Action        | Package                                   |
      | remove        | libzstd-0:1.3.6-1.fc29.x86_64             |
      | remove        | glibc-0:2.28-26.fc29.x86_64               |
      | remove        | glibc-common-0:2.28-26.fc29.x86_64        |
      | remove        | glibc-all-langpacks-0:2.28-26.fc29.x86_64 |
      | remove-unused | basesystem-0:11-6.fc29.noarch             |
      | remove-unused | filesystem-0:3.9-2.fc29.x86_64            |
      | remove-unused | setup-0:2.12.1-1.fc29.noarch              |
      | reinstall     | CQRlib-0:1.1.2-16.fc29.x86_64             |


Scenario: Remove single package from repository
Given I use repository "dnf-ci-fedora"
 When I execute dnf with args "install setup"
 Then the exit code is 0
  And Transaction is following
      | Action        | Package                                   |
      | install       | setup-0:2.12.1-1.fc29.noarch              |
 When I execute dnf with args "repo-packages dnf-ci-fedora remove-or-reinstall setup"
 Then the exit code is 0
  And Transaction is following
      | Action        | Package                                   |
      | remove        | setup-0:2.12.1-1.fc29.noarch              |


Scenario: Reinstall single package from repository
Given I use repository "dnf-ci-fedora-updates"
 When I execute dnf with args "install CQRlib"
 Then the exit code is 0
  And Transaction is following
      | Action        | Package                                   |
      | install       | CQRlib-0:1.1.2-16.fc29.x86_64             |
Given I use repository "dnf-ci-fedora-updates-testing"
 When I execute dnf with args "repo-packages dnf-ci-fedora-updates remove-or-reinstall CQRlib"
 Then the exit code is 0
  And Transaction is following
      | Action        | Package                                   |
      | reinstall     | CQRlib-0:1.1.2-16.fc29.x86_64             |
