# missing modularity features: https://github.com/rpm-software-management/dnf5/issues/146
@xfail
Feature: Tests for the correct creation and usage of the modulefailsafe file


Background: Copy the dnf-ci-fedora modular repository (to allow modulemd removal and so on)
  Given I copy repository "dnf-ci-fedora-modular" for modification
    And I use repository "dnf-ci-fedora-modular" with configuration
        | key                 | value |
        | skip_if_unavailable | 1     |
    And I use repository "dnf-ci-fedora-modular-updates"
    And I use repository "dnf-ci-fedora"


# nodejs RPMs in used repositories (+ info about module streams):
#     dnf-ci-fedora:
#         nodejs-1:5.12.1-1.fc29.x86_64
#     dnf-ci-fedora-modular:
#         nodejs-1:5.3.1-1.module_2011+41787af0.x86_64
#         nodejs-1:8.11.4-1.module_2030+42747d40.x86_64
#         nodejs-1:10.11.0-1.module_2200+adbac02b.x86_64
#         nodejs-1:11.0.0-1.module_2311+8d497411.x86_64
#         - nodejs:5 - nodejs-1:5.3.1-1.module_2011+41787af0.x86_64
#         - nodejs:8 - nodejs-1:8.11.4-1.module_2030+42747d40.x86_64
#         - nodejs:10 - nodejs-1:10.11.0-1.module_2200+adbac02b.x86_64
#         - nodejs:11 - nodejs-1:11.0.0-1.module_2311+8d497411.x86_64
#     dnf-ci-fedora-modular-updates:
#         nodejs-1:10.14.1-1.module_2533+7361f245.x86_64
#         nodejs-1:11.1.0-1.module_2379+8d497405.x86_64
#         nodejs-1:12.1.0-1.module_2379+8d497405.x86_64
#         nodejs-1:8.11.4-1.module_2030+42747d40.x86_64
#         nodejs-1:8.14.0-1.x86_64
#         - nodejs:8 - nodejs-1:8.11.4-1.module_2030+42747d40.x86_64
#         - nodejs:10 - nodejs-1:10.14.1-1.module_2533+7361f245.x86_64
#         - nodejs:11 - nodejs-1:11.1.0-1.module_2379+8d497405.x86_64
#         - nodejs:12 - nodejs-1:12.1.0-1.module_2379+8d497405.x86_64


@bz1616167
@bz1623128
Scenario: Fail-safe modulemd copy is created for every enabled stream and nothing else
   Then file "/var/lib/dnf/modulefailsafe/" does not exist
   When I execute dnf with args "module enable nodejs:5 postgresql:10"
   Then the exit code is 0
    And Transaction contains
        | Action                    | Package                |
        | module-stream-enable      | nodejs:5               |
        | module-stream-enable      | postgresql:10          |
    And modules state is following
        | Module         | State     | Stream    | Profiles  |
        | nodejs         | enabled   | 5         |           |
        | postgresql     | enabled   | 10        |           |
   Then file "/var/lib/dnf/modulefailsafe/" exists
   When I execute "ls {context.dnf.installroot}/var/lib/dnf/modulefailsafe/"
   Then stdout is
        """
        nodejs:5:x86_64.yaml
        postgresql:10:x86_64.yaml
        """


@bz1616167
@bz1623128
Scenario: Fail-safe modulemd copy is created ONLY for the stream of the highest available version and correctly updated
   Then file "/var/lib/dnf/modulefailsafe/" does not exist
   When I execute dnf with args "module enable nodejs:8"
   Then the exit code is 0
    And Transaction contains
        | Action                    | Package                |
        | module-stream-enable      | nodejs:8               |
    And modules state is following
        | Module         | State     | Stream    | Profiles  |
        | nodejs         | enabled   | 8         |           |
   Then file "/var/lib/dnf/modulefailsafe/" exists
   When I execute "ls {context.dnf.installroot}/var/lib/dnf/modulefailsafe/"
   Then stdout is
        """
        nodejs:8:x86_64.yaml
        """
    And file "/var/lib/dnf/modulefailsafe/nodejs:8:x86_64.yaml" contains lines
        """
        version: 20181216123422
        """
    And file "/var/lib/dnf/modulefailsafe/nodejs:8:x86_64.yaml" does not contain lines
        """
        version: 20180801080000
        """
    And I execute dnf with args "install wget --disablerepo dnf-ci-fedora-modular-updates"
   When I execute "ls {context.dnf.installroot}/var/lib/dnf/modulefailsafe/"
   Then stdout is
        """
        nodejs:8:x86_64.yaml
        """
    And file "/var/lib/dnf/modulefailsafe/nodejs:8:x86_64.yaml" contains lines
        """
        version: 20180801080000
        """
    And file "/var/lib/dnf/modulefailsafe/nodejs:8:x86_64.yaml" does not contain lines
        """
        version: 20181216123422
        """
    And I execute dnf with args "reinstall wget"
   When I execute "ls {context.dnf.installroot}/var/lib/dnf/modulefailsafe/"
   Then stdout is
        """
        nodejs:8:x86_64.yaml
        """
    And file "/var/lib/dnf/modulefailsafe/nodejs:8:x86_64.yaml" contains lines
        """
        version: 20181216123422
        """
    And file "/var/lib/dnf/modulefailsafe/nodejs:8:x86_64.yaml" does not contain lines
        """
        version: 20180801080000
        """


@bz1616167
@bz1623128
Scenario: Fail-safe modulemd copy is NOT updated when there is no change
   Then file "/var/lib/dnf/modulefailsafe/" does not exist
   When I execute dnf with args "module enable nodejs:8"
   Then the exit code is 0
    And Transaction contains
        | Action                    | Package                |
        | module-stream-enable      | nodejs:8               |
    And modules state is following
        | Module         | State     | Stream    | Profiles  |
        | nodejs         | enabled   | 8         |           |
   When I execute "stat {context.dnf.installroot}/var/lib/dnf/modulefailsafe/nodejs:8:x86_64.yaml | grep Modify > {context.dnf.installroot}/temp-stat-before"
    And I execute dnf with args "install wget"
    And I execute dnf with args "swap wget flac"
    And I execute dnf with args "module reset postgresql"
    And I execute dnf with args "module list"
    And I execute dnf with args "module disable postgresql"
    And I execute dnf with args "remove flac"
    And I execute dnf with args "module enable postgresql"
    And I execute "stat {context.dnf.installroot}/var/lib/dnf/modulefailsafe/nodejs:8:x86_64.yaml | grep Modify > {context.dnf.installroot}/temp-stat-after"
   Then the files "{context.dnf.installroot}/temp-stat-before" and "{context.dnf.installroot}/temp-stat-after" do not differ


@bz1616167
@bz1623128
Scenario Outline: Fail-safe modulemd copy is NOT deleted after 'dnf clean all' or modular repository disablement/removal
   When I execute dnf with args "module enable nodejs:5 postgresql:10"
   Then the exit code is 0
    And Transaction contains
        | Action                    | Package                |
        | module-stream-enable      | nodejs:5               |
        | module-stream-enable      | postgresql:10          |
    And modules state is following
        | Module         | State     | Stream    | Profiles  |
        | nodejs         | enabled   | 5         |           |
        | postgresql     | enabled   | 10        |           |
   Then file "/var/lib/dnf/modulefailsafe/" exists
   When I execute "ls {context.dnf.installroot}/var/lib/dnf/modulefailsafe/"
   Then stdout is
        """
        nodejs:5:x86_64.yaml
        postgresql:10:x86_64.yaml
        """
   When I copy directory "{context.dnf.installroot}/var/lib/dnf/modulefailsafe/" to "/temp-modulefailsafe"
  # TODO this is weird, the steps used in this Given don't seem to have any impact on the rest of the scenario
  Given I execute step "<step>"
   Then the exit code is 0
    And file "/var/lib/dnf/modulefailsafe/" exists
   When I execute "ls {context.dnf.installroot}/var/lib/dnf/modulefailsafe/"
   Then stdout is
        """
        nodejs:5:x86_64.yaml
        postgresql:10:x86_64.yaml
        """
    And the files "{context.dnf.installroot}/var/lib/dnf/modulefailsafe/nodejs:5:x86_64.yaml" and "{context.dnf.installroot}/temp-modulefailsafe/nodejs:5:x86_64.yaml" do not differ
    And the files "{context.dnf.installroot}/var/lib/dnf/modulefailsafe/postgresql:10:x86_64.yaml" and "{context.dnf.installroot}/temp-modulefailsafe/postgresql:10:x86_64.yaml" do not differ

Examples:
    | step                                                                                       |
    | Given I execute dnf with args "clean all"                                                  |
    | Given I delete directory "/temp-repos/dnf-ci-fedora-modular"                               |
    | Given I delete file "/temp-repos/dnf-ci-fedora-modular/repodata/*modules.yaml*" with globs |


@bz1616167
@bz1623128
Scenario Outline: Fail-safe modulemd copy is deleted after module disable/reset
   When I execute dnf with args "module enable nodejs:5 postgresql:10"
   Then the exit code is 0
    And Transaction contains
        | Action                    | Package                |
        | module-stream-enable      | nodejs:5               |
        | module-stream-enable      | postgresql:10          |
    And modules state is following
        | Module         | State     | Stream    | Profiles  |
        | nodejs         | enabled   | 5         |           |
        | postgresql     | enabled   | 10        |           |
   Then file "/var/lib/dnf/modulefailsafe/" exists
   When I execute "ls {context.dnf.installroot}/var/lib/dnf/modulefailsafe/"
   Then stdout is
        """
        nodejs:5:x86_64.yaml
        postgresql:10:x86_64.yaml
        """
   When I copy directory "{context.dnf.installroot}/var/lib/dnf/modulefailsafe/" to "/temp-modulefailsafe"
    And I execute dnf with args "module <action> nodejs"
   Then the exit code is 0
    And file "/var/lib/dnf/modulefailsafe/" exists
   When I execute "ls {context.dnf.installroot}/var/lib/dnf/modulefailsafe/"
   Then stdout is
        """
        postgresql:10:x86_64.yaml
        """
    And the files "{context.dnf.installroot}/var/lib/dnf/modulefailsafe/postgresql:10:x86_64.yaml" and "{context.dnf.installroot}/temp-modulefailsafe/postgresql:10:x86_64.yaml" do not differ

Examples:
        | action   |
        | disable  |
        | reset    |


@bz1847035
Scenario: Fail-safe modulemd copy is deleted after module disable all modules
   When I execute dnf with args "module enable nodejs:5"
   Then the exit code is 0
    And Transaction contains
        | Action                    | Package                |
        | module-stream-enable      | nodejs:5               |
    And modules state is following
        | Module         | State     | Stream    | Profiles  |
        | nodejs         | enabled   | 5         |           |
   Then file "/var/lib/dnf/modulefailsafe/" exists
   When I execute "ls {context.dnf.installroot}/var/lib/dnf/modulefailsafe/"
   Then stdout is
        """
        nodejs:5:x86_64.yaml
        """
   When I execute dnf with args "module disable '*'"
   Then the exit code is 0
    And file "/var/lib/dnf/modulefailsafe/" exists
   When I execute "ls {context.dnf.installroot}/var/lib/dnf/modulefailsafe/"
   Then stdout is empty


@bz1616167
@bz1623128
Scenario: Fail-safe modulemd copy is created during transaction (module enable, install upgrade)
   Then file "/var/lib/dnf/modulefailsafe/" does not exist
   When I execute dnf with args "module enable nodejs:5"
   Then the exit code is 0
    And Transaction contains
        | Action                    | Package                |
        | module-stream-enable      | nodejs:5               |
    And modules state is following
        | Module         | State     | Stream    | Profiles  |
        | nodejs         | enabled   | 5         |           |
   When I execute "ls {context.dnf.installroot}/var/lib/dnf/modulefailsafe/"
   Then stdout is
        """
        nodejs:5:x86_64.yaml
        """
   When I execute "rm {context.dnf.installroot}/var/lib/dnf/modulefailsafe/ -r"
   Then file "/var/lib/dnf/modulefailsafe/" does not exist
   When I execute dnf with args "install wget"
   Then the exit code is 0
    And file "/var/lib/dnf/modulefailsafe/" exists
   When I execute "ls {context.dnf.installroot}/var/lib/dnf/modulefailsafe/"
   Then stdout is
        """
        nodejs:5:x86_64.yaml
        """
   When I execute "rm {context.dnf.installroot}/var/lib/dnf/modulefailsafe/ -r"
   Then file "/var/lib/dnf/modulefailsafe/" does not exist
   When I use repository "dnf-ci-fedora-updates"
    And I execute dnf with args "upgrade wget"
   Then the exit code is 0
    And file "/var/lib/dnf/modulefailsafe/" exists
   When I execute "ls {context.dnf.installroot}/var/lib/dnf/modulefailsafe/"
   Then stdout is
        """
        nodejs:5:x86_64.yaml
        """


@bz1616167
@bz1623128
Scenario: When modular RPM is installed and the modular repo is disabled and fail-safe modulemd copy is deleted, the RPM can be upgraded to a non-modular RPM
  Given I successfully execute dnf with args "module enable nodejs:5"
   When I execute dnf with args "install nodejs-5*"
   Then the exit code is 0
    And Transaction contains
        | Action                    | Package                                            |
        | install                   | nodejs-1:5.3.1-1.module_2011+41787af0.x86_64       |
  Given I execute dnf with args "clean all"
    And I execute "rm {context.dnf.installroot}/var/lib/dnf/modulefailsafe/ -r"
   When I execute dnf with args "upgrade nodejs --disablerepo dnf-ci-fedora-modular"
   Then the exit code is 0
    And Transaction is following
        | Action                    | Package                                            |
        | upgrade                   | nodejs-1:8.14.0-1.x86_64                           |


@bz1616167
@bz1623128
Scenario: When modular RPM is installed and the modular repo is disabled and '/var/cache/dnf' and '/var/cache/yum' are deleted, the RPM can't be upgraded to a non-modular RPM
  Given I successfully execute dnf with args "module enable nodejs:5"
   When I execute dnf with args "install nodejs"
   Then the exit code is 0
    And Transaction contains
        | Action                    | Package                                            |
        | install                   | nodejs-1:5.3.1-1.module_2011+41787af0.x86_64       |
  Given I execute dnf with args "clean all"
    And I execute "rm {context.dnf.installroot}/var/cache/dnf {context.dnf.installroot}/var/cache/yum -rf"
   When I execute dnf with args "upgrade nodejs --disablerepo dnf-ci-fedora-modular"
   Then the exit code is 0
    And Transaction is empty
