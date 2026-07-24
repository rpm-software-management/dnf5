Feature: distro-sync with --install-from-repo and --from-repo


Background: Install some RPMs from one repository
  Given I use repository "from-repo"
   When I execute dnf with args "install abcde pkgB"
   Then the exit code is 0
    And Transaction is following
        | Action        | Package                               |
        | install       | abcde-0:2.9.2-1.fc29.noarch           |
        | install       | pkgB-0:1.0-1.fc29.x86_64              |
        | install-dep   | dependency1-0:1.0-1.fc29.x86_64       |
        | install-dep   | wget-0:1.19.5-5.fc29.x86_64           |
        | install-weak  | flac-0:1.3.2-8.fc29.x86_64            |
  Given I use repository "from-repo-updates1"
    And I use repository "from-repo-updates2"
   When I execute dnf with args "install basesystem"
   Then the exit code is 0
    And Transaction is following
        | Action        | Package                               |
        | install       | basesystem-0:12-2.fc29.noarch         |
        | install-dep   | filesystem-0:3.10-1.fc29.x86_64       |
        | install-dep   | setup-0:2.14.2-1.fc29.noarch          |


Scenario: distro-sync all packages installed from "from-repo" and install new dependency
   When I execute dnf with args "distro-sync --installed-from-repo=from-repo"
   Then the exit code is 0
    And Transaction is following
        | Action        | Package                                   |
        | upgrade       | abcde-0:2.9.3-1.fc29.noarch               |
        | upgrade       | dependency1-0:1.2-1.fc29.x86_64           |
        | upgrade       | flac-0:1.3.3-3.fc29.x86_64                |
        | upgrade       | pkgB-0:1.2-1.fc29.x86_64                  |
        | upgrade       | wget-0:1.19.8-2.fc29.x86_64               |
        | install-dep   | dependency3-0:1.0-1.fc29.x86_64           |


Scenario: distro-sync all packages (spec by '*') installed from "from-repo" and install new dependency
   When I execute dnf with args "distro-sync --installed-from-repo=from-repo '*'"
   Then the exit code is 0
    And Transaction is following
        | Action        | Package                                   |
        | upgrade       | abcde-0:2.9.3-1.fc29.noarch               |
        | upgrade       | dependency1-0:1.2-1.fc29.x86_64           |
        | upgrade       | flac-0:1.3.3-3.fc29.x86_64                |
        | upgrade       | pkgB-0:1.2-1.fc29.x86_64                  |
        | upgrade       | wget-0:1.19.8-2.fc29.x86_64               |
        | install-dep   | dependency3-0:1.0-1.fc29.x86_64           |


Scenario: distro-sync all packages installed from "from-repo" and install to "from-repo-updates1"; install new dependency from another repo
   When I execute dnf with args "distro-sync --installed-from-repo=from-repo  --from-repo=from-repo-updates1 '*'"
   Then the exit code is 0
    And Transaction is following
        | Action        | Package                                   |
        | upgrade       | abcde-0:2.9.3-1.fc29.noarch               |
        | upgrade       | flac-0:1.3.3-1.fc29.x86_64                |
        | upgrade       | pkgB-0:1.1-1.fc29.x86_64                  |
        | upgrade       | wget-0:1.19.6-5.fc29.x86_64               |
        | install-dep   | dependency2-0:1.0-1.fc29.x86_64           |


Scenario: distro-sync all packages installed from "from-repo-updates2" to "from-repo-updates1"
   When I execute dnf with args "distro-sync --installed-from-repo=from-repo-updates2 --from-repo=from-repo-updates1 '*'"
   Then the exit code is 0
    And Transaction is following
        | Action        | Package                                   |
        | downgrade     | basesystem-0:11-8.fc29.noarch             |


Scenario: distro-sync all packages installed from "from-repo" and "from-repo-updates2" to "from-repo-updates1"; install dependency from another repo
   When I execute dnf with args "distro-sync --installed-from-repo=from-repo,from-repo-updates2  --from-repo=from-repo-updates1 '*'"
   Then the exit code is 0
    And Transaction is following
        | Action        | Package                                   |
        | upgrade       | abcde-0:2.9.3-1.fc29.noarch               |
        | upgrade       | flac-0:1.3.3-1.fc29.x86_64                |
        | upgrade       | pkgB-0:1.1-1.fc29.x86_64                  |
        | upgrade       | wget-0:1.19.6-5.fc29.x86_64               |
        | downgrade     | basesystem-0:11-8.fc29.noarch             |
        | install-dep   | dependency2-0:1.0-1.fc29.x86_64           |


Scenario: distro-sync all pkgs installed from "from-repo" and "from-repo-updates2" (globs in def) to "from-repo-updates1"; install dependency from another repo
   When I execute dnf with args "distro-sync --installed-from-repo=from-repo,from-repo-updates2  --from-repo=from-repo-updates1 '*'"
   Then the exit code is 0
    And Transaction is following
        | Action        | Package                                   |
        | upgrade       | abcde-0:2.9.3-1.fc29.noarch               |
        | upgrade       | flac-0:1.3.3-1.fc29.x86_64                |
        | upgrade       | pkgB-0:1.1-1.fc29.x86_64                  |
        | upgrade       | wget-0:1.19.6-5.fc29.x86_64               |
        | downgrade     | basesystem-0:11-8.fc29.noarch             |
        | install-dep   | dependency2-0:1.0-1.fc29.x86_64           |


Scenario: distrosync packages installed from "obsoletes-yum"; obsoleters are installed
  Given I use repository "obsoletes-yum"
   When I execute dnf with args "install wood-1.0"
   Then the exit code is 0
    And Transaction is following
        | Action        | Package                                   |
        | install       | wood-0:1.0-1.fc29.x86_64                  |
   When I execute dnf with args "distro-sync --installed-from-repo=obsoletes-yum"
   Then the exit code is 0
    And Transaction is following
        | Action        | Package                                   |
        | install       | copper-0:1.0-1.fc29.x86_64                |
        | install       | iron-0:1.0-1.fc29.x86_64                  |
        | upgrade       | wood-0:2.0-1.fc29.x86_64                  |


Scenario: distrosync packages (spec '*') installed from "obsoletes-yum"; obsoleters are installed
  Given I use repository "obsoletes-yum"
   When I execute dnf with args "install wood-1.0"
   Then the exit code is 0
    And Transaction is following
        | Action        | Package                                   |
        | install       | wood-0:1.0-1.fc29.x86_64                  |
   When I execute dnf with args "distro-sync --installed-from-repo=obsoletes-yum '*'"
   Then the exit code is 0
    And Transaction is following
        | Action        | Package                                   |
        | install       | copper-0:1.0-1.fc29.x86_64                |
        | install       | iron-0:1.0-1.fc29.x86_64                  |
        | upgrade       | wood-0:2.0-1.fc29.x86_64                  |
