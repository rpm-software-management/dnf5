Feature: Upgrade with duplicate multilib packages in rpmdb


# https://github.com/rpm-software-management/dnf5/issues/518
Scenario: Upgrade reports duplicate versions across architectures in rpmdb
  Given I use repository "upgrade-multilib"
    And I successfully execute dnf with args "install duplicate-versions-0:1.0-1.x86_64 duplicate-versions-0:1.0-1.i686"
   Then Transaction is following
        | Action  | Package                                 |
        | install | duplicate-versions-0:1.0-1.x86_64       |
        | install | duplicate-versions-0:1.0-1.i686         |
  # Force-install duplicate-versions-2.0 for x86_64 alongside the old version to
  # simulate an interrupted upgrade (new version installed, old not yet
  # removed).  This leaves the rpmdb in a broken state with:
  #   duplicate-versions-1.0-1.i686
  #   duplicate-versions-1.0-1.x86_64
  #   duplicate-versions-2.0-1.x86_64
  Given I successfully execute rpm with args "-i --force {context.dnf.fixturesdir}/repos/upgrade-multilib/x86_64/duplicate-versions-2.0-1.x86_64.rpm"
   When I execute rpm with args "-q duplicate-versions"
   Then stdout is
        """
        duplicate-versions-1.0-1.i686
        duplicate-versions-1.0-1.x86_64
        duplicate-versions-2.0-1.x86_64
        """
   When I execute dnf with args "upgrade duplicate-versions"
   Then the exit code is 1
    And DNF Transaction is following
        | Action        | Package                                 |
        | upgrade       | duplicate-versions-0:2.0-1.i686         |
        | replaced      | duplicate-versions-0:1.0-1.i686         |
    And stderr contains lines
    """
    Transaction failed: Rpm transaction check failed.
        - The rpm transaction would unexpectedly remove packages not planned by the solver: duplicate-versions-0:1.0-1.x86_64, duplicate-versions-0:2.0-1.x86_64. This typically happens when the rpmdb contains duplicate package versions after an interrupted upgrade. Please remove the duplicate packages using "dnf remove" and retry.
    """
