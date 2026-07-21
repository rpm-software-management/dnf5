# Modularity is disabled since RHEL 11
@not.with_os=rhel__ge__11
Feature: Enable module streams with modular dependencies


Background:
  Given I use repository "dnf-ci-thirdparty-modular"


# Module defaults from /etc/dnf/modules.defaults.d/ are not loaded
# https://github.com/rpm-software-management/dnf5/issues/1853
@xfail
Scenario: Enabling a default stream depending on a default stream
  Given I create file "/etc/dnf/modules.defaults.d/defaults.yaml" with
        """
        ---
        document: modulemd-defaults
        version: 1
        data:
            module: beverage
            stream: soda
            profiles:
                default: [default]
        ...
        ---
        document: modulemd-defaults
        version: 1
        data:
            module: fluid
            stream: water
            profiles:
                default: [default]
        ...
        """
   When I execute dnf with args "module enable beverage"
   Then the exit code is 0
    And Transaction is following
        | Action                   | Package                |
        | module-stream-enable     | beverage:soda          |
        | module-stream-enable     | fluid:water            |
    And modules state is following
        | Module       | State     | Stream     | Profiles  |
        | beverage     | enabled   | soda       |           |
        | fluid        | enabled   | water      |           |


# Module defaults from /etc/dnf/modules.defaults.d/ are not loaded
# https://github.com/rpm-software-management/dnf5/issues/1853
@xfail
@bz1648839
Scenario: Enabling a default stream depending on a non-default stream
  Given I create file "/etc/dnf/modules.defaults.d/defaults.yaml" with
        """
        ---
        document: modulemd-defaults
        version: 1
        data:
            module: beverage
            stream: soda
            profiles:
                default: [default]
        ...
        ---
        document: modulemd-defaults
        version: 1
        data:
            module: fluid
            stream: oil
            profiles:
                default: [default]
        ...
        """
   When I execute dnf with args "module enable beverage"
   Then the exit code is 0
    And Transaction is following
        | Action                   | Package                |
        | module-stream-enable     | beverage:soda          |
        | module-stream-enable     | fluid:water            |
    And modules state is following
        | Module       | State     | Stream     | Profiles  |
        | beverage     | enabled   | soda       |           |
        | fluid        | enabled   | water      |           |

Scenario: Enabling a non-default stream depending on a default stream
  Given I create file "/etc/dnf/modules.defaults.d/defaults.yaml" with
        """
        ---
        document: modulemd-defaults
        version: 1
        data:
            module: beverage
            stream: beer
            profiles:
                default: [default]
        ...
        ---
        document: modulemd-defaults
        version: 1
        data:
            module: fluid
            stream: water
            profiles:
                default: [default]
        ...
        """
   When I execute dnf with args "module enable beverage:soda"
   Then the exit code is 0
    And Transaction is following
        | Action                   | Package                |
        | module-stream-enable     | beverage:soda          |
        | module-stream-enable     | fluid:water            |
    And modules state is following
        | Module       | State     | Stream     | Profiles  |
        | beverage     | enabled   | soda       |           |
        | fluid        | enabled   | water      |           |


# rely on merging bz1649261 fix
Scenario: Enabling a disabled stream depending on a default stream
  Given I create file "/etc/dnf/modules.defaults.d/defaults.yaml" with
        """
        ---
        document: modulemd-defaults
        version: 1
        data:
            module: beverage
            stream: soda
            profiles:
                default: [default]
        ...
        ---
        document: modulemd-defaults
        version: 1
        data:
            module: fluid
            stream: water
            profiles:
                default: [default]
        ...
        """
   When I execute dnf with args "module disable beverage"
   Then the exit code is 0
    And Transaction is following
        | Action                   | Package                |
        | module-disable           | beverage               |
   When I execute dnf with args "module enable beverage:soda"
   Then the exit code is 0
    And Transaction is following
        | Action                   | Package                |
        | module-stream-enable     | beverage:soda          |
        | module-stream-enable     | fluid:water            |
    And modules state is following
        | Module       | State     | Stream     | Profiles  |
        | beverage     | enabled   | soda       |           |
        | fluid        | enabled   | water      |           |


# rely on merging bz1649261 fix
Scenario: Enabling a disabled stream depending on a non-default stream
  Given I create file "/etc/dnf/modules.defaults.d/defaults.yaml" with
        """
        ---
        document: modulemd-defaults
        version: 1
        data:
            module: beverage
            stream: beer
            profiles:
                default: [default]
        ...
        ---
        document: modulemd-defaults
        version: 1
        data:
            module: fluid
            stream: oil
            profiles:
                default: [default]
        ...
        """
   When I execute dnf with args "module disable beverage"
   Then the exit code is 0
    And Transaction is following
        | Action                   | Package                |
        | module-disable           | beverage               |
   When I execute dnf with args "module enable beverage:soda"
   Then the exit code is 0
    And Transaction is following
        | Action                   | Package                |
        | module-stream-enable     | beverage:soda          |
        | module-stream-enable     | fluid:water            |
    And modules state is following
        | Module       | State     | Stream     | Profiles  |
        | beverage     | enabled   | soda       |           |
        | fluid        | enabled   | water      |           |


@bz1622566
Scenario: Enabling a non-default stream depending on a non-default stream
   When I execute dnf with args "module enable food-type:meat"
   Then the exit code is 0
    And Transaction is following
        | Action                   | Package                |
        | module-stream-enable     | food-type:meat         |
        | module-stream-enable     | ingredience:chicken    |
    And modules state is following
        | Module       | State     | Stream     | Profiles  |
        | food-type    | enabled   | meat       |           |
        | ingredience  | enabled   | chicken    |           |


Scenario: Enable a module and its dependencies by specifying profile
   When I execute dnf with args "module enable food-type:meat/default"
   Then the exit code is 0
    And Transaction is following
        | Action                   | Package                |
        | module-stream-enable     | food-type:meat         |
        | module-stream-enable     | ingredience:chicken    |
    And modules state is following
        | Module       | State     | Stream     | Profiles  |
        | food-type    | enabled   | meat       |           |
        | ingredience  | enabled   | chicken    |           |


# Commented out in case the behaviour is reverted, complementary scenario follows next
#@xfail @bz1647804
#Scenario: Disable a module and all modules that are dependent on it
#   When I execute dnf with args "module enable food-type:meat"
#   Then the exit code is 0
#    And Transaction is following
#        | Action                   | Package                |
#        | module-stream-enable     | food-type:meat         |
#        | module-stream-enable     | ingredience:chicken    |
#    And modules state is following
#        | Module       | State     | Stream     | Profiles  |
#        | food-type    | enabled   | meat       |           |
#        | ingredience  | enabled   | chicken    |           |
#   When I execute dnf with args "module disable ingredience:chicken"
#   Then the exit code is 0
#    And Transaction is following
#        | Action                   | Package                |
#        | module-disable           | food-type              |
#        | module-disable           | ingredience            |
#    And modules state is following
#        | Module       | State     | Stream     | Profiles  |
#        | food-type    | disabled  |            |           |
#        | ingredience  | disabled  |            |           |


@not.with_os=rhel__eq__8
Scenario: Module cannot be disabled if there are other enabled streams requiring it
   When I execute dnf with args "module enable food-type:meat"
   Then the exit code is 0
    And Transaction is following
        | Action                   | Package                |
        | module-stream-enable     | food-type:meat         |
        | module-stream-enable     | ingredience:chicken    |
    And modules state is following
        | Module       | State     | Stream     | Profiles  |
        | food-type    | enabled   | meat       |           |
        | ingredience  | enabled   | chicken    |           |
   When I execute dnf with args "module disable ingredience:chicken"
   Then the exit code is 1
    And stderr contains "Failed to resolve the transaction:"
    And stderr contains "Modular dependency problems:"
    And stderr contains "Problem: module food-type:meat:1:.x86_64 requires module\(ingredience:chicken\)"
    And modules state is following
        | Module       | State     | Stream     | Profiles  |
        | food-type    | enabled   | meat       |           |
        | ingredience  | enabled   | chicken    |           |


Scenario: Enable the default stream of a module and its dependencies
   When I execute dnf with args "module enable food-type"
   Then the exit code is 0
    And Transaction is following
        | Action                   | Package                |
        | module-stream-enable     | food-type:fruit        |
        | module-stream-enable     | ingredience:orange     |
    And modules state is following
        | Module       | State     | Stream     | Profiles  |
        | food-type    | enabled   | fruit      |           |
        | ingredience  | enabled   | orange     |           |


# Commented out in case the behaviour is reverted, complementary scenario follows next
#@xfail @bz1648882
#Scenario: Enable a disabled module and its dependencies
#   When I execute dnf with args "module disable food-type:meat ingredience:chicken"
#   Then the exit code is 0
#    And Transaction is following
#        | Action                   | Package                |
#        | module-disable           | food-type              |
#        | module-disable           | ingredience            |
#    And modules state is following
#        | Module       | State     | Stream     | Profiles  |
#        | food-type    | disabled  |            |           |
#        | ingredience  | disabled  |            |           |
#   When I execute dnf with args "module enable food-type:meat"
#   Then the exit code is 0
#    And Transaction is following
#        | Action                   | Package                |
#        | module-enable            | food-type              |
#        | module-enable            | ingredience            |
#    And modules state is following
#        | Module       | State     | Stream     | Profiles  |
#        | food-type    | enabled   | meat       |           |
#        | ingredience  | enabled   | chicken    |           |


# rely on merging bz1649261 fix
@not.with_os=rhel__eq__8
Scenario: Cannot enable a stream depending on a disabled module
   When I execute dnf with args "module disable food-type:meat ingredience:chicken"
   Then the exit code is 0
    And Transaction is following
        | Action                   | Package                |
        | module-disable           | food-type              |
        | module-disable           | ingredience            |
    And modules state is following
        | Module       | State     | Stream     | Profiles  |
        | food-type    | disabled  |            |           |
        | ingredience  | disabled  |            |           |
   When I execute dnf with args "module enable food-type:meat"
   Then the exit code is 1
    And stderr contains "Failed to resolve the transaction:"
    And stderr contains "Modular dependency problems:"
    And stderr contains "module ingredience:chicken:1:.x86_64 is disabled"
    And modules state is following
        | Module       | State     | Stream     | Profiles  |
        | food-type    | disabled  |            |           |
        | ingredience  | disabled  |            |           |


Scenario: Enable a module stream dependent on a module with a default stream
   When I execute dnf with args "module enable food-type:edible"
   Then the exit code is 0
    And Transaction is following
        | Action                   | Package                |
        | module-stream-enable     | food-type:edible       |
        | module-stream-enable     | ingredience:orange     |
    And modules state is following
        | Module       | State     | Stream     | Profiles  |
        | food-type    | enabled   | edible     |           |
        | ingredience  | enabled   | orange     |           |


Scenario: Enable a module stream dependent on a module without default stream
  Given I use repository "dnf-ci-fedora-modular-updates"
   When I execute dnf with args "module enable nodejs:12"
   Then the exit code is 0
    And Transaction contains
        | Action                   | Package                |
        | module-stream-enable     | nodejs:12              |
    And modules state is following
        | Module       | State     | Stream     | Profiles  |
        | nodejs       | enabled   | 12         |           |
        | postgresql   | enabled   | ?          |           |
