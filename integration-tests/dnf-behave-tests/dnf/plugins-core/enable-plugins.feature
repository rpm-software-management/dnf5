Feature: Tests for report nonexisting plugin

Background:
  Given I use repository "dnf-ci-fedora"

@bz1673289 @bz1467304
Scenario: Report nonexisting plugin to disable
   When I execute dnf with args "repoquery empty --disableplugin=NotExisting"
   Then the exit code is 0
    And stderr is
    """
    No matches were found for the following plugin name patterns while disabling libdnf plugins: NotExisting
    <REPOSYNC>
    """

@bz1673289 @bz1467304
Scenario: Report nonexisting plugin to enable
   When I execute dnf with args "repoquery empty --enableplugin=NotExisting"
   Then the exit code is 0
    And stderr is
    """
    No matches were found for the following plugin name patterns while enabling libdnf plugins: NotExisting
    <REPOSYNC>
    """
