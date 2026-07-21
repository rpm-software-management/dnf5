Feature: DNF test of append option

# The excludepks append option was choosen for testing.
# Both paths "--exclude" and "--setopt=excludepkgs" are tested.

Background: Enable repository and set excludes in configuration
  Given I use repository "dnf-ci-fedora-updates"
    And I configure dnf with
        | key     | value     |
        | exclude | lame, lz4 |


Scenario: Test that excludes from config file are applied
   When I execute dnf with args "repoquery abcde lame lz4 wget"
   Then the exit code is 0
    And stderr is
        """
        <REPOSYNC>
        """
    And stdout is
        """
        abcde-0:2.9.3-1.fc29.noarch
        abcde-0:2.9.3-1.fc29.src
        wget-0:1.19.6-5.fc29.src
        wget-0:1.19.6-5.fc29.x86_64
        """


Scenario: Test adding of excludes
   When I execute dnf with args "--exclude=lz4 --exclude=wget repoquery abcde lame lz4 wget"
   Then the exit code is 0
    And stderr is
        """
        <REPOSYNC>
        """
    And stdout is
        """
        abcde-0:2.9.3-1.fc29.noarch
        abcde-0:2.9.3-1.fc29.src
        """


Scenario: Test adding of excludes using --setopt
   When I execute dnf with args "--setopt=excludepkgs=lz4 --setopt=excludepkgs=wget repoquery abcde lame lz4 wget"
   Then the exit code is 0
    And stderr is
        """
        <REPOSYNC>
        """
    And stdout is
        """
        abcde-0:2.9.3-1.fc29.noarch
        abcde-0:2.9.3-1.fc29.src
        """


Scenario: Test adding of excludes short notation
   When I execute dnf with args "--exclude=lz4,wget repoquery abcde lame lz4 wget"
   Then the exit code is 0
    And stderr is
        """
        <REPOSYNC>
        """
    And stdout is
        """
        abcde-0:2.9.3-1.fc29.noarch
        abcde-0:2.9.3-1.fc29.src
        """


Scenario: Test adding of excludes short notation using --setopt
   When I execute dnf with args "--setopt=excludepkgs=lz4,wget repoquery abcde lame lz4 wget"
   Then the exit code is 0
    And stderr is
        """
        <REPOSYNC>
        """
    And stdout is
        """
        abcde-0:2.9.3-1.fc29.noarch
        abcde-0:2.9.3-1.fc29.src
        """


Scenario: Test removing of existing excludes
   When I execute dnf with args "--exclude= repoquery abcde lame lz4 wget"
   Then the exit code is 0
    And stderr is
        """
        <REPOSYNC>
        """
    And stdout is
        """
        abcde-0:2.9.3-1.fc29.noarch
        abcde-0:2.9.3-1.fc29.src
        lame-0:3.100-5.fc29.src
        lame-0:3.100-5.fc29.x86_64
        lz4-0:1.8.2-2.fc29.i686
        lz4-0:1.8.2-2.fc29.src
        lz4-0:1.8.2-2.fc29.x86_64
        wget-0:1.19.6-5.fc29.src
        wget-0:1.19.6-5.fc29.x86_64
        """


Scenario: Test removing of existing excludes using --setopt
   When I execute dnf with args "--setopt=excludepkgs= repoquery abcde lame lz4 wget"
   Then the exit code is 0
    And stderr is
        """
        <REPOSYNC>
        """
    And stdout is
        """
        abcde-0:2.9.3-1.fc29.noarch
        abcde-0:2.9.3-1.fc29.src
        lame-0:3.100-5.fc29.src
        lame-0:3.100-5.fc29.x86_64
        lz4-0:1.8.2-2.fc29.i686
        lz4-0:1.8.2-2.fc29.src
        lz4-0:1.8.2-2.fc29.x86_64
        wget-0:1.19.6-5.fc29.src
        wget-0:1.19.6-5.fc29.x86_64
        """


Scenario: Test replacing of existing excludes
   When I execute dnf with args "--exclude= --exclude=lz4 --exclude=wget repoquery abcde lame lz4 wget"
   Then the exit code is 0
    And stderr is
        """
        <REPOSYNC>
        """
    And stdout is
        """
        abcde-0:2.9.3-1.fc29.noarch
        abcde-0:2.9.3-1.fc29.src
        lame-0:3.100-5.fc29.src
        lame-0:3.100-5.fc29.x86_64
        """


Scenario: Test replacing of existing excludes using --setopt
   When I execute dnf with args "--setopt=excludepkgs= --setopt=excludepkgs=lz4 --setopt=excludepkgs=wget repoquery abcde lame lz4 wget"
   Then the exit code is 0
    And stderr is
        """
        <REPOSYNC>
        """
    And stdout is
        """
        abcde-0:2.9.3-1.fc29.noarch
        abcde-0:2.9.3-1.fc29.src
        lame-0:3.100-5.fc29.src
        lame-0:3.100-5.fc29.x86_64
        """


Scenario: Test replacing of existing excludes short notation
   When I execute dnf with args "--exclude=,lz4,wget repoquery abcde lame lz4 wget"
   Then the exit code is 0
    And stderr is
        """
        <REPOSYNC>
        """
    And stdout is
        """
        abcde-0:2.9.3-1.fc29.noarch
        abcde-0:2.9.3-1.fc29.src
        lame-0:3.100-5.fc29.src
        lame-0:3.100-5.fc29.x86_64
        """


Scenario: Test replacing of existing excludes short notation using --setopt
   When I execute dnf with args "--setopt=excludepkgs=,lz4,wget repoquery abcde lame lz4 wget"
   Then the exit code is 0
    And stderr is
        """
        <REPOSYNC>
        """
    And stdout is
        """
        abcde-0:2.9.3-1.fc29.noarch
        abcde-0:2.9.3-1.fc29.src
        lame-0:3.100-5.fc29.src
        lame-0:3.100-5.fc29.x86_64
        """


@bz1788154
Scenario: Test adding excludes (empty values in the middle of short notation are ignored)
   When I execute dnf with args "--exclude=lz4,,wget repoquery abcde lame lz4 wget"
   Then the exit code is 0
    And stderr is
        """
        <REPOSYNC>
        """
    And stdout is
        """
        abcde-0:2.9.3-1.fc29.noarch
        abcde-0:2.9.3-1.fc29.src
        """


@bz1788154
Scenario: Test adding excludes (empty values in the middle of short notation are ignored) using --setopt
   When I execute dnf with args "--setopt=excludepkgs=lz4,,wget repoquery abcde lame lz4 wget"
   Then the exit code is 0
    And stderr is
        """
        <REPOSYNC>
        """
    And stdout is
        """
        abcde-0:2.9.3-1.fc29.noarch
        abcde-0:2.9.3-1.fc29.src
        """


@bz1788154
Scenario: Test adding excludes (empty value at the end of short notation is ignored)
   When I execute dnf with args "--exclude=lz4,wget, repoquery abcde lame lz4 wget"
   Then the exit code is 0
    And stderr is
        """
        <REPOSYNC>
        """
    And stdout is
        """
        abcde-0:2.9.3-1.fc29.noarch
        abcde-0:2.9.3-1.fc29.src
        """


@bz1788154
Scenario: Test adding excludes (empty value at the end of short notation is ignored) using --setopt
   When I execute dnf with args "--setopt=excludepkgs=lz4,wget, repoquery abcde lame lz4 wget"
   Then the exit code is 0
    And stderr is
        """
        <REPOSYNC>
        """
    And stdout is
        """
        abcde-0:2.9.3-1.fc29.noarch
        abcde-0:2.9.3-1.fc29.src
        """
