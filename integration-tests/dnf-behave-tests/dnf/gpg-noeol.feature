@dnf5daemon
Feature: Testing gpg signed packages by keys without any EOL characters at EOF

# Masterkey signed packages in repository dnf-ci-gpg-noeol:
#     abcde
#     wget

Background: Add repository with gpgcheck=1
  Given I use repository "dnf-ci-gpg-noeol" with configuration
        | key      | value                                                                                  |
        | gpgcheck | 1                                                                                      |
        | gpgkey   | file://{context.dnf.fixturesdir}/gpgkeys/keys/dnf-ci-gpg-noeol/dnf-ci-gpg-noeol-public |
   When I execute dnf with args "repolist"
   Then the exit code is 0
    And stdout contains "dnf-ci-gpg-noeol\s+dnf-ci-gpg-noeol"
      # At the start of each test, there is only one (default) key
   When I execute rpm with args "-q gpg-pubkey"
   Then the exit code is 0
    And stdout matches line by line
        """
        gpg-pubkey-*
        """


# used to be disabled for Fedora due to a rpm failure,
# however, it works OK now (F39)
@bz1733971
Scenario: Import the GPG key without any EOL characters at EOF
   When I execute rpm with args "--import {context.dnf.fixturesdir}/gpgkeys/keys/dnf-ci-gpg-noeol/dnf-ci-gpg-noeol-public"
   Then the exit code is 0


Scenario: Install signed package from repository
   When I execute dnf with args "install abcde"
   Then the exit code is 0
    And Transaction is following
        | Action        | Package                     |
        | install       | abcde-0:2.9.2-1.fc29.noarch |
        | install-dep   | wget-0:1.19.5-5.fc29.x86_64 |
