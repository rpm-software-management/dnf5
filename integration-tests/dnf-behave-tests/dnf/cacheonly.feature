Feature: Tests for cacheonly functionality


Background:
  Given I use repository "dnf-ci-fedora"


Scenario: Cannot work with empty cache when using -C
  Given I execute dnf with args "install wget -C"
   Then the exit code is 1
    And stderr contains lines
    """
    Cache-only enabled but no cache for repository "dnf-ci-fedora"
    """


Scenario: Cannot install a remote package when using -C
  Given I successfully execute dnf with args "makecache"
   When I execute dnf with args "install wget -C"
   Then the exit code is 1
    And stderr contains lines
    """
    Cannot download the "wget-1.19.5-5.fc29.x86_64" package, cacheonly option is activated.
    """


Scenario: Install previously downloaded package using -C
  Given I successfully execute dnf with args "download wget"
   When I execute dnf with args "install wget-1.19.5-5.fc29.x86_64.rpm -C"
   Then the exit code is 0
    And Transaction is following
        | Action        | Package                         |
        | install       | wget-0:1.19.5-5.fc29.x86_64     |


Scenario: Install locally cached package using -C
  Given I successfully execute dnf with args "install wget --downloadonly --setopt=keepcache=True"
   When I execute dnf with args "install wget -C"
   Then the exit code is 0
    And Transaction is following
        | Action        | Package                         |
        | install       | wget-0:1.19.5-5.fc29.x86_64     |


Scenario: Cannot work with empty cache when using cacheonly=metadata
  Given I execute dnf with args "install wget --setopt=cacheonly=metadata"
   Then the exit code is 1
    And stderr contains lines
    """
    Cache-only enabled but no cache for repository "dnf-ci-fedora"
    """


Scenario: Install a remote package when using cacheonly=metadata
  Given I successfully execute dnf with args "makecache"
   When I execute dnf with args "install wget --setopt=cacheonly=metadata"
   Then the exit code is 0
    And Transaction is following
        | Action        | Package                         |
        | install       | wget-0:1.19.5-5.fc29.x86_64     |


Scenario: Local key is imported during installation with -C option
  Given I drop repository "dnf-ci-fedora"
    And I use repository "dnf-ci-gpg"
    And I configure repository "dnf-ci-gpg" with
        | key      | value                                                                          |
        | gpgcheck | 1                                                                              |
        | gpgkey   | file://{context.dnf.fixturesdir}/gpgkeys/keys/dnf-ci-gpg/dnf-ci-gpg-public     |
    And I successfully execute dnf with args "install setup --downloadonly --setopt=keepcache=True"
   When I execute dnf with args "install setup -C"
   Then Transaction is following
        | Action        | Package                        |
        | install       | setup-0:2.12.1-1.fc29.noarch   |
    And stderr contains "The key was successfully imported."
