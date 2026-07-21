Feature: list with --installed-from-repo


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


Scenario: List all packages installed from repo "from-repo"
   When I execute dnf with args "list --installed-from-repo=from-repo"
   Then the exit code is 0
    And stderr is
      """
      <REPOSYNC>
      """
    And stdout matches line by line
      """
      Installed packages
      dependency3.x86_64 +1.0-1.fc29 +from-repo
      lz4.x86_64 +1.7.5-2.fc26 +from-repo
      """


Scenario: List all packages installed from repo "from-repo-updates1"
   When I execute dnf with args "list --installed-from-repo=from-repo-updates1"
   Then the exit code is 0
    And stderr is
      """
      <REPOSYNC>
      """
    And stdout matches line by line
      """
      Installed packages
      abcde.noarch +2.9.3-1.fc29 +from-repo-updates1
      filesystem.x86_64 +3.10-1.fc29 +from-repo-updates1
      """


Scenario: List all packages installed from repo "from-repo-updates2" using '*' package specification
   When I execute dnf with args "list --installed-from-repo=from-repo-updates2 '*'"
   Then the exit code is 0
    And stderr is
      """
      <REPOSYNC>
      """
    And stdout matches line by line
      """
      Installed packages
      basesystem.noarch +12-2.fc29 +from-repo-updates2
      dependency1.x86_64 +1.2-1.fc29 +from-repo-updates2
      flac.x86_64 +1.3.3-3.fc29 +from-repo-updates2
      lame.x86_64 +3.100-5.fc29 +from-repo-updates2
      lame-libs.x86_64 +3.100-5.fc29 +from-repo-updates2
      pkgB.x86_64 +1.2-1.fc29 +from-repo-updates2
      setup.noarch +2.14.2-1.fc29 +from-repo-updates2
      wget.x86_64 +1.19.8-2.fc29 +from-repo-updates2
      """


Scenario: List packages installed from repo "from-repo-updates2" using 'l*' package specification
   When I execute dnf with args "list --installed-from-repo=from-repo-updates2 'l*'"
   Then the exit code is 0
    And stderr is
      """
      <REPOSYNC>
      """
    And stdout matches line by line
      """
      Installed packages
      lame.x86_64 +3.100-5.fc29 +from-repo-updates2
      lame-libs.x86_64 +3.100-5.fc29 +from-repo-updates2
      """


Scenario: Test case with 2 repositories defined in installed from repo
   When I execute dnf with args "list --installed-from-repo=from-repo,from-repo-updates1"
   Then the exit code is 0
    And stderr is
      """
      <REPOSYNC>
      """
    And stdout matches line by line
      """
      Installed packages
      abcde.noarch +2.9.3-1.fc29 +from-repo-updates1
      dependency3.x86_64 +1.0-1.fc29 +from-repo
      filesystem.x86_64 +3.10-1.fc29 +from-repo-updates1
      lz4.x86_64 +1.7.5-2.fc26 +from-repo
      """


Scenario: Test case with wildcards in installed from repo definition
   When I execute dnf with args "list --installed-from-repo=from-*-updates?"
   Then the exit code is 0
    And stderr is
      """
      <REPOSYNC>
      """
    And stdout matches line by line
      """
      Installed packages
      abcde.noarch +2.9.3-1.fc29 +from-repo-updates1
      basesystem.noarch +12-2.fc29 +from-repo-updates2
      dependency1.x86_64 +1.2-1.fc29 +from-repo-updates2
      filesystem.x86_64 +3.10-1.fc29 +from-repo-updates1
      flac.x86_64 +1.3.3-3.fc29 +from-repo-updates2
      lame.x86_64 +3.100-5.fc29 +from-repo-updates2
      lame-libs.x86_64 +3.100-5.fc29 +from-repo-updates2
      pkgB.x86_64 +1.2-1.fc29 +from-repo-updates2
      setup.noarch +2.14.2-1.fc29 +from-repo-updates2
      wget.x86_64 +1.19.8-2.fc29 +from-repo-updates2
      """


# https://github.com/rpm-software-management/dnf5/issues/2712
Scenario: --installed-from-repo without --installed implies --installed
   When I execute dnf with args "--repo=from-repo-updates? list --installed-from-repo=from-repo-updates2 'l*'"
   Then the exit code is 0
    And stderr is
      """
      <REPOSYNC>
      """
    And stdout matches line by line
      """
      Installed packages
      lame.x86_64 +3.100-5.fc29 +from-repo-updates2
      lame-libs.x86_64 +3.100-5.fc29 +from-repo-updates2
      """
