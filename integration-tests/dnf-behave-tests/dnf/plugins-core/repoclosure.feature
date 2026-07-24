Feature: Repoclosure command tests


# we need to override the default value for the `best` config option
Background:
  Given I configure dnf with
        | key   | value |
        | best  | False |


Scenario: Run repoclosure without any options
  Given I use repository "repoclosure"
    And I use repository "repoclosure-lookaside"
   When I execute dnf with args "repoclosure"
   Then the exit code is 1
    And stdout is
    """
    package: dedalo-1.0-1.fc29.noarch from repoclosure-lookaside
      unresolved deps (1):
        the-other-lib
    package: labirinto-3.0-1.fc29.noarch from repoclosure-lookaside
      unresolved deps (1):
        the-other-lib
    """
    And stderr is
        """
        <REPOSYNC>
        Error: Repoclosure ended with unresolved dependencies (2) across 2 packages.
        """


Scenario: Run repoclosure with a package specified
  Given I use repository "repoclosure"
    And I use repository "repoclosure-lookaside"
   When I execute dnf with args "repoclosure dedalo"
   Then the exit code is 1
    And stdout is
    """
    package: dedalo-1.0-1.fc29.noarch from repoclosure-lookaside
      unresolved deps (1):
        the-other-lib
    """
    And stderr is
        """
        <REPOSYNC>
        Error: Repoclosure ended with unresolved dependencies (1) across 1 packages.
        """


Scenario: Run repoclosure with --arch option
  Given I use repository "repoclosure"
    And I use repository "repoclosure-lookaside"
   When I execute dnf with args "repoclosure --arch=x86_64"
   Then the exit code is 0


Scenario: Check closure only for specified repo
  Given I use repository "repoclosure"
    And I use repository "repoclosure-lookaside"
   When I execute dnf with args "repoclosure --check repoclosure"
   Then the exit code is 0


Scenario: Check closure with --newest (consider only the latest package in each repo)
  Given I use repository "repoclosure"
    And I use repository "repoclosure-lookaside"
   When I execute dnf with args "repoclosure --newest"
   Then the exit code is 1
    And stdout is
    """
    package: dedalo-1.0-1.fc29.noarch from repoclosure-lookaside
      unresolved deps (1):
        the-other-lib
    package: labirinto-3.0-1.fc29.noarch from repoclosure-lookaside
      unresolved deps (1):
        the-other-lib
    package: labirinto-4.0-1.fc29.x86_64 from repoclosure-lookaside
      unresolved deps (1):
        the-lib = 3.0
    """
    And stderr is
        """
        <REPOSYNC>
        Error: Repoclosure ended with unresolved dependencies (3) across 3 packages.
        """


Scenario: Check closure with --best
  Given I use repository "repoclosure"
    And I use repository "repoclosure-lookaside"
   When I execute dnf with args "repoclosure --best"
   Then the exit code is 1
    And stdout is
    """
    package: dedalo-1.0-1.fc29.noarch from repoclosure-lookaside
      unresolved deps (1):
        the-other-lib
    package: labirinto-1.0-1.fc29.x86_64 from repoclosure
      unresolved deps (1):
        the-lib = 1.0
    package: labirinto-2.0-1.fc29.x86_64 from repoclosure
      unresolved deps (1):
        the-lib = 2.0
    package: labirinto-3.0-1.fc29.noarch from repoclosure-lookaside
      unresolved deps (1):
        the-other-lib
    package: labirinto-4.0-1.fc29.x86_64 from repoclosure-lookaside
      unresolved deps (1):
        the-lib = 3.0
    """
    And stderr is
        """
        <REPOSYNC>
        Error: Repoclosure ended with unresolved dependencies (5) across 5 packages.
        """


Scenario: Check closure with both --newest and --best
  Given I use repository "repoclosure"
    And I use repository "repoclosure-lookaside"
   When I execute dnf with args "repoclosure --newest --best"
   Then the exit code is 1
    And stdout is
    """
    package: dedalo-1.0-1.fc29.noarch from repoclosure-lookaside
      unresolved deps (1):
        the-other-lib
    package: labirinto-2.0-1.fc29.x86_64 from repoclosure
      unresolved deps (1):
        the-lib = 2.0
    package: labirinto-3.0-1.fc29.noarch from repoclosure-lookaside
      unresolved deps (1):
        the-other-lib
    package: labirinto-4.0-1.fc29.x86_64 from repoclosure-lookaside
      unresolved deps (1):
        the-lib = 3.0
    """
    And stderr is
        """
        <REPOSYNC>
        Error: Repoclosure ended with unresolved dependencies (4) across 4 packages.
        """
