# Modularity is disabled since RHEL 11
@not.with_os=rhel__ge__11
Feature: Advisory aplicability on a modular system


# Missing module install command
# https://github.com/rpm-software-management/dnf5/issues/146
@xfail
@bz1622614
Scenario: List available updates for installed streams (updates available)
Given I use repository "dnf-ci-fedora-modular"
  And I use repository "dnf-ci-fedora"
 When I execute dnf with args "module enable postgresql:9.6"
 Then the exit code is 0
  And modules state is following
      | Module     | State     | Stream    | Profiles  |
      | postgresql | enabled   | 9.6       |           |
 When I execute dnf with args "module install postgresql/default"
 Then the exit code is 0
  And Transaction contains
      | Action                    | Package                                                 |
      | install-group             | postgresql-server-0:9.6.8-1.module_1710+b535a823.x86_64 |
      | install-dep               | postgresql-0:9.6.8-1.module_1710+b535a823.x86_64        |
      | install-dep               | postgresql-libs-0:9.6.8-1.module_1710+b535a823.x86_64   |
      | module-profile-install    | postgresql/default                                      |
Given I use repository "dnf-ci-fedora-modular-updates"
 When I execute dnf with args "updateinfo list"
 Then the exit code is 0
  And stderr is
      """
      <REPOSYNC>
      """
  And stdout is
      """
      FEDORA-2019-0329090518 enhancement postgresql-9.6.11-1.x86_64
      """


# Missing module install command
# https://github.com/rpm-software-management/dnf5/issues/146
@xfail
@bz1622614
Scenario: Updates for non enabled streams are hidden
Given I use repository "dnf-ci-fedora-modular"
  And I use repository "dnf-ci-fedora"
 When I execute dnf with args "module install postgresql:6/default"
 Then the exit code is 0
  And Transaction contains
      | Action                    | Package                                               |
      | install-group             | postgresql-server-0:6.1-1.module_2514+aa9aadc5.x86_64 |
      | install-dep               | postgresql-0:6.1-1.module_2514+aa9aadc5.x86_64        |
      | install-dep               | postgresql-libs-0:6.1-1.module_2514+aa9aadc5.x86_64   |
      | module-profile-install    | postgresql/default                                    |
Given I use repository "dnf-ci-fedora-modular-updates"
 Then I execute dnf with args "updateinfo list"
 Then the exit code is 0
  And stderr is
      """
      <REPOSYNC>
      """
  And stdout is empty


# Reported as https://github.com/rpm-software-management/dnf5/issues/1856
@xfail
@bz1804234
Scenario: having installed packages from one collection and enabled all modules from another doesn't activate advisory
Given I use repository "dnf-ci-fedora"
  And I execute dnf with args "install nodejs"
  And I use repository "dnf-ci-fedora-modular-updates"
  And I execute dnf with args "module enable postgresql:9.6"
 When I execute dnf with args "updateinfo list"
 Then stderr is
      """
      <REPOSYNC>
      """
  And stdout is empty


# Missing module install command
# https://github.com/rpm-software-management/dnf5/issues/146
@xfail
@bz1804234
Scenario: having installed packages from all collections but enabled modules only for one shows just the one
Given I use repository "dnf-ci-fedora"
  And I execute dnf with args "install nodejs"
  And I use repository "dnf-ci-fedora-modular"
  And I successfully execute dnf with args "module enable postgresql:9.6"
  And I successfully execute dnf with args "module install postgresql/default"
  And I use repository "dnf-ci-fedora-modular-updates"
 When I execute dnf with args "updateinfo list"
 Then stderr is
      """
      <REPOSYNC>
      """
  And stdout is
      """
      FEDORA-2019-0329090518 enhancement nodejs-1:8.14.0-1.x86_64
      FEDORA-2019-0329090518 enhancement postgresql-9.6.11-1.x86_64
      """


# Missing module install command
# https://github.com/rpm-software-management/dnf5/issues/146
@xfail
Scenario: having two active collections shows packages from both
Given I use repository "dnf-ci-fedora"
  And I successfully execute dnf with args "install nodejs"
  And I use repository "dnf-ci-fedora-modular"
  And I successfully execute dnf with args "module enable postgresql:9.6"
  And I successfully execute dnf with args "module install postgresql/default"
  And I use repository "dnf-ci-fedora-modular-updates"
  And I successfully execute dnf with args "module enable nodejs:8"
 When I execute dnf with args "updateinfo list"
 Then stderr is
      """
      <REPOSYNC>
      """
  And stdout is
      """
      FEDORA-2019-0329090518 enhancement nodejs-1:8.14.0-1.x86_64
      FEDORA-2019-0329090518 enhancement postgresql-9.6.11-1.x86_64
      """


# Reported as https://github.com/rpm-software-management/dnf5/issues/1811
@xfail
@bz1804234
Scenario: Show applicable advisories only from active contexts
Given I use repository "dnf-ci-multicontext-modular-advisory"
 When I execute dnf with args "module enable perl:5.24"
 Then the exit code is 0
  And modules state is following
      | Module     | State     | Stream    | Profiles  |
      | perl       | enabled   | 5.24      |           |

 When I execute dnf with args "install test-perl-DBI-0:1-1.module_el8+7554+8763afg8.x86_64"
 Then the exit code is 0
  And Transaction is following
      | Action                | Package                                                |
      | install               | test-perl-DBI-0:1-1.module_el8+7554+8763afg8.x86_64    |
      | install-dep           | test-perl-0:5.24-2.module_el8+4182+3467aeg6.x86_64     |
      | module-stream-enable  | perl-DBI:master                                        |
  When I execute dnf with args "updateinfo list"
  Then the exit code is 0
   And stderr is
      """
      <REPOSYNC>
      """
   And stdout is
      """
      FEDORA-2019-0329090518 enhancement test-perl-DBI-1-2.module_el8+6587+9879afr5.x86_64
      """
  When I execute dnf with args "check-update --enhancement"
  Then the exit code is 100
   And stdout contains "test-perl-DBI.x86_64\s+1-2.module_el8\+6587\+9879afr5\s+dnf-ci-multicontext-modular-advisory"
  When I execute dnf with args "update --enhancement"
  Then the exit code is 0
   And Transaction is following
      | Action                | Package                                               |
      | upgrade               | test-perl-DBI-0:1-2.module_el8+6587+9879afr5.x86_64   |
  When I execute dnf with args "updateinfo"
  Then the exit code is 0
   And stderr is
       """
       <REPOSYNC>
       """
   And stdout is empty
