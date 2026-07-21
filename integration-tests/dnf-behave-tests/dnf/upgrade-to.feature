Feature: Upgrade one package to a specific version


Scenario: Upgrade to a specific version
  Given I use repository "dnf-ci-fedora"
   When I execute dnf with args "install flac"
   Then the exit code is 0
    And Transaction is following
        | Action        | Package                                   |
        | install       | flac-0:1.3.2-8.fc29.x86_64                |

  Given I use repository "dnf-ci-fedora-updates"
  When I execute dnf with args "upgrade flac-0:1.3.3-2.fc29"
   Then the exit code is 0
    And Transaction is following
        | Action        | Package                                   |
        | upgrade       | flac-0:1.3.3-2.fc29.x86_64                |
