# rpm on RHEL7 does not support rich dependencies
@not.with_os=rhel__eq__7
Feature: Rich dependencies testing


Background: Use dnf-ci-rich repository
    Given I use repository "dnf-ci-rich"


# pancake
#   requires: (milk or soymilk)
#   recommends: milk

Scenario: Required one of packages with recommended
     When I execute dnf with args "install pancake"
     Then the exit code is 0
      And Transaction is following
        | Action        | Package                           |
        | install       | pancake-0:1.0-1.x86_64            |
        | install-dep   | milk-0:1.0-1.x86_64               |


Scenario: Required one of packages with recommended, even if the other is installed
    Given I execute dnf with args "install soymilk"
     Then the exit code is 0
      And Transaction is following
        | Action        | Package                           |
        | install       | soymilk-0:1.0-1.x86_64            |
     When I execute dnf with args "install pancake"
     Then the exit code is 0
      And Transaction is following
        | Action        | Package                           |
        | install       | pancake-0:1.0-1.x86_64            |
        | install-weak  | milk-0:1.0-1.x86_64               |


# soup
#   requires: (water and (cream if dill))

Scenario: Conditional dependency (condition is not met)
     When I execute dnf with args "install soup"
     Then the exit code is 0
      And Transaction is following
        | Action        | Package                           |
        | install       | soup-0:1.0-1.x86_64               |
        | install-dep   | water-0:1.0-1.x86_64              |


Scenario: Conditional dependency (condition is met)
    Given I execute dnf with args "install dill"
     Then the exit code is 0
      And Transaction is following
        | Action        | Package                           |
        | install       | dill-0:1.0-1.x86_64               |
     When I execute dnf with args "install soup"
     Then the exit code is 0
      And Transaction is following
        | Action        | Package                           |
        | install       | soup-0:1.0-1.x86_64               |
        | install-dep   | water-0:1.0-1.x86_64              |
        | install-dep   | cream-0:1.0-1.x86_64              |


# sauce
#   requires: (flour and ((milk or cream) if dill))
# milk
#   conflicts: water

Scenario: Conditional dependency with "one of" and conflict (condition is not met)
     When I execute dnf with args "install sauce"
     Then the exit code is 0
      And Transaction is following
        | Action        | Package                           |
        | install       | sauce-0:1.0-1.x86_64              |
        | install-dep   | flour-0:1.0-1.x86_64              |


Scenario: Conditional dependency with "one of" and conflict (condition is met)
    Given I execute dnf with args "install dill water"
     Then the exit code is 0
      And Transaction is following
        | Action        | Package                           |
        | install       | water-0:1.0-1.x86_64              |
        | install       | dill-0:1.0-1.x86_64               |
     When I execute dnf with args "install sauce"
     Then the exit code is 0
      And Transaction is following
        | Action        | Package                           |
        | install       | sauce-0:1.0-1.x86_64              |
        | install-dep   | flour-0:1.0-1.x86_64              |
        | install-dep   | cream-0:1.0-1.x86_64              |


# porridge
#   requires: (milk if (flour or oat))

Scenario: Conditional dependency on "one of" packages
    Given I execute dnf with args "install oat"
     Then the exit code is 0
      And Transaction is following
        | Action        | Package                           |
        | install       | oat-0:1.0-1.x86_64                |
     When I execute dnf with args "install porridge"
     Then the exit code is 0
      And Transaction is following
        | Action        | Package                           |
        | install       | porridge-0:1.0-1.x86_64           |
        | install-dep   | milk-0:1.0-1.x86_64               |


Scenario: Gradually removing conditional dependencies
    Given I execute dnf with args "install porridge oat"
     Then the exit code is 0
      And Transaction is following
        | Action        | Package                           |
        | install       | porridge-0:1.0-1.x86_64           |
        | install       | oat-0:1.0-1.x86_64                |
        | install-dep   | milk-0:1.0-1.x86_64               |
     When I execute dnf with args "remove oat"
     Then the exit code is 0
      And Transaction is following
        | Action        | Package                           |
        | remove        | oat-0:1.0-1.x86_64                |
     When I execute dnf with args "remove milk"
     Then the exit code is 0
      And Transaction is following
        | Action        | Package                           |
        | remove        | milk-0:1.0-1.x86_64               |
        | unchanged     | porridge-0:1.0-1.x86_64           |


@not.with_dnf=4
Scenario: Command-line installation if condition is not met
    Given I execute dnf with args "install '(milk if flour)'"
     Then the exit code is 0
      And Transaction is empty


@not.with_dnf=4
Scenario: Command-line installation if condition is met
    Given I execute dnf with args "install flour"
     Then the exit code is 0
      And Transaction is following
        | Action        | Package                           |
        | install       | flour-0:1.0-1.x86_64              |
    Given I execute dnf with args "install '(milk if flour)'"
     Then the exit code is 0
      And Transaction is following
        | Action        | Package                           |
        | install       | milk-0:1.0-1.x86_64               |


@not.with_dnf=4
Scenario: Command-line installation if condition is not met, but package explicitly listed
    Given I execute dnf with args "install '(milk if flour)' flour"
     Then the exit code is 0
      And Transaction is following
        | Action        | Package                           |
        | install       | flour-0:1.0-1.x86_64              |
        | install       | milk-0:1.0-1.x86_64               |


@bz2185061
Scenario: Install pkg with weak complex deps and don't install the condition pkg with broken deps
    Given I execute dnf with args "install breakfast"
     Then the exit code is 0
      And Transaction is following
        | Action        | Package                           |
        | install       | breakfast-0:1.0-1.x86_64          |
        | install-weak  | water-0:1.0-1.x86_64              |
