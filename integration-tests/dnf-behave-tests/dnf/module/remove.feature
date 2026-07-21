# Modularity is disabled since RHEL 11
@not.with_os=rhel__ge__11
# Missing module remove command
# https://github.com/rpm-software-management/dnf5/issues/146
@xfail
Feature: Module profile removal


Background:
Given I use repository "dnf-ci-fedora-modular"
  And I use repository "dnf-ci-fedora"
 When I execute dnf with args "module enable nodejs:8"
 Then the exit code is 0
  And modules state is following
      | Module    | State     | Stream    | Profiles  |
      | nodejs    | enabled   | 8         |           |
 When I execute dnf with args "module install nodejs/default"
 Then the exit code is 0
  And Transaction contains
      | Action                    | Package                                       |
      | install-group             | nodejs-1:8.11.4-1.module_2030+42747d40.x86_64 |
      | install-group             | npm-1:8.11.4-1.module_2030+42747d40.x86_64    |
      | module-profile-install    | nodejs/default                                |


# https://bugzilla.redhat.com/show_bug.cgi?id=1581609
@bz1581609
@bz1583596
Scenario: I can remove an installed module profile specifying stream name
 When I execute dnf with args "module remove nodejs:8"
 Then the exit code is 0
  And modules state is following
      | Module    | State     | Stream    | Profiles  |
      | nodejs    | enabled   | 8         |           |
  And modules state is following
      | Module    | State     | Stream    | Profiles  |
      | nodejs    | enabled   | 8         |           |
  And Transaction contains
      | Action                    | Package                                       |
      | remove                    | nodejs-1:8.11.4-1.module_2030+42747d40.x86_64 |
      | remove                    | npm-1:8.11.4-1.module_2030+42747d40.x86_64    |
      | module-profile-disable    | nodejs/default                                |


# https://bugzilla.redhat.com/show_bug.cgi?id=1581621
# https://bugzilla.redhat.com/show_bug.cgi?id=1629841
@bz1583596
@bz1629841 @bz1581624
Scenario: I can remove an installed module profile using "module remove <module_spec>"
 When I execute dnf with args "module install nodejs/minimal"
 Then Transaction contains
      | Action                    | Package                                       |
      | unchanged                 | nodejs-1:8.11.4-1.module_2030+42747d40.x86_64 |
      | module-profile-install    | nodejs/minimal                                |
 When I execute dnf with args "module install nodejs/development"
 Then Transaction contains
      | Action                    | Package                                             |
      | install-group             | nodejs-devel-1:8.11.4-1.module_2030+42747d40.x86_64 |
      | module-profile-install    | nodejs/development                                  |
 When I execute dnf with args "module remove nodejs/minimal"
 Then the exit code is 0
 Then Transaction is following
      | Action                    | Package                                             |
      | unchanged                 | nodejs-devel-1:8.11.4-1.module_2030+42747d40.x86_64 |
      # cannot remove nodejs because it's needed by other profiles
      | unchanged                 | nodejs-1:8.11.4-1.module_2030+42747d40.x86_64 |
      | module-profile-disable    | nodejs/minimal                                      |
  And modules state is following
      | Module    | State     | Stream    | Profiles             |
      | nodejs    | enabled   | 8         | default, development |


@bz1629848
Scenario: Removing of a non-installed profiles would pass
 When I execute dnf with args "module remove nodejs/development"
 Then the exit code is 0
 Then Transaction is empty
  And modules state is following
      | Module    | State     | Stream    | Profiles  |
      | nodejs    | enabled   | 8         | default   |
  And modules state is following
      | Module    | State     | Stream    | Profiles |
      | nodejs    | enabled   | 8         | default  |
  And stdout contains "Nothing to do."
  And stderr contains "Unable to match profile in argument nodejs/development"


@bz1583596
Scenario: I can remove multiple profiles
 When I execute dnf with args "module install nodejs/minimal"
 Then Transaction contains
      | Action                    | Package                                       |
      | unchanged                 | nodejs-1:8.11.4-1.module_2030+42747d40.x86_64 |
      | module-profile-install    | nodejs/minimal                                |
 When I execute dnf with args "module install nodejs/development"
 Then Transaction contains
      | Action                    | Package                                             |
      | install-group             | nodejs-devel-1:8.11.4-1.module_2030+42747d40.x86_64 |
      | module-profile-install    | nodejs/development                                  |
  And modules state is following
      | Module    | State     | Stream    | Profiles                      |
      | nodejs    | enabled   | 8         | default, minimal, development |
 When I execute dnf with args "module remove nodejs/development nodejs:8/default"
 Then Transaction is following
      | Action                 | Package                                             |
      | remove                 | nodejs-devel-1:8.11.4-1.module_2030+42747d40.x86_64 |
      | module-profile-disable | nodejs/default                                      |
      | module-profile-disable | nodejs/development                                  |
  And modules state is following
      | Module    | State     | Stream    | Profiles  |
      | nodejs    | enabled   | 8         | minimal   |


# https://bugzilla.redhat.com/show_bug.cgi?id=1648264
@bz1583596
@bz1648264
Scenario: I can remove an installed module profile using "remove @<module_spec>"
 When I execute dnf with args "module install nodejs/minimal"
 Then Transaction contains
      | Action                    | Package                                       |
      | unchanged                 | nodejs-1:8.11.4-1.module_2030+42747d40.x86_64 |
      | module-profile-install    | nodejs/minimal                                |
 When I execute dnf with args "module install nodejs/development"
 Then Transaction contains
      | Action                    | Package                                             |
      | install-group             | nodejs-devel-1:8.11.4-1.module_2030+42747d40.x86_64 |
      | module-profile-install    | nodejs/development                                  |
 When I execute dnf with args "remove @nodejs/minimal"
 Then the exit code is 0
 Then Transaction is following
      | Action                    | Package                                             |
      | unchanged                 | nodejs-devel-1:8.11.4-1.module_2030+42747d40.x86_64 |
      # cannot remove nodejs because it's needed by other profiles
      | unchanged                 | nodejs-1:8.11.4-1.module_2030+42747d40.x86_64 |
      | module-profile-disable    | nodejs/minimal                                      |
  And modules state is following
      | Module    | State     | Stream    | Profiles             |
      | nodejs    | enabled   | 8         | default, development |

@bz1700529
Scenario: Remove module profile when userinstalled package requires its package
Given I use repository "dnf-ci-fifthparty"
  And I use repository "dnf-ci-fifthparty-modular"
   # install module that contains package luke
   When I execute dnf with args "module install jedi:1/duo"
   Then the exit code is 0
    And Transaction contains
        | Action                    | Package                |
        | install-group             | luke-0:1.0-1.x86_64    |
        | install-group             | obi-wan-0:1.0-1.x86_64 |
        | module-profile-install    | jedi/duo               |
   # install package leia that requires luke
   When I execute dnf with args "install leia"
   Then the exit code is 0
    And Transaction contains
        | Action                    | Package                |
        | install                   | leia-0:1.0-1.x86_64    |
   # remove the profile
   When I execute dnf with args "module remove jedi"
   Then the exit code is 0
    And Transaction contains
        | Action                   | Package                 |
        | remove                   | obi-wan-0:1.0-1.x86_64  |
        | unchanged                | luke-0:1.0-1.x86_64     |
        | module-profile-disable   | jedi/duo                |
   # remove package leia, luke should be also removed due to having reason 'dep'
   When I execute dnf with args "remove leia"
   Then the exit code is 0
    And Transaction contains
        | Action                    | Package                |
        | remove                    | leia-0:1.0-1.x86_64    |
        | remove-unused             | luke-0:1.0-1.x86_64    |


Scenario: Remove command respects --all switch to remove all packages
  Given I successfully execute dnf with args "install nodejs-docs"
   When I execute dnf with args "module remove nodejs --all"
   Then the exit code is 0
    And Transaction is following
        | Action                    | Package                                       |
        | remove                    | nodejs-1:8.11.4-1.module_2030+42747d40.x86_64 |
        # nodejs-docs was not installed as part of profile, without --all it
        # would not get removed
        | remove                    | nodejs-docs-1:8.11.4-1.module_2030+42747d40.noarch |
        | remove                    | npm-1:8.11.4-1.module_2030+42747d40.x86_64    |
        | remove-unused             | basesystem-0:11-6.fc29.noarch                 |
        | remove-unused             | filesystem-0:3.9-2.fc29.x86_64                |
        | remove-unused             | setup-0:2.12.1-1.fc29.noarch                  |
        | remove-unused             | glibc-0:2.28-9.fc29.x86_64                    |
        | remove-unused             | glibc-all-langpacks-0:2.28-9.fc29.x86_64      |
        | remove-unused             | glibc-common-0:2.28-9.fc29.x86_64             |
        | module-profile-disable    | nodejs/default                                |


# modular package luke is part of both jedi:1[d] and empire:1[d] streams
Scenario: Packages belonging to multiple modules are not removed with --all
  Given I use repository "dnf-ci-fifthparty"
    And I use repository "dnf-ci-fifthparty-modular"
    And I successfully execute dnf with args "module enable jedi:1"
    And I successfully execute dnf with args "install luke obi-wan"
   When I execute dnf with args "module remove --all jedi"
   Then the exit code is 0
    And Transaction is following
        | Action                    | Package                |
        | remove                    | obi-wan-0:1.0-1.x86_64 |
    And stdout contains "Package luke-1.0-1.x86_64 belongs to multiple modules, skipping"
  Given I successfully execute dnf with args "module disable empire"
   When I execute dnf with args "module remove --all jedi"
   Then the exit code is 0
    And Transaction is following
        | Action                    | Package                |
        | remove                    | luke-0:1.0-1.x86_64    |
    And stdout does not contain "belongs to multiple modules, skipping"

@bz1904490
Scenario: module removed with --all and not existing module argument - no traceback
  Given I use repository "dnf-ci-fifthparty"
    And I use repository "dnf-ci-fifthparty-modular"
   When I execute dnf with args "module remove --all noexists"
   Then the exit code is 0
    And Transaction is empty
    And stderr is
        """
        Problems in request:
        missing groups or modules: noexists
        """
