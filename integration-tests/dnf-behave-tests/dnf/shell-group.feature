# missing shell command: https://github.com/rpm-software-management/dnf5/issues/153
@xfail
Feature: Shell group


# DNF-CI-Testgroup structure in dnf-ci-thirdparty:
#   mandatory: filesystem (requires setup)
#   default: lame (requires lame-libs)
#   optional: flac
#   conditional: wget, requires filesystem-content

# DNF-CI-Testgroup structure in dnf-ci-thirdparty-updates:
#   mandatory: filesystem (requires setup)
#   mandatory: libzstd (requires capabilities provided by glibc)
#   default: lame (requires lame-libs)
#   optional: flac
#   conditional: wget, requires filesystem-content


Scenario: Using dnf shell, install a package group
  Given I use repository "dnf-ci-fedora"
    And I use repository "dnf-ci-thirdparty"
   When I open dnf shell session
    And I execute in dnf shell "group install DNF-CI-Testgroup"
    And I execute in dnf shell "run"
   Then Transaction is following
        | Action        | Package                                     |
        | install-group | filesystem-0:3.9-2.fc29.x86_64              |
        | install-group | lame-0:3.100-4.fc29.x86_64                  |
        | install-dep   | setup-0:2.12.1-1.fc29.noarch                |
        | install-dep   | lame-libs-0:3.100-4.fc29.x86_64             |
        | group-install | DNF-CI-Testgroup                            |
   When I execute in dnf shell "exit"
   Then stdout contains "Leaving Shell"


Scenario Outline: Using dnf shell, upgrade a package group using alias <upgrade alias>
  Given I use repository "dnf-ci-fedora"
    And I use repository "dnf-ci-thirdparty"
    And I use repository "dnf-ci-fedora-updates" with configuration
        | key     | value |
        | enabled | 0     |
    And I use repository "dnf-ci-thirdparty-updates" with configuration
        | key     | value |
        | enabled | 0     |
   When I open dnf shell session
    And I execute in dnf shell "group install DNF-CI-Testgroup"
    And I execute in dnf shell "run"
   Then Transaction is following
        | Action        | Package                                     |
        | install-group | filesystem-0:3.9-2.fc29.x86_64              |
        | install-group | lame-0:3.100-4.fc29.x86_64                  |
        | install-dep   | setup-0:2.12.1-1.fc29.noarch                |
        | install-dep   | lame-libs-0:3.100-4.fc29.x86_64             |
        | group-install | DNF-CI-Testgroup                            |
   When I execute in dnf shell "repo enable dnf-ci-fedora-updates dnf-ci-thirdparty-updates"
    And I execute in dnf shell "group <upgrade alias> DNF-CI-Testgroup"
    And I execute in dnf shell "run"
   Then Transaction is following
        | Action        | Package                                     |
        | upgrade       | lame-0:3.100-5.fc29.x86_64                  |
        | upgrade       | lame-libs-0:3.100-5.fc29.x86_64             |
        | install-group | libzstd-0:1.3.6-1.fc29.x86_64               |
        | install-dep   | glibc-0:2.28-26.fc29.x86_64                 |
        | install-dep   | glibc-common-0:2.28-26.fc29.x86_64          |
        | install-dep   | glibc-all-langpacks-0:2.28-26.fc29.x86_64   |
        | install-dep   | basesystem-0:11-6.fc29.noarch               |
        | group-upgrade | DNF-CI-Testgroup                            |
   When I execute in dnf shell "exit"
   Then stdout contains "Leaving Shell"

Examples:
  | upgrade alias       |
  | upgrade             |
  | update              |


# TODO(nsella) Unknown argument "shell" for command "microdnf"
Scenario: Using dnf shell, remove a package group
  Given I use repository "dnf-ci-fedora"
    And I use repository "dnf-ci-thirdparty"
   When I open dnf shell session
    And I execute in dnf shell "group install DNF-CI-Testgroup"
    And I execute in dnf shell "run"
   Then Transaction is following
        | Action        | Package                                     |
        | install-group | filesystem-0:3.9-2.fc29.x86_64              |
        | install-group | lame-0:3.100-4.fc29.x86_64                  |
        | install-dep   | setup-0:2.12.1-1.fc29.noarch                |
        | install-dep   | lame-libs-0:3.100-4.fc29.x86_64             |
        | group-install | DNF-CI-Testgroup                            |
   When I execute in dnf shell "group remove DNF-CI-Testgroup"
    And I execute in dnf shell "run"
   Then Transaction is following
        | Action        | Package                                     |
        | remove        | filesystem-0:3.9-2.fc29.x86_64              |
        | remove        | lame-0:3.100-4.fc29.x86_64                  |
        | remove-unused | setup-0:2.12.1-1.fc29.noarch                |
        | remove-unused | lame-libs-0:3.100-4.fc29.x86_64             |
        | group-remove  | DNF-CI-Testgroup                            |
   When I execute in dnf shell "exit"
   Then stdout contains "Leaving Shell"


Scenario: Using dnf shell, fail to install a non-existent package group
  Given I use repository "dnf-ci-fedora"
   When I open dnf shell session
    And I execute in dnf shell "group install NoSuchGroup"
   Then stdout contains "Module or Group '.*NoSuchGroup.*' is not available\."
    And I execute in dnf shell "run"
   Then Transaction is empty
   When I execute in dnf shell "exit"
   Then stdout contains "Leaving Shell"


Scenario: Using dnf shell, fail to remove a non-existent package group
  Given I use repository "dnf-ci-fedora"
   When I open dnf shell session
    And I execute in dnf shell "group remove NoSuchGroup"
   Then stdout contains "Group '.*NoSuchGroup.*' is not installed\."
    And I execute in dnf shell "run"
   Then Transaction is empty
   When I execute in dnf shell "exit"
   Then stdout contains "Leaving Shell"
