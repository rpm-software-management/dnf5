Feature: Tests for optional metadata loading functionality


Background:
  Given I use repository "advisories-and-groups"
    And I successfully execute dnf with args "makecache --setopt=optional_metadata_types=,"
    And I execute "find | sort" in "{context.dnf.installroot}/var/cache/dnf"
   Then stdout matches line by line
    """
    \.
    \./advisories-and-groups-[0-9a-f]{16}
    \./advisories-and-groups-[0-9a-f]{16}/repodata
    \./advisories-and-groups-[0-9a-f]{16}/repodata/primary\.xml\.*
    \./advisories-and-groups-[0-9a-f]{16}/repodata/repomd\.xml
    \./advisories-and-groups-[0-9a-f]{16}/solv
    \./advisories-and-groups-[0-9a-f]{16}/solv/advisories-and-groups\.solv
    """


Scenario: Optional metadata are loaded when explicitly requested by the option
  Given I execute dnf with args "makecache --setopt=optional_metadata_types=,comps"
   Then the exit code is 0
    And stderr matches line by line
        """
        Updating and loading repositories:
         advisories-and-groups test repository .*
        Repositories loaded.
        """
    And stdout is
        """
        Metadata cache created.
        """
   When I execute "find | sort" in "{context.dnf.installroot}/var/cache/dnf"
   Then stdout matches line by line
    """
    \.
    \./advisories-and-groups-[0-9a-f]{16}
    \./advisories-and-groups-[0-9a-f]{16}/repodata
    \./advisories-and-groups-[0-9a-f]{16}/repodata/comps\.xml\.*
    \./advisories-and-groups-[0-9a-f]{16}/repodata/primary\.xml\.*
    \./advisories-and-groups-[0-9a-f]{16}/repodata/repomd\.xml
    \./advisories-and-groups-[0-9a-f]{16}/solv
    \./advisories-and-groups-[0-9a-f]{16}/solv/advisories-and-groups-group\.solvx
    \./advisories-and-groups-[0-9a-f]{16}/solv/advisories-and-groups\.solv
    """


Scenario: Invalid metadata type is ignored when processing the option
  Given I execute dnf with args "makecache --setopt=optional_metadata_types=,abcdef"
   Then the exit code is 0
    And stderr is
        """
        <REPOSYNC>
        """
    And stdout is
        """
        Metadata cache created.
        """


Scenario: Optional metadata are loaded when requested by command
  Given I execute dnf with args "advisory list --setopt=optional_metadata_types=,"
   Then the exit code is 0
    And stderr matches line by line
        """
        Updating and loading repositories:
         advisories-and-groups test repository .*
        Repositories loaded.
        """
   When I execute "find | sort" in "{context.dnf.installroot}/var/cache/dnf"
   Then stdout matches line by line
    """
    \.
    \./advisories-and-groups-[0-9a-f]{16}
    \./advisories-and-groups-[0-9a-f]{16}/repodata
    \./advisories-and-groups-[0-9a-f]{16}/repodata/[0-9a-f]+-updateinfo\.xml\.*
    \./advisories-and-groups-[0-9a-f]{16}/repodata/primary\.xml\.*
    \./advisories-and-groups-[0-9a-f]{16}/repodata/repomd\.xml
    \./advisories-and-groups-[0-9a-f]{16}/solv
    \./advisories-and-groups-[0-9a-f]{16}/solv/advisories-and-groups-updateinfo\.solvx
    \./advisories-and-groups-[0-9a-f]{16}/solv/advisories-and-groups\.solv
    """


Scenario: Operation returns an error when metadata are not present in cacheonly mode
  Given I execute dnf with args "advisory list --cacheonly --setopt=optional_metadata_types=,"
   Then the exit code is 1
    And stderr is
        """
        <REPOSYNC>
        Cache-only enabled but no cache for repository "advisories-and-groups"
        """


Scenario: Filelists metadata are loaded when filepath spec is provided
  Given I execute dnf with args "repoquery /some/file --setopt=optional_metadata_types=,"
   Then the exit code is 0
    And stderr matches line by line
        """
        Updating and loading repositories:
         advisories-and-groups test repository .*
        Repositories loaded.
        """
   When I execute "find | sort" in "{context.dnf.installroot}/var/cache/dnf"
   Then stdout matches line by line
    """
    \.
    \./advisories-and-groups-[0-9a-f]{16}
    \./advisories-and-groups-[0-9a-f]{16}/repodata
    \./advisories-and-groups-[0-9a-f]{16}/repodata/filelists\.xml\.*
    \./advisories-and-groups-[0-9a-f]{16}/repodata/primary\.xml\.*
    \./advisories-and-groups-[0-9a-f]{16}/repodata/repomd\.xml
    \./advisories-and-groups-[0-9a-f]{16}/solv
    \./advisories-and-groups-[0-9a-f]{16}/solv/advisories-and-groups-filelists\.solvx
    \./advisories-and-groups-[0-9a-f]{16}/solv/advisories-and-groups\.solv
    """


Scenario: Comps metadata are loaded when group spec is provided
  Given I execute dnf with args "repoquery @group --setopt=optional_metadata_types=,"
   Then the exit code is 0
    And stderr matches line by line
        """
        Updating and loading repositories:
         advisories-and-groups test repository .*
        Repositories loaded.
        """
   When I execute "find | sort" in "{context.dnf.installroot}/var/cache/dnf"
   Then stdout matches line by line
    """
    \.
    \./advisories-and-groups-[0-9a-f]{16}
    \./advisories-and-groups-[0-9a-f]{16}/repodata
    \./advisories-and-groups-[0-9a-f]{16}/repodata/comps\.xml\.*
    \./advisories-and-groups-[0-9a-f]{16}/repodata/primary\.xml\.*
    \./advisories-and-groups-[0-9a-f]{16}/repodata/repomd\.xml
    \./advisories-and-groups-[0-9a-f]{16}/solv
    \./advisories-and-groups-[0-9a-f]{16}/solv/advisories-and-groups-group\.solvx
    \./advisories-and-groups-[0-9a-f]{16}/solv/advisories-and-groups\.solv
    """
