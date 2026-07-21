Feature: check-upgrade commands

@bz1769466
Scenario: check for updates according to priority
Given I use repository "dnf-ci-fedora"
 When I execute dnf with args "install glibc"
 Then the exit code is 0
  And Transaction is following
      | Action        | Package                                   |
      | install       | glibc-0:2.28-9.fc29.x86_64                |
      | install-dep   | basesystem-0:11-6.fc29.noarch             |
      | install-dep   | filesystem-0:3.9-2.fc29.x86_64            |
      | install-dep   | setup-0:2.12.1-1.fc29.noarch              |
      | install-dep   | glibc-common-0:2.28-9.fc29.x86_64         |
      | install-dep   | glibc-all-langpacks-0:2.28-9.fc29.x86_64  |
Given I use repository "dnf-ci-fedora-updates"
 When I execute dnf with args "check-upgrade"
 Then the exit code is 100
 Then stdout contains "glibc.x86_64\s+2.28-26.fc29\s+dnf-ci-fedora-updates"
 Then stdout contains "glibc-common.x86_64\s+2.28-26.fc29\s+dnf-ci-fedora-updates"
 Then stdout contains "glibc-all-langpacks.x86_64\s+2.28-26.fc29\s+dnf-ci-fedora-updates"
Given I use repository "dnf-ci-fedora-updates" with configuration
      | key           | value   |
      | priority      | 100     |
 When I execute dnf with args "check-upgrade"
 Then the exit code is 0
 When I execute dnf with args "upgrade"
 Then the exit code is 0
  And Transaction is empty


Scenario: check for updates according to priority and utilize `--json` output
Given I use repository "dnf-ci-fedora"
  And I successfully execute dnf with args "install glibc"
  And I use repository "dnf-ci-fedora-updates"
 When I execute dnf with args "check-upgrade --json"
 Then stdout json matches
  """
  {
    "upgrades": [
      { "name": "glibc", "arch": "x86_64", "evr": "2.28-26.fc29", "repository": "dnf-ci-fedora-updates" },
      { "name": "glibc-common", "arch": "x86_64", "evr": "2.28-26.fc29", "repository": "dnf-ci-fedora-updates" },
      { "name": "glibc-all-langpacks", "arch": "x86_64", "evr": "2.28-26.fc29", "repository": "dnf-ci-fedora-updates" }
    ],
    "obsoleting_packages": [
      {
        "name": "glibc", "arch": "x86_64", "evr": "2.28-26.fc29", "repository": "dnf-ci-fedora-updates",
        "obsoletes": [
          { "name": "glibc", "arch": "x86_64", "evr": "2.28-9.fc29", "repository": "dnf-ci-fedora" }
        ]
      }
    ]
  }
  """
Given I use repository "dnf-ci-fedora-updates" with configuration
      | key           | value   |
      | priority      | 100     |
 When I execute dnf with args "check-upgrade --json"
 Then stdout json matches
  """
  {}
  """
 When I execute dnf with args "upgrade"
 Then the exit code is 0
  And Transaction is empty

@bz2101421
Scenario: --security check-upgrade doesn't show pkgs from resolved advisories (when obsoletes are involved)
Given I use repository "check-update"
  And I execute dnf with args "install A-1-1"
 When I execute dnf with args "upgrade --security"
 Then the exit code is 0
  And Transaction is empty
 When I execute dnf with args "check-upgrade --security"
 Then the exit code is 0
  And stderr is
      """
      <REPOSYNC>
      """
  And stdout is
      """
      No security updates needed, but 1 update(s) available
      """


@bz2151910
Scenario: check-upgrade does not report source packages as upgrades
Given I use repository "dnf-ci-fedora"
  And I use repository "dnf-ci-fedora-updates"
  # exclude binary abcde-2.9.3-1.fc29.noarch package, only source package abcde-2.9.3-1.fc29.src is left in updates repo
  And I configure dnf with
      | key                          | value                            |
      | exclude                      | abcde-2.9.3-1.fc29.noarch        |
  And I successfully execute dnf with args "install abcde-2.9.2-1.fc29.noarch"
 When I execute dnf with args "check-upgrade abcde"
 # abcde-2.9.3-1.fc29.src is not reported as an upgrade
 Then the exit code is 0
  And stderr is
     """
     <REPOSYNC>
     """
  And stdout is empty
