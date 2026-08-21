# Modularity is disabled since RHEL 11
@not.with_os=rhel__ge__11
Feature: Not fail if conflicts in module metadata.

  @bz1656019
  Scenario: Enabling two repository with conflicting defaults - only remove defaults
    Given I use repository "dnf-ci-thirdparty-modular-updates"
      And I use repository "dnf-ci-thirdparty-modular-updates-conflict"
     When I execute dnf with args "module list"
     Then the exit code is 0
      And module list is
      | Name        | Stream     | Profiles |
      | cookbook    | 1          | ham-and-eggs, orange-juice, axe-soup |
      | ingredience | chicken    | default |
      | ingredience | egg        | default |
      | ingredience | orange     | default |
      | ingredience | strawberry | default |

     When I execute dnf with args "module list --disablerepo=dnf-ci-thirdparty-modular-updates-conflict"
     Then the exit code is 0
      And module list is
      | Name        | Stream     | Profiles |
      | cookbook    | 1          | ham-and-eggs, orange-juice, axe-soup |
      | ingredience | chicken    | default |
      | ingredience | egg        | default |
      | ingredience | orange [d] | default |
      | ingredience | strawberry | default |

     When I execute dnf with args "module list --disablerepo=dnf-ci-thirdparty-modular-updates"
     Then the exit code is 0
      And module list is
      | Name        | Stream      | Profiles|
      | ingredience | chicken [d] | default |
      | ingredience | egg         | default |
      | ingredience | orange      | default |


Scenario: Two streams that have matching NSVCA but different content (conflict) are dropped by libmodulemd, but defaults are unaffected
Given I use repository "dnf-ci-thirdparty-modular-updates"
  And I use repository "dnf-ci-thirdparty-modular-updates-duplicate"
 When I execute dnf with args "module list"
 Then the exit code is 0
  And module list is
      | Name        | Stream     | Profiles                             |
      | cookbook    | 1          | ham-and-eggs, orange-juice, axe-soup |
      | ingredience | chicken    | default                              |
      | ingredience | egg        | default                              |
      | ingredience | orange [d] | default                              |
      | ingredience | strawberry | default                              |
