# gpgkey file:// URLs are resolved on the host, not in the installroot, and
# auto_import_local_keys only trusts keys inside /etc/pki/rpm-gpg on the host.
# The scenarios therefore stage the fixture key on the host (the "//" prefix
# escapes the installroot), which is why the feature is destructive.
@destructive
Feature: Import local OpenPGP keys without confirmation with auto_import_local_keys


Background: A fixture key is installed in the trusted directory on the host
  Given I create directory "//etc/pki/rpm-gpg"
    And I copy file "{context.dnf.fixturesdir}/gpgkeys/keys/dnf-ci-gpg/dnf-ci-gpg-public" to "//etc/pki/rpm-gpg/RPM-GPG-KEY-dnf-ci-gpg"


Scenario: Repository metadata key is imported without confirmation
  Given I do not assume yes
    And I copy repository "simple-base" for modification
    And I sign repository "simple-base" metadata with "{context.dnf.fixturesdir}/gpgkeys/keys/dnf-ci-gpg/dnf-ci-gpg-private"
    And I use repository "simple-base" with configuration
        | key                    | value                                          |
        | pkg_gpgcheck           | 0                                              |
        | repo_gpgcheck          | 1                                              |
        | auto_import_local_keys | 1                                              |
        | gpgkey                 | file:///etc/pki/rpm-gpg/RPM-GPG-KEY-dnf-ci-gpg |
   When I execute dnf with args "makecache"
   Then the exit code is 0
    And stderr contains "The key was successfully imported."
      # The key details and the import question are never shown
    And stderr does not contain "Importing OpenPGP key"
    And stderr does not contain "Is this ok"


Scenario: Package signing key is imported into the RPM database without confirmation
  Given I use repository "dnf-ci-gpg" with configuration
        | key                    | value                                          |
        | pkg_gpgcheck           | 1                                              |
        | repo_gpgcheck          | 0                                              |
        | auto_import_local_keys | 1                                              |
        | gpgkey                 | file:///etc/pki/rpm-gpg/RPM-GPG-KEY-dnf-ci-gpg |
   When I execute dnf with args "install setup"
   Then the exit code is 0
    And Transaction is following
        | Action        | Package                      |
        | install       | setup-0:2.12.1-1.fc29.noarch |
      # The key details and the import question are never shown
    And stderr does not contain "Importing OpenPGP key"
      # The key is now in the RPM database
      # (the braces are doubled because there is .format() used for the string)
   When I execute rpm with args "-q gpg-pubkey --qf 'gpg(%{{packager}})\n'"
   Then the exit code is 0
    And stdout contains "gpg\(dnf-ci-gpg\)"


Scenario: Nothing is imported with assumeno
  Given I do not assume yes
    And I use repository "dnf-ci-gpg" with configuration
        | key                    | value                                          |
        | pkg_gpgcheck           | 1                                              |
        | repo_gpgcheck          | 0                                              |
        | auto_import_local_keys | 1                                              |
        | gpgkey                 | file:///etc/pki/rpm-gpg/RPM-GPG-KEY-dnf-ci-gpg |
   When I execute dnf with args "install setup --assumeno"
   Then the exit code is 1
    And RPMDB Transaction is empty
   When I execute rpm with args "-q gpg-pubkey --qf 'gpg(%{{packager}})\n'"
   Then the exit code is 0
    And stdout does not contain "gpg\(dnf-ci-gpg\)"


Scenario: Symlink in the trusted directory pointing outside of it still asks for confirmation
  Given I copy file "{context.dnf.fixturesdir}/gpgkeys/keys/dnf-ci-gpg/dnf-ci-gpg-public" to "//tmp/RPM-GPG-KEY-dnf-ci-gpg-untrusted"
    And I create symlink "//etc/pki/rpm-gpg/RPM-GPG-KEY-linked" to file "//tmp/RPM-GPG-KEY-dnf-ci-gpg-untrusted"
    And I use repository "dnf-ci-gpg" with configuration
        | key                    | value                                      |
        | pkg_gpgcheck           | 1                                          |
        | repo_gpgcheck          | 0                                          |
        | auto_import_local_keys | 1                                          |
        | gpgkey                 | file:///etc/pki/rpm-gpg/RPM-GPG-KEY-linked |
   When I execute dnf with args "install setup"
   Then the exit code is 0
      # The key went through the confirmation (accepted by -y), not the automatic path
    And stderr contains "Importing OpenPGP key"
    And stderr contains "The key was successfully imported."
