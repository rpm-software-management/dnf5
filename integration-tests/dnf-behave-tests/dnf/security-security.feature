Feature: Test security options for update


Background: Use repository with advisories
  Given I use repository "dnf-ci-security"
   When I execute dnf with args "install security_A-1.0-1 security_B-1.0-1"
   Then the exit code is 0
    And Transaction is following
        | Action        | Package                   |
        | install       | security_A-0:1.0-1.x86_64 |
        | install       | security_B-0:1.0-1.x86_64 |


Scenario: Security check-upgrade when there are no such updates
  Given I drop repository "dnf-ci-security"
    And I use repository "dnf-ci-fedora"
   When I execute dnf with args "check-update --security"
   Then the exit code is 0
    And stdout does not contain "security_A"
    And stdout does not contain "security_B"


Scenario: Security check-upgrade when there are such updates
   When I execute dnf with args "check-upgrade --security"
   Then the exit code is 100
    And stdout contains "security_A.*1\.0-4\s+dnf-ci-security"
    And stdout does not contain "security_B"


@bz1918475
Scenario: Security update
   When I execute dnf with args "upgrade-minimal --security"
   Then the exit code is 0
    And Transaction is following
        | Action        | Package                   |
        | upgrade       | security_A-0:1.0-3.x86_64 |
  When I execute dnf with args "upgrade --security"
  Then the exit code is 0
    And Transaction is empty


@bz1918475
Scenario: Security upgrade-minimal when exact version is not available
   When I execute dnf with args "upgrade-minimal --security -x security_A-0:1.0-3.x86_64"
   Then the exit code is 0
    And Transaction is following
        | Action        | Package                   |
        | upgrade       | security_A-0:1.0-4.x86_64 |


@bz1918475
Scenario: Security update with priority setting
  Given I use repository "dnf-ci-security-priority" with configuration
      | key           | value   |
      | priority      | 1       |
   When I execute dnf with args "upgrade --security "
   Then the exit code is 0
    And Transaction is following
        | Action        | Package                     |
        | upgrade       | security_A-0:1.0-3.8.x86_64 |


@bz1918475
Scenario Outline: Security upgrade-minimal with priority setting and args: <Args>
  Given I use repository "dnf-ci-security-priority" with configuration
      | key           | value   |
      | priority      | 1       |
   When I execute dnf with args "upgrade-minimal <Args> --security "
   Then the exit code is 0
    And Transaction is following
        | Action        | Package                     |
        | upgrade       | security_A-0:1.0-3.1.x86_64 |

Examples:
    | Args       |
    |            |
    | security_A |


Scenario Outline: Security <command>
   When I execute dnf with args "<command> --security"
   Then the exit code is 0
    And Transaction is following
        | Action        | Package                   |
        | upgrade       | <package>                 |

Examples:
    | command                    | package                   |
    | upgrade                    | security_A-0:1.0-4.x86_64 |
    | upgrade-minimal            | security_A-0:1.0-3.x86_64 |
    | upgrade-minimal security_A | security_A-0:1.0-3.x86_64 |


Scenario Outline: Security <command> with bzs explicitly mentioned
   When I execute dnf with args "<command> --security --bz 123 --bzs=234,345"
   Then the exit code is 0
    And Transaction contains
        | Action        | Package                   |
        | upgrade       | <package>                 |

Examples:
    | command           | package                   |
    | upgrade           | security_A-0:1.0-4.x86_64 |
    | upgrade-minimal   | security_A-0:1.0-4.x86_64 |
