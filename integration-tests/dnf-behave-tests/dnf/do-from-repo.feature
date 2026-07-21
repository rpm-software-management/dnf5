Feature: "do" command with "--from-repo"


Background: Install some RPMs from repositories
  Given I use repository "from-repo"
   When I execute dnf with args "do --action=install abcde pkgB"
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
   When I execute dnf with args "do --action=install basesystem"
   Then the exit code is 0
    And Transaction is following
        | Action        | Package                               |
        | install       | basesystem-0:12-2.fc29.noarch         |
        | install-dep   | filesystem-0:3.10-1.fc29.x86_64       |
        | install-dep   | setup-0:2.14.2-1.fc29.noarch          |


Scenario: Upgrade pkgB, use package from "from-repo-updates1" and install new dependency from another repo
   When I execute dnf with args "do --action=upgrade --from-repo=from-repo-updates1 pkgB"
   Then the exit code is 0
    And Transaction is following
        | Action        | Package                                   |
        | upgrade       | pkgB-0:1.1-1.fc29.x86_64                  |
        | install-dep   | dependency2-0:1.0-1.fc29.x86_64           |


Scenario: Upgrade packages, use packages from "from-repo-updates1" and install new dependency from another repo
   When I execute dnf with args "do --action=upgrade --from-repo=from-repo-updates1 '*'"
   Then the exit code is 0
    And Transaction is following
        | Action        | Package                                   |
        | upgrade       | abcde-0:2.9.3-1.fc29.noarch               |
        | upgrade       | flac-0:1.3.3-1.fc29.x86_64                |
        | upgrade       | pkgB-0:1.1-1.fc29.x86_64                  |
        | upgrade       | wget-0:1.19.6-5.fc29.x86_64               |
        | install-dep   | dependency2-0:1.0-1.fc29.x86_64           |


Scenario: Downgrade basesystem, use package from "from-repo"
   When I execute dnf with args "do --action=downgrade --from-repo=from-repo basesystem"
   Then the exit code is 0
    And Transaction is following
        | Action        | Package                                   |
        | downgrade     | basesystem-0:11-6.fc29.noarch             |


Scenario: Downgrade packages, use packages from "from-repo-updates1"
   When I execute dnf with args "do --action=downgrade --from-repo=from-repo-updates1 '*'"
   Then the exit code is 0
    And Transaction is following
        | Action        | Package                                   |
        | downgrade     | basesystem-0:11-8.fc29.noarch             |


Scenario: Reinstall setup, use package from "frem-repo-updates2"
   When I execute dnf with args "do --action=reinstall --from-repo=from-repo-updates2 setup"
   Then the exit code is 0
    And Transaction is following
        | Action        | Package                               |
        | reinstall     | setup-0:2.14.2-1.fc29.noarch          |


Scenario: Test reinstall with two repos in "--from-repo="
   When I execute dnf with args "do --action=reinstall --from-repo=from-repo,from-repo-updates2 '*'"
   Then the exit code is 0
    And Transaction is following
        | Action        | Package                               |
        | reinstall     | abcde-0:2.9.2-1.fc29.noarch           |
        | reinstall     | basesystem-0:12-2.fc29.noarch         |
        | reinstall     | dependency1-0:1.0-1.fc29.x86_64       |
        | reinstall     | flac-0:1.3.2-8.fc29.x86_64            |
        | reinstall     | pkgB-0:1.0-1.fc29.x86_64              |
        | reinstall     | setup-0:2.14.2-1.fc29.noarch          |
        | reinstall     | wget-0:1.19.5-5.fc29.x86_64           |


Scenario: Test reinstall with glob pattern in "--from-repo="
   When I execute dnf with args "do --action=reinstall --from-repo=from-*-updates? '*'"
   Then the exit code is 0
    And Transaction is following
        | Action        | Package                               |
        | reinstall     | basesystem-0:12-2.fc29.noarch         |
        | reinstall     | filesystem-0:3.10-1.fc29.x86_64       |
        | reinstall     | setup-0:2.14.2-1.fc29.noarch          |


Scenario: Test reinstall,upgrade,install; one transaction; first action different "--from-repo", package lame not installed - not in requested repo
 Test reinstall with "--from-repo=from-repo-updates2"; and upgrade, install with another "--from-repo=from-repo-updates1"
   When I execute dnf with args "do --skip-unavailable --action=reinstall --from-repo=from-repo-updates2 '*' --action=upgrade --from-repo=from-repo-updates1 '*' --action=install lame"
   Then the exit code is 0
    And Transaction is following
        | Action        | Package                               |
        | upgrade       | abcde-0:2.9.3-1.fc29.noarch           |
        | upgrade       | flac-0:1.3.3-1.fc29.x86_64            |
        | upgrade       | pkgB-0:1.1-1.fc29.x86_64              |
        | upgrade       | wget-0:1.19.6-5.fc29.x86_64           |
        | install-dep   | dependency2-0:1.0-1.fc29.x86_64       |
        | reinstall     | basesystem-0:12-2.fc29.noarch         |
        | reinstall     | setup-0:2.14.2-1.fc29.noarch          |


Scenario: Test reinstall,upgrade,install; one transaction; each action different "--from-repo"
   When I execute dnf with args "do --action=reinstall --from-repo=from-repo-updates2 '*' --action=upgrade --from-repo=from-repo-updates1 '*' --action=install --from-repo=from-repo lame"
   Then the exit code is 0
    And Transaction is following
        | Action        | Package                               |
        | install       | lame-3.100-4.fc29.x86_64              |
        | upgrade       | abcde-0:2.9.3-1.fc29.noarch           |
        | upgrade       | flac-0:1.3.3-1.fc29.x86_64            |
        | upgrade       | pkgB-0:1.1-1.fc29.x86_64              |
        | upgrade       | wget-0:1.19.6-5.fc29.x86_64           |
        | install-dep   | dependency2-0:1.0-1.fc29.x86_64       |
        | install-dep   | lame-libs-3.100-4.fc29.x86_64         |
        | reinstall     | basesystem-0:12-2.fc29.noarch         |
        | reinstall     | setup-0:2.14.2-1.fc29.noarch          |
