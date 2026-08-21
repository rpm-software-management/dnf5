# Modularity is disabled since RHEL 11
@not.with_os=rhel__ge__11
# Missing module install command
# https://github.com/rpm-software-management/dnf5/issues/146
@xfail
Feature: Installing modules with modular dependencies


Background:
  Given I use repository "dnf-ci-thirdparty-modular"


Scenario: Install a module that requires a module, specifying one stream in Requires
   When I execute dnf with args "module install food-type:meat/default"
   Then the exit code is 0
    And Transaction is following
        | Action                    | Package                   |
        | module-profile-install    | food-type/default         |
        | module-stream-enable      | food-type:meat            |
        | module-stream-enable      | ingredience:chicken       |
    And modules state is following
        | Module        | State     | Stream      | Profiles    |
        | food-type     | enabled   | meat        | default     |
        | ingredience   | enabled   | chicken     |             |


@bz1651701
Scenario: Install a module that requires a module, specifying multiple streams in Requires
   When I execute dnf with args "module install food-type:fruit/default"
   Then the exit code is 0
    And Transaction contains
        | Action                    | Package                   |
        | module-profile-install    | food-type/default         |
        | module-stream-enable      | food-type:fruit           |
    And modules state is following
        | Module        | State     | Stream      | Profiles    |
        | food-type     | enabled   | fruit       | default     |
        | ingredience   | enabled   | orange      |             |


Scenario: Install a module that requires a module, not specifying any stream in Requires
   When I execute dnf with args "module install food-type:edible/default"
   Then the exit code is 0
    And Transaction contains
        | Action                    | Package                   |
        | module-profile-install    | food-type/default         |
        | module-stream-enable      | food-type:edible          |
    And modules state is following
        | Module        | State     | Stream       | Profiles   |
        | food-type     | enabled   | edible       | default    |
        | ingredience   | enabled   | orange       |            |


Scenario: Install a module that requires a module, excluding one stream in Requires
   When I execute dnf with args "module install food-type:vegetarian/default"
   Then the exit code is 0
    And Transaction contains
        | Action                    | Package                   |
        | module-profile-install    | food-type/default         |
        | module-stream-enable      | food-type:vegetarian      |
    And modules state is following
        | Module        | State     | Stream       | Profiles   |
        | food-type     | enabled   | vegetarian   | default    |
        | ingredience   | enabled   | orange       |            |


Scenario: Install a module that requires a module, excluding multiple streams in Requires
   When I execute dnf with args "module install food-type:vegan/default"
   Then the exit code is 0
    And Transaction contains
        | Action                    | Package                   |
        | module-profile-install    | food-type/default         |
        | module-stream-enable      | food-type:vegan           |
    And modules state is following
        | Module        | State     | Stream       | Profiles   |
        | food-type     | enabled   | vegan        | default    |
        | ingredience   | enabled   | orange       |            |


Scenario: Install a module that requires a module, excluding all of the streams in Requires
   When I execute dnf with args "module install food-type:hungry/default"
   Then the exit code is 1
    And Transaction is empty
    And module list is empty


Scenario: Install a module that requires a module, specifying nonexisting stream in Requires
   When I execute dnf with args "module install food-type:dairy/default"
   Then the exit code is 1
    And Transaction is empty
    And module list is empty


Scenario: Install a module that requires a module, excluding nonexisting stream in Requires
   When I execute dnf with args "module install food-type:lactose-free/default"
   Then the exit code is 0
    And Transaction contains
        | Action                    | Package                   |
        | module-profile-install    | food-type/default         |
        | module-stream-enable      | food-type:lactose-free    |
    And modules state is following
        | Module        | State     | Stream       | Profiles   |
        | food-type     | enabled   | lactose-free | default    |
        | ingredience   | enabled   | orange       |            |
