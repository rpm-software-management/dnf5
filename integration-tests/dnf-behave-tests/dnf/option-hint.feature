Feature: Hints for misplaced options


# Modularity is disabled since RHEL 11
@use.with_os=rhel__ge__11
Scenario: command specific long option used with root dnf command
   When I execute dnf with args "--skip-broken update"
   Then the exit code is 2
    And stdout is empty
    And stderr is
        """
        Unknown argument "--skip-broken" for command "dnf5". Add "--help" for more information about the arguments.
        The argument is available for commands: environment install, group install, replay, reinstall, downgrade, debuginfo-install, distro-sync, install, do. (It has to be placed after the command.)
        """


# Modularity is disabled since RHEL 11
@not.with_os=rhel__ge__11
Scenario: command specific long option used with root dnf command
   When I execute dnf with args "--skip-broken update"
   Then the exit code is 2
    And stdout is empty
    And stderr is
        """
        Unknown argument "--skip-broken" for command "dnf5". Add "--help" for more information about the arguments.
        The argument is available for commands: module enable, environment install, group install, replay, reinstall, downgrade, debuginfo-install, distro-sync, install, do. (It has to be placed after the command.)
        """


Scenario: command specific short alias option used with root dnf command
   When I execute dnf with args "-l update"
   Then the exit code is 2
    And stdout is empty
    And stderr is
        """
        Unknown argument "-l" for command "dnf5". Add "--help" for more information about the arguments.
        The argument is available for commands: repoquery. (It has to be placed after the command.)
        """


Scenario: command specific long alias option with value used with root dnf command
   When I execute dnf with args "--qf='%{{name}}' update"
   Then the exit code is 2
    And stdout is empty
    And stderr is
        """
        Unknown argument "--qf=%{{name}}" for command "dnf5". Add "--help" for more information about the arguments.
        The argument is available for commands: repoquery. (It has to be placed after the command.)
        """


Scenario: multiple command specific long options used with root dnf command
   When I execute dnf with args "--downloadonly --skip-broken update"
   Then the exit code is 2
    And stdout is empty
    # Hint is present only for the first option
    And stderr is
        """
        Unknown argument "--downloadonly" for command "dnf5". Add "--help" for more information about the arguments.
        The argument is available for commands: environment install, group upgrade, environment upgrade, group install, reinstall, downgrade, distro-sync, upgrade, install, do. (It has to be placed after the command.)
        """


Scenario: no hint is present for non-existing option
   When I execute dnf with args "--non-existing update"
   Then the exit code is 2
    And stdout is empty
    # Hint is present only for the first option
    And stderr is
        """
        Unknown argument "--non-existing" for command "dnf5". Add "--help" for more information about the arguments.
        """
