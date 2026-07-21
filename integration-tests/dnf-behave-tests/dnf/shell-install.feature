# missing shell command: https://github.com/rpm-software-management/dnf5/issues/153
@xfail
Feature: Shell install


Scenario: Using dnf shell, install an RPM
  Given I use repository "dnf-ci-fedora"
   When I open dnf shell session
    And I execute in dnf shell "install filesystem wget"
    And I execute in dnf shell "run"
   Then Transaction is following
        | Action        | Package                                   |
        | install       | filesystem-0:3.9-2.fc29.x86_64            |
        | install       | wget-0:1.19.5-5.fc29.x86_64               |
        | install-dep   | setup-0:2.12.1-1.fc29.noarch              |
   When I execute in dnf shell "exit"
   Then stdout contains "Leaving Shell"


@bz1658579
Scenario: Using dnf shell, fail to install an RPM when no repositories are enabled
   When I open dnf shell session
    And I execute in dnf shell "install setup"
   Then Transaction is empty
    And stdout contains "Error: There are no enabled repositories in ".*/etc/yum.repos.d", ".*/etc/yum/repos.d", ".*/etc/distro.repos.d"\."
   When I execute in dnf shell "run"
   Then Transaction is empty
   When I execute in dnf shell "exit"
   Then stdout contains "Leaving Shell"


Scenario: Using dnf shell, fail to install a non-existent RPM
  Given I use repository "dnf-ci-fedora"
   When I open dnf shell session
    And I execute in dnf shell "install NoSuchPackage"
   Then Transaction is empty
    And stdout contains "No match for argument: .*NoSuchPackage"
   When I execute in dnf shell "run"
   Then Transaction is empty
   When I execute in dnf shell "exit"
   Then stdout contains "Leaving Shell"


@bz1773483
Scenario: Using dnf shell, fail to install local file when goal is not empty
  Given I use repository "dnf-ci-fedora"
   When I open dnf shell session
    And I execute in dnf shell "install setup-0:2.12.1-1.fc29.noarch"
   When I execute in dnf shell "install {context.dnf.fixturesdir}/repos/dnf-ci-fedora/x86_64/filesystem-3.9-2.fc29.x86_64.rpm"
   Then stdout contains "Error: Cannot add local packages, because transaction job already exists"
   When I execute in dnf shell "run"
   Then Transaction is following
        | Action        | Package                                   |
        | install       | setup-0:2.12.1-1.fc29.noarch              |
   When I execute in dnf shell "exit"
   Then stdout contains "Leaving Shell"
