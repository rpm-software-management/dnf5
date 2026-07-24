Feature: downgrade with --install-from-repo


Background: Install some RPMs from one repository
  Given I use repository "from-repo"
    And I use repository "from-repo-updates1"
   When I execute dnf with args "install basesystem"
   Then the exit code is 0
    And Transaction is following
        | Action        | Package                               |
        | install       | basesystem-0:11-8.fc29.noarch         |
        | install-dep   | filesystem-0:3.10-1.fc29.x86_64       |
        | install-dep   | setup-0:2.12.1-1.fc29.noarch          |
  Given I use repository "from-repo-updates2"
   When I execute dnf with args "install abcde pkgB"
   Then the exit code is 0
    And Transaction is following
        | Action        | Package                               |
        | install       | abcde-0:2.9.3-1.fc29.noarch           |
        | install       | pkgB-0:1.2-1.fc29.x86_64              |
        | install-dep   | dependency1-0:1.2-1.fc29.x86_64       |
        | install-dep   | dependency3-0:1.0-1.fc29.x86_64       |
        | install-dep   | wget-0:1.19.8-2.fc29.x86_64           |
        | install-weak  | flac-0:1.3.3-3.fc29.x86_64            |


Scenario: Downgrade all packages installed from "from-repo-updates2" and install new dependency
   When I execute dnf with args "downgrade --installed-from-repo=from-repo-updates2 '*'"
   Then the exit code is 0
    And Transaction is following
        | Action        | Package                                   |
        | downgrade     | flac-0:1.3.3-2.fc29.x86_64                |
        | downgrade     | dependency1-0:1.0-1.fc29.x86_64           |
        | downgrade     | pkgB-0:1.1-1.fc29.x86_64                  |
        | downgrade     | wget-0:1.19.6-5.fc29.x86_64               |
        | install-dep   | dependency2-0:1.0-1.fc29.x86_64           |


Scenario: Downgrade all packages installed from "from-repo-updates1"
   When I execute dnf with args "downgrade --installed-from-repo=from-repo-updates1 '*'"
   Then the exit code is 0
    And Transaction is following
        | Action        | Package                                   |
        | downgrade     | abcde-0:2.9.2-1.fc29.noarch               |
        | downgrade     | basesystem-0:11-6.fc29.noarch             |
        | downgrade     | filesystem-0:3.9-2.fc29.x86_64            |


Scenario: Downgrade all packages installed from "from-repo-updates1" and "from-repo-updates2" and install new dependency
   When I execute dnf with args "downgrade --installed-from-repo=from-repo-updates1,from-repo-updates2 '*'"
   Then the exit code is 0
    And Transaction is following
        | Action        | Package                                   |
        | downgrade     | abcde-0:2.9.2-1.fc29.noarch               |
        | downgrade     | basesystem-0:11-6.fc29.noarch             |
        | downgrade     | filesystem-0:3.9-2.fc29.x86_64            |
        | downgrade     | flac-0:1.3.3-2.fc29.x86_64                |
        | downgrade     | dependency1-0:1.0-1.fc29.x86_64           |
        | downgrade     | pkgB-0:1.1-1.fc29.x86_64                  |
        | downgrade     | wget-0:1.19.6-5.fc29.x86_64               |
        | install-dep   | dependency2-0:1.0-1.fc29.x86_64           |


Scenario: Downgrade all packages installed from repos defined by glob pattern and install new dependency
   When I execute dnf with args "downgrade --installed-from-repo=from-*-updates? '*'"
   Then the exit code is 0
    And Transaction is following
        | Action        | Package                                   |
        | downgrade     | abcde-0:2.9.2-1.fc29.noarch               |
        | downgrade     | basesystem-0:11-6.fc29.noarch             |
        | downgrade     | filesystem-0:3.9-2.fc29.x86_64            |
        | downgrade     | flac-0:1.3.3-2.fc29.x86_64                |
        | downgrade     | dependency1-0:1.0-1.fc29.x86_64           |
        | downgrade     | pkgB-0:1.1-1.fc29.x86_64                  |
        | downgrade     | wget-0:1.19.6-5.fc29.x86_64               |
        | install-dep   | dependency2-0:1.0-1.fc29.x86_64           |


Scenario: Downgrade pkg* packages installed from repos defined by glob pattern and install new dependency
   When I execute dnf with args "downgrade --installed-from-repo=from-*-updates? 'pkg*'"
   Then the exit code is 0
    And Transaction is following
        | Action        | Package                                   |
        | downgrade     | pkgB-0:1.1-1.fc29.x86_64                  |
        | install-dep   | dependency2-0:1.0-1.fc29.x86_64           |


Scenario: Downgrade all packages installed from from-repo-updates2, to pkgs from from-repo-updates1, but dependency from another repo
   When I execute dnf with args "downgrade --installed-from-repo=from-repo-updates2 --from-repo=from-repo-updates1 '*'"
   Then the exit code is 0
    And Transaction is following
        | Action        | Package                                   |
        | downgrade     | flac-0:1.3.3-1.fc29.x86_64                |
        | downgrade     | pkgB-0:1.1-1.fc29.x86_64                  |
        | downgrade     | wget-0:1.19.6-5.fc29.x86_64               |
        | install-dep   | dependency2-0:1.0-1.fc29.x86_64           |
