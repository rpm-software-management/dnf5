Feature: Test the automatic filelists metadata retry

# RequiresFileDep requires /var/ProvidesFileDep, a file provided only by
# ProvidesFileDep and only discoverable via filelists metadata (/var isn't
# among the standard directories primary metadata auto-generates file
# provides for). optional_metadata_types is deliberately kept to
# comps,updateinfo so filelists is never loaded upfront.

Background: Set dnf for strict behavior
  Given I use repository "resolve-hints"
    And I configure dnf with
        | key                       | value             |
        | best                      | True              |
        | skip_unavailable          | False             |
        | skip_broken               | False             |
        | optional_metadata_types   | comps,updateinfo  |

Scenario: A missing file dependency is automatically resolved by loading filelists metadata
   When I execute dnf with args "install RequiresFileDep"
   Then the exit code is 0
    And Transaction is following
        | Action      | Package                        |
        | install     | RequiresFileDep-0:1.0-1.noarch |
        | install-dep | ProvidesFileDep-0:1.0-1.noarch |
    And stderr contains "Automatically downloaded and loaded filelists metadata to retry dependency resolution."

Scenario: filelists_auto_load=False disables the automatic retry
  Given I configure dnf with
        | key                 | value |
        | filelists_auto_load | False |
   When I execute dnf with args "install RequiresFileDep"
   Then the exit code is 1
    And stderr is
        """
        <REPOSYNC>
        Failed to resolve the transaction:
        Problem: conflicting requests
          - nothing provides /var/ProvidesFileDep needed by RequiresFileDep-1.0-1.noarch from resolve-hints
        You can try to add to command line:
          --setopt=optional_metadata_types=filelists to load additional filelists metadata
          --skip-broken to skip uninstallable packages
        """

Scenario: Preloading filelists metadata upfront skips the automatic retry
   When I execute dnf with args "install RequiresFileDep --setopt=optional_metadata_types=filelists"
   Then the exit code is 0
    And Transaction is following
        | Action      | Package                        |
        | install     | RequiresFileDep-0:1.0-1.noarch |
        | install-dep | ProvidesFileDep-0:1.0-1.noarch |
    And stderr does not contain "Automatically downloaded and loaded filelists metadata to retry dependency resolution."

Scenario: The automatic retry still fails for a file dependency no repository provides
   When I execute dnf with args "install RequiresMissingFileDep"
   Then the exit code is 1
    And stderr is
        """
        <REPOSYNC>
        Failed to resolve the transaction:
        Problem: conflicting requests
          - nothing provides /var/MissingFileDep needed by RequiresMissingFileDep-1.0-1.noarch from resolve-hints
        Automatically downloaded and loaded filelists metadata to retry dependency resolution.
        You can try to add to command line:
          --skip-broken to skip uninstallable packages
        """
