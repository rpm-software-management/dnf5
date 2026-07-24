# Modularity is disabled since RHEL 11
@not.with_os=rhel__ge__11
@jiraRHELPLAN-6073
Feature: Filter RPMs by enabled and default module streams


Background:
Given I use repository "dnf-ci-fedora-modular"


@xfail
# Reported as https://github.com/rpm-software-management/dnf5/issues/1811
Scenario: default from module is preferred over ursine pkg
Given I use repository "dnf-ci-fedora"
 When I execute dnf with args "install ninja-build"
 Then the exit code is 0
  And Transaction is following
    | Action                    | Package                                           |
    | install                   | ninja-build-0:1.8.2-4.module_1991+4e5efe2f.x86_64 |
    | install-dep               | setup-0:2.12.1-1.fc29.noarch                      |
    | install-dep               | glibc-common-0:2.28-9.fc29.x86_64                 |
    | install-dep               | glibc-0:2.28-9.fc29.x86_64                        |
    | install-dep               | glibc-all-langpacks-0:2.28-9.fc29.x86_64          |
    | install-dep               | filesystem-0:3.9-2.fc29.x86_64                    |
    | install-dep               | basesystem-0:11-6.fc29.noarch                     |
    | module-stream-enable      | ninja:master                                      |


Scenario: enabled module is preferred over ursine pkg
Given I use repository "dnf-ci-fedora"
 When I execute dnf with args "module enable ninja"
 Then the exit code is 0
 When I execute dnf with args "install ninja-build"
 Then the exit code is 0
  And Transaction is following
    | Action                    | Package                                           |
    | install                   | ninja-build-0:1.8.2-4.module_1991+4e5efe2f.x86_64 |
    | install-dep               | setup-0:2.12.1-1.fc29.noarch                      |
    | install-dep               | glibc-common-0:2.28-9.fc29.x86_64                 |
    | install-dep               | glibc-0:2.28-9.fc29.x86_64                        |
    | install-dep               | glibc-all-langpacks-0:2.28-9.fc29.x86_64          |
    | install-dep               | filesystem-0:3.9-2.fc29.x86_64                    |
    | install-dep               | basesystem-0:11-6.fc29.noarch                     |


Scenario: disabled module is not used
Given I use repository "dnf-ci-fedora"
 When I execute dnf with args "module disable ninja"
 Then the exit code is 0
 When I execute dnf with args "install ninja-build"
 Then the exit code is 0
  And Transaction is following
    | Action                    | Package                           |
    | install                   | ninja-build-0:1.8.2-5.fc29.x86_64 |


Scenario: ursine pkg is preferred over module without default
Given I use repository "dnf-ci-fedora"
 When I execute dnf with args "install dwm"
 Then the exit code is 0
  And Transaction is following
    | Action                    | Package                           |
    | install                   | dwm-0:6.1-1.x86_64                |


Scenario: RPMs from non-active streams are not available
 When I execute dnf with args "module disable nodejs:8"
 Then I execute dnf with args "list --available dwm.x86_64"
  And the exit code is 1
 Then I execute dnf with args "list --available nodejs-devel-10.11.0"
  And the exit code is 1
 Then I execute dnf with args "list --available nodejs-devel-11.0.0"
  And the exit code is 1
 Then I execute dnf with args "list --available nodejs-10.11.0-1.module_2200+adbac02b.x86_64"
  And the exit code is 1
 Then I execute dnf with args "list --available nodejs-8.11.4-1.module_2030+42747d40.x86_64"
  And the exit code is 1
 Then I execute dnf with args "list --available dwm.x86_64"
  And the exit code is 1
