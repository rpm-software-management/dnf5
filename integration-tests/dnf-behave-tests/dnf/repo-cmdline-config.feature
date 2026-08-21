Feature: Configure repos through cmd line options


Scenario: Exit with an error when trying to --enable-repo='*' when there are no repo files
   When I execute dnf with args "repo list --enable-repo='*'"
   Then the exit code is 2
    And stderr is
    """
    No matching repositories for *. To use repositories from a host system, pass --use-host-config option. Add "--help" for more information about the arguments.
    """


Scenario: Exit with an error when trying to enable missing repo
  Given I use repository "simple-base"
   When I execute dnf with args "repo list --enable-repo='missing-repo'"
   Then the exit code is 2
    And stderr is
    """
    No matching repositories for missing-repo. To use repositories from a host system, pass --use-host-config option. Add "--help" for more information about the arguments.
    """


Scenario: Exit with an error when trying to enable multiple missing repos
  Given I use repository "simple-base"
   When I execute dnf with args "repo list --enable-repo='missing-repo1' --enable-repo='missing-repo2'"
   Then the exit code is 2
    And stderr is
    """
    No matching repositories for missing-repo1, missing-repo2. To use repositories from a host system, pass --use-host-config option. Add "--help" for more information about the arguments.
    """


Scenario: Enabling enabled repo is ok
  Given I use repository "simple-base"
   When I execute dnf with args "repo list --enable-repo='simple-base'"
   Then the exit code is 0
    And stdout is
    """
    repo id     repo name
    simple-base simple-base test repository
    """


Scenario: Exit with an error when trying to --disable-repo='*' when there are no repo files
   When I execute dnf with args "repo list --disable-repo='*'"
   Then the exit code is 2
    And stderr is
    """
    No matching repositories for *. To use repositories from a host system, pass --use-host-config option. Add "--help" for more information about the arguments.
    """


# When there are no repositories the --repo options error report includes "*" because it
# is used internally to disable all repos. This might be confusing to users but the situation
# where there are no repos should be pretty rare.
Scenario: Exit with an error when using --repo but there are no repositories
   When I execute dnf with args "repo list --repo='missing-repo'"
   Then the exit code is 2
    And stderr is
    """
    No matching repositories for *, missing-repo. To use repositories from a host system, pass --use-host-config option. Add "--help" for more information about the arguments.
    """


Scenario: Exit with an error when --repo specifies invalid repository
  Given I use repository "simple-base"
   When I execute dnf with args "repo list --repo='missing-repo'"
   Then the exit code is 2
    And stderr is
    """
    No matching repositories for missing-repo. To use repositories from a host system, pass --use-host-config option. Add "--help" for more information about the arguments.
    """


Scenario: Exit with an error when --setop specifies invalid repository
   When I execute dnf with args "repo list --setopt=missing-repo.best=true"
   Then the exit code is 2
    And stderr is
    """
    No matching repositories for missing-repo. To use repositories from a host system, pass --use-host-config option. Add "--help" for more information about the arguments.
    """


Scenario: Exit with an error when trying --setop for all repos but there are none
   When I execute dnf with args "repo list --setopt=*.best=true"
   Then the exit code is 2
    And stderr is
    """
    No matching repositories for *. To use repositories from a host system, pass --use-host-config option. Add "--help" for more information about the arguments.
    """
