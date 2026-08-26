Feature: Repo list (alias repolist) when there are no repositories


Scenario: Repolist without arguments in an empty installroot
   When I execute dnf with args "repolist"
   Then the exit code is 0
    And stdout is empty
    And stderr contains "No repositories were loaded from the installroot. To use the configuration and repositories of the host system, pass --use-host-config."


Scenario: Repo list with "--enabled" in an empty installroot
   When I execute dnf with args "repo list --enabled"
   Then the exit code is 0
    And stdout is empty
    And stderr contains "No repositories were loaded from the installroot. To use the configuration and repositories of the host system, pass --use-host-config."


Scenario: Repo list with "--disabled" in an empty installroot
   When I execute dnf with args "repo list --disabled"
   Then the exit code is 0
    And stdout is empty
    And stderr contains "No repositories were loaded from the installroot. To use the configuration and repositories of the host system, pass --use-host-config."


Scenario: Repo list with "--all" in an empty installroot
   When I execute dnf with args "repo list --all"
   Then the exit code is 0
    And stdout is empty
    And stderr contains "No repositories were loaded from the installroot. To use the configuration and repositories of the host system, pass --use-host-config."


# The default test installroot has no repositories configured, so an empty
# result always triggers the installroot hint (see previous scenarios).
# Following scenarios set up existing repositories so the "empty view" reporting is
# exercised on its own, verifying it does NOT fall back to the installroot hint
# nor to the "no repositories are enabled" message when repositories do exist.

Scenario: Repo list "--disabled" when all existing repositories are enabled
  Given I use repository "dnf-ci-fedora"
   When I execute dnf with args "repo list --disabled"
   Then the exit code is 0
    And stdout is empty
    And stderr is
    """
    No matches found.
    """

Scenario: Repo list "--enabled" when all existing repositories are disabled
  Given I use repository "dnf-ci-fedora" with configuration
        | key     | value |
        | enabled | 0     |
   When I execute dnf with args "repo list --enabled"
   Then the exit code is 0
    And stdout is empty
    And stderr is
    """
    No matches found: no repositories are enabled.
    """


Scenario: Repo list "--disabled" with a spec matching only an enabled repository
  Given I use repository "dnf-ci-fedora"
   When I execute dnf with args "repo list --disabled dnf-ci-fedora"
   Then the exit code is 0
    And stdout is empty
    And stderr is
    """
    No matches found.
    """


Scenario: Repo list with an unmatched spec when repositories are enabled
  Given I use repository "dnf-ci-fedora"
   When I execute dnf with args "repo list nonexistent-repo"
   Then the exit code is 0
    And stdout is empty
    And stderr is
    """
    No matches found.
    """
   When I execute dnf with args "repo list --disabled nonexistent-repo"
   Then the exit code is 0
    And stdout is empty
    And stderr is
    """
    No matches found.
    """


Scenario: Repo list partial match reports only the unmatched spec
  Given I use repository "dnf-ci-fedora"
   When I execute dnf with args "repo list dnf-ci-fedora nonexistent-repo"
   Then the exit code is 0
    And stdout is
    """
    repo id       repo name
    dnf-ci-fedora dnf-ci-fedora test repository
    """
    And stderr is
    """
    No matches found for "nonexistent-repo".
    """
