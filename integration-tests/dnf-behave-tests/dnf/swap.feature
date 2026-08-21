Feature: Test for swap command


Background: Enable repositories
  Given I use repository "dnf-ci-fedora"
  Given I use repository "dnf-ci-fedora-updates"
  Given I use repository "dnf-ci-thirdparty"


Scenario: Switch packages by swap command
   When I execute dnf with args "install CQRlib-devel CQRlib"
   Then the exit code is 0
    And Transaction is following
        | Action        | Package                                   |
        | install       | CQRlib-0:1.1.2-16.fc29.x86_64             |
        | install       | CQRlib-devel-0:1.1.2-16.fc29.x86_64       |
   When I execute dnf with args "swap CQRlib SuperRipper"
   Then the exit code is 0
    And Transaction is following
        | Action        | Package                                   |
        | remove        | CQRlib-0:1.1.2-16.fc29.x86_64             |
        | install       | SuperRipper-0:1.0-1.x86_64                |
        | install-dep   | abcde-0:2.9.3-1.fc29.noarch               |
        | install-dep   | wget-0:1.19.6-5.fc29.x86_64               |
        | install-weak  | FlacBetterEncoder-0:1.0-1.x86_64          |
        | install-weak  | flac-0:1.3.3-3.fc29.x86_64                |


Scenario: Switch packages and their subpackages by swap command with wildcards
   When I execute dnf with args "install CQRlib-devel CQRlib CQRlib-extension"
   Then the exit code is 0
    And Transaction is following
        | Action        | Package                                   |
        | install       | CQRlib-0:1.1.2-16.fc29.x86_64             |
        | install       | CQRlib-devel-0:1.1.2-16.fc29.x86_64       |
        | install       | CQRlib-extension-0:1.5-2.x86_64           |
   When I execute dnf with args "swap CQRlib\* SuperRipper\*"
   Then the exit code is 0
    And Transaction is following
        | Action        | Package                                   |
        | remove        | CQRlib-0:1.1.2-16.fc29.x86_64             |
        | remove        | CQRlib-devel-0:1.1.2-16.fc29.x86_64       |
        | remove        | CQRlib-extension-0:1.5-2.x86_64           |
        | install       | SuperRipper-extension-0:1.1-1.x86_64      |
        | install       | SuperRipper-0:1.0-1.x86_64                |
        | install-dep   | abcde-0:2.9.3-1.fc29.noarch               |
        | install-dep   | wget-0:1.19.6-5.fc29.x86_64               |
        | install-weak  | flac-0:1.3.3-3.fc29.x86_64                |
        | install-weak  | FlacBetterEncoder-0:1.0-1.x86_64          |
   When I execute dnf with args "install CQRlib-devel"
   Then the exit code is 0
    And Transaction is following
        | Action        | Package                                   |
        | install       | CQRlib-devel-0:1.1.2-16.fc29.x86_64       |


# Reported as https://github.com/rpm-software-management/dnf5/issues/916
@xfail
Scenario: Switch groups by swap command
   When I execute dnf with args "group install cqrlib-non-devel"
   Then the exit code is 0
    And Transaction is following
        | Action        | Package                                   |
        | install-group | CQRlib-0:1.1.2-16.fc29.x86_64             |
        | install-group | CQRlib-extension-0:1.5-2.x86_64           |
        | group-install | CQRlib-non-devel                          |
   When I execute dnf with args "install CQRlib-devel"
   Then the exit code is 0
    And Transaction is following
        | Action        | Package                                   |
        | install       | CQRlib-devel-0:1.1.2-16.fc29.x86_64       |
   When I execute dnf with args "swap @CQRlib-non-devel @SuperRipper-and-deps"
   Then the exit code is 0
    And Transaction is following
        | Action        | Package                                   |
        | unchanged     | CQRlib-0:1.1.2-16.fc29.x86_64             |
        | remove        | CQRlib-extension-0:1.5-2.x86_64           |
        | install-group | SuperRipper-extension-0:1.1-1.x86_64      |
        | install-group | SuperRipper-0:1.0-1.x86_64                |
        | install-group | abcde-0:2.9.3-1.fc29.noarch               |
        | install-group | wget-0:1.19.6-5.fc29.x86_64               |
        | install-group | flac-0:1.3.3-3.fc29.x86_64                |
        | install-group | FlacBetterEncoder-0:1.0-1.x86_64          |
        | group-remove  | CQRlib-non-devel                          |
        | group-install | SuperRipper-and-deps                      |


@bz2036434
Scenario: Swap packages using rpm file path
  Given I successfully execute dnf with args "install CQRlib-devel CQRlib"
   When I execute dnf with args "swap CQRlib {context.dnf.fixturesdir}/repos/dnf-ci-thirdparty/x86_64/SuperRipper-1.0-1.x86_64.rpm"
   Then the exit code is 0
    And Transaction is following
        | Action        | Package                                   |
        | remove        | CQRlib-0:1.1.2-16.fc29.x86_64             |
        | install       | SuperRipper-0:1.0-1.x86_64                |
        | install-dep   | abcde-0:2.9.3-1.fc29.noarch               |
        | install-dep   | wget-0:1.19.6-5.fc29.x86_64               |
        | install-weak  | FlacBetterEncoder-0:1.0-1.x86_64          |
        | install-weak  | flac-0:1.3.3-3.fc29.x86_64                |
