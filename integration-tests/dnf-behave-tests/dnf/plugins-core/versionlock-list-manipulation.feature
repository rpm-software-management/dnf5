Feature: Versionlock command can maintain versionlock.list file


Background: Set up versionlock infrastructure in the installroot
  Given I create file "/etc/dnf/versionlock.toml" with
    """
    """
    And I use repository "dnf-ci-fedora"


Scenario: Basic commands add/exclude/list/delete/clear for manipulation with versionlock.list file are working
   When I execute dnf with args "install wget"
   Then the exit code is 0
    And Transaction is following
        | Action        | Package                               |
        | install       | wget-0:1.19.5-5.fc29.x86_64           |
   # add command
   When I execute dnf with args "versionlock add wget"
   Then the exit code is 0
    And stderr is
    """
    <REPOSYNC>
    """
    And stdout is
    """
    Adding versionlock on "wget = 1.19.5-5.fc29".
    """
   When I execute dnf with args "versionlock list"
   Then the exit code is 0
    And stderr is
    """
    <REPOSYNC>
    """
    And stdout matches line by line
    """
    # Added by 'versionlock add' command on .*
    Package name: wget
    evr = 1.19.5-5.fc29
    """
   # exclude command
   When I execute dnf with args "versionlock exclude lame"
   Then the exit code is 0
    And stderr is
    """
    <REPOSYNC>
    """
    And stdout is
    """
    Adding versionlock exclude on "lame = 3.100-4.fc29".
    """
   When I execute dnf with args "versionlock list"
   Then the exit code is 0
    And stderr is
    """
    <REPOSYNC>
    """
    And stdout matches line by line
    """
    # Added by 'versionlock add' command on .*
    Package name: wget
    evr = 1.19.5-5.fc29

    # Added by 'versionlock exclude' command on .*
    Package name: lame
    evr != 3.100-4.fc29
    """
   # delete command
   When I execute dnf with args "versionlock delete wget"
   Then the exit code is 0
    And stderr is
    """
    <REPOSYNC>
    """
    And stdout matches line by line
    """
    Deleting versionlock entry:
    # Added by 'versionlock add' command on .*
    Package name: wget
    evr = 1.19.5-5.fc29
    """
   When I execute dnf with args "versionlock list"
   Then the exit code is 0
    And stderr is
    """
    <REPOSYNC>
    """
    And stdout matches line by line
    """
    # Added by 'versionlock exclude' command on .*
    Package name: lame
    evr != 3.100-4.fc29
    """
   # delete command on excluded package
   When I execute dnf with args "versionlock delete lame"
   Then the exit code is 0
    And stderr is
    """
    <REPOSYNC>
    """
    And stdout matches line by line
    """
    Deleting versionlock entry:
    # Added by 'versionlock exclude' command on .*
    Package name: lame
    evr != 3.100-4.fc29
    """
   When I execute dnf with args "versionlock list"
   Then the exit code is 0
    And stderr is
    """
    <REPOSYNC>
    """
    And stdout is empty
   # clear command
   When I execute dnf with args "versionlock add wget"
   Then the exit code is 0
    And stderr is
    """
    <REPOSYNC>
    """
    And stdout is
    """
    Adding versionlock on "wget = 1.19.5-5.fc29".
    """
   When I execute dnf with args "versionlock clear"
   Then the exit code is 0
    And stderr is
    """
    <REPOSYNC>
    """
    And stdout is empty
   When I execute dnf with args "versionlock list"
   Then the exit code is 0
    And stderr is
    """
    <REPOSYNC>
    """
    And stdout is empty


@bz1785563
Scenario: versionlock will print just necessary information with -q option
  Given I use repository "dnf-ci-fedora"
  Given I execute dnf with args "versionlock add wget"
  When I execute dnf with args "-q versionlock list"
  Then the exit code is 0
  And stdout matches line by line
    """
    # Added by 'versionlock add' command on .*
    Package name: wget
    evr = 1.19.5-5.fc29
    """


@bz1782052
@bz1845270
Scenario: Prevent duplicate entries in versionlock.list
  Given I use repository "dnf-ci-fedora"
    And I successfully execute dnf with args "install wget"
    And I successfully execute dnf with args "versionlock add wget"
   When I execute dnf with args "versionlock add wget"
   Then the exit code is 0
    And stderr is
    """
    <REPOSYNC>
    Package "wget" is already locked.
    """


@xfail
# currently the conflict between add and exclude is not detected.
# The reason is that we need multiple entries for the same name
# to handle locking version-1 OR version-2
# Reported as https://github.com/rpm-software-management/dnf5/issues/1832
@bz1782052
Scenario: Prevent conflicting entries in versionlock.list
  Given I use repository "dnf-ci-fedora"
    And I successfully execute dnf with args "install wget"
    And I successfully execute dnf with args "versionlock add wget"
   When I execute dnf with args "versionlock exclude wget"
   Then the exit code is 1
    And stderr is
    """
    <REPOSYNC>
    Error: Package wget-0:1.19.5-5.fc29.* is already locked
    """


@bz2013324
Scenario: I can exclude mutliple packages when one is already excluded
  Given I use repository "dnf-ci-fedora"
    And I successfully execute dnf with args "versionlock exclude wget"
   When I execute dnf with args "versionlock exclude abcde wget"
   Then the exit code is 0
    And stdout is
    """
    Adding versionlock exclude on "abcde = 2.9.2-1.fc29".
    """
    And stderr is
    """
    <REPOSYNC>
    Package "wget" is already excluded.
    """


@bz2013324
Scenario: I can lock mutliple packages when one is already locked
  Given I use repository "dnf-ci-fedora"
    And I successfully execute dnf with args "versionlock add wget"
   When I execute dnf with args "versionlock add abcde wget"
   Then the exit code is 0
    And stdout is
    """
    Adding versionlock on "abcde = 2.9.2-1.fc29".
    """
    And stderr is
    """
    <REPOSYNC>
    Package "wget" is already locked.
    """


# https://github.com/rpm-software-management/dnf5/issues/1895
Scenario: I can lock multiple packages with the same nevra and name matching wildcard
   When I execute dnf with args "versionlock add lz4*"
   Then the exit code is 0
    And stdout is
    """
    Adding versionlock on "lz4 = 1.7.5-2.fc26".
    Adding versionlock on "lz4-debuginfo = 1.7.5-2.fc26".
    Adding versionlock on "lz4-devel = 1.7.5-2.fc26".
    Adding versionlock on "lz4-static = 1.7.5-2.fc26".
    """
    And stderr is
    """
    <REPOSYNC>
    """
