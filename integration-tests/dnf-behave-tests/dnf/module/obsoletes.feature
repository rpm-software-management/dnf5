# Modularity is disabled since RHEL 11
@not.with_os=rhel__ge__11
@xfail
# Obsoletes are not implemented in dnf5
# Mentioned in https://github.com/rpm-software-management/dnf5/issues/146
Feature: obsoletes reset/change streams in any transaction according to metadata


Background:
Given I use repository "dnf-ci-fedora"
  And I use repository "dnf-ci-fedora-modular"
  And I configure dnf with
      | key                  | value |
      | module_obsoletes     | True  |
      | module_stream_switch | True  |


Scenario: stream is not switched according to obsoletes if options not enabled
Given I execute dnf with args "module install nodejs:5/minimal"
  And I configure dnf with
      | key                  | value |
      | module_obsoletes     |       |
      | module_stream_switch |       |
 When I use repository "dnf-ci-fedora-modular-obsoletes"
  And I execute dnf with args "install wget"
 Then the exit code is 0
  And modules state is following
      | Module    | State     | Stream    | Profiles  |
      | nodejs    | enabled   | 5         | minimal   |


Scenario: stream is switched according to obsoletes in new metadata
Given I execute dnf with args "module install nodejs:5/minimal"
 When I use repository "dnf-ci-fedora-modular-obsoletes"
  And I execute dnf with args "install wget"
 Then the exit code is 0
  And modules state is following
      | Module    | State     | Stream    | Profiles  |
      | nodejs    | enabled   | 10        |           |


Scenario: two stream changes, one of which is obsoletes, do not throw exception
Given I execute dnf with args "module enable nodejs:5"
 When I use repository "dnf-ci-fedora-modular-obsoletes"
  And I execute dnf with args "module reset nodejs"
 Then the exit code is 0
  And modules state is following
      | Module    | State     | Stream    | Profiles  |
      | nodejs    | available |           |           |


Scenario: stream is resetted according to obsoletes in new metadata
Given I execute dnf with args "module install nodejs:8/minimal"
 When I use repository "dnf-ci-fedora-modular-obsoletes"
  And I execute dnf with args "install wget"
 Then the exit code is 0
  And modules state is following
      | Module    | State     | Stream    | Profiles  |
      | nodejs    |           |           |           |


Scenario: stream is not resetted before the obsoletes_date in new metadata
Given I execute dnf with args "module install nodejs:10/minimal"
 When I use repository "dnf-ci-fedora-modular-obsoletes"
  And I execute dnf with args "install wget"
 Then the exit code is 0
  And modules state is following
      | Module    | State     | Stream    | Profiles  |
      | nodejs    | enabled   | 10        | minimal   |


Scenario: stream with context is affected by general obsoletes without context specified
Given I execute dnf with args "module install nodejs:11/minimal"
 When I use repository "dnf-ci-fedora-modular-obsoletes"
  And I execute dnf with args "install wget"
 Then the exit code is 0
  And modules state is following
      | Module    | State     | Stream    | Profiles  |
      | nodejs    |           |           |           |


Scenario: stream with two obsoletes is switched according to the newer one and switching to a different module resets the original one
Given I execute dnf with args "module install nodejs:5/minimal"
 When I use repository "dnf-ci-fedora-modular-obsoletes"
  And I use repository "dnf-ci-fedora-modular-obsoletes-newer"
  And I execute dnf with args "install wget"
 Then the exit code is 0
  And modules state is following
      | Module     | State      | Stream    | Profiles  |
      | dwm        | enabled    | 6.0       |           |
      | nodejs     |            |           |           |


Scenario: obsoletes is applied before the current transaction is resolved (effects from obsoletes switch affect the current transaction)
Given I execute dnf with args "module install nodejs:5/minimal"
 When I use repository "dnf-ci-fedora-modular-obsoletes"
  And I use repository "dnf-ci-fedora-modular-obsoletes-newer"
  And I execute dnf with args "install dwm-6.0"
 Then the exit code is 0
  And modules state is following
      | Module     | State      | Stream    | Profiles  |
      | dwm        | enabled    | 6.0       |           |
      | nodejs     |            |           |           |


Scenario: stream is not affected if a valid obsoletes is superseded by a newer one which resets obsoletes
Given I execute dnf with args "module install nodejs:8/minimal"
 When I use repository "dnf-ci-fedora-modular-obsoletes"
  And I use repository "dnf-ci-fedora-modular-obsoletes-newer"
  And I execute dnf with args "install wget"
 Then the exit code is 0
  And modules state is following
      | Module     | State      | Stream    | Profiles  |
      | nodejs     | enabled    | 8         | minimal   |


Scenario: stream is affected if a valid obsoletes with reset is superseded by a newer obsoletes
Given I execute dnf with args "module install postgresql:6/client"
 When I use repository "dnf-ci-fedora-modular-obsoletes"
  And I use repository "dnf-ci-fedora-modular-obsoletes-newer"
  And I execute dnf with args "install wget"
 Then the exit code is 0
  And modules state is following
      | Module     | State      | Stream    | Profiles  |
      | postgresql |            |           |           |


Scenario: active obsoletes isn't affected by future obsoletes (with eol_date)
Given I execute dnf with args "module install nodejs:11/minimal"
 When I use repository "dnf-ci-fedora-modular-obsoletes"
 When I use repository "dnf-ci-fedora-modular-obsoletes-newer"
  And I execute dnf with args "install wget"
 Then the exit code is 0
  And modules state is following
      | Module    | State     | Stream    | Profiles  |
      | nodejs    |           |           |           |


Scenario: obsoletes with obsoleted_by and passed eol_date works
Given I execute dnf with args "module install postgresql:9.6/client"
 When I use repository "dnf-ci-fedora-modular-obsoletes"
  And I use repository "dnf-ci-fedora-modular-obsoletes-newer"
  And I execute dnf with args "install wget"
 Then the exit code is 0
  And modules state is following
      | Module     | State      | Stream    | Profiles  |
      | postgresql |            |           |           |
      | dwm        | enabled    | 6.0       |           |


Scenario: setting reset into the future (with eol_date) is an error (invalid metadata)
Given I create file "invalid_modules.yaml" with
      """
      ---
      document: modulemd-obsoletes
      version: 1
      data:
          modified: 2020-05-01T00:00Z
          module: nodejs
          stream: 5
          message: "invalid, cannot have both reset and eol_date"
          reset: true
          eol_date: 2009-01-01T00:00Z
      ...

      """
  And I copy repository "dnf-ci-fedora-modular" for modification
  And I execute "modifyrepo_c --mdtype modules {context.dnf.installroot}/invalid_modules.yaml /{context.dnf.repos[dnf-ci-fedora-modular].path}/repodata"
  And I use repository "dnf-ci-fedora-modular"
 When I execute dnf with args "module install nodejs:5/minimal"
 Then stderr contains "Module yaml error: Obsoletes cannot have both eol_date and reset attributes set."
