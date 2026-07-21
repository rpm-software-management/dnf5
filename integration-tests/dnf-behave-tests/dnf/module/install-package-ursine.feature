# Modularity is disabled since RHEL 11
@not.with_os=rhel__ge__11
Feature: Installing package from ursine repo

Background: Enable repositories
  Given I use repository "dnf-ci-thirdparty"
    And I use repository "dnf-ci-fedora"


@bz1636390
Scenario: I can install a package from ursine repo when the same pkg is available in non-enabled non-default module stream
   When I execute dnf with args "install wget"
   Then the exit code is 0
    And Transaction contains
        | Action                | Package                                           |
        | install               | wget-0:1.19.5-5.fc29.x86_64                       |


Scenario: I can see installed non-modular content listed in dnf list installed
  Given I successfully execute dnf with args "install wget"
   When I execute dnf with args "list --installed"
   Then the exit code is 0
    And stdout contains "wget\.x86_64\s+1\.19\.5-5\.fc29\s+dnf-ci-fedora"
   When I execute dnf with args "module enable DnfCiModuleNoDefaults:stable"
   Then the exit code is 0
   When I execute dnf with args "list --installed"
   Then the exit code is 0
    And stdout contains "wget\.x86_64\s+1\.19\.5-5\.fc29\s+dnf-ci-fedora"


Scenario: I can't reinstall installed non-modular content which is masked by active modular content
  Given I successfully execute dnf with args "install wget"
    And I successfully execute dnf with args "module enable DnfCiModuleNoDefaults:stable"
   When I execute dnf with args "reinstall wget"
   Then the exit code is 1
    And stderr contains "Installed packages for argument 'wget' are not available in repositories in the same version, available versions: wget-0:1.18.5-5.module.x86_64, cannot reinstall."


Scenario: I can remove installed non-modular content
  Given I successfully execute dnf with args "install wget"
    And I successfully execute dnf with args "module enable DnfCiModuleNoDefaults:stable"
   When I execute dnf with args "remove wget"
   Then the exit code is 0
    And Transaction contains
        | Action                | Package                                           |
        | remove                | wget-0:1.19.5-5.fc29.x86_64                       |


Scenario: I can't install a package from ursine repo when the same pkg is available in enabled non-default module stream
  Given I successfully execute dnf with args "module enable DnfCiModuleNoDefaults:stable"
   When I execute dnf with args "install wget"
   Then the exit code is 0
    And Transaction contains
        | Action                | Package                                           |
        | install               | wget-0:1.18.5-5.module.x86_64                     |


Scenario: I can install a package from ursine repo when the same pkg is available in disabled non-default module stream
  Given I successfully execute dnf with args "module disable DnfCiModuleNoDefaults:stable"
   When I execute dnf with args "install wget"
   Then the exit code is 0
    And Transaction contains
        | Action                | Package                                           |
        | install               | wget-0:1.19.5-5.fc29.x86_64                       |


Scenario: I can upgrade installed non-modular pkg by active modular content
  Given I successfully execute dnf with args "install wget"
    And I successfully execute dnf with args "module enable DnfCiModuleNoDefaults:development"
   When I execute dnf with args "upgrade wget"
   Then the exit code is 0
    And Transaction is following
        | Action                | Package                                           |
        | upgrade               | wget-0:1.20.5-5.module.x86_64                     |
