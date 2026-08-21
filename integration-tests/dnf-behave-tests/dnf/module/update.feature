# Modularity is disabled since RHEL 11
@not.with_os=rhel__ge__11
# Missing module update command
# https://github.com/rpm-software-management/dnf5/issues/146
@xfail
Feature: Updating module profiles


Background:
Given I use repository "dnf-ci-fedora-modular"
  And I use repository "dnf-ci-fedora"


Scenario: I can update a module profile to a newer version
 When I execute dnf with args "module enable nodejs:8"
 Then the exit code is 0
  And modules state is following
      | Module    | State     | Stream    | Profiles  |
      | nodejs    | enabled   | 8          |           |
 When I execute dnf with args "module install nodejs/default"
 Then the exit code is 0
  And Transaction contains
      | Action                    | Package                                       |
      | install-group             | nodejs-1:8.11.4-1.module_2030+42747d40.x86_64 |
      | install-group             | npm-1:8.11.4-1.module_2030+42747d40.x86_64    |
      | module-profile-install    | nodejs/default                                |
Given I use repository "dnf-ci-fedora-modular-updates"
 When I execute dnf with args "module update nodejs/default"
 Then the exit code is 0
  And Transaction is following
      | Action                    | Package                                       |
      | upgrade                   | npm-1:8.14.0-1.module_2030+42747d41.x86_64    |
      | unchanged                 | nodejs-1:8.11.4-1.module_2030+42747d40.x86_64 |


Scenario: Disabled but installed profile should not be receiving updates
 When I execute dnf with args "module install nodejs:8/default"
 Then the exit code is 0
 When I execute dnf with args "module disable nodejs"
 Then the exit code is 0
Given I use repository "dnf-ci-fedora-modular-updates"
 When I execute dnf with args "module update nodejs/default"
 Then the exit code is 0
  And Transaction is empty


Scenario: I try to update a module when no update is available
 When I execute dnf with args "module enable nodejs:8"
 Then the exit code is 0
  And modules state is following
      | Module    | State     | Stream    | Profiles  |
      | nodejs    | enabled   | 8          |           |
 When I execute dnf with args "module install nodejs/default"
 Then the exit code is 0
  And Transaction contains
      | Action                    | Package                                       |
      | install-group             | nodejs-1:8.11.4-1.module_2030+42747d40.x86_64 |
      | install-group             | npm-1:8.11.4-1.module_2030+42747d40.x86_64    |
      | module-profile-install    | nodejs/default                                |
 When I execute dnf with args "module update nodejs/default"
 Then the exit code is 0
  And stdout contains "Nothing to do."
  And Transaction is empty


# (original comment): Dnf does not remove any packages as of now
# TODO(ales): and does it install them?
@xfail
Scenario: I can update a module profile with package changes
 When I execute dnf with args "module enable nodejs:10"
 Then the exit code is 0
  And modules state is following
      | Module    | State     | Stream    | Profiles  |
      | nodejs    | enabled   | 10        |           |
 When I execute dnf with args "module install nodejs/default"
 Then the exit code is 0
  And Transaction contains
      | Action                    | Package                                        |
      | install-group             | nodejs-1:10.11.0-1.module_2200+adbac02b.x86_64 |
      | install-group             | npm-1:10.11.0-1.module_2200+adbac02b.x86_64    |
      | module-profile-install    | nodejs/default                                 |
Given I use repository "dnf-ci-fedora-modular-updates"
 When I execute dnf with args "module update nodejs/default"
 Then the exit code is 0
 And Transaction contains
 | Action                   | Package                                              |
 | upgrade                  | nodejs-1:10.14.1-1.module_2533+7361f245.x86_64       |
 | remove                   | npm-1:10.11.0-1.module_2200+adbac02b.x86_64          |
 | install                  | nodejs-devel-1:10.14.1-1.module_2533+7361f245.x86_64 |


@bz1582548 @bz1582546
Scenario: default stream is used for new deps during an update
 When I execute dnf with args "module enable nodejs:11"
 Then the exit code is 0
  And modules state is following
      | Module    | State     | Stream    | Profiles  |
      | nodejs    | enabled   | 11        |           |
 When I execute dnf with args "module install nodejs:11:20180920144611/default"
 Then the exit code is 0
  And Transaction contains
      | Action                    | Package                                       |
      | install-group             | nodejs-1:11.0.0-1.module_2311+8d497411.x86_64 |
      | install-group             | npm-1:11.0.0-1.module_2311+8d497411.x86_64    |
      | module-profile-install    | nodejs/default                                |
Given I use repository "dnf-ci-fedora-modular-updates"
 When I execute dnf with args "module update nodejs"
 Then the exit code is 0
  And Transaction is following
      | Action                    | Package                                               |
      | upgrade                   | npm-1:11.1.0-1.module_2379+8d497405.x86_64            |
      | upgrade                   | nodejs-1:11.1.0-1.module_2379+8d497405.x86_64         |
      | install-dep               | wget-0:1.19.5-5.fc29.x86_64                           |
      | install-dep               | postgresql-0:9.6.8-1.module_1710+b535a823.x86_64      |
      | install-dep               | postgresql-libs-0:9.6.8-1.module_1710+b535a823.x86_64 |
  And modules state is following
      | Module     | State     | Stream    | Profiles  |
      | nodejs     | enabled   | 11        | default   |
      | postgresql | enabled   | 9.6       |           |


# bz#1583059
Scenario: Both ursine packages and modules are updated during dnf update
 When I execute dnf with args "module enable nodejs:8"
 Then the exit code is 0
  And modules state is following
      | Module    | State     | Stream    | Profiles  |
      | nodejs    | enabled   | 8         |           |
Given I use repository "dnf-ci-thirdparty"
 When I execute dnf with args "install CQRlib CQRlib-extension"
 Then the exit code is 0
  And Transaction is following
      | Action                    | Package                                       |
      | install                   | CQRlib-0:1.1.1-4.fc29.x86_64                  |
      | install                   | CQRlib-extension-0:1.5-2.x86_64               |
 When I execute dnf with args "module install nodejs/default"
 Then the exit code is 0
  And Transaction contains
      | Action                    | Package                                       |
      | install-group             | nodejs-1:8.11.4-1.module_2030+42747d40.x86_64 |
      | install-group             | npm-1:8.11.4-1.module_2030+42747d40.x86_64    |
      | module-profile-install    | nodejs/default                                |
Given I use repository "dnf-ci-fedora-modular-updates"
Given I use repository "dnf-ci-thirdparty-updates"
 When I execute dnf with args "update"
 Then the exit code is 0
   And Transaction is following
       | Action                    | Package                                       |
       | upgrade                   | npm-1:8.14.0-1.module_2030+42747d41.x86_64    |
       | unchanged                 | nodejs-1:8.11.4-1.module_2030+42747d40.x86_64 |
       | upgrade                   | CQRlib-extension-0:1.6-2.x86_64               |
       | install-dep               | SuperRipper-0:1.2-1.x86_64                    |


@bz1647429
Scenario: Update module packages even if no profiles are installed
 When I execute dnf with args "module enable nodejs:11"
 Then the exit code is 0
 When I execute dnf with args "install nodejs-1:11.0.0-1.module_2311+8d497411.x86_64"
 Then the exit code is 0
  And Transaction contains
      | Action                    | Package                                       |
      | install                   | nodejs-1:11.0.0-1.module_2311+8d497411.x86_64 |
  And modules state is following
      | Module    | State     | Stream    | Profiles  |
      | nodejs    | enabled   | 11        |           |
Given I use repository "dnf-ci-fedora-modular-updates"
When I execute dnf with args "module update nodejs"
 Then the exit code is 0
  And Transaction contains
      | Action                    | Package                                        |
      | upgrade                   | nodejs-1:11.1.0-1.module_2379+8d497405.x86_64  |
  And modules state is following
      | Module    | State     | Stream    | Profiles  |
      | nodejs    | enabled   | 11        |           |


@bz1647429
Scenario: Update module packages and dependent packages when no profiles are installed
  Given I use repository "dnf-ci-thirdparty-modular"
   When I execute dnf with args "module enable cookbook:1 ingredience:egg"
   Then the exit code is 0
   When I execute dnf with args "install axe egg blender whisk"
   Then the exit code is 0
    And Transaction is following
        | Action                   | Package                   |
        | install                  | axe-0:1.0-1.x86_64        |
        | install                  | blender-0:1.0-1.x86_64    |
        | install                  | egg-0:1.0-1.x86_64        |
        | install                  | whisk-0:1.0-1.x86_64      |
    And modules state is following
        | Module      | State      | Stream    | Profiles      |
        | cookbook    | enabled    | 1         |               |
  Given I use repository "dnf-ci-thirdparty-modular-updates"
   When I execute dnf with args "module update cookbook"
   Then the exit code is 0
    And Transaction is following
        | Action                   | Package                   |
        | upgrade                  | axe-0:2.0-1.x86_64        |
        | upgrade                  | blender-0:2.0-1.x86_64    |
        | upgrade                  | egg-0:2.0-1.x86_64        |
        | upgrade                  | whisk-0:2.0-1.x86_64      |
    And modules state is following
        | Module      | State      | Stream    | Profiles      |
        | cookbook    | enabled    | 1         |               |


@bz1647429
Scenario: Update module packages and dependent packages when some profiles are installed
  Given I use repository "dnf-ci-thirdparty-modular"
   When I execute dnf with args "module enable cookbook:1 ingredience:egg"
   Then the exit code is 0
   When I execute dnf with args "module install cookbook:1/axe-soup"
   Then the exit code is 0
    And Transaction is following
        | Action                   | Package                   |
        | install-group            | axe-0:1.0-1.x86_64        |
        | module-profile-install   | cookbook/axe-soup         |
   When I execute dnf with args "module install cookbook:1/ham-and-eggs"
   Then the exit code is 0
    And Transaction is following
        | Action                   | Package                   |
        | install-group            | egg-0:1.0-1.x86_64        |
        | install-group            | ham-0:1.0-1.x86_64        |
        | install-group            | spatula-0:1.0-1.x86_64    |
        | module-profile-install   | cookbook/ham-and-eggs     |
   When I execute dnf with args "install whisk"
   Then the exit code is 0
    And Transaction is following
        | Action                   | Package                   |
        | install                  | whisk-0:1.0-1.x86_64      |
    And modules state is following
        | Module      | State      | Stream    | Profiles      |
        | cookbook    | enabled    | 1         | axe-soup, ham-and-eggs   |
  Given I use repository "dnf-ci-thirdparty-modular-updates"
   When I execute dnf with args "module update cookbook"
   Then the exit code is 0
    And Transaction is following
        | Action                   | Package                   |
        | upgrade                  | axe-0:2.0-1.x86_64        |
        | upgrade                  | egg-0:2.0-1.x86_64        |
        | upgrade                  | ham-0:2.0-1.x86_64        |
        | upgrade                  | spatula-0:2.0-1.x86_64    |
        | upgrade                  | whisk-0:2.0-1.x86_64      |
    And modules state is following
        | Module      | State      | Stream    | Profiles      |
        | cookbook    | enabled    | 1         | axe-soup, ham-and-eggs  |


@bz1647429
Scenario: Update module profiles that contains non-modular packages and packages from different modules
  Given I use repository "dnf-ci-thirdparty-modular"
   When I execute dnf with args "module enable cookbook:1 ingredience:egg"
   Then the exit code is 0
   When I execute dnf with args "module install cookbook:1/axe-soup"
   Then the exit code is 0
    And Transaction is following
        | Action                   | Package                   |
        | install-group            | axe-0:1.0-1.x86_64        |
        | module-profile-install   | cookbook/axe-soup         |
   When I execute dnf with args "module install cookbook:1/ham-and-eggs"
   Then the exit code is 0
    And Transaction is following
        | Action                   | Package                   |
        | install-group            | egg-0:1.0-1.x86_64        |
        | install-group            | ham-0:1.0-1.x86_64        |
        | install-group            | spatula-0:1.0-1.x86_64    |
        | module-profile-install   | cookbook/ham-and-eggs     |
    And modules state is following
        | Module      | State      | Stream    | Profiles      |
        | cookbook    | enabled    | 1         | axe-soup, ham-and-eggs  |
  Given I use repository "dnf-ci-thirdparty-modular-updates"
   When I execute dnf with args "module update cookbook:1/axe-soup cookbook:1/ham-and-eggs"
   Then the exit code is 0
    And Transaction is following
        | Action                   | Package                   |
        | upgrade                  | axe-0:2.0-1.x86_64        |
        | upgrade                  | egg-0:2.0-1.x86_64        |
        | upgrade                  | ham-0:2.0-1.x86_64        |
        | upgrade                  | spatula-0:2.0-1.x86_64    |
    And modules state is following
        | Module      | State      | Stream    | Profiles      |
        | cookbook    | enabled    | 1         | axe-soup, ham-and-eggs  |
