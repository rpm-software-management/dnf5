Feature: Test for dnf repoquery --whatrequires

Scenario: List all profiders of file including real file provider and only provider
  Given I use repository "dnf-ci-whatprovides"
    # Following repository only adds balast packages
    And I use repository "dnf-ci-fedora"
   When I execute dnf with args "repoquery --whatprovides /etc/dummy.conf"
   Then the exit code is 0
    And stderr is
      """
      <REPOSYNC>
      """
    And stdout is
      """
      dnf-ci-alfa-0:1.0-1.noarch
      dnf-ci-beta-0:1.0-1.noarch
      """
