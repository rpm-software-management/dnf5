Feature: Repository overriding configuration files


Background: Set repositories
  Given I use repository "dnf-ci-fedora"
    And I use repository "dnf-ci-thirdparty-updates"
    And I use repository "dnf-ci-fedora-updates" with configuration
        | key     | value |
        | enabled | 0     |
    And I use repository "dnf-ci-thirdparty" with configuration
        | key     | value |
        | enabled | 0     |


Scenario: Test the overrides in "/usr/share/dnf5/repos.override.d"
  Given I create file "/usr/share/dnf5/repos.override.d/10-disable-all.repo" with
      """
      [*]
      enabled=0
      """
    And I create file "/usr/share/dnf5/repos.override.d/20-enable-some.repo" with
      """
      [dnf-ci-fedora-updates]
      enabled=1
      """
   When I execute dnf with args "repo list"
   Then the exit code is 0
    And stdout does not contain "dnf-ci-fedora\s+dnf-ci-fedora"
    And stdout does not contain "dnf-ci-thirdparty-updates\s+dnf-ci-thirdparty-updates"
    And stdout contains "dnf-ci-fedora-updates\s+dnf-ci-fedora-updates"
    And stdout does not contain "dnf-ci-thirdparty\s+dnf-ci-thirdparty"


Scenario: The conf file in "/etc/dnf/repos.override.d" hides the file of the same name in "/usr/share/dnf5/repos.override.d"
  Given I create file "/usr/share/dnf5/repos.override.d/20-enable-some.repo" with
      """
      [dnf-ci-fedora-updates]
      enabled=1
      """
    And I create file "/etc/dnf/repos.override.d/20-enable-some.repo" with
      """
      [dnf-ci-thirdparty]
      enabled=1
      """
   When I execute dnf with args "repo list"
   Then the exit code is 0
    And stdout contains "dnf-ci-fedora\s+dnf-ci-fedora"
    And stdout contains "dnf-ci-thirdparty-updates\s+dnf-ci-thirdparty-updates"
    And stdout does not contain "dnf-ci-fedora-updates\s+dnf-ci-fedora-updates"
    And stdout contains "dnf-ci-thirdparty\s+dnf-ci-thirdparty"


Scenario: The configuration files are applied in alphabetical order by their names.
  Given I create file "/usr/share/dnf5/repos.override.d/10-disable-all.repo" with
      """
      [*]
      enabled=0
      """
    And I create file "/usr/share/dnf5/repos.override.d/20-enable-some.repo" with
      """
      [dnf-ci-fedora-updates]
      enabled=1
      """
    And I create file "/etc/dnf/repos.override.d/20-enable-some.repo" with
      """
      [dnf-ci-fedora]
      enabled=1

      [dnf-ci-thirdparty]
      enabled=1
      """
    And I create file "/usr/share/dnf5/repos.override.d/30-disable-some.repo" with
      """
      [dnf-ci-fedora]
      enabled=0
      """
   When I execute dnf with args "repo list"
   Then the exit code is 0
    And stdout does not contain "dnf-ci-fedora\s+dnf-ci-fedora"
    And stdout does not contain "dnf-ci-thirdparty-updates\s+dnf-ci-thirdparty-updates"
    And stdout does not contain "dnf-ci-fedora-updates\s+dnf-ci-fedora-updates"
    And stdout contains "dnf-ci-thirdparty\s+dnf-ci-thirdparty"


Scenario: Test globs in repoid
  Given I create file "/usr/share/dnf5/repos.override.d/10-disable-all.repo" with
      """
      [*]
      enabled=0

      [d?f-[a-c]i-fed*]
      enabled=1
      """
   When I execute dnf with args "repo list"
   Then the exit code is 0
    And stdout contains "dnf-ci-fedora\s+dnf-ci-fedora"
    And stdout does not contain "dnf-ci-thirdparty-updates\s+dnf-ci-thirdparty-updates"
    And stdout contains "dnf-ci-fedora-updates\s+dnf-ci-fedora-updates"
    And stdout does not contain "dnf-ci-thirdparty\s+dnf-ci-thirdparty"


Scenario: Test vars in repoid
  Given I create file "/etc/dnf/vars/distrib" with
      """
      fedora
      """
    And I create file "/usr/share/dnf5/repos.override.d/10-disable-all.repo" with
      """
      [*]
      enabled=0

      [dnf-ci-${distrib}-updates]
      enabled=1
      """
   When I execute dnf with args "repo list"
   Then the exit code is 0
    And stdout does not contain "dnf-ci-fedora\s+dnf-ci-fedora"
    And stdout does not contain "dnf-ci-thirdparty-updates\s+dnf-ci-thirdparty-updates"
    And stdout contains "dnf-ci-fedora-updates\s+dnf-ci-fedora-updates"
    And stdout does not contain "dnf-ci-thirdparty\s+dnf-ci-thirdparty"


Scenario: Test vars in configuration
  Given I create file "/etc/dnf/vars/enabled" with
      """
      1
      """
    And I create file "/etc/dnf/vars/disabled" with
      """
      0
      """
    And I create file "/usr/share/dnf5/repos.override.d/10-disable-all.repo" with
      """
      [*]
      enabled=${disabled}

      [dnf-ci-fedora-updates]
      enabled=${enabled}
      """
   When I execute dnf with args "repo list"
   Then the exit code is 0
    And stdout does not contain "dnf-ci-fedora\s+dnf-ci-fedora"
    And stdout does not contain "dnf-ci-thirdparty-updates\s+dnf-ci-thirdparty-updates"
    And stdout contains "dnf-ci-fedora-updates\s+dnf-ci-fedora-updates"
    And stdout does not contain "dnf-ci-thirdparty\s+dnf-ci-thirdparty"
