@dnf5daemon
@not.with_mode=dnf5
Feature: D-Bus api: rpm remove

Background: Prepare repositories, an installed package and a daemon session
  Given I use repository "dbus-api-base"
    And I use repository "dbus-api-second"
    And I successfully execute rpm with args "-U {context.dnf.fixturesdir}/repos/dbus-api-base/noarch/one-1-1.noarch.rpm"
    And I open a dnf5daemon session

Scenario: Remove an installed package
   When I remove packages "one" via D-Bus
    And I resolve the D-Bus goal
   Then the D-Bus transaction is
        | action | reason | full_nevra       | repo_id  | from_repo_id | package_reason |
        | Remove | User   | one-0:1-1.noarch | @System | <unknown>    | External User  |
   When I run the D-Bus transaction
    And I execute rpm with args "-q one"
   Then the exit code is 1

Scenario: Remove of a package that is not installed
   When I remove packages "two" via D-Bus
    And I resolve the D-Bus goal
   Then the D-Bus goal resolve result is 1
    And the D-Bus transaction is empty
    And the D-Bus transaction problems are
        """
        No packages to remove for argument: two
        """
