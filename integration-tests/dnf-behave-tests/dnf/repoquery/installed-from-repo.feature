Feature: repoquery with --installed-from-repo


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
   When I execute dnf with args "repoquery --installed-from-repo=from-repo"
   Then the exit code is 0
    And stderr is
      """
      <REPOSYNC>
      """
    And stdout is
      """
      dependency3-0:1.0-1.fc29.x86_64
      lz4-0:1.7.5-2.fc26.x86_64
      """


Scenario: List all packages installed from repo "from-repo-updates1"
   When I execute dnf with args "repoquery --installed-from-repo=from-repo-updates1"
   Then the exit code is 0
    And stderr is
      """
      <REPOSYNC>
      """
    And stdout is
      """
      abcde-0:2.9.3-1.fc29.noarch
      filesystem-0:3.10-1.fc29.x86_64
      """


Scenario: List all packages installed from repo "from-repo-updates2" using '*' package specification
   When I execute dnf with args "repoquery --installed-from-repo=from-repo-updates2 '*'"
   Then the exit code is 0
    And stderr is
      """
      <REPOSYNC>
      """
    And stdout is
      """
      basesystem-0:12-2.fc29.noarch
      dependency1-0:1.2-1.fc29.x86_64
      flac-0:1.3.3-3.fc29.x86_64
      lame-0:3.100-5.fc29.x86_64
      lame-libs-0:3.100-5.fc29.x86_64
      pkgB-0:1.2-1.fc29.x86_64
      setup-0:2.14.2-1.fc29.noarch
      wget-0:1.19.8-2.fc29.x86_64
      """


Scenario: List packages installed from repo "from-repo-updates2" using 'l*' package specification
   When I execute dnf with args "repoquery --installed-from-repo=from-repo-updates2 'l*'"
   Then the exit code is 0
    And stderr is
      """
      <REPOSYNC>
      """
    And stdout is
      """
      lame-0:3.100-5.fc29.x86_64
      lame-libs-0:3.100-5.fc29.x86_64
      """


Scenario: Test case with 2 repositories defined in installed from repo
   When I execute dnf with args "repoquery --installed-from-repo=from-repo,from-repo-updates1"
   Then the exit code is 0
    And stderr is
      """
      <REPOSYNC>
      """
    And stdout is
      """
      abcde-0:2.9.3-1.fc29.noarch
      dependency3-0:1.0-1.fc29.x86_64
      filesystem-0:3.10-1.fc29.x86_64
      lz4-0:1.7.5-2.fc26.x86_64
      """


Scenario: Test case with wildcards in installed from repo definition
   When I execute dnf with args "repoquery --installed-from-repo=from-*-updates?"
   Then the exit code is 0
    And stderr is
      """
      <REPOSYNC>
      """
    And stdout is
      """
      abcde-0:2.9.3-1.fc29.noarch
      basesystem-0:12-2.fc29.noarch
      dependency1-0:1.2-1.fc29.x86_64
      filesystem-0:3.10-1.fc29.x86_64
      flac-0:1.3.3-3.fc29.x86_64
      lame-0:3.100-5.fc29.x86_64
      lame-libs-0:3.100-5.fc29.x86_64
      pkgB-0:1.2-1.fc29.x86_64
      setup-0:2.14.2-1.fc29.noarch
      wget-0:1.19.8-2.fc29.x86_64
      """
