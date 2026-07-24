Feature: Only download rpms with --downloadonly and store them in cache

Scenario: Install/reinstall/upgrade work correctly with --downloadonly option
  Given I use repository "dnf-ci-fedora"
   When I execute dnf with args "install --downloadonly wget"
   Then the exit code is 0
    And RPMDB Transaction is empty

   When I execute dnf with args "install wget"
   Then the exit code is 0
    And stderr contains "Need to download 0 B."
    And RPMDB Transaction is following
        | Action        | Package                                   |
        | install       | wget-0:1.19.5-5.fc29.x86_64               |

   When I execute dnf with args "reinstall --downloadonly wget"
   Then the exit code is 0
    And RPMDB Transaction is empty
   When I execute dnf with args "reinstall wget"
   Then stderr contains "Need to download 0 B."

  Given I use repository "dnf-ci-fedora-updates"
   When I execute dnf with args "upgrade --downloadonly wget"
   Then the exit code is 0
    And RPMDB Transaction is empty
   When I execute rpm with args "-q wget"
   Then stdout contains "wget-1.19.5-5.fc29.x86_64"

   When I execute dnf with args "upgrade wget"
   Then the exit code is 0
    And stderr contains "Need to download 0 B."
    And RPMDB Transaction is following
        | Action        | Package                                   |
        | upgrade       | wget-0:1.19.6-5.fc29.x86_64               |


Scenario: Downgrade works correctly with --downloadonly option
  Given I use repository "dnf-ci-fedora-updates"
   When I execute dnf with args "install wget"
   Then the exit code is 0
  Given I use repository "dnf-ci-fedora"
   When I execute dnf with args "downgrade --downloadonly wget"
   Then the exit code is 0
    And RPMDB Transaction is empty

   When I execute dnf with args "downgrade wget"
   Then the exit code is 0
    And stderr contains "Need to download 0 B."
    And RPMDB Transaction is following
        | Action        | Package                                   |
        | downgrade     | wget-0:1.19.5-5.fc29.x86_64               |

Scenario: Group install works correctly with --downloadonly
  Given I use repository "dnf-ci-thirdparty"
    And I use repository "dnf-ci-fedora"
   When I execute dnf with args "group install --downloadonly dnf-ci-testgroup"
   Then the exit code is 0
    And RPMDB Transaction is empty
   When I execute dnf with args "group install dnf-ci-testgroup"
   Then the exit code is 0
    And stderr contains "Need to download 0 B."
