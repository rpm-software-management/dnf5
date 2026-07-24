Feature: Transaction history rollback between states with the same version of a package installed


Background:
  Given I use repository "dnf-ci-fedora"
    And I successfully execute dnf with args "install flac"


# https://github.com/rpm-software-management/dnf/issues/2031
@GH-2031
Scenario: Rollback removal and installation of the same package
  Given I successfully execute dnf with args "remove flac"
    And I successfully execute dnf with args "install flac"
    And I successfully execute dnf with args "history list"
   Then stdout is history list
        | Id | Command | Action  | Altered |
        | 3  |         |         | 1       |
        | 2  |         |         | 1       |
        | 1  |         |         | 1       |
  Given I successfully execute dnf with args "history rollback 1"
   Then Transaction is empty


# https://issues.redhat.com/browse/RHEL-17494
@RHEL-17494
Scenario: Two rollbacks of the package upgrade
  Given I use repository "dnf-ci-fedora-updates"
    And I successfully execute dnf with args "upgrade flac"
    And I successfully execute dnf with args "history list"
   Then stdout is history list
        | Id | Command | Action  | Altered |
        | 2  |         |         | 2       |
        | 1  |         |         | 1       |
  Given I successfully execute dnf with args "history rollback 1"
   Then Transaction is following
        | Action        | Package                   |
        | downgrade     | flac-1.3.2-8.fc29.x86_64  |
  Given I successfully execute dnf with args "history rollback 1"
   Then Transaction is empty
