Feature: Group and environment upgrade

# comps-upgrade-1 repo:
#   a-group:
#     mandatory: A-mandatory
#     default: A-default
#     optional: A-optional
#     conditional: A-conditional-true (if dummy), A-conditional-false (if nonexistent)
#   AB-group:
#     mandatory: A-mandatory
#     default: A-default
#     optional: A-optional
#     conditional: A-conditional-true (if dummy), A-conditional-false (if nonexistent)
#   AB-environment:
#     grouplist: a-group
#   empty-group
#   empty-environment
#   optional-environment
#     optionlist: a-group

Scenario: Fail to install excluded group
  Given I use repository "comps-upgrade-1"
   When I execute dnf with args "group install a-group --setopt=excludegroups=a-group"
   Then the exit code is 1
    And Transaction is empty
    And stderr contains "No match for argument: a-group"

Scenario: Fail to install excluded environment
  Given I use repository "comps-upgrade-1"
   When I execute dnf with args "group install AB-environment --setopt=excludeenvs=AB-environment"
   Then the exit code is 1
    And Transaction is empty
    And stderr contains "No match for argument: AB-environment"

Scenario: Install an environment when some of its groups are excluded
  Given I use repository "comps-upgrade-1"
   When I execute dnf with args "group install AC-environment --setopt=excludegroups=a-group"
   Then the exit code is 0
    And Transaction is following
        | Action        | Package                            |
        | install-group | C-mandatory-0:1.0-1.x86_64         |
        | group-install | C-group                            |
        | env-install   | AC-environment                     |

Scenario: Install an environment when all of its groups are excluded
  Given I use repository "comps-upgrade-1"
   When I execute dnf with args "group install AC-environment --setopt=excludegroups=a-group,C-group"
   Then the exit code is 0
    And Transaction is following
        | Action        | Package                            |
        | env-install   | AC-environment                     |

Scenario: Install comps that are not excluded
  Given I use repository "comps-upgrade-1"
   When I execute dnf with args "group install *-group *-environment --setopt=excludegroups=a-group --setopt=excludeenvs=AB*-environment"
   Then the exit code is 0
    And Transaction is following
        | Action        | Package                            |
        | install-group | A-mandatory-0:1.0-1.x86_64         |
        | install-group | A-default-0:1.0-1.x86_64           |
        | install-group | C-mandatory-0:1.0-1.x86_64         |
        | group-install | AB-group                           |
        | group-install | C-group                            |
        | group-install | empty-group                        |
        | env-install   | empty-environment                  |
        | env-install   | optional-environment               |
        | env-install   | AC-environment                     |

Scenario: Install comps when there are non-existent excluded comps
  Given I use repository "comps-upgrade-1"
   When I execute dnf with args "group install a-group AB-environment --setopt=excludegroups=nonexistent-group --setopt=excludeenvs=nonexistent-environment"
   Then the exit code is 0
    And Transaction is following
        | Action        | Package                            |
        | install-group | A-mandatory-0:1.0-1.x86_64         |
        | install-group | A-default-0:1.0-1.x86_64           |
        | group-install | A-group - repo#1                   |
        | env-install   | AB-environment                     |

Scenario: Install a group when it's excluded but all excludes are disabled
  Given I use repository "comps-upgrade-1"
   When I execute dnf with args "group install a-group --setopt=excludegroups=a-group --setopt=disable_excludes=*"
   Then the exit code is 0
    And Transaction is following
        | Action        | Package                            |
        | install-group | A-mandatory-0:1.0-1.x86_64         |
        | install-group | A-default-0:1.0-1.x86_64           |
        | group-install | A-group - repo#1                   |

Scenario: Install an environment when it's excluded but all excludes are disabled
  Given I use repository "comps-upgrade-1"
   When I execute dnf with args "group install empty-environment --setopt=excludeenvs=empty-environment --setopt=disable_excludes=*"
   Then the exit code is 0
    And Transaction is following
        | Action        | Package                            |
        | env-install   | empty-environment                  |

Scenario: Install a group when it's excluded but main excludes are disabled
  Given I use repository "comps-upgrade-1"
   When I execute dnf with args "group install a-group --setopt=excludegroups=a-group --setopt=disable_excludes=main"
   Then the exit code is 0
    And Transaction is following
        | Action        | Package                            |
        | install-group | A-mandatory-0:1.0-1.x86_64         |
        | install-group | A-default-0:1.0-1.x86_64           |
        | group-install | A-group - repo#1                   |

Scenario: Install an environment when it's excluded but main excludes are disabled
  Given I use repository "comps-upgrade-1"
   When I execute dnf with args "group install empty-environment --setopt=excludeenvs=empty-environment --setopt=disable_excludes=main"
   Then the exit code is 0
    And Transaction is following
        | Action        | Package                            |
        | env-install   | empty-environment                  |

Scenario: Fail to install a group when it's excluded, even though repo excludes are disabled
  Given I use repository "comps-upgrade-1"
   When I execute dnf with args "group install a-group --setopt=excludegroups=a-group --setopt=disable_excludes=comps-upgrade-1"
   Then the exit code is 1
    And Transaction is empty
    And stderr contains "No match for argument: a-group"

Scenario: Fail to install an environment when it's excluded, even though repo excludes are disabled
  Given I use repository "comps-upgrade-1"
   When I execute dnf with args "group install empty-environment --setopt=excludeenvs=empty-environment --setopt=disable_excludes=comps-upgrade-1"
   Then the exit code is 1
    And Transaction is empty
    And stderr contains "No match for argument: empty-environment"

Scenario: Fail to install a group excluded using globs
  Given I use repository "comps-upgrade-1"
   When I execute dnf with args "group install a-group --setopt=excludegroups=a-*"
   Then the exit code is 1
    And Transaction is empty
    And stderr contains "No match for argument: a-group"

Scenario: Fail to install an environment excluded using globs
  Given I use repository "comps-upgrade-1"
   When I execute dnf with args "group install AB-environment --setopt=excludeenvs=AB-*"
   Then the exit code is 1
    And Transaction is empty
    And stderr contains "No match for argument: AB-environment"

Scenario: Install an environment when its group is excluded using globs
  Given I use repository "comps-upgrade-1"
   When I execute dnf with args "group install AB-environment --setopt=excludegroups=a-*"
   Then the exit code is 0
    And Transaction is following
        | Action        | Package                            |
        | env-install   | AB-environment                     |
