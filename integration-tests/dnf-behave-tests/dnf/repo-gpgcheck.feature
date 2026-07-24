Feature: Checking GPG signatures of repository metadata

Background: Sign simple-base repo and serve its public key
  Given I copy repository "simple-base" for modification
    And I sign repository "simple-base" metadata with "{context.dnf.fixturesdir}/gpgkeys/keys/dnf-ci-gpg/dnf-ci-gpg-private"
    And I start http server "key_server" at "{context.dnf.fixturesdir}/gpgkeys/keys/dnf-ci-gpg"

Scenario: Remote gpg key is imported for local repo
  Given I use repository "simple-base" with configuration
        | key           | value                                                              |
        | pkg_gpgcheck  | 0                                                                  |
        | repo_gpgcheck | 1                                                                  |
        | gpgkey        | http://localhost:{context.dnf.ports[key_server]}/dnf-ci-gpg-public |
   When I execute dnf with args "install labirinto"
   Then the exit code is 0
    And Transaction is following
        | Action        | Package                       |
        | install       | labirinto-0:1.0-1.fc29.x86_64 |
    And stderr contains "The key was successfully imported."


Scenario: Remote gpg key is imported for remote repo
  Given I configure repository "simple-base" with
        | key           | value                                                              |
        | pkg_gpgcheck  | 0                                                                  |
        | repo_gpgcheck | 1                                                                  |
        | gpgkey        | http://localhost:{context.dnf.ports[key_server]}/dnf-ci-gpg-public |
    And I use repository "simple-base" as http
   When I execute dnf with args "install labirinto"
   Then the exit code is 0
    And Transaction is following
        | Action        | Package                       |
        | install       | labirinto-0:1.0-1.fc29.x86_64 |
    And stderr contains "The key was successfully imported."


Scenario: Remote gpg key is imported for already cached repo
  Given I configure repository "simple-base" with
        | key           | value                                                              |
        | pkg_gpgcheck  | 0                                                                  |
        | repo_gpgcheck | 0                                                                  |
        | gpgkey        | http://localhost:{context.dnf.ports[key_server]}/dnf-ci-gpg-public |
    And I use repository "simple-base" as http
    And I successfully execute dnf with args "install labirinto"
    And I configure repository "simple-base" with
        | key           | value |
        | repo_gpgcheck | 1     |
   When I execute dnf with args "reinstall labirinto"
   Then the exit code is 0
    And Transaction is following
        | Action        | Package                       |
        | reinstall     | labirinto-0:1.0-1.fc29.x86_64 |
    And stderr contains "The key was successfully imported."
