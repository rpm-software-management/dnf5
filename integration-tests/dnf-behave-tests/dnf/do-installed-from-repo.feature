Feature: "do" command with "--installed-from-repo" and "--from-repo"


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


Scenario: Upgrade pkgB installed from "from-repo" and install new dependency
   When I execute dnf with args "do --action=upgrade --installed-from-repo=from-repo pkgB"
   Then the exit code is 0
    And Transaction is following
        | Action        | Package                                   |
        | upgrade       | pkgB-0:1.2-1.fc29.x86_64                  |
        | install-dep   | dependency3-0:1.0-1.fc29.x86_64           |


Scenario: Upgrade packages defined by 'p*' installed from "from-repo" and install new dependency
   When I execute dnf with args "do --action=upgrade --installed-from-repo=from-repo 'p*'"
   Then the exit code is 0
    And Transaction is following
        | Action        | Package                                   |
        | upgrade       | pkgB-0:1.2-1.fc29.x86_64                  |
        | install-dep   | dependency3-0:1.0-1.fc29.x86_64           |


Scenario: Upgrade packages defined by 'p*' fails, none is installed from "from-repo-updates1"
   When I execute dnf with args "do --action=upgrade --installed-from-repo=from-repo-updates1 'p*'"
   Then the exit code is 1
    And stderr is
    """
    <REPOSYNC>
    Failed to resolve the transaction:
    Packages for argument 'p*' available, but not installed.
    """


Scenario: Upgrade packages installed from "from-repo"
   When I execute dnf with args "do --action=upgrade --installed-from-repo=from-repo '*'"
   Then the exit code is 0
    And Transaction is following
        | Action        | Package                                   |
        | upgrade       | abcde-0:2.9.3-1.fc29.noarch               |
        | upgrade       | dependency1-0:1.2-1.fc29.x86_64           |
        | upgrade       | flac-0:1.3.3-3.fc29.x86_64                |
        | upgrade       | pkgB-0:1.2-1.fc29.x86_64                  |
        | upgrade       | wget-0:1.19.8-2.fc29.x86_64               |
        | install-dep   | dependency3-0:1.0-1.fc29.x86_64           |


Scenario: Upgrade packages installed from "obsoletes-yum"; obsoleters are installed
  Given I use repository "obsoletes-yum"
   When I execute dnf with args "do --action=install wood-1.0"
   Then the exit code is 0
    And Transaction is following
        | Action        | Package                                   |
        | install       | wood-0:1.0-1.fc29.x86_64                  |
   When I execute dnf with args "do --action=upgrade --installed-from-repo=obsoletes-yum '*'"
   Then the exit code is 0
    And Transaction is following
        | Action        | Package                                   |
        | install       | copper-0:1.0-1.fc29.x86_64                |
        | install       | iron-0:1.0-1.fc29.x86_64                  |
        | upgrade       | wood-0:2.0-1.fc29.x86_64                  |


Scenario: Uprade packages installed from "from-repo", use packages from "from-repo-updates1"
   When I execute dnf with args "do --action=upgrade --installed-from-repo=from-repo --from-repo=from-repo-updates1 '*'"
   Then the exit code is 0
    And Transaction is following
        | Action        | Package                                   |
        | upgrade       | abcde-0:2.9.3-1.fc29.noarch               |
        | upgrade       | flac-0:1.3.3-1.fc29.x86_64                |
        | upgrade       | pkgB-0:1.1-1.fc29.x86_64                  |
        | upgrade       | wget-0:1.19.6-5.fc29.x86_64               |
        | install-dep   | dependency2-0:1.0-1.fc29.x86_64           |


Scenario: Downgrade packages installed from repo "from-repo-updates1"
   When I execute dnf with args "do --action=downgrade --installed-from-repo=from-repo-updates1 '*'"
   Then the exit code is 0
    And Transaction is following
        | Action        | Package                                   |
        | downgrade     | filesystem-0:3.9-2.fc29.x86_64            |


Scenario: Reinstall package setup instaled from repo "from-repo-updates2"
   When I execute dnf with args "do --action=reinstall --installed-from-repo=from-repo-updates2 setup"
   Then the exit code is 0
    And Transaction is following
        | Action        | Package                               |
        | reinstall     | setup-0:2.14.2-1.fc29.noarch          |


Scenario: Reinstall packages installed from defined list of repositories
   When I execute dnf with args "do --action=reinstall --installed-from-repo=from-repo,from-repo-updates2 '*'"
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


Scenario: Reinstall packages from repositories with globs in definition
   When I execute dnf with args "do --action=reinstall --installed-from-repo=from-*-updates? '*'"
   Then the exit code is 0
    And Transaction is following
        | Action        | Package                               |
        | reinstall     | basesystem-0:12-2.fc29.noarch         |
        | reinstall     | filesystem-0:3.10-1.fc29.x86_64       |
        | reinstall     | setup-0:2.14.2-1.fc29.noarch          |


Scenario: Remove packages installed from "from-repo-updates1"
   When I execute dnf with args "do --allowerasing --action=remove --installed-from-repo=from-repo-updates1 '*'"
   Then the exit code is 0
    And Transaction is following
        | Action        | Package                               |
        | remove        | filesystem-0:3.10-1.fc29.x86_64       |
        | remove-dep    | basesystem-0:12-2.fc29.noarch         |
        | remove-unused | setup-0:2.14.2-1.fc29.noarch          |


Scenario: Test reinstall,upgrade,install; one transaction; first action different "--installed-from-repo"; no upgrades for packages installed from "from-repo-updates1"
   When I execute dnf with args "do --action=reinstall --installed-from-repo=from-repo-updates2 '*' --action=upgrade --installed-from-repo=from-repo-updates1 '*' --action=install lame"
   Then the exit code is 0
    And Transaction is following
        | Action        | Package                               |
        | install       | lame-3.100-5.fc29.x86_64              |
        | install-dep   | lame-libs-3.100-5.fc29.x86_64         |
        | reinstall     | basesystem-0:12-2.fc29.noarch         |
        | reinstall     | setup-0:2.14.2-1.fc29.noarch          |


Scenario: Test reinstall,upgrade,install; one transaction; first action different "--installed-from-repo"
   When I execute dnf with args "do --action=reinstall --installed-from-repo=from-repo-updates2 '*' --action=upgrade --installed-from-repo=from-repo '*' --action=install lame"
   Then the exit code is 0
    And Transaction is following
        | Action        | Package                               |
        | install       | lame-3.100-5.fc29.x86_64              |
        | upgrade       | abcde-0:2.9.3-1.fc29.noarch           |
        | upgrade       | dependency1-0:1.2-1.fc29.x86_64       |
        | upgrade       | flac-0:1.3.3-3.fc29.x86_64            |
        | upgrade       | pkgB-0:1.2-1.fc29.x86_64              |
        | upgrade       | wget-0:1.19.8-2.fc29.x86_64           |
        | install-dep   | dependency3-0:1.0-1.fc29.x86_64       |
        | install-dep   | lame-libs-3.100-5.fc29.x86_64         |
        | reinstall     | basesystem-0:12-2.fc29.noarch         |
        | reinstall     | setup-0:2.14.2-1.fc29.noarch          |


Scenario: Test reinstall,upgrade,install; one transaction; transaction fails; package lame was not installed from "--from-repo-updates1"
   When I execute dnf with args "do --action=reinstall --installed-from-repo=from-repo-updates2 '*' --action=upgrade --installed-from-repo=from-repo --from-repo=from-repo-updates1 '*' --action=install lame"
   Then the exit code is 1
    And stderr is
    """
    <REPOSYNC>
    Failed to resolve the transaction:
    No match for argument 'lame' in repositories 'from-repo-updates1'
    """


Scenario: Test reinstall,upgrade,install; one transaction; package lame skipped - was not installed from "--from-repo-updates1"
   When I execute dnf with args "do --skip-unavailable --action=reinstall --installed-from-repo=from-repo-updates2 '*' --action=upgrade --installed-from-repo=from-repo --from-repo=from-repo-updates1 '*' --action=install lame"
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
