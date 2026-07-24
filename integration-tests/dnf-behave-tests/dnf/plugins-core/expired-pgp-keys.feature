Feature: expired-pgp-keys plugin functionality


Background:
  Given I enable plugin "expired-pgp-keys"
    And I configure dnf with
      | key            | value                                              |
      | pluginconfpath | {context.dnf.installroot}/etc/dnf/libdnf5-plugins  |
    And I create file "/etc/dnf/libdnf5-plugins/expired-pgp-keys.conf" with
    """
    [main]
    enabled = 1
    """
    And I use repository "dnf-ci-gpg-expiry" with configuration
        | key      | value                                                                                    |
        | gpgcheck | 1                                                                                        |
        | gpgkey   | file://{context.dnf.fixturesdir}/gpgkeys/keys/dnf-ci-gpg-expiry/dnf-ci-gpg-expiry-public |
    And I use repository "simple-base"
    And I successfully execute dnf with args "install wget"


Scenario: When OpenPGP key is expired, its removal is triggered before transaction
  Given I move the clock forward to "2 years"
    And I successfully execute dnf with args "install vagare"
   Then stderr contains lines matching
    """
    The following (Open)?PGP key \(0x.*\) is about to be removed:
     Reason     : Expired on .*
     UserID     : "dnf-ci-gpg-expiry"
    """


Scenario: When OpenPGP key is expired, its removal is not triggered on non-transactional operations
  Given I move the clock forward to "2 years"
    And I successfully execute dnf with args "repoquery vagare"
   Then stderr does not contain "The following (Open)?PGP key \(0x.*\) is about to be removed:"
