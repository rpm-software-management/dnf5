Feature: Test config options includepkgs and excludepkgs with option disable_excludes


Scenario Outline: Install an RPM that is NOT in includepkgs in <conf>, with option disable_excludes equal to <disable-conf>
  Given I use repository "dnf-ci-fedora"
   When I execute dnf with args "install flac --setopt=<prefix>includepkgs=setup --setopt=disable_excludes=<disable-conf>"
   Then the exit code is 0
    And Transaction is following
        | Action        | Package                          |
        | install       | flac-0:1.3.2-8.fc29.x86_64       |

Examples:
  | conf   | prefix         | disable-conf   |
  | main   |                | main           |
  | main   |                | *              |
  | repo   | dnf-ci-fedora. | dnf-ci-fedora  |
  | repo   | dnf-ci-fedora. | *              |


Scenario Outline: Install an RPM that is in excludepkgs in <conf>, with option disable_excludes equal to <disable-conf>
  Given I use repository "dnf-ci-fedora"
   When I execute dnf with args "install flac --setopt=<prefix>excludepkgs=flac --setopt=disable_excludes=<disable-conf>"
   Then the exit code is 0
    And Transaction is following
        | Action        | Package                          |
        | install       | flac-0:1.3.2-8.fc29.x86_64       |

Examples:
  | conf   | prefix         | disable-conf   |
  | main   |                | main           |
  | main   |                | *              |
  | repo   | dnf-ci-fedora. | dnf-ci-fedora  |
  | repo   | dnf-ci-fedora. | *              |


Scenario: Fail to install an RPM that is NOT in includepkgs in main conf, with option disable_excludes equal to <repo-id>
  Given I use repository "dnf-ci-fedora"
   When I execute dnf with args "install flac --setopt=includepkgs=setup --setopt=disable_excludes=dnf-ci-fedora"
   Then the exit code is 1
    And Transaction is empty


Scenario: Fail to install an RPM that is in excludepkgs in main conf, with option disable_excludes equal to <repo-id>
  Given I use repository "dnf-ci-fedora"
   When I execute dnf with args "install flac --setopt=excludepkgs=flac --setopt=disable_excludes=dnf-ci-fedora"
   Then the exit code is 1
    And Transaction is empty


Scenario: Fail to install an RPM that is NOT in includepkgs in repo conf, with option disable_excludes equal to main
  Given I use repository "dnf-ci-fedora"
   When I execute dnf with args "install flac --setopt=dnf-ci-fedora.includepkgs=setup --setopt=disable_excludes=main"
   Then the exit code is 1
    And Transaction is empty


Scenario: Fail to install an RPM that is in excludepkgs in repo conf, with option disable_excludes equal to main
  Given I use repository "dnf-ci-fedora"
   When I execute dnf with args "install flac --setopt=dnf-ci-fedora.excludepkgs=flac --setopt=disable_excludes=main"
   Then the exit code is 1
    And Transaction is empty
