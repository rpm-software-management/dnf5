# missing shell command: https://github.com/rpm-software-management/dnf5/issues/153
@xfail
Feature: Shell swap

Background:
Given I use repository "dnf-ci-fedora"
  And I use repository "dnf-ci-fedora-updates"
  And I use repository "dnf-ci-thirdparty"


Scenario: Switch packages and their subpackages by swap command (using wildcards)
 When I open dnf shell session
  And I execute in dnf shell "install CQRlib-devel CQRlib CQRlib-extension"
  And I execute in dnf shell "run"
 Then Transaction is following
      | Action        | Package                                   |
      | install       | CQRlib-0:1.1.2-16.fc29.x86_64             |
      | install       | CQRlib-devel-0:1.1.2-16.fc29.x86_64       |
      | install       | CQRlib-extension-0:1.5-2.x86_64           |
  And I execute in dnf shell "swap CQRlib\* SuperRipper\*"
  And I execute in dnf shell "run"
  And Transaction is following
      | Action        | Package                                   |
      | install       | SuperRipper-extension-0:1.1-1.x86_64      |
      | install       | SuperRipper-0:1.0-1.x86_64                |
      | install-dep   | abcde-0:2.9.3-1.fc29.noarch               |
      | install-dep   | wget-0:1.19.6-5.fc29.x86_64               |
      | install-weak  | flac-0:1.3.3-3.fc29.x86_64                |
      | install-weak  | FlacBetterEncoder-0:1.0-1.x86_64          |
      | remove        | CQRlib-0:1.1.2-16.fc29.x86_64             |
      | remove        | CQRlib-devel-0:1.1.2-16.fc29.x86_64       |
      | remove        | CQRlib-extension-0:1.5-2.x86_64           |
  And I execute in dnf shell "install CQRlib-devel"
  And I execute in dnf shell "run"
  And Transaction is following
      | Action        | Package                                   |
      | install       | CQRlib-devel-0:1.1.2-16.fc29.x86_64       |
 When I execute in dnf shell "exit"
 Then stdout contains "Leaving Shell"


Scenario: Switch groups by swap command
 When I open dnf shell session
  And I execute in dnf shell "groupinstall CQRlib-non-devel"
  And I execute in dnf shell "run"
 Then Transaction is following
      | Action        | Package                                   |
      | install-group | CQRlib-0:1.1.2-16.fc29.x86_64             |
      | install-group | CQRlib-extension-0:1.5-2.x86_64           |
      | group-install | CQRlib-non-devel                          |
  And I execute in dnf shell "install CQRlib-devel"
  And I execute in dnf shell "run"
  And Transaction is following
      | Action        | Package                                   |
      | install       | CQRlib-devel-0:1.1.2-16.fc29.x86_64       |
  And I execute in dnf shell "swap @CQRlib-non-devel @SuperRipper-and-deps"
  And I execute in dnf shell "run"
  And Transaction is following
      | Action        | Package                                   |
      | remove        | CQRlib-extension-0:1.5-2.x86_64           |
      | install-group | SuperRipper-extension-0:1.1-1.x86_64      |
      | install-group | SuperRipper-0:1.0-1.x86_64                |
      | install-group | abcde-0:2.9.3-1.fc29.noarch               |
      | install-group | flac-0:1.3.3-3.fc29.x86_64                |
      | install-group | wget-0:1.19.6-5.fc29.x86_64               |
      | install-group | FlacBetterEncoder-0:1.0-1.x86_64          |
      | group-remove  | CQRlib-non-devel                          |
      | group-install | SuperRipper-and-deps                      |
