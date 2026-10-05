Feature: Error handling of the versionlock plugin


Scenario: A malformed versionlock.toml reports a clean error instead of aborting
  Given I create file "/etc/dnf/versionlock.toml" with
    """
    version = "1.0"
    [unterminated
    """
    And I use repository "dnf-ci-fedora"
   When I execute dnf with args "repoquery wget"
   Then the exit code is 1
    And stderr contains "Error parsing versionlock file"
