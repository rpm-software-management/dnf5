Feature: Tests missing or misconfigured versionlock.list file in versionlock plugin


Background: Set up versionlock infrastructure in the installroot
  Given I create file "/etc/dnf/versionlock.toml" with
    """
    """
  Given I use repository "dnf-ci-fedora"
    And I use repository "dnf-ci-fedora-updates"

Scenario: dnf versionlock clear will create empty file if versionlock.list is missing
  Given I delete file "/etc/dnf/versionlock.toml"
  When I execute dnf with args "versionlock clear"
  Then the exit code is 0
  And file "/etc/dnf/versionlock.toml" exists
  And file "/etc/dnf/versionlock.toml" contents is
    """
    packages = []
    version = "1.0"
    """
