Feature: Test that proper hints are printed after resolving failure

Background: Set dnf for strict behavior
  Given I use repository "resolve-hints"
    And I configure dnf with
        | key                       | value             |
        | best                      | True              |
        | skip_unavailable          | False             |
        | skip_broken               | False             |
        | optional_metadata_types   | comps,updateinfo  |


# To print the hints we need a failing transaction - thus adding
# non-existing DoesNotExist package to all install commands.

Scenario: --use-host-config is hinted if no repos are loaded from installroot
  Given I drop repository "resolve-hints"
   When I execute dnf with args "install DoesNotExist"
   Then the exit code is 1
    And stderr is
        """
        <REPOSYNC>
        Failed to resolve the transaction:
        No match for argument: DoesNotExist
        No repositories were loaded from the installroot. To use the configuration and repositories of the host system, pass --use-host-config.
        """

Scenario: --skip-unavailable is hinted on non-existent package installation attempt
   When I execute dnf with args "install DoesNotExist"
   Then the exit code is 1
    And stderr is
        """
        <REPOSYNC>
        Failed to resolve the transaction:
        No match for argument: DoesNotExist
        You can try to add to command line:
          --skip-unavailable to skip unavailable packages
        """

Scenario: --skip-unavailable is not printed if the option is already present
   When I execute dnf with args "install DoesNotExist --skip-unavailable"
   Then the exit code is 0
    And Transaction is empty
    And stderr is
        """
        <REPOSYNC>
        No match for argument: DoesNotExist
        """

Scenario: --no-best is hinted if the best candidate cannot be installed
   When I execute dnf with args "install DoesNotExist NoBest"
   Then the exit code is 1
    And stderr is
        """
        <REPOSYNC>
        Failed to resolve the transaction:
        No match for argument: DoesNotExist
        Problem: cannot install the best candidate for the job
          - nothing provides DoesNotExist needed by NoBest-2.0-1.noarch from resolve-hints
        You can try to add to command line:
          --skip-unavailable to skip unavailable packages
          --no-best to not limit the transaction to the best candidates
          --skip-broken to skip uninstallable packages
        """

Scenario: --no-best is not printed if the option is already present
   When I execute dnf with args "install DoesNotExist NoBest --no-best"
   Then the exit code is 1
    And stderr is
        """
        <REPOSYNC>
        Failed to resolve the transaction:
        No match for argument: DoesNotExist
        Problem: cannot install the best candidate for the job
          - nothing provides DoesNotExist needed by NoBest-2.0-1.noarch from resolve-hints
        You can try to add to command line:
          --skip-unavailable to skip unavailable packages
        """

Scenario: --allowerasing is hinted on attempt to install conflicting packages
  Given I successfully execute dnf with args "install ConflictingOne"
   When I execute dnf with args "install DoesNotExist ConflictingTwo"
   Then the exit code is 1
    And stderr is
        """
        <REPOSYNC>
        Failed to resolve the transaction:
        No match for argument: DoesNotExist
        Problem: problem with installed package
          - installed package ConflictingOne-1.0-1.noarch conflicts with ConflictingTwo provided by ConflictingTwo-1.0-1.noarch from resolve-hints
          - package ConflictingOne-1.0-1.noarch from resolve-hints conflicts with ConflictingTwo provided by ConflictingTwo-1.0-1.noarch from resolve-hints
          - conflicting requests
        You can try to add to command line:
          --skip-unavailable to skip unavailable packages
          --allowerasing to allow removing of installed packages to resolve problems
          --skip-broken to skip uninstallable packages
        """

Scenario: --allowerasing is not printed if the option is already present
  Given I successfully execute dnf with args "install ConflictingOne"
   When I execute dnf with args "install DoesNotExist ConflictingTwo --allowerasing"
   Then the exit code is 1
    And stderr is
        """
        <REPOSYNC>
        Failed to resolve the transaction:
        No match for argument: DoesNotExist
        You can try to add to command line:
          --skip-unavailable to skip unavailable packages
        """

Scenario: filelists metadata is hinted on missing file dependency
   When I execute dnf with args "install DoesNotExist RequiresFileDep"
   Then the exit code is 1
    And stderr is
        """
        <REPOSYNC>
        Failed to resolve the transaction:
        No match for argument: DoesNotExist
        Problem: conflicting requests
          - nothing provides /var/ProvidesFileDep needed by RequiresFileDep-1.0-1.noarch from resolve-hints
        You can try to add to command line:
          --skip-unavailable to skip unavailable packages
          --setopt=optional_metadata_types=filelists to load additional filelists metadata
          --skip-broken to skip uninstallable packages
        """

Scenario: filelists metadata is not printed if the option is already present
   When I execute dnf with args "install DoesNotExist RequiresFileDep --setopt=optional_metadata_types=filelists"
   Then the exit code is 1
    And stderr is
        """
        <REPOSYNC>
        Failed to resolve the transaction:
        No match for argument: DoesNotExist
        You can try to add to command line:
          --skip-unavailable to skip unavailable packages
        """

Scenario: --skip-broken is hinted on non-installable package
   When I execute dnf with args "install DoesNotExist RequirementsUnmet"
   Then the exit code is 1
    And stderr is
        """
        <REPOSYNC>
        Failed to resolve the transaction:
        No match for argument: DoesNotExist
        Problem: conflicting requests
          - nothing provides DoesNotExist needed by RequirementsUnmet-1.0-1.noarch from resolve-hints
        You can try to add to command line:
          --skip-unavailable to skip unavailable packages
          --skip-broken to skip uninstallable packages
        """

Scenario: --skip-broken is not printed if the option is already present
   When I execute dnf with args "install DoesNotExist RequirementsUnmet --skip-broken"
   Then the exit code is 1
    And stderr is
        """
        <REPOSYNC>
        Failed to resolve the transaction:
        No match for argument: DoesNotExist
        Problem: conflicting requests
          - nothing provides DoesNotExist needed by RequirementsUnmet-1.0-1.noarch from resolve-hints
        You can try to add to command line:
          --skip-unavailable to skip unavailable packages
        """
