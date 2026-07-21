Feature: downgrade with --from-repo


Background: Install some RPMs from one repository
  Given I use repository "from-repo"
   When I execute dnf with args "install abcde"
   Then the exit code is 0
    And Transaction is following
        | Action        | Package                               |
        | install       | abcde-0:2.9.2-1.fc29.noarch           |
        | install-dep   | wget-0:1.19.5-5.fc29.x86_64           |
        | install-weak  | flac-0:1.3.2-8.fc29.x86_64            |
  Given I use repository "from-repo"
    And I use repository "from-repo-updates2"
   When I execute dnf with args "install pkgB"
   Then the exit code is 0
    And Transaction is following
        | Action        | Package                               |
        | install       | pkgB-0:1.2-1.fc29.x86_64              |
        | install-dep   | dependency1-0:1.2-1.fc29.x86_64       |
        | install-dep   | dependency3-0:1.0-1.fc29.x86_64       |


Scenario: Downgrade pkgB from "from-repo-updates1" and install new dependency from another repo
  Given I use repository "from-repo"
    And I use repository "from-repo-updates1"
    And I use repository "from-repo-updates2"
   When I execute dnf with args "downgrade --from-repo=from-repo-updates1 pkgB"
   Then the exit code is 0
    And Transaction is following
        | Action        | Package                                   |
        | downgrade     | pkgB-0:1.1-1.fc29.x86_64                  |
        | install-dep   | dependency2-0:1.0-1.fc29.x86_64           |


Scenario: Downgrade all RPMs "from-repo-updates1" and install new dependency from another repo
  Given I use repository "from-repo"
    And I use repository "from-repo-updates1"
    And I use repository "from-repo-updates2"
   When I execute dnf with args "downgrade --from-repo=from-repo-updates1 '*'"
   Then the exit code is 0
    And Transaction is following
        | Action        | Package                                   |
        | downgrade     | pkgB-0:1.1-1.fc29.x86_64                  |
        | install-dep   | dependency2-0:1.0-1.fc29.x86_64           |
