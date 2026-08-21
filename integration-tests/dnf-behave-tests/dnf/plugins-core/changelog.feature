Feature: dnf changelog command


Background:
  Given I use repository "dnf-ci-changelog"


Scenario: Listing changelogs since given date
   When I execute dnf with args "changelog pkg-with-changelogs --since 2023-01-01"
   Then the exit code is 0
    And stderr is
    """
    <REPOSYNC>
    """
    And stdout is
    """
    Listing changelogs since Sun Jan  1 00:00:00 2023
    Changelogs for pkg-with-changelogs-1.0-3.src, pkg-with-changelogs-1.0-3.x86_64
    * Mon May 01 12:00:00 2023 DNF5 Team <pkgs@dnf.team> - 1.0-3
    - New downstream release 1.0-3

    * Fri Jan 20 12:00:00 2023 DNF5 Team <pkgs@dnf.team> - 1.0-2
    - New downstream release 1.0-2
    """


Scenario: Passing invalid format to the --since option
   When I execute dnf with args "changelog pkg-with-changelogs --since ABCDE"
   Then the exit code is 2
    And stderr is
    """
    Invalid date passed: "ABCDE". Dates in "YYYY-MM-DD" format are expected. Add "--help" for more information about the arguments.
    """
