# Modularity is disabled since RHEL 11
@not.with_os=rhel__ge__11
# Missing module provides command
# https://github.com/rpm-software-management/dnf5/issues/146
@xfail
Feature: Module provides command


Background:
Given I use repository "dnf-ci-fedora-modular"
  And I use repository "dnf-ci-fedora"


@bz1629667
Scenario: I can get list of all modules providing specific package
 When I execute dnf with args "module provides nodejs-devel"
 Then the exit code is 0
  And stderr is
      """
      <REPOSYNC>
      """
  And stdout is
      """
      nodejs-devel-1:10.11.0-1.module_2200+adbac02b.x86_64
      Module   : nodejs:10:20180920144631:6c81f848:x86_64
      Profiles : development
      Repo     : dnf-ci-fedora-modular
      Summary  : Javascript runtime

      nodejs-devel-1:11.0.0-1.module_2311+8d497411.x86_64
      Module   : nodejs:11:20180920144611:6c81f848:x86_64
      Profiles : development
      Repo     : dnf-ci-fedora-modular
      Summary  : Javascript runtime

      nodejs-devel-1:5.3.1-1.module_2011+41787af0.x86_64
      Module   : nodejs:5:20150811143428:6c81f848:x86_64
      Profiles : development
      Repo     : dnf-ci-fedora-modular
      Summary  : Javascript runtime module with quite a long
               : summary that contains an empty line.

      nodejs-devel-1:8.11.4-1.module_2030+42747d40.x86_64
      Module   : nodejs:8:20180801080000:6c81f848:x86_64
      Profiles : development
      Repo     : dnf-ci-fedora-modular
      Summary  : Javascript runtime
      """


@bz1623866
Scenario: I can get list of enabled modules providing specific package
Given I successfully execute dnf with args "module enable nodejs:8"
 When I execute dnf with args "module provides nodejs-devel"
 Then the exit code is 0
  And stderr is
      """
      <REPOSYNC>
      """
  And stdout is
      """
      nodejs-devel-1:10.11.0-1.module_2200+adbac02b.x86_64
      Module   : nodejs:10:20180920144631:6c81f848:x86_64
      Profiles : development
      Repo     : dnf-ci-fedora-modular
      Summary  : Javascript runtime

      nodejs-devel-1:11.0.0-1.module_2311+8d497411.x86_64
      Module   : nodejs:11:20180920144611:6c81f848:x86_64
      Profiles : development
      Repo     : dnf-ci-fedora-modular
      Summary  : Javascript runtime

      nodejs-devel-1:5.3.1-1.module_2011+41787af0.x86_64
      Module   : nodejs:5:20150811143428:6c81f848:x86_64
      Profiles : development
      Repo     : dnf-ci-fedora-modular
      Summary  : Javascript runtime module with quite a long
               : summary that contains an empty line.

      nodejs-devel-1:8.11.4-1.module_2030+42747d40.x86_64
      Module   : nodejs:8:20180801080000:6c81f848:x86_64
      Profiles : development
      Repo     : dnf-ci-fedora-modular
      Summary  : Javascript runtime
      """


@bz1633151
Scenario: I see packages only once when they are availiable and installed
Given I successfully execute dnf with args "module enable nodejs:8"
 When I execute dnf with args "module provides nodejs-devel"
 Then the exit code is 0
  And stderr is
      """
      <REPOSYNC>
      """
  And stdout is
      """
      nodejs-devel-1:10.11.0-1.module_2200+adbac02b.x86_64
      Module   : nodejs:10:20180920144631:6c81f848:x86_64
      Profiles : development
      Repo     : dnf-ci-fedora-modular
      Summary  : Javascript runtime

      nodejs-devel-1:11.0.0-1.module_2311+8d497411.x86_64
      Module   : nodejs:11:20180920144611:6c81f848:x86_64
      Profiles : development
      Repo     : dnf-ci-fedora-modular
      Summary  : Javascript runtime

      nodejs-devel-1:5.3.1-1.module_2011+41787af0.x86_64
      Module   : nodejs:5:20150811143428:6c81f848:x86_64
      Profiles : development
      Repo     : dnf-ci-fedora-modular
      Summary  : Javascript runtime module with quite a long
               : summary that contains an empty line.

      nodejs-devel-1:8.11.4-1.module_2030+42747d40.x86_64
      Module   : nodejs:8:20180801080000:6c81f848:x86_64
      Profiles : development
      Repo     : dnf-ci-fedora-modular
      Summary  : Javascript runtime
      """
 When I execute dnf with args "module install nodejs:8/development"
 Then the exit code is 0
  And Transaction contains
      | Action                    | Package                                             |
      | install-group             | nodejs-devel-1:8.11.4-1.module_2030+42747d40.x86_64 |
      | module-profile-install    | nodejs/development                                  |
 When I execute dnf with args "module provides nodejs-devel"
 Then the exit code is 0
  And stderr is
      """
      <REPOSYNC>
      """
  And stdout is
      """
      nodejs-devel-1:10.11.0-1.module_2200+adbac02b.x86_64
      Module   : nodejs:10:20180920144631:6c81f848:x86_64
      Profiles : development
      Repo     : dnf-ci-fedora-modular
      Summary  : Javascript runtime

      nodejs-devel-1:11.0.0-1.module_2311+8d497411.x86_64
      Module   : nodejs:11:20180920144611:6c81f848:x86_64
      Profiles : development
      Repo     : dnf-ci-fedora-modular
      Summary  : Javascript runtime

      nodejs-devel-1:5.3.1-1.module_2011+41787af0.x86_64
      Module   : nodejs:5:20150811143428:6c81f848:x86_64
      Profiles : development
      Repo     : dnf-ci-fedora-modular
      Summary  : Javascript runtime module with quite a long
               : summary that contains an empty line.

      nodejs-devel-1:8.11.4-1.module_2030+42747d40.x86_64
      Module   : nodejs:8:20180801080000:6c81f848:x86_64
      Profiles : development
      Repo     : dnf-ci-fedora-modular
      Summary  : Javascript runtime
      """


Scenario: There is not output when no module provides the package
Given I successfully execute dnf with args "makecache"
 When I execute dnf with args "module provides NoSuchPackage"
 Then the exit code is 0
  And stdout is
      """
      <REPOSYNC>
      """


Scenario: An error is printed when no arguments are provided (dnf)
Given I set dnf command to "dnf"
 When I execute dnf with args "module provides"
 Then the exit code is 1
  And stderr is
      """
      Error: dnf module provides: too few arguments
      """

Scenario: An error is printed when no arguments are provided (yum)
Given I set dnf command to "yum"
 When I execute dnf with args "module provides"
 Then the exit code is 1
  And stderr is
      """
      Error: yum module provides: too few arguments
      """
