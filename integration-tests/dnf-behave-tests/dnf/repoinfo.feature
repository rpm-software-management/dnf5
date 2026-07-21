Feature: Repoinfo


Background: Using repositories dnf-ci-fedora and dnf-ci-thirdparty-updates
  Given I use repository "dnf-ci-fedora"
    And I use repository "dnf-ci-thirdparty-updates"
    And I use repository "dnf-ci-fedora-updates" with configuration
        | key     | value |
        | enabled | 0     |
    And I use repository "dnf-ci-thirdparty" with configuration
        | key     | value |
        | enabled | 0     |

@bz1793950
Scenario: Repo info without arguments
   When I execute dnf with args "repo info"
   Then the exit code is 0
    And stderr is
        """
        <REPOSYNC>
        """
    And stdout matches line by line
        """
        Repo ID              : dnf-ci-fedora
        Name                 : dnf-ci-fedora test repository
        Status               : enabled
        Priority             : 99
        Cost                 : 1000
        Type                 : available
        Metadata expire      : .*
        Skip if unavailable  : false
        Config file          : .*/etc/yum.repos.d/dnf-ci-fedora.repo
        URLs                 :
          Base URL           : .*/fixtures/repos/dnf-ci-fedora
        OpenPGP              :
          Verify repodata    : false
          Verify packages    : true
        Repodata info        :
          Available packages : 289
          Total packages     : 289
          Size               : 2\.[0-9] MiB
          Revision           : 1550000000
          Updated            : .*

        Repo ID              : dnf-ci-thirdparty-updates
        Name                 : dnf-ci-thirdparty-updates test repository
        Status               : enabled
        Priority             : 99
        Cost                 : 1000
        Type                 : available
        Metadata expire      : .*
        Skip if unavailable  : false
        Config file          : .*/etc/yum.repos.d/dnf-ci-thirdparty-updates.repo
        URLs                 :
          Base URL           : .*/fixtures/repos/dnf-ci-thirdparty-updates
        OpenPGP              :
          Verify repodata    : false
          Verify packages    : true
        Repodata info        :
          Available packages : 6
          Total packages     : 6
          Size               : [0-9][0-9](\.[0-9])? KiB
          Revision           : 1550000000
          Updated            : .*
        """

@bz1793950
Scenario: Repo info without arguments and option --all
   When I execute dnf with args "repo info --all"
   Then the exit code is 0
    And stderr is
        """
        <REPOSYNC>
        """
    And stdout matches line by line
        """
        Repo ID              : dnf-ci-fedora
        Name                 : dnf-ci-fedora test repository
        Status               : enabled
        Priority             : 99
        Cost                 : 1000
        Type                 : available
        Metadata expire      : .*
        Skip if unavailable  : false
        Config file          : .*/etc/yum.repos.d/dnf-ci-fedora.repo
        URLs                 :
          Base URL           : .*/fixtures/repos/dnf-ci-fedora
        OpenPGP              :
          Verify repodata    : false
          Verify packages    : true
        Repodata info        :
          Available packages : 289
          Total packages     : 289
          Size               : 2\.[0-9] MiB
          Revision           : 1550000000
          Updated            : .*

        Repo ID             : dnf-ci-fedora-updates
        Name                : dnf-ci-fedora-updates test repository
        Status              : disabled
        Priority            : 99
        Cost                : 1000
        Type                : available
        Metadata expire     : .*
        Skip if unavailable : false
        Config file         : .*/etc/yum.repos.d/dnf-ci-fedora-updates.repo
        URLs                :
          Base URL          : .*/fixtures/repos/dnf-ci-fedora-updates
        OpenPGP             :
          Verify repodata   : false
          Verify packages   : true

        Repo ID             : dnf-ci-thirdparty
        Name                : dnf-ci-thirdparty test repository
        Status              : disabled
        Priority            : 99
        Cost                : 1000
        Type                : available
        Metadata expire     : .*
        Skip if unavailable : false
        Config file         : .*/etc/yum.repos.d/dnf-ci-thirdparty.repo
        URLs                :
          Base URL          : .*/fixtures/repos/dnf-ci-thirdparty
        OpenPGP             :
          Verify repodata   : false
          Verify packages   : true

        Repo ID              : dnf-ci-thirdparty-updates
        Name                 : dnf-ci-thirdparty-updates test repository
        Status               : enabled
        Priority             : 99
        Cost                 : 1000
        Type                 : available
        Metadata expire      : .*
        Skip if unavailable  : false
        Config file          : .*/etc/yum.repos.d/dnf-ci-thirdparty-updates.repo
        URLs                 :
          Base URL           : .*/fixtures/repos/dnf-ci-thirdparty-updates
        OpenPGP              :
          Verify repodata    : false
          Verify packages    : true
        Repodata info        :
          Available packages : 6
          Total packages     : 6
          Size               : [0-9][0-9](\.[0-9])? KiB
          Revision           : 1550000000
          Updated            : .*
        """

@bz1793950
Scenario: Repoinfo without arguments but with excludes
   When I execute dnf with args "repo info --exclude=*"
   Then the exit code is 0
    And stderr is
        """
        <REPOSYNC>
        """
    And stdout matches line by line
        """
        <REPOSYNC>
        Repo ID              : dnf-ci-fedora
        Name                 : dnf-ci-fedora test repository
        Status               : enabled
        Priority             : 99
        Cost                 : 1000
        Type                 : available
        Metadata expire      : .*
        Skip if unavailable  : false
        Config file          : .*/etc/yum.repos.d/dnf-ci-fedora.repo
        URLs                 :
          Base URL           : .*/fixtures/repos/dnf-ci-fedora
        OpenPGP              :
          Verify repodata    : false
          Verify packages    : true
        Repodata info        :
          Available packages : 0
          Total packages     : 289
          Size               : 2\.[0-9] MiB
          Revision           : 1550000000
          Updated            : .*

        Repo ID              : dnf-ci-thirdparty-updates
        Name                 : dnf-ci-thirdparty-updates test repository
        Status               : enabled
        Priority             : 99
        Cost                 : 1000
        Type                 : available
        Metadata expire      : .*
        Skip if unavailable  : false
        Config file          : .*/etc/yum.repos.d/dnf-ci-thirdparty-updates.repo
        URLs                 :
          Base URL           : .*/fixtures/repos/dnf-ci-thirdparty-updates
        OpenPGP              :
          Verify repodata    : false
          Verify packages    : true
        Repodata info        :
          Available packages : 0
          Total packages     : 6
          Size               : [0-9][0-9](\.[0-9])? KiB
          Revision           : 1550000000
          Updated            : .*
        """
