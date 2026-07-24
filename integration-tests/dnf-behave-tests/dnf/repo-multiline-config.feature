Feature: correctly reading multiline repo config


Scenario: multiline config for gpg works with local repo
Given I use repository "dnf-ci-gpg"
  And I create and substitute file "/etc/yum.repos.d/dnf-ci-gpg.repo" with
      """
      [dnf-ci-gpg]
      name=dnf-ci-gpg test repository
      enabled=1
      baseurl=file://{context.dnf.fixturesdir}/repos/dnf-ci-gpg
      gpgcheck=1
      gpgkey=file://{context.dnf.fixturesdir}/gpgkeys/keys/dnf-ci-gpg/dnf-ci-gpg-public
             file://{context.dnf.fixturesdir}/gpgkeys/keys/dnf-ci-gpg-subkey/dnf-ci-gpg-subkey-public
      """
 When I execute dnf with args "install setup"
 Then the exit code is 0
  And Transaction is following
      | Action        | Package                      |
      | install       | setup-0:2.12.1-1.fc29.noarch |


@bz1807864
Scenario: multiline config for gpg works with remote repo
Given I use repository "dnf-ci-gpg" as http
  And I create and substitute file "/etc/yum.repos.d/dnf-ci-gpg.repo" with
      """
      [dnf-ci-gpg]
      name=dnf-ci-gpg test repository
      enabled=1
      baseurl=http://localhost:{context.dnf.ports[dnf-ci-gpg]}
      gpgcheck=1
      gpgkey=file://{context.dnf.fixturesdir}/gpgkeys/keys/dnf-ci-gpg/dnf-ci-gpg-public
             file://{context.dnf.fixturesdir}/gpgkeys/keys/dnf-ci-gpg-subkey/dnf-ci-gpg-subkey-public
      """
 When I execute dnf with args "install setup"
 Then the exit code is 0
  And Transaction is following
      | Action        | Package                      |
      | install       | setup-0:2.12.1-1.fc29.noarch |


@bz1807864
Scenario: multiline multivalue comma and space separated config for gpg works with remote repo
Given I use repository "dnf-ci-gpg" as http
  And I create and substitute file "/etc/yum.repos.d/dnf-ci-gpg.repo" with
      """
      [dnf-ci-gpg]
      name=dnf-ci-gpg test repository
      enabled=1
      baseurl=http://localhost:{context.dnf.ports[dnf-ci-gpg]}
      gpgcheck=1
      gpgkey=file://{context.dnf.fixturesdir}/gpgkeys/keys/dnf-ci-gpg/dnf-ci-gpg-public file://{context.dnf.fixturesdir}/gpgkeys/keys/dnf-ci-gpg-noeol/dnf-ci-gpg-noeol-public
             file://{context.dnf.fixturesdir}/gpgkeys/keys/dnf-ci-gpg-subkey/dnf-ci-gpg-subkey-public, file://{context.dnf.fixturesdir}/gpgkeys/keys/dnf-ci-gpg-updates/dnf-ci-gpg-updates-public
      """
 When I execute dnf with args "install setup"
 Then the exit code is 0
  And Transaction is following
      | Action        | Package                      |
      | install       | setup-0:2.12.1-1.fc29.noarch |


Scenario: multiline config for baseurl
Given I use repository "dnf-ci-gpg" as http
  And I create and substitute file "/etc/yum.repos.d/dnf-ci-gpg.repo" with
      """
      [dnf-ci-gpg]
      name=dnf-ci-gpg test repository
      enabled=1
      baseurl=http://invalid.url file:///also/invalid
              file:///the/last/is/valid, http://localhost:{context.dnf.ports[dnf-ci-gpg]}
      gpgcheck=0
      """
 When I execute dnf with args "install setup"
 Then the exit code is 0
  And Transaction is following
      | Action        | Package                      |
      | install       | setup-0:2.12.1-1.fc29.noarch |
