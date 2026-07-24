Feature: Install remove test

Background: Use install-remove repository
  Given I use repository "dnf-ci-install-remove"

# tea requires water and provides hot-beverage
Scenario Outline: Install remove <spec type> that requires only name
   When I execute dnf with args "install <spec>"
   Then the exit code is 0
    And Transaction is following
        | Action        | Package                           |
        | install       | tea-0:1.0-1.x86_64                |
        | install-dep   | water-0:1.0-1.x86_64              |
    And package state is
        | package             | reason     | from_repo             |
        | tea-1.0-1.x86_64    | User       | dnf-ci-install-remove |
        | water-1.0-1.x86_64  | Dependency | dnf-ci-install-remove |
    And dnf5 transaction items for transaction "last" are
        | action  | package            | reason     | repository            |
        | Install | tea-0:1.0-1.x86_64   | User       | dnf-ci-install-remove |
        | Install | water-0:1.0-1.x86_64 | Dependency | dnf-ci-install-remove |
   When I execute dnf with args "install tea"
   Then the exit code is 0
    And Transaction is empty
   When I execute dnf with args "remove tea"
   Then the exit code is 0
    And Transaction is following
        | Action        | Package                           |
        | remove        | tea-0:1.0-1.x86_64                |
        | remove-unused | water-0:1.0-1.x86_64              |

Examples:
    | spec type         | spec          |
    | package           | tea           |
    | provide           | hot-beverage  |


Scenario: Install remove package via rpm
  Given I use repository "dnf-ci-fedora"
  Given I successfully execute dnf with args "install basesystem filesystem"
   Then Transaction is following
        | Action        | Package                           |
        | install       | basesystem-0:11-6.fc29.noarch     |
        | install       | filesystem-0:3.9-2.fc29.x86_64    |
        | install-dep   | setup-0:2.12.1-1.fc29.noarch      |
    And package state is
        | package                      | reason     | from_repo     |
        | basesystem-11-6.fc29.noarch  | User       | dnf-ci-fedora |
        | filesystem-3.9-2.fc29.x86_64 | User       | dnf-ci-fedora |
        | setup-2.12.1-1.fc29.noarch   | Dependency | dnf-ci-fedora |
    And dnf5 transaction items for transaction "last" are
        | action  | package                        | reason     | repository    |
        | Install | basesystem-0:11-6.fc29.noarch  | User       | dnf-ci-fedora |
        | Install | filesystem-0:3.9-2.fc29.x86_64 | User       | dnf-ci-fedora |
        | Install | setup-0:2.12.1-1.fc29.noarch   | Dependency | dnf-ci-fedora |
  Given I successfully execute rpm with args "-e basesystem filesystem setup"
   When I execute dnf with args "install basesystem"
   Then the exit code is 0
    And Transaction is following
        | Action        | Package                           |
        | install       | basesystem-0:11-6.fc29.noarch     |
        | install-dep   | filesystem-0:3.9-2.fc29.x86_64    |
        | install-dep   | setup-0:2.12.1-1.fc29.noarch      |
    And package state is
        | package                      | reason     | from_repo     |
        | basesystem-11-6.fc29.noarch  | User       | dnf-ci-fedora |
        | filesystem-3.9-2.fc29.x86_64 | Dependency | dnf-ci-fedora |
        | setup-2.12.1-1.fc29.noarch   | Dependency | dnf-ci-fedora |
    And dnf5 transaction items for transaction "last" are
        | action  | package                        | reason     | repository    |
        | Install | basesystem-0:11-6.fc29.noarch  | User       | dnf-ci-fedora |
        | Install | filesystem-0:3.9-2.fc29.x86_64 | Dependency | dnf-ci-fedora |
        | Install | setup-0:2.12.1-1.fc29.noarch   | Dependency | dnf-ci-fedora |


# coffee requires water and sugar == 1
@dnf5daemon
Scenario: Install remove package that requires exact version
   When I execute dnf with args "install coffee"
   Then the exit code is 0
    And Transaction is following
        | Action        | Package                           |
        | install       | coffee-0:1.0-1.x86_64             |
        | install-dep   | sugar-0:1.0-1.x86_64              |
        | install-dep   | water-0:1.0-1.x86_64              |
   When I execute dnf with args "remove coffee"
   Then the exit code is 0
    And Transaction is following
        | Action        | Package                           |
        | remove        | coffee-0:1.0-1.x86_64             |
        | remove-unused | sugar-0:1.0-1.x86_64              |
        | remove-unused | water-0:1.0-1.x86_64              |


# chockolate  requires sugar>=2 and milk==1
@dnf5daemon
Scenario: Install remove package that requires version >=
   When I execute dnf with args "install chockolate"
   Then the exit code is 0
    And Transaction is following
        | Action        | Package                           |
        | install       | chockolate-0:1.0-1.x86_64         |
        | install-dep   | sugar-0:2.0-1.x86_64              |
        | install-dep   | milk-0:1.0-1.x86_64               |
   When I execute dnf with args "remove chockolate"
   Then the exit code is 0
    And Transaction is following
        | Action        | Package                           |
        | remove        | chockolate-0:1.0-1.x86_64         |
        | remove-unused | sugar-0:2.0-1.x86_64              |
        | remove-unused | milk-0:1.0-1.x86_64               |


# mate requires water >= 2
Scenario: Install remove package that requires version >=, not satisfiable
   When I execute dnf with args "install mate"
   Then the exit code is 1
    And stderr contains "nothing provides water >= 2 needed by mate-1.0-1.x86_64 from dnf-ci-install-remove"


# both coffee and tea require water
@dnf5daemon
Scenario: Install remove two package with shared dependency
   When I execute dnf with args "install tea coffee"
   Then the exit code is 0
    And Transaction is following
        | Action        | Package                           |
        | install       | coffee-0:1.0-1.x86_64             |
        | install       | tea-0:1.0-1.x86_64                |
        | install-dep   | sugar-0:1.0-1.x86_64              |
        | install-dep   | water-0:1.0-1.x86_64              |
   When I execute dnf with args "remove tea"
   Then the exit code is 0
    And Transaction is following
        | Action        | Package                           |
        | remove        | tea-0:1.0-1.x86_64                |
        | present       | water-0:1.0-1.x86_64              |
   When I execute dnf with args "remove coffee"
   Then the exit code is 0
    And Transaction is following
        | Action        | Package                           |
        | remove        | coffee-0:1.0-1.x86_64             |
        | remove-unused | sugar-0:1.0-1.x86_64              |
        | remove-unused | water-0:1.0-1.x86_64              |


@dnf5daemon
Scenario: Install remove rpm file from local path
   When I execute dnf with args "install {context.scenario.repos_location}/dnf-ci-install-remove/x86_64/water-1.0-1.x86_64.rpm"
   Then the exit code is 0
    And Transaction is following
        | Action        | Package                           |
        | install       | water-0:1.0-1.x86_64              |
   When I execute dnf with args "remove water"
   Then the exit code is 0
    And Transaction is following
        | Action        | Package                           |
        | remove        | water-0:1.0-1.x86_64              |


@dnf5daemon
Scenario: Install remove *.rpm from local path
   When I execute dnf with args "install {context.scenario.repos_location}/dnf-ci-install-remove/x86_64/water_{{still,carbonated}}-1*.rpm"
   Then the exit code is 0
    And Transaction is following
        | Action        | Package                           |
        | install       | water_still-0:1.0-1.x86_64        |
        | install       | water_carbonated-0:1.0-1.x86_64   |
   When I execute dnf with args "remove water*"
   Then the exit code is 0
    And Transaction is following
        | Action        | Package                           |
        | remove        | water_still-0:1.0-1.x86_64        |
        | remove        | water_carbonated-0:1.0-1.x86_64   |


# https://github.com/rpm-software-management/dnf5/issues/1787
Scenario: Install a package that was already installed via rpm
   When I execute rpm with args "-i {context.scenario.repos_location}/dnf-ci-install-remove/x86_64/water-1.0-1.x86_64.rpm"
   Then the exit code is 0
   When I execute dnf with args "install water"
   Then the exit code is 0
   When I execute dnf with args "repoquery --installed --queryformat=%{{reason}} water"
   Then stdout is
        """
        User
        """
    And dnf5 transaction items for transaction "last" are
        | action        | package              | reason | repository |
        | Reason Change | water-0:1.0-1.x86_64 | User   | @System    |


Scenario: Install remove group
   When I execute dnf with args "install @beverages"
   Then the exit code is 0
    And Transaction is following
        | Action        | Package                           |
        | group-install | Beverages                         |
        | install-group | tea-0:1.0-1.x86_64                |
        | install-group | water_still-0:1.0-1.x86_64        |
        | install-dep   | water-0:1.0-1.x86_64              |
   When I execute dnf with args "group list beverages"
   Then the exit code is 0
    And stderr is
        """
        <REPOSYNC>
        """
    And stdout matches line by line
        """
        ID +Name +Installed
        beverages +Beverages +yes
        """
   When I execute dnf with args "install water_carbonated"
   Then the exit code is 0
    And Transaction is following
        | Action        | Package                           |
        | install       | water_carbonated-0:1.0-1.x86_64   |
   When I execute dnf with args "group remove beverages"
   Then the exit code is 0
    And Transaction is following
        | Action        | Package                           |
        | group-remove  | Beverages                         |
        | remove        | tea-0:1.0-1.x86_64                |
        | remove        | water_still-0:1.0-1.x86_64        |
        | remove-unused | water-0:1.0-1.x86_64              |
        | present       | water_carbonated-0:1.0-1.x86_64   |


Scenario: Install remove group with optional packages
   When I execute dnf with args "group install --with-optional beverages"
   Then the exit code is 0
    And Transaction is following
        | Action        | Package                           |
        | group-install | Beverages                         |
        | install-group | tea-0:1.0-1.x86_64                |
        | install-group | water_still-0:1.0-1.x86_64        |
        | install-group | water_carbonated-0:1.0-1.x86_64   |
        | install-dep   | water-0:1.0-1.x86_64              |
   When I execute dnf with args "remove @beverages"
   Then the exit code is 0
    And Transaction is following
        | Action        | Package                           |
        | group-remove  | Beverages                         |
        | remove        | tea-0:1.0-1.x86_64                |
        | remove        | water_still-0:1.0-1.x86_64        |
        | remove        | water_carbonated-0:1.0-1.x86_64   |
        | remove-unused | water-0:1.0-1.x86_64              |


Scenario: Install remove group with already installed package with dependency
   When I execute dnf with args "install tea"
   Then the exit code is 0
    And Transaction is following
        | Action        | Package                           |
        | install       | tea-0:1.0-1.x86_64                |
        | install-dep   | water-0:1.0-1.x86_64              |
   When I execute dnf with args "install @beverages"
   Then the exit code is 0
    And Transaction is following
        | Action        | Package                           |
        | group-install | Beverages                         |
        | install-group | water_still-0:1.0-1.x86_64        |
        | present       | tea-0:1.0-1.x86_64                |
        | present       | water-0:1.0-1.x86_64              |
   When I execute dnf with args "group remove beverages"
   Then the exit code is 0
    And Transaction is following
        | Action        | Package                           |
        | group-remove  | Beverages                         |
        | remove        | water_still-0:1.0-1.x86_64        |
        | present       | tea-0:1.0-1.x86_64                |
        | present       | water-0:1.0-1.x86_64              |


Scenario: Install remove group with already installed package
   When I execute dnf with args "install water_still"
   Then the exit code is 0
    And Transaction is following
        | Action        | Package                           |
        | install       | water_still-0:1.0-1.x86_64        |
   When I execute dnf with args "install @beverages"
   Then the exit code is 0
    And Transaction is following
        | Action        | Package                           |
        | group-install | Beverages                         |
        | install-group | tea-0:1.0-1.x86_64                |
        | install-dep   | water-0:1.0-1.x86_64              |
        | present       | water_still-0:1.0-1.x86_64        |
   When I execute dnf with args "group remove beverages"
   Then the exit code is 0
    And Transaction is following
        | Action        | Package                           |
        | group-remove  | Beverages                         |
        | remove        | tea-0:1.0-1.x86_64                |
        | remove-unused | water-0:1.0-1.x86_64              |
        | present       | water_still-0:1.0-1.x86_64        |


@dnf5daemon
# coffee requires water and sugar, water is user-installed dependency
Scenario: User-installed packages are not removed as unused dependencies
  Given I use repository "dnf-ci-install-remove"
    And I successfully execute dnf with args "install coffee water"
   Then Transaction is following
        | Action        | Package                           |
        | install       | coffee-0:1.0-1.x86_64             |
        | install       | water-0:1.0-1.x86_64              |
        | install-dep   | sugar-0:1.0-1.x86_64              |
   When I execute dnf with args "remove coffee"
   Then the exit code is 0
    And Transaction is following
        | Action        | Package                           |
        | remove        | coffee-0:1.0-1.x86_64             |
        | remove-unused | sugar-0:1.0-1.x86_64              |


@dnf5daemon
Scenario: Install with glob
   When I execute dnf with args "install 'water_*'"
   Then the exit code is 0
    And Transaction is following
        | Action        | Package                           |
        | install       | water_still-0:1.0-1.x86_64        |
        | install       | water_carbonated-0:1.0-1.x86_64   |
   When I execute dnf with args "remove 'water_*'"
   Then the exit code is 0
    And Transaction is following
        | Action        | Package                           |
        | remove        | water_still-0:1.0-1.x86_64        |
        | remove        | water_carbonated-0:1.0-1.x86_64   |


# There is no --advisories=... option for dnf5daemon-client yet.
# @dnf5daemon
# https://github.com/rpm-software-management/dnf5/issues/766
Scenario: Install with glob and empty advisory
   When I execute dnf with args "install 'water_*' --advisories=nonexistent"
   Then the exit code is 1
    And stderr is
    """
    <REPOSYNC>
    No advisory found matching the requested name: "nonexistent"
    """

#Scenario: Install remove package from url
