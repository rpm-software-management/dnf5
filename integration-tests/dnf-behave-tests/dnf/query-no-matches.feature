Feature: Unified error reporting for query commands


# https://github.com/rpm-software-management/dnf5/issues/1075
# Query commands should report:
# - when no repositories are enabled, a message about no repos (or an
#   installroot hint when no repos are configured in the installroot)
# - "No matches found for <spec>." when some arguments don't match
# - Domain-specific messages when there are no candidates at all


# ==================
# group list / info
# ==================

Scenario: group list reports no repos when all repos disabled
 Given I use repository "comps-group-list" with configuration
       | key     | value |
       | enabled | 0     |
  When I execute dnf with args "group list"
  Then the exit code is 0
   And stdout is empty
   And stderr contains "No matches found: no repositories are enabled."


Scenario: group info reports no repos when all repos disabled
 Given I use repository "comps-group-list" with configuration
       | key     | value |
       | enabled | 0     |
  When I execute dnf with args "group info"
  Then the exit code is 0
   And stdout is empty
   And stderr contains "No matches found: no repositories are enabled."


Scenario: group list with spec reports no repos when all repos disabled
 Given I use repository "comps-group-list" with configuration
       | key     | value |
       | enabled | 0     |
  When I execute dnf with args "group list test-group"
  Then the exit code is 0
   And stdout is empty
   And stderr contains "No matches found: no repositories are enabled."


Scenario: group list --installed with no groups installed
 Given I use repository "comps-group-list"
  When I execute dnf with args "group list --installed"
  Then the exit code is 0
   And stderr contains "No matches found: no groups are installed."


# Use a repo without comps data so no groups come from repos.
# The only groups are installed ones from system state (groups.toml).
Scenario: group list --available with no groups available
 Given I use repository "dnf-ci-fedora"
   And I create file "/usr/lib/sysimage/libdnf5/groups.toml" with
       """
       version = "1.0"
       [groups]
       [groups.test-group]
       userinstalled = true
       """
   And I successfully execute dnf with args "group list"
  Then stdout is
       """
       <REPOSYNC>
       ID                   Name Installed
       test-group                      yes
       Groups: 1 (1 installed, 0 available)
       """
  When I execute dnf with args "group list --available"
  Then the exit code is 0
   And stderr contains "No matches found: no groups are available."


Scenario: group list reports unmatched spec when some match
 Given I use repository "comps-group-list"
  When I execute dnf with args "group list test-group nonexistent"
  Then the exit code is 0
   And stdout contains "test-group"
   And stderr contains "No matches found for \"nonexistent\"."


Scenario: group list reports when all specs unmatched
 Given I use repository "comps-group-list"
  When I execute dnf with args "group list nonexistent1 nonexistent2"
  Then the exit code is 0
   And stdout is empty
   And stderr contains "No matches found."


Scenario: group info reports unmatched spec when some match
 Given I use repository "comps-group-list"
  When I execute dnf with args "group info test-group nonexistent"
  Then the exit code is 0
   And stdout contains "test-group"
   And stderr contains "No matches found for \"nonexistent\"."


# ========================
# environment list / info
# ========================

Scenario: environment list reports no repos when all repos disabled
 Given I use repository "comps-group-list" with configuration
       | key     | value |
       | enabled | 0     |
  When I execute dnf with args "environment list"
  Then the exit code is 0
   And stdout is empty
   And stderr contains "No matches found: no repositories are enabled."


Scenario: environment info reports no repos when all repos disabled
 Given I use repository "comps-group-list" with configuration
       | key     | value |
       | enabled | 0     |
  When I execute dnf with args "environment info"
  Then the exit code is 0
   And stdout is empty
   And stderr contains "No matches found: no repositories are enabled."


Scenario: environment list --installed with no environments installed
 Given I use repository "comps-group-list"
  When I execute dnf with args "environment list --installed"
  Then the exit code is 0
   And stdout is empty
   And stderr contains "No matches found: no environments are installed."


# Use a repo without comps data so no environments come from repos.
Scenario: environment list --available with no environments available
 Given I use repository "dnf-ci-fedora"
   And I create file "/usr/lib/sysimage/libdnf5/environments.toml" with
       """
       version = "1.0"
       [environments]
       test-environment = {groups = [], userinstalled = true}
       """
   And I successfully execute dnf with args "environment list"
  Then stdout is
       """
       <REPOSYNC>
       ID                   Name Installed
       test-environment                yes
       """
  When I execute dnf with args "environment list --available"
  Then the exit code is 0
   And stdout is empty
   And stderr contains "No matches found: no environments are available."


Scenario: environment list reports unmatched spec when some match
 Given I use repository "comps-group-list"
  When I execute dnf with args "environment list test-environment nonexistent"
  Then the exit code is 0
   And stdout contains "test-environment"
   And stderr contains "No matches found for \"nonexistent\"."


Scenario: environment list reports when all specs unmatched
 Given I use repository "comps-group-list"
  When I execute dnf with args "environment list nonexistent1 nonexistent2"
  Then the exit code is 0
   And stdout is empty
   And stderr contains "No matches found."


Scenario: environment info reports unmatched spec when some match
 Given I use repository "comps-group-list"
  When I execute dnf with args "environment info test-environment nonexistent"
  Then the exit code is 0
   And stdout contains "test-environment"
   And stderr contains "No matches found for \"nonexistent\"."


# ==================
# repo list / info
# ==================

Scenario: repo list reports no repos when all repos disabled
 Given I use repository "dnf-ci-fedora" with configuration
       | key     | value |
       | enabled | 0     |
  When I execute dnf with args "repo list"
  Then the exit code is 0
   And stdout is empty
   And stderr contains "No matches found: no repositories are enabled."


Scenario: repo info reports no repos when all repos disabled
 Given I use repository "dnf-ci-fedora" with configuration
       | key     | value |
       | enabled | 0     |
  When I execute dnf with args "repo info"
  Then the exit code is 0
   And stdout is empty
   And stderr contains "No matches found: no repositories are enabled."


Scenario: repo list reports unmatched spec when some match
 Given I use repository "dnf-ci-fedora"
   And I use repository "dnf-ci-fedora-updates"
  When I execute dnf with args "repo list dnf-ci-fedora nonexistent"
  Then the exit code is 0
   And stdout contains "dnf-ci-fedora"
   And stderr contains "No matches found for \"nonexistent\"."


Scenario: repo list reports when all specs unmatched
 Given I use repository "dnf-ci-fedora"
  When I execute dnf with args "repo list nonexistent1 nonexistent2"
  Then the exit code is 0
   And stdout is empty
   And stderr contains "No matches found."


Scenario: repo info reports unmatched spec when some match
 Given I use repository "dnf-ci-fedora"
   And I use repository "dnf-ci-fedora-updates"
  When I execute dnf with args "repo info dnf-ci-fedora nonexistent"
  Then the exit code is 0
   And stdout contains "dnf-ci-fedora"
   And stderr contains "No matches found for \"nonexistent\"."


# ==================
# advisory list / info
# ==================

Scenario: advisory list reports no repos when all repos disabled
 Given I use repository "dnf-ci-fedora" with configuration
       | key     | value |
       | enabled | 0     |
  When I execute dnf with args "advisory list"
  Then the exit code is 0
   And stdout is empty
   And stderr contains "No matches found: no repositories are enabled."


Scenario: advisory info reports no repos when all repos disabled
 Given I use repository "dnf-ci-fedora" with configuration
       | key     | value |
       | enabled | 0     |
  When I execute dnf with args "advisory info"
  Then the exit code is 0
   And stdout is empty
   And stderr contains "No matches found: no repositories are enabled."


Scenario: advisory summary reports no repos when all repos disabled
 Given I use repository "dnf-ci-fedora" with configuration
       | key     | value |
       | enabled | 0     |
  When I execute dnf with args "advisory summary"
  Then the exit code is 0
   And stderr contains "No matches found: no repositories are enabled."


# ==================
# repoquery
# ==================

Scenario: repoquery reports no repos when all repos disabled
 Given I use repository "dnf-ci-fedora" with configuration
       | key     | value |
       | enabled | 0     |
  When I execute dnf with args "repoquery"
  Then the exit code is 0
   And stdout is empty
   And stderr contains "No matches found: no repositories are enabled."


Scenario: repoquery with spec reports no repos when all repos disabled
 Given I use repository "dnf-ci-fedora" with configuration
       | key     | value |
       | enabled | 0     |
  When I execute dnf with args "repoquery nonexistent"
  Then the exit code is 0
   And stdout is empty
   And stderr contains "No matches found: no repositories are enabled."


Scenario: repoquery --installed does not report no repos enabled
  When I execute dnf with args "repoquery --installed"
  Then the exit code is 0
   And stdout is empty
   And stderr does not contain "no repositories are enabled"


# ==================
# list / info
# ==================

Scenario: list reports no repos when all repos disabled
 Given I use repository "dnf-ci-fedora" with configuration
       | key     | value |
       | enabled | 0     |
  When I execute dnf with args "list"
  Then the exit code is 0
   And stdout is empty
   And stderr contains "No matches found: no repositories are enabled."


Scenario: list with spec reports no repos when all repos disabled
 Given I use repository "dnf-ci-fedora" with configuration
       | key     | value |
       | enabled | 0     |
  When I execute dnf with args "list nonexistent"
  Then the exit code is 1
   And stdout is empty
   And stderr contains "No matches found: no repositories are enabled."


Scenario: list --installed does not report no repos enabled
  When I execute dnf with args "list --installed"
  Then the exit code is 0
   And stdout is empty
   And stderr does not contain "no repositories are enabled"


Scenario: list reports unmatched spec when some match
 Given I use repository "dnf-ci-fedora"
  When I execute dnf with args "list setup nonexistent"
  Then the exit code is 0
   And stdout contains "setup"
   And stderr contains "No matches found for \"nonexistent\"."


Scenario: list reports when all specs unmatched
 Given I use repository "dnf-ci-fedora"
  When I execute dnf with args "list nonexistent1 nonexistent2"
  Then the exit code is 1
   And stdout is empty
   And stderr contains "No matches found."


# ==================
# module list / info
# ==================

@not.with_os=rhel__ge__11
Scenario: module list reports no repos when all repos disabled
 Given I use repository "dnf-ci-fedora-modular" with configuration
       | key     | value |
       | enabled | 0     |
  When I execute dnf with args "module list"
  Then the exit code is 0
   And stdout is empty
   And stderr contains "No matches found: no repositories are enabled."


@not.with_os=rhel__ge__11
Scenario: module list reports unmatched spec when some match
 Given I use repository "dnf-ci-fedora-modular"
  When I execute dnf with args "module list nodejs nonexistent"
  Then the exit code is 0
   And stdout contains "nodejs"
   And stderr contains "No matches found for \"nonexistent\"."


@not.with_os=rhel__ge__11
Scenario: module list reports when all specs unmatched
 Given I use repository "dnf-ci-fedora-modular"
  When I execute dnf with args "module list nonexistent1 nonexistent2"
  Then the exit code is 0
   And stdout is empty
   And stderr contains "No matches found."


@not.with_os=rhel__ge__11
Scenario: module list reports no module streams exist
 Given I use repository "dnf-ci-fedora"
  When I execute dnf with args "module list"
  Then the exit code is 0
   And stdout is empty
   And stderr contains "No matches found: no module streams exist."


# ==================
# search
# ==================

Scenario: search reports no repos when all repos disabled
 Given I use repository "dnf-ci-fedora" with configuration
       | key     | value |
       | enabled | 0     |
  When I execute dnf with args "search nonexistent"
  Then the exit code is 0
   And stdout is empty
   And stderr contains "No matches found: no repositories are enabled."


Scenario: search reports no matches found
 Given I use repository "dnf-ci-fedora"
  When I execute dnf with args "search nonexistent"
  Then the exit code is 0
   And stdout is empty
   And stderr contains "No matches found."


# ==================
# provides
# ==================

Scenario: provides reports no repos when all repos disabled
 Given I use repository "dnf-ci-fedora" with configuration
       | key     | value |
       | enabled | 0     |
  When I execute dnf with args "provides webclient"
  Then the exit code is 1
   And stdout is empty
   And stderr contains "No matches found: no repositories are enabled."


Scenario: provides prints installroot hint when no repos configured
  When I execute dnf with args "provides webclient"
  Then the exit code is 1
   And stdout is empty
   And stderr contains "No repositories were loaded from the installroot. To use the configuration and repositories of the host system, pass --use-host-config."


# ==================
# installroot hint
# ==================

Scenario: group list prints installroot hint when no repos configured
  When I execute dnf with args "group list"
  Then the exit code is 0
   And stdout is empty
   And stderr contains "No repositories were loaded from the installroot. To use the configuration and repositories of the host system, pass --use-host-config."


Scenario: environment list prints installroot hint when no repos configured
  When I execute dnf with args "environment list"
  Then the exit code is 0
   And stdout is empty
   And stderr contains "No repositories were loaded from the installroot. To use the configuration and repositories of the host system, pass --use-host-config."


Scenario: advisory list prints installroot hint when no repos configured
  When I execute dnf with args "advisory list"
  Then the exit code is 0
   And stdout is empty
   And stderr contains "No repositories were loaded from the installroot. To use the configuration and repositories of the host system, pass --use-host-config."


Scenario: repoquery prints installroot hint when no repos configured
  When I execute dnf with args "repoquery"
  Then the exit code is 0
   And stdout is empty
   And stderr contains "No repositories were loaded from the installroot. To use the configuration and repositories of the host system, pass --use-host-config."


Scenario: list prints installroot hint when no repos configured
  When I execute dnf with args "list"
  Then the exit code is 0
   And stdout is empty
   And stderr contains "No repositories were loaded from the installroot. To use the configuration and repositories of the host system, pass --use-host-config."


Scenario: search prints installroot hint when no repos configured
  When I execute dnf with args "search nonexistent"
  Then the exit code is 0
   And stdout is empty
   And stderr contains "No repositories were loaded from the installroot. To use the configuration and repositories of the host system, pass --use-host-config."


Scenario: copr list prints installroot hint when no repos configured
  When I execute dnf with args "copr list"
  Then the exit code is 0
   And stdout is empty
   And stderr contains "No repositories were loaded from the installroot. To use the configuration and repositories of the host system, pass --use-host-config."
