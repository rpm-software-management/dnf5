Feature: Checking GPG signatures of transaction packages


Background:
  Given I use repository "dnf-ci-gpg"


Scenario: Check GPG signatures with gpgcheck turned off
   When I execute dnf with args "install wget --no-gpgchecks"
   Then the exit code is 0
    And Transaction is following
        | Action        | Package                        |
        | install       | wget-0:1.19.5-5.fc29.x86_64    |


Scenario: Check GPG signatures with recovery when keys are imported
  Given I configure repository "dnf-ci-gpg" with
        | key      | value                                                                          |
        | gpgcheck | 1                                                                              |
        | gpgkey   | file://{context.dnf.fixturesdir}/gpgkeys/keys/dnf-ci-gpg/dnf-ci-gpg-public     |
   When I execute dnf with args "install setup"
   Then the exit code is 0
    And Transaction is following
        | Action        | Package                        |
        | install       | setup-0:2.12.1-1.fc29.noarch   |
    And stderr contains "The key was successfully imported."


Scenario: Remote gpg key is imported for packages
  Given I start http server "key_server" at "{context.dnf.fixturesdir}/gpgkeys/keys/dnf-ci-gpg"
    And I use repository "dnf-ci-gpg" with configuration
        | key           | value                                                              |
        | pkg_gpgcheck  | 1                                                                  |
        | repo_gpgcheck | 0                                                                  |
        | gpgkey        | http://localhost:{context.dnf.ports[key_server]}/dnf-ci-gpg-public |
   When I execute dnf with args "install setup"
   Then the exit code is 0
    And Transaction is following
        | Action        | Package                      |
        | install       | setup-0:2.12.1-1.fc29.noarch |
    And stderr contains "The key was successfully imported."
