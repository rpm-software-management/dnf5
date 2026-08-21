# Modularity is disabled since RHEL 11
@not.with_os=rhel__ge__11
Feature: RPMs can be installed locally regardless the modular content


Background: Enable dnf-ci-fedora and dnf-ci-fedora-modular
  Given I use repository "dnf-ci-fedora"
    And I use repository "dnf-ci-fedora-modular"


@bz1582105
Scenario: Install a local RPM with different version available in enabled stream
   When I execute dnf with args "module enable postgresql"
   Then the exit code is 0
    And Transaction is following
        | Action                    | Package             |
        | module-stream-enable      | postgresql:9.6      |
    And modules state is following
        | Module      | State     | Stream    | Profiles  |
        | postgresql  | enabled   | 9.6       |           |
   When I execute dnf with args "install {context.dnf.fixturesdir}/repos/dnf-ci-fedora/x86_64/postgresql-libs-9.6.5-1.fc29.x86_64.rpm {context.dnf.fixturesdir}/repos/dnf-ci-fedora/x86_64/postgresql-9.6.5-1.fc29.x86_64.rpm"
   Then the exit code is 0
    And Transaction contains
        | Action       | Package                                   |
        | install      | postgresql-libs-0:9.6.5-1.fc29.x86_64     |
        | install      | postgresql-0:9.6.5-1.fc29.x86_64          |


# rely on merging bz1649261 fix
@bz1582105
Scenario: Install a local RPM that belongs to a disabled module
   When I execute dnf with args "module disable postgresql"
   Then the exit code is 0
    And Transaction is following
        | Action                    | Package             |
        | module-disable            | postgresql          |
    And modules state is following
        | Module      | State     | Stream    | Profiles  |
        | postgresql  | disabled  |           |           |
   When I execute dnf with args "install {context.dnf.fixturesdir}/repos/dnf-ci-fedora-modular/x86_64/postgresql-libs-9.6.8-1.module_1710+b535a823.x86_64.rpm {context.dnf.fixturesdir}/repos/dnf-ci-fedora-modular/x86_64/postgresql-9.6.8-1.module_1710+b535a823.x86_64.rpm"
   Then the exit code is 0
    And Transaction contains
        | Action       | Package                                                 |
        | install      | postgresql-libs-0:9.6.8-1.module_1710+b535a823.x86_64   |
        | install      | postgresql-0:9.6.8-1.module_1710+b535a823.x86_64        |
