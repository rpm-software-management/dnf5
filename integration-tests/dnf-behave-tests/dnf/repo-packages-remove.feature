@xfail
# repository-packages remove is missing: https://github.com/rpm-software-management/dnf5/issues/953
Feature: repo-packages remove


  Scenario: Remove packages from available repository, also remove their
    dependencies and packages that depend on them (even from other repositories)
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
 When I execute dnf with args "repo-packages dnf-ci-fedora remove"
 Then the exit code is 0
  And Transaction is following
      | Action        | Package                                   |
      | remove        | CQRlib-devel-0:1.1.2-16.fc29.x86_64       |
      | remove        | basesystem-0:11-6.fc29.noarch             |
      | remove        | filesystem-0:3.9-2.fc29.x86_64            |
      | remove        | setup-0:2.12.1-1.fc29.noarch              |
      | remove-dep    | libzstd-0:1.3.6-1.fc29.x86_64             |
      | remove-unused | glibc-0:2.28-26.fc29.x86_64               |
      | remove-unused | glibc-common-0:2.28-26.fc29.x86_64        |
      | remove-unused | glibc-all-langpacks-0:2.28-26.fc29.x86_64 |
      | remove-unused | CQRlib-0:1.1.2-16.fc29.x86_64             |
