# Modularity is disabled since RHEL 11
@not.with_os=rhel__ge__11
# Missing module install command
# https://github.com/rpm-software-management/dnf5/issues/146
@xfail
Feature: Installing modules from MdDocuments with static_context=True


Background:
  Given I set default module platformid to "platform:f29"


@not.with_os=rhel__eq__8
Scenario: Install module, profile from the latest module context without static_context
  Given I use repository "dnf-ci-multicontext-hybrid-multiversion-modular"
   When I execute dnf with args "module enable postgresql:9.6"
   Then the exit code is 0
    And modules state is following
        | Module        | State     | Stream    | Profiles  |
        | postgresql    | enabled   |    9.6    |           |
   When I execute dnf with args "module install nodejs:5/testlatest"
   Then the exit code is 0
    And modules state is following
        | Module   | State     | Stream    | Profiles   |
        | nodejs   |  enabled  |     5     | testlatest |
    And Transaction is following
        | Action                    | Package                                          |
        | module-stream-enable      | nodejs:5                                         |
        | module-profile-install    | nodejs/testlatest                                |
        | install-group             | postgresql-0:9.6.8-1.module_1710+b535a823.x86_64 |

@not.with_os=rhel__eq__8
Scenario: Install module, profile from the latest module context with static_context-=true
  Given I use repository "dnf-ci-multicontext-hybrid-multiversion-modular-static-context"
   When I execute dnf with args "module enable postgresql:9.6"
   Then the exit code is 0
    And modules state is following
        | Module        | State     | Stream    | Profiles  |
        | postgresql    | enabled   |    9.6    |           |
   When I execute dnf with args "module install nodejs:5/testlatest"
   Then the exit code is 0
    And modules state is following
        | Module   | State     | Stream    | Profiles   |
        | nodejs   |  enabled  |     5     | testlatest |
    And Transaction is following
        | Action                    | Package                                             |
        | module-stream-enable      | nodejs:5                                            |
        | module-profile-install    | nodejs/testlatest                                   |
        | install-group             | postgresql-0:9.6.8-1.module_1710+b535a823_V3.x86_64 |

@not.with_os=rhel__eq__8
Scenario: Install module, profile and the latest package with static_context=true
  Given I use repository "dnf-ci-multicontext-hybrid-multiversion-modular-static-context"
   When I execute dnf with args "module enable postgresql:9.6"
   Then the exit code is 0
    And modules state is following
        | Module        | State     | Stream    | Profiles  |
        | postgresql    | enabled   |    9.6    |           |
   When I execute dnf with args "module install nodejs:5/minimal"
   Then the exit code is 0
    And modules state is following
        | Module   | State     | Stream    | Profiles   |
        | nodejs   |  enabled  |     5     | minimal |
    And Transaction is following
        | Action                    | Package                                         |
        | module-stream-enable      | nodejs:5                                        |
        | module-profile-install    | nodejs/minimal                                  |
        | install-group             | nodejs-1:5.4.1-2.module_2011+41787af1_V3.x86_64 |

@not.with_os=rhel__eq__8
Scenario: Install and upgrade from context with broken dependencies => static_context=true
  Given I use repository "dnf-ci-static-context-requires-change"
   When I execute dnf with args "module install nodejs:5/minimal"
   Then the exit code is 0
    And Transaction is following
        | Action                    | Package                                         |
        | module-stream-enable      | nodejs:5                                        |
        | module-stream-enable      | postgresql:9.6                                  |
        | module-profile-install    | nodejs/minimal                                  |
        | install-group             | nodejs-1:5.4.1-2.module_2011+41787af1_V3.x86_64 |
  Given I use repository "dnf-ci-static-context-requires-change-updates"
   When I execute dnf with args "upgrade"
   Then the exit code is 0
    And Transaction is following
        | Action                    | Package                                         |
        | upgrade                   | nodejs-1:6.5.1-2.module_3012+41787ba4_V3.x86_64 |
   When I execute dnf with args "repoquery nodejs"
   Then the exit code is 0
    And stdout is
  """
  nodejs-1:5.3.1-1.module_2011+41787af0_V3.src
  nodejs-1:5.3.1-1.module_2011+41787af0_V3.x86_64
  nodejs-1:5.4.1-2.module_2011+41787af1_V3.src
  nodejs-1:5.4.1-2.module_2011+41787af1_V3.x86_64
  nodejs-1:6.5.1-2.module_3012+41787ba4_V3.src
  nodejs-1:6.5.1-2.module_3012+41787ba4_V3.x86_64
  """

@not.with_os=rhel__eq__8
Scenario: Install and upgrade from context with broken dependencies => static_context=true
  Given I use repository "dnf-ci-static-context-requires-change"
   When I execute dnf with args "module install nodejs:5/nodejs-postgresql --best"
   Then the exit code is 0
    And Transaction is following
        | Action                    | Package                                                    |
        | module-stream-enable      | nodejs:5                                                   |
        | module-stream-enable      | postgresql:9.6                                             |
        | module-profile-install    | nodejs/nodejs-postgresql                                   |
        | install-group             | nodejs-postgresql-1:5.4.1-2.module_2011+41787af1_V3.x86_64 |
        | install-dep               | postgresql-9.6.8-1.module_1710+b535a823_V3.x86_64 |
  Given I use repository "dnf-ci-static-context-requires-change-updates"
   When I execute dnf with args "upgrade --best"
   Then the exit code is 1
    And stderr is
  """
  Modular dependency problem:

   Problem: module nodejs:5:30180801080004:6c81f848.x86_64 requires module(postgresql:9.8), but none of the providers can be installed
    - module postgresql:9.6:30180816142114:7c81f878.x86_64 conflicts with module(postgresql) provided by postgresql:9.8:20180816142114:9c81f899.x86_64
    - module postgresql:9.8:20180816142114:9c81f899.x86_64 conflicts with module(postgresql) provided by postgresql:9.6:30180816142114:7c81f878.x86_64
    - cannot install the best candidate for the job
    - conflicting requests
  Error:
   Problem: package nodejs-postgresql-1:6.5.1-2.module_3012+41787ba4_V3.x86_64 requires postgresql = 9.8.1, but none of the providers can be installed
    - cannot install the best update candidate for package nodejs-postgresql-1:5.4.1-2.module_2011+41787af1_V3.x86_64
    - package postgresql-9.8.1-1.module_9790+c535b823_V3.x86_64 is filtered out by modular filtering
  """
    And Transaction is empty
   When I execute dnf with args "module switch-to postgresql:9.8"
   Then the exit code is 0
    And Transaction is following
        | Action                    | Package                                                    |
        | module-stream-switch      | postgresql:9.6 -> 9.8                                                 |
        | upgrade                   | postgresql-9.8.1-1.module_9790+c535b823_V3.x86_64          |
        | upgrade                   | nodejs-postgresql-1:6.5.1-2.module_3012+41787ba4_V3.x86_64 |
