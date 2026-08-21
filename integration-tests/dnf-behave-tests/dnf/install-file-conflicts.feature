Feature: Tests for file conflicts reporting

Background:
  Given I use repository "dnf-ci-install-conflicts"


Scenario: An error is reported when a package with a file conflict is tried to be installed
  Given I successfully execute dnf with args "install package-one-1.0-1.x86_64"
   Then Transaction is following
        | Action                    | Package                      |
        | install                   | package-one-0:1.0-1.x86_64   |
   When I execute dnf with args "install package-two-1.0-1.x86_64"
   Then the exit code is 1
    And stderr contains lines
        """
        Transaction failed: Rpm transaction failed.
          - file /usr/lib/package/conflicting-file from install of package-two-0:1.0-1.x86_64 conflicts with file from package package-one-0:1.0-1.x86_64
        """
