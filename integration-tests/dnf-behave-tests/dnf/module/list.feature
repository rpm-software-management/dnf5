# Modularity is disabled since RHEL 11
@not.with_os=rhel__ge__11
# Missing module install command
# https://github.com/rpm-software-management/dnf5/issues/146
@xfail
Feature: Modules listing

Background:
  Given I use repository "dnf-ci-fedora-modular"
    And I use repository "dnf-ci-fedora-modular-updates"
    And I use repository "dnf-ci-fedora"
    And I successfully execute dnf with args "module enable nodejs:8"
    And I successfully execute dnf with args "module install nodejs:8/minimal"
    And I successfully execute dnf with args "module enable postgresql:11"
    And I successfully execute dnf with args "module install postgresql:11/client"

Scenario: I can list all available modules
   When I execute dnf with args "module list"
   Then the exit code is 0
    And stdout contains "Javascript runtime"
    And module list is
        | Repository            | Name      | Stream     | Profiles                              |
        | dnf-ci-fedora-modular | meson     | master [d] | default [d]                           |
        | dnf-ci-fedora-modular | nodejs    | 5          | development, minimal, default         |
        | dnf-ci-fedora-modular | nodejs    | 8 [d][e]   | development, minimal [i], default [d] |
        | dnf-ci-fedora-modular | nodejs    | 10         | development, minimal, default [d]     |
        | dnf-ci-fedora-modular | nodejs    | 11         | development, minimal, default         |
        | dnf-ci-fedora-modular | postgresql| 9.6 [d]    | client, server, default [d]           |
        | dnf-ci-fedora-modular | postgresql| 6          | client, server, default               |
        | dnf-ci-fedora-modular | ninja     | master [d] | default [d]                           |
        | dnf-ci-fedora-modular | ninja     | development| default [d]                           |
        | dnf-ci-fedora-modular | ninja     | legacy     | default                               |
        | dnf-ci-fedora-modular | dwm       | 6.0        | default                               |
        | dnf-ci-fedora-modular-updates | nodejs        | 8 [d][e]  | development, minimal [i], default [d] |
        | dnf-ci-fedora-modular-updates | nodejs        | 10        | development, minimal, default [d]     |
        | dnf-ci-fedora-modular-updates | nodejs        | 11        | development, minimal, default         |
        | dnf-ci-fedora-modular-updates | nodejs        | 12        | development, minimal, default         |
        | dnf-ci-fedora-modular-updates | postgresql    | 9.6 [d]   | client, server, default [d]           |
        | dnf-ci-fedora-modular-updates | postgresql    | 10        | client, server, default               |
        | dnf-ci-fedora-modular-updates | postgresql    | 11 [e]    | client [i], server, default           |


Scenario: I can list modules by glob
   When I execute dnf with args "module list node*"
   Then the exit code is 0
    And module list is
        | Repository                    | Name          | Stream    | Profiles                              |
        | dnf-ci-fedora-modular         | nodejs        | 5         | development, minimal, default         |
        | dnf-ci-fedora-modular         | nodejs        | 8 [d][e]  | development, minimal [i], default [d] |
        | dnf-ci-fedora-modular         | nodejs        | 10        | development, minimal, default [d]     |
        | dnf-ci-fedora-modular         | nodejs        | 11        | development, minimal, default         |
        | dnf-ci-fedora-modular-updates | nodejs        | 8 [d][e]  | development, minimal [i], default [d] |
        | dnf-ci-fedora-modular-updates | nodejs        | 10        | development, minimal, default [d]     |
        | dnf-ci-fedora-modular-updates | nodejs        | 11        | development, minimal, default         |
        | dnf-ci-fedora-modular-updates | nodejs        | 12        | development, minimal, default         |


Scenario: I can list modules by glob:glob
   When I execute dnf with args "module list node*:*0"
   Then the exit code is 0
    And module list is
        | Repository                    | Name          | Stream    | Profiles                              |
        | dnf-ci-fedora-modular         | nodejs        | 10        | development, minimal, default [d]     |
        | dnf-ci-fedora-modular-updates | nodejs        | 10        | development, minimal, default [d]     |


Scenario: I can list enabled modules
   When I execute dnf with args "module list --enabled"
   Then the exit code is 0
    And module list is
        | Repository                    | Name          | Stream    | Profiles                              |
        | dnf-ci-fedora-modular         | nodejs        | 8 [d][e]  | development, minimal [i], default [d] |
        | dnf-ci-fedora-modular-updates | nodejs        | 8 [d][e]  | development, minimal [i], default [d] |
        | dnf-ci-fedora-modular-updates | postgresql    | 11 [e]    | client [i], server, default           |


Scenario: I can list installed modules
   When I execute dnf with args "module list --installed"
   Then the exit code is 0
    And module list is
        | Repository                    | Name          | Stream    | Profiles                              |
        | dnf-ci-fedora-modular         | nodejs        | 8 [d][e]  | development, minimal [i], default [d] |
        | dnf-ci-fedora-modular-updates | nodejs        | 8 [d][e]  | development, minimal [i], default [d] |
        | dnf-ci-fedora-modular-updates | postgresql    | 11 [e]    | client [i], server, default           |


@bz1647382
Scenario: I can list disabled modules (when there are no disabled modules)
   When I execute dnf with args "module list --disabled"
   Then the exit code is 0
    And stderr is empty
    And module list is empty

@bz1647382
Scenario: Get error message when list of non-existent module is requested
   When I execute dnf with args "module list non-existing-module"
   Then the exit code is 1
    And stderr contains "Error: No matching Modules to list"

Scenario: I can list disabled modules
  Given I successfully execute dnf with args "module disable postgresql"
   When I execute dnf with args "module list --disabled"
   Then the exit code is 0
    And module list is
        | Repository                    | Name          | Stream     | Profiles                    |
        | dnf-ci-fedora-modular         | postgresql    | 9.6 [d][x] | client, server, default [d] |
        | dnf-ci-fedora-modular         | postgresql    | 6 [x]      | client, server, default     |
        | dnf-ci-fedora-modular-updates | postgresql    | 9.6 [d][x] | client, server, default [d] |
        | dnf-ci-fedora-modular-updates | postgresql    | 10 [x]     | client, server, default     |
        | dnf-ci-fedora-modular-updates | postgresql    | 11 [x]     | client, server, default     |


Scenario: I can limit the scope through providing specific module names
   When I execute dnf with args "module list nodejs"
   Then the exit code is 0
    And module list is
        | Repository                    | Name          | Stream    | Profiles                      |
        | dnf-ci-fedora-modular         | nodejs        | 5         | development, minimal, default |
        | dnf-ci-fedora-modular         | nodejs        | 8 [d][e]     | development, minimal [i], default [d]|
        | dnf-ci-fedora-modular         | nodejs        | 10        | development, minimal, default [d]|
        | dnf-ci-fedora-modular         | nodejs        | 11        | development, minimal, default |
        | dnf-ci-fedora-modular-updates | nodejs        | 8 [d][e]     | development, minimal [i], default [d]|
        | dnf-ci-fedora-modular-updates | nodejs        | 10        | development, minimal, default [d]|
        | dnf-ci-fedora-modular-updates | nodejs        | 11        | development, minimal, default |
        | dnf-ci-fedora-modular-updates | nodejs        | 12        | development, minimal, default |


Scenario: I can limit the scope of enabled modules through providing specific module names
   When I execute dnf with args "module list --enabled nodejs"
   Then the exit code is 0
    And module list is
        | Repository                    | Name          | Stream    | Profiles                      |
        | dnf-ci-fedora-modular         | nodejs        | 8 [d][e]     | development, minimal [i], default [d]|
        | dnf-ci-fedora-modular-updates | nodejs        | 8 [d][e]     | development, minimal [i], default [d]|


Scenario: I can limit the scope of installed modules through providing specific module names
   When I execute dnf with args "module list --installed nodejs"
   Then the exit code is 0
    And module list is
        | Repository                    | Name          | Stream    | Profiles                      |
        | dnf-ci-fedora-modular         | nodejs        | 8 [d][e]     | development, minimal [i], default [d]|
        | dnf-ci-fedora-modular-updates | nodejs        | 8 [d][e]     | development, minimal [i], default [d]|


Scenario: I can limit the scope of disabled modules through providing specific module names
  Given I successfully execute dnf with args "module disable postgresql"
   When I execute dnf with args "module list --disabled postgresql nodejs"
   Then the exit code is 0
    And module list is
        | Repository                    | Name          | Stream     | Profiles                    |
        | dnf-ci-fedora-modular         | postgresql    | 9.6 [d][x] | client, server, default [d] |
        | dnf-ci-fedora-modular         | postgresql    | 6 [x]      | client, server, default     |
        | dnf-ci-fedora-modular-updates | postgresql    | 9.6 [d][x] | client, server, default [d] |
        | dnf-ci-fedora-modular-updates | postgresql    | 10 [x]     | client, server, default     |
        | dnf-ci-fedora-modular-updates | postgresql    | 11 [x]     | client, server, default     |


@bz1590358
Scenario: Modules are ordered by repository then module name and stream name
   When I execute dnf with args "module list"
   Then the exit code is 0
    And stdout section "dnf-ci-fedora-modular test repository" contains "nodejs\s+5.*\n\s*nodejs\s+8.*\n\s*nodejs\s+10.*\n\s*nodejs\s+11"
    And stdout section "dnf-ci-fedora-modular test repository" contains "postgresql\s+6.*\n\s*postgresql\s+9.6"
    And stdout section "dnf-ci-fedora-modular-updates test repository" contains "nodejs\s+8.*\n\s*nodejs\s+10.*\n\s*nodejs\s+11.*\n\s*nodejs\s+12"
    And stdout section "dnf-ci-fedora-modular-updates test repository" contains "postgresql\s+9.6.*\n\s*postgresql\s+10.*\n\s*postgresql\s+11"


Scenario: Correctly display module summary that contains empty lines
   When I execute dnf with args "module list nodejs:5"
   Then the exit code is 0
    And stdout section "dnf-ci-fedora-modular test repository" contains "Javascript runtime module with quite a long summary that contains an empty line."
