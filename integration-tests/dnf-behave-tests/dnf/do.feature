Feature: "do" command


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


Scenario: Test action upgrade specified package
   When I execute dnf with args "do --action=upgrade pkgB"
   Then the exit code is 0
    And Transaction is following
        | Action        | Package                                   |
        | upgrade       | pkgB-0:1.2-1.fc29.x86_64                  |
        | install-dep   | dependency3-0:1.0-1.fc29.x86_64           |


Scenario: Test action upgrade all packages
   When I execute dnf with args "do --action=upgrade '*'"
   Then the exit code is 0
    And Transaction is following
        | Action        | Package                                   |
        | upgrade       | abcde-0:2.9.3-1.fc29.noarch               |
        | upgrade       | dependency1-0:1.2-1.fc29.x86_64           |
        | upgrade       | flac-0:1.3.3-3.fc29.x86_64                |
        | upgrade       | pkgB-0:1.2-1.fc29.x86_64                  |
        | upgrade       | wget-0:1.19.8-2.fc29.x86_64               |
        | install-dep   | dependency3-0:1.0-1.fc29.x86_64           |


Scenario: Test action downgrade specified package
   When I execute dnf with args "do --action=downgrade filesystem"
   Then the exit code is 0
    And Transaction is following
        | Action        | Package                                   |
        | downgrade     | filesystem-0:3.9-2.fc29.x86_64            |


Scenario: Test action downgrade all packages
   When I execute dnf with args "do --action=downgrade '*'"
   Then the exit code is 0
    And Transaction is following
        | Action        | Package                                   |
        | downgrade     | basesystem-0:11-8.fc29.noarch             |
        | downgrade     | filesystem-0:3.9-2.fc29.x86_64            |
        | downgrade     | setup-0:2.12.1-1.fc29.noarch              |


Scenario: Test action remove specified package
   When I execute dnf with args "do --action=remove abcde"
   Then the exit code is 0
    And Transaction is following
        | Action        | Package                               |
        | remove        | abcde-0:2.9.2-1.fc29.noarch           |
        | remove-unused | flac-0:1.3.2-8.fc29.x86_64            |
        | remove-unused | wget-0:1.19.5-5.fc29.x86_64           |


Scenario: Test action remove; glob pattern in specified packages
   When I execute dnf with args "do --allowerasing --action=remove 'f*'"
   Then the exit code is 0
    And Transaction is following
        | Action        | Package                               |
        | remove        | filesystem-0:3.10-1.fc29.x86_64       |
        | remove        | flac-0:1.3.2-8.fc29.x86_64            |
        | remove-dep    | basesystem-0:12-2.fc29.noarch         |
        | remove-unused | setup-0:2.14.2-1.fc29.noarch          |


Scenario: Test action reinstall specified packages
   When I execute dnf with args "do --action=reinstall abcde filesystem flac"
   Then the exit code is 0
    And Transaction is following
        | Action        | Package                               |
        | reinstall     | abcde-0:2.9.2-1.fc29.noarch           |
        | reinstall     | filesystem-0:3.10-1.fc29.x86_64       |
        | reinstall     | flac-0:1.3.2-8.fc29.x86_64            |


Scenario: Test action reinstall; glob pattern in specified packages
   When I execute dnf with args "do --action=reinstall abcde 'f*'"
   Then the exit code is 0
    And Transaction is following
        | Action        | Package                               |
        | reinstall     | abcde-0:2.9.2-1.fc29.noarch           |
        | reinstall     | filesystem-0:3.10-1.fc29.x86_64       |
        | reinstall     | flac-0:1.3.2-8.fc29.x86_64            |


Scenario: Test actions upgrade/downgrade/remove/install/reinstall together in one command (one transaction)
  Given I use repository "from-repo"
   When I execute dnf with args "do --action=upgrade abcde pkgB --action=downgrade filesystem --action=remove flac --action=install lame --action=reinstall wget"
   Then the exit code is 0
    And Transaction is following
        | Action        | Package                               |
        | install       | lame-3.100-5.fc29.x86_64              |
        | install-dep   | dependency3-0:1.0-1.fc29.x86_64       |
        | install-dep   | lame-libs-3.100-5.fc29.x86_64         |
        | reinstall     | wget-0:1.19.5-5.fc29.x86_64           |
        | upgrade       | abcde-0:2.9.3-1.fc29.noarch           |
        | upgrade       | pkgB-0:1.2-1.fc29.x86_64              |
        | downgrade     | filesystem-0:3.9-2.fc29.x86_64        |
        | remove        | flac-0:1.3.2-8.fc29.x86_64            |
