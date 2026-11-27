@dnf5daemon
@not.with_mode=dnf5
Feature: D-Bus api: repository list

Background: Prepare repositories and a daemon session
  Given I use repository "dbus-api-base"
    And I use repository "dbus-api-second"
    And I open a dnf5daemon session

Scenario: List all repositories
   When I list repositories via D-Bus
   Then the D-Bus repository list is
        | id              | name                            | enabled |
        | dbus-api-base   | dbus-api-base test repository   | True    |
        | dbus-api-second | dbus-api-second test repository | True    |

Scenario: List repositories matching a pattern
   When I list repositories via D-Bus matching "dbus-api-base"
   Then the D-Bus repository list is
        | id            | name                          | enabled |
        | dbus-api-base | dbus-api-base test repository | True    |
