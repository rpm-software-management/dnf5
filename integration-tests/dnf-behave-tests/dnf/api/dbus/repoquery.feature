@dnf5daemon
@not.with_mode=dnf5
Feature: D-Bus api: package list

Background: Prepare repositories and a daemon session
  Given I use repository "dbus-api-base"
    And I use repository "dbus-api-second"
    And I open a dnf5daemon session

Scenario: List all available packages
   When I list packages via D-Bus with attributes "full_nevra repo_id"
   Then the D-Bus package list is
        | full_nevra       | repo_id         |
        | one-0:1-1.noarch | dbus-api-base   |
        | one-0:1-1.src    | dbus-api-base   |
        | one-0:2-1.noarch | dbus-api-base   |
        | one-0:2-1.src    | dbus-api-base   |
        | two-0:2-2.noarch | dbus-api-second |
        | two-0:2-2.src    | dbus-api-second |

Scenario: List available packages matching a pattern
   When I list packages via D-Bus matching "one" with attributes "full_nevra repo_id"
   Then the D-Bus package list is
        | full_nevra       | repo_id       |
        | one-0:1-1.noarch | dbus-api-base |
        | one-0:1-1.src    | dbus-api-base |
        | one-0:2-1.noarch | dbus-api-base |
        | one-0:2-1.src    | dbus-api-base |
