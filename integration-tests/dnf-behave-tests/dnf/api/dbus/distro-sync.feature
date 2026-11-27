@dnf5daemon
@not.with_mode=dnf5
Feature: D-Bus api: rpm distro-sync

Background: Prepare repositories, an installed package and a daemon session
  Given I use repository "dbus-api-base"
    And I use repository "dbus-api-second"
    And I successfully execute rpm with args "-U {context.dnf.fixturesdir}/repos/dbus-api-base/noarch/one-1-1.noarch.rpm"
    And I open a dnf5daemon session

Scenario: Distro-sync an installed package
   When I distro-sync packages "one" via D-Bus
    And I resolve the D-Bus goal
   Then the D-Bus transaction is
        | action   | reason        | full_nevra       | repo_id       | from_repo_id | package_reason |
        | Upgrade  | External User | one-0:2-1.noarch | dbus-api-base |              | External User  |
        | Replaced | External User | one-0:1-1.noarch | @System       | <unknown>    | External User  |
   When I run the D-Bus transaction
    And I execute rpm with args "-q one"
   Then the exit code is 0
    And stdout is
        """
        one-2-1.noarch
        """

Scenario: Distro-sync restricted to a repository that does not contain the package
   When I distro-sync packages "one" via D-Bus with options
        | key      | value           |
        | repo_ids | dbus-api-second |
    And I resolve the D-Bus goal
   Then the D-Bus goal resolve result is 2
    And the D-Bus transaction is empty
    And the D-Bus transaction problems are
        """
        No match for argument 'one' in repositories 'dbus-api-second'
        """
