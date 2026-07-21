Feature: Testing gpgcheck_policy option

# The gpgcheck_policy option (global, [main] section only) controls how
# setting gpgcheck=1 (alias for pkg_gpgcheck) expands to additional options:
#   legacy (default): no expansion, gpgcheck only sets pkg_gpgcheck
#   full:   gpgcheck=1 additionally sets repo_gpgcheck=1
#   all:    gpgcheck=1 additionally sets repo_gpgcheck=1 and localpkg_gpgcheck=1
#
# Explicitly set options in the same config section override the expansion.
#
# simple-base packages are signed by "default-key" which is imported at test start.
# unsigned repo packages are intentionally unsigned.


Scenario: Default (legacy) policy - gpgcheck=1 does not enforce repo_gpgcheck
  Given I use repository "simple-base" with configuration
        | key          | value                                                                        |
        | gpgcheck     | 1                                                                            |
        | gpgkey       | file://{context.dnf.fixturesdir}/gpgkeys/keys/default-key/default-key-public |
   # Repo metadata is not signed, but legacy policy does not expand to repo_gpgcheck
   When I execute dnf with args "install labirinto"
   Then the exit code is 0
    And Transaction is following
        | Action  | Package                       |
        | install | labirinto-0:1.0-1.fc29.x86_64 |


Scenario: Full policy with gpgcheck=1 and signed metadata succeeds
  Given I configure dnf with
        | key              | value |
        | gpgcheck_policy  | full  |
    And I copy repository "simple-base" for modification
    And I sign repository "simple-base" metadata with "{context.dnf.fixturesdir}/gpgkeys/keys/default-key/default-key-private"
    And I use repository "simple-base" with configuration
        | key          | value                                                                        |
        | gpgcheck     | 1                                                                            |
        | gpgkey       | file://{context.dnf.fixturesdir}/gpgkeys/keys/default-key/default-key-public |
   When I execute dnf with args "install labirinto"
   Then the exit code is 0
    And Transaction is following
        | Action  | Package                       |
        | install | labirinto-0:1.0-1.fc29.x86_64 |


Scenario: Full policy with gpgcheck=1 and unsigned metadata fails
  Given I configure dnf with
        | key              | value |
        | gpgcheck_policy  | full  |
    And I use repository "simple-base" with configuration
        | key          | value                                                                        |
        | gpgcheck     | 1                                                                            |
        | gpgkey       | file://{context.dnf.fixturesdir}/gpgkeys/keys/default-key/default-key-public |
   # Full policy expands gpgcheck=1 to also set repo_gpgcheck=1; metadata is not signed
   When I execute dnf with args "install labirinto"
   Then the exit code is 1
    And stderr contains "GPG verification is enabled, but GPG signature is not available"


Scenario: Explicit repo_gpgcheck=0 overrides full policy
  Given I configure dnf with
        | key              | value |
        | gpgcheck_policy  | full  |
    And I use repository "simple-base" with configuration
        | key           | value                                                                        |
        | gpgcheck      | 1                                                                            |
        | repo_gpgcheck | 0                                                                            |
        | gpgkey        | file://{context.dnf.fixturesdir}/gpgkeys/keys/default-key/default-key-public |
   # Explicit repo_gpgcheck=0 overrides the full policy expansion
   When I execute dnf with args "install labirinto"
   Then the exit code is 0
    And Transaction is following
        | Action  | Package                       |
        | install | labirinto-0:1.0-1.fc29.x86_64 |


Scenario: Explicit pkg_gpgcheck=0 overrides gpgcheck=1 and allows unsigned packages
  Given I use repository "unsigned" with configuration
        | key          | value |
        | gpgcheck     | 1     |
        | pkg_gpgcheck | 0     |
   # gpgcheck=1 sets pkg_gpgcheck=1, but explicit pkg_gpgcheck=0 overrides
   When I execute dnf with args "install sarcina"
   Then the exit code is 0
    And Transaction is following
        | Action  | Package                     |
        | install | sarcina-0:2.0-1.fc29.x86_64 |


Scenario: gpgcheck=0 disables checks regardless of policy
  Given I configure dnf with
        | key              | value |
        | gpgcheck_policy  | full  |
    And I use repository "unsigned" with configuration
        | key      | value |
        | gpgcheck | 0     |
   # gpgcheck=0 sets pkg_gpgcheck=0; full policy expands to repo_gpgcheck=0
   When I execute dnf with args "install sarcina"
   Then the exit code is 0
    And Transaction is following
        | Action  | Package                     |
        | install | sarcina-0:2.0-1.fc29.x86_64 |


Scenario: gpgcheck=1 with legacy policy rejects unsigned packages
  Given I use repository "unsigned" with configuration
        | key      | value |
        | gpgcheck | 1     |
   # gpgcheck=1 sets pkg_gpgcheck=1, which rejects unsigned packages
   When I execute dnf with args "install sarcina"
   Then the exit code is 1
    And stderr contains "The package is not signed"
