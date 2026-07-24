# Modularity is disabled since RHEL 11
@not.with_os=rhel__ge__11
Feature: Dependency resolution must occur to determine the appropriate dependent stream+context to use

Background:
  Given I use repository "dnf-ci-thirdparty-modular"

# Missing module install command
# https://github.com/rpm-software-management/dnf5/issues/146
@xfail
Scenario: Appropriate context is selected depending on the enabled required module stream
   When I execute dnf with args "module enable biotope:wood"
   Then the exit code is 0
    And modules state is following
        | Module       | State     | Stream     | Profiles  |
        | biotope      | enabled   | wood       |           |
   When I execute dnf with args "module install berry:raspberry/default"
   Then the exit code is 0
    And Transaction is following
        | Action                    | Package                       |
        | module-stream-enable      | berry:raspberry               |
        | module-profile-install    | berry/default                 |
        | install-group             | raspberry-0:1.0-1.wood.x86_64 |


# Missing module install command
# https://github.com/rpm-software-management/dnf5/issues/146
@xfail
Scenario: Appropriate context is selected depending on the enabled required module stream - cross check
   When I execute dnf with args "module enable biotope:garden"
   Then the exit code is 0
    And modules state is following
        | Module       | State     | Stream     | Profiles  |
        | biotope      | enabled   | garden     |           |
   When I execute dnf with args "module install berry:raspberry/default"
   Then the exit code is 0
    And Transaction is following
        | Action                    | Package                           |
        | module-stream-enable      | berry:raspberry                   |
        | module-profile-install    | berry/default                     |
        | install-group             | raspberry-0:1.0-1.garden.x86_64   |


# Missing module install command
# https://github.com/rpm-software-management/dnf5/issues/146
@xfail
Scenario: Any suitable context is selected when more options are possible
   When I execute dnf with args "module install berry:raspberry/default"
   Then the exit code is 0
    And modules state is following
        | Module       | State     | Stream     | Profiles  |
        | berry        | enabled   | raspberry  | default   |
        | biotope      | enabled   | ?          |           |
   When I execute rpm with args "-q raspberry"
   Then the exit code is 0


@not.with_os=rhel__eq__8
Scenario: An error is printed with no stream and context is possible to enable
   When I execute dnf with args "module enable biotope:pond"
   Then the exit code is 0
   When I execute dnf with args "module enable berry:raspberry"
   Then the exit code is 1
    And stderr contains "Modular dependency problems:"
    And stderr contains "module biotope:pond.* conflicts with module\(biotope\) provided by biotope:wood:1:\.x86_64"
    And stderr contains "module biotope:wood:1:\.x86_64 conflicts with module\(biotope\) provided by biotope:pond:1:\.x86_64"


# Missing module install command
# https://github.com/rpm-software-management/dnf5/issues/146
@xfail
@not.with_os=rhel__eq__8
@bz1670496
Scenario: An error is printed when trying to install different context
   When I execute dnf with args "module enable biotope:pond"
   Then the exit code is 0
   When I execute dnf with args "module install berry:raspberry/default"
   Then the exit code is 1
    And stderr contains "Modular dependency problems:"
    And stderr contains "module biotope:wood.* conflicts with module\(biotope\) provided by biotope:pond:1:\.x86_64"
    And stderr contains "module biotope:pond.* conflicts with module\(biotope\) provided by biotope:wood:1:\.x86_64"
