Feature: Test groups and environments with the same id


Scenario: Install a group with the same id as an environment
 Given I use repository "comps-same-id-1"
  When I execute dnf with args "group install same-id"
  Then the exit code is 0
    And Transaction is following
        | Action        | Package                         |
        | group-install | Same id group                   |
        | install-group | C-mandatory-1.0-1.x86_64        |
    And file "/usr/lib/sysimage/libdnf5/comps_groups/groups/same-id.xml" exists
    And file "/usr/lib/sysimage/libdnf5/comps_groups/environments/same-id.xml" does not exist
    And file "/usr/lib/sysimage/libdnf5/comps_groups/same-id.xml" does not exist


Scenario: Install an environment with the same id as a group
 Given I use repository "comps-same-id-1"
  When I execute dnf with args "environment install same-id"
  Then the exit code is 0
    And Transaction is following
        | Action        | Package                         |
        | env-install   | Same id env                     |
        | group-install | AB-group                        |
        | install-group | A-mandatory-1.0-1.x86_64        |
    And file "/usr/lib/sysimage/libdnf5/comps_groups/groups/same-id.xml" does not exist
    And file "/usr/lib/sysimage/libdnf5/comps_groups/environments/same-id.xml" exists
    And file "/usr/lib/sysimage/libdnf5/comps_groups/same-id.xml" does not exist


Scenario: Install both group and environment with the same id
 Given I use repository "comps-same-id-1"
  When I execute dnf with args "install @same-id"
  Then the exit code is 0
    And Transaction is following
        | Action        | Package                         |
        | env-install   | Same id env                     |
        | group-install | AB-group                        |
        | install-group | A-mandatory-1.0-1.x86_64        |
        | group-install | Same id group                   |
        | install-group | C-mandatory-1.0-1.x86_64        |
    And file "/usr/lib/sysimage/libdnf5/comps_groups/groups/same-id.xml" exists
    And file "/usr/lib/sysimage/libdnf5/comps_groups/environments/same-id.xml" exists
    And file "/usr/lib/sysimage/libdnf5/comps_groups/same-id.xml" does not exist


Scenario: Remove a group with the same id as an environment
 Given I use repository "comps-same-id-1"
   And I execute dnf with args "install @same-id"
  When I execute dnf with args "group remove same-id"
  Then the exit code is 0
    And Transaction is following
        | Action        | Package                         |
        | group-remove  | Same id group                   |
        | remove        | C-mandatory-1.0-1.x86_64        |
    And file "/usr/lib/sysimage/libdnf5/comps_groups/groups/same-id.xml" does not exist
    And file "/usr/lib/sysimage/libdnf5/comps_groups/environments/same-id.xml" exists
    And file "/usr/lib/sysimage/libdnf5/comps_groups/same-id.xml" does not exist


Scenario: Remove an environment with the same id as a group
 Given I use repository "comps-same-id-1"
   And I execute dnf with args "install @same-id"
  When I execute dnf with args "environment remove same-id"
  Then the exit code is 0
    And Transaction is following
        | Action        | Package                         |
        | env-remove    | Same id env                     |
        | group-remove  | AB-group                        |
        | remove        | A-mandatory-1.0-1.x86_64        |
    And file "/usr/lib/sysimage/libdnf5/comps_groups/groups/same-id.xml" exists
    And file "/usr/lib/sysimage/libdnf5/comps_groups/environments/same-id.xml" does not exist
    And file "/usr/lib/sysimage/libdnf5/comps_groups/same-id.xml" does not exist


Scenario: Remove both group and environment with the same id
 Given I use repository "comps-same-id-1"
   And I execute dnf with args "install @same-id"
  When I execute dnf with args "remove @same-id"
  Then the exit code is 0
    And Transaction is following
        | Action        | Package                         |
        | env-remove    | Same id env                     |
        | group-remove  | AB-group                        |
        | remove        | A-mandatory-1.0-1.x86_64        |
        | group-remove  | Same id group                   |
        | remove        | C-mandatory-1.0-1.x86_64        |
    And file "/usr/lib/sysimage/libdnf5/comps_groups/groups/same-id.xml" does not exist
    And file "/usr/lib/sysimage/libdnf5/comps_groups/environments/same-id.xml" does not exist
    And file "/usr/lib/sysimage/libdnf5/comps_groups/same-id.xml" does not exist


Scenario: Upgrade a group with the same id as an environment
 Given I use repository "comps-same-id-1"
   And I execute dnf with args "install @same-id"
   And I use repository "comps-same-id-2"
  When I execute dnf with args "group upgrade same-id"
  Then the exit code is 0
    And Transaction is following
        | Action        | Package                         |
        | group-upgrade | Same id group                   |
        | install-group | D-mandatory-1.0-1.x86_64        |
    And file "/usr/lib/sysimage/libdnf5/comps_groups/groups/same-id.xml" exists
    And file "/usr/lib/sysimage/libdnf5/comps_groups/environments/same-id.xml" exists
    And file "/usr/lib/sysimage/libdnf5/comps_groups/same-id.xml" does not exist


Scenario: Upgrade an environment with the same id as a group
 Given I use repository "comps-same-id-1"
   And I execute dnf with args "install @same-id"
   And I use repository "comps-same-id-2"
  When I execute dnf with args "environment upgrade same-id"
  Then the exit code is 0
    And Transaction is following
        | Action        | Package                         |
        | env-upgrade   | Same id env                     |
        | group-upgrade | AB-group                        |
        | install-group | B-mandatory-1.0-1.x86_64        |
    And file "/usr/lib/sysimage/libdnf5/comps_groups/groups/same-id.xml" exists
    And file "/usr/lib/sysimage/libdnf5/comps_groups/environments/same-id.xml" exists
    And file "/usr/lib/sysimage/libdnf5/comps_groups/same-id.xml" does not exist


Scenario: Upgrade both group and environment with the same id
 Given I use repository "comps-same-id-1"
   And I execute dnf with args "install @same-id"
   And I use repository "comps-same-id-2"
  When I execute dnf with args "upgrade @same-id"
  Then the exit code is 0
    And Transaction is following
        | Action        | Package                         |
        | env-upgrade   | Same id env                     |
        | group-upgrade | AB-group                        |
        | install-group | B-mandatory-1.0-1.x86_64        |
        | group-upgrade | Same id group                   |
        | install-group | D-mandatory-1.0-1.x86_64        |
    And file "/usr/lib/sysimage/libdnf5/comps_groups/groups/same-id.xml" exists
    And file "/usr/lib/sysimage/libdnf5/comps_groups/environments/same-id.xml" exists
    And file "/usr/lib/sysimage/libdnf5/comps_groups/same-id.xml" does not exist


Scenario: Install a group with the same id and name as an environment
 Given I use repository "comps-same-id-1"
  When I execute dnf with args "group install same-id-and-name"
  Then the exit code is 0
    And Transaction is following
        | Action        | Package                         |
        | group-install | Same id and name group and env  |
        | install-group | C-mandatory-1.0-1.x86_64        |
    And file "/usr/lib/sysimage/libdnf5/comps_groups/groups/same-id-and-name.xml" exists
    And file "/usr/lib/sysimage/libdnf5/comps_groups/environments/same-id-and-name.xml" does not exist
    And file "/usr/lib/sysimage/libdnf5/comps_groups/same-id-and-name.xml" does not exist


Scenario: Install an environment with the same id and name as a group
 Given I use repository "comps-same-id-1"
  When I execute dnf with args "environment install same-id-and-name"
  Then the exit code is 0
    And Transaction is following
        | Action        | Package                         |
        | env-install   | Same id and name group and env  |
        | group-install | AB-group                        |
        | install-group | A-mandatory-1.0-1.x86_64        |
    And file "/usr/lib/sysimage/libdnf5/comps_groups/groups/same-id-and-name.xml" does not exist
    And file "/usr/lib/sysimage/libdnf5/comps_groups/environments/same-id-and-name.xml" exists
    And file "/usr/lib/sysimage/libdnf5/comps_groups/same-id-and-name.xml" does not exist


Scenario: Install a group using globs that also match an environment
 Given I use repository "comps-same-id-1"
  When I execute dnf with args "group install same-prefix-*"
  Then the exit code is 0
    And Transaction is following
        | Action        | Package                         |
        | group-install | Same prefix group               |
        | install-group | C-mandatory-1.0-1.x86_64        |
    And file "/usr/lib/sysimage/libdnf5/comps_groups/groups/same-prefix-group.xml" exists
    And file "/usr/lib/sysimage/libdnf5/comps_groups/environments/same-prefix-env.xml" does not exist


Scenario: Install an environment using globs that also match a group
 Given I use repository "comps-same-id-1"
  When I execute dnf with args "environment install same-prefix-*"
  Then the exit code is 0
    And Transaction is following
        | Action        | Package                         |
        | env-install   | Same prefix env                 |
        | group-install | AB-group                        |
        | install-group | A-mandatory-1.0-1.x86_64        |
    And file "/usr/lib/sysimage/libdnf5/comps_groups/groups/same-prefix-group.xml" does not exist
    And file "/usr/lib/sysimage/libdnf5/comps_groups/environments/same-prefix-env.xml" exists


Scenario: Install both group and environment using globs
 Given I use repository "comps-same-id-1"
  When I execute dnf with args "install @same-prefix-*"
  Then the exit code is 0
    And Transaction is following
        | Action        | Package                         |
        | env-install   | Same prefix env                 |
        | group-install | AB-group                        |
        | install-group | A-mandatory-1.0-1.x86_64        |
        | group-install | Same prefix group               |
        | install-group | C-mandatory-1.0-1.x86_64        |
    And file "/usr/lib/sysimage/libdnf5/comps_groups/groups/same-prefix-group.xml" exists
    And file "/usr/lib/sysimage/libdnf5/comps_groups/environments/same-prefix-env.xml" exists
