Feature: reinstall with --installed-from-repo


Background: Installing packages from multiple repositories
  Given I use repository "from-repo"
    And I use repository "from-repo-updates1"
    And I use repository "from-repo-updates2"
   When I execute dnf with args "install abcde basesystem lame lz4 pkgB"
   Then the exit code is 0
    And Transaction is following
        | Action        | Package                               |
        | install       | abcde-0:2.9.3-1.fc29.noarch           |
        | install       | basesystem-0:12-2.fc29.noarch         |
        | install       | lame-0:3.100-5.fc29.x86_64            |
        | install       | lz4-0:1.7.5-2.fc26.x86_64             |
        | install       | pkgB-0:1.2-1.fc29.x86_64              |
        | install-dep   | dependency1-0:1.2-1.fc29.x86_64       |
        | install-dep   | dependency3-0:1.0-1.fc29.x86_64       |
        | install-dep   | filesystem-0:3.10-1.fc29.x86_64       |
        | install-dep   | lame-libs-0:3.100-5.fc29.x86_64       |
        | install-dep   | setup-0:2.14.2-1.fc29.noarch          |
        | install-dep   | wget-0:1.19.8-2.fc29.x86_64           |
        | install-weak  | flac-0:1.3.3-3.fc29.x86_64            |


Scenario: Reinstall all packages installed from repo "from-repo-updates1"
   When I execute dnf with args "reinstall --installed-from-repo=from-repo-updates1 '*'"
   Then the exit code is 0
    And Transaction is following
        | Action        | Package                               |
        | reinstall     | abcde-0:2.9.3-1.fc29.noarch           |
        | reinstall     | filesystem-0:3.10-1.fc29.x86_64       |


Scenario: Reinstall specified packages installed from repo "from-repo"
   When I execute dnf with args "reinstall --installed-from-repo=from-repo --skip-unavailable lz4 abcde 'l*'"
   Then the exit code is 0
    And Transaction is following
        | Action        | Package                               |
        | reinstall     | lz4-0:1.7.5-2.fc26.x86_64             |


Scenario: Reinstall specified packages installed from list of repositories
   When I execute dnf with args "reinstall --installed-from-repo=from-repo,from-repo-updates1 lz4 abcde 'l*'"
   Then the exit code is 0
    And Transaction is following
        | Action        | Package                               |
        | reinstall     | abcde-0:2.9.3-1.fc29.noarch           |
        | reinstall     | lz4-0:1.7.5-2.fc26.x86_64             |


Scenario: Reinstall specified packages installed from repositories definew with wildcards
   When I execute dnf with args "reinstall --installed-from-repo=from-*-updates? --skip-unavailable lz4 abcde 'l*'"
   Then the exit code is 0
    And Transaction is following
        | Action        | Package                               |
        | reinstall     | abcde-0:2.9.3-1.fc29.noarch           |
        | reinstall     | lame-0:3.100-5.fc29.x86_64            |
        | reinstall     | lame-libs-0:3.100-5.fc29.x86_64       |
