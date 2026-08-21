Feature: Testing gpgcheck


# Masterkey signed packages in repository dnf-ci-gpg:
#     setup
#     abcde
#     broken-package
#     glibc
#     glibc-common
#     glibc-all-langpacks
# Subkey signed packages in repository dnf-ci-gpg:
#     filesystem
#     filesystem-content
# Incorrectly signed packages:
#     basesystem in dnf-ci-gpg is signed with key from dnf-ci-gpg-updates
#     basesystem in dnf-ci-gpg-updates is signed with key from dnf-ci-gpg


Background: Add repository with gpgcheck=1
  Given I use repository "dnf-ci-gpg" with configuration
        | key      | value                                                                                                                                                               |
        | gpgcheck | 1                                                                                                                                                                   |
        | gpgkey   | file://{context.dnf.fixturesdir}/gpgkeys/keys/dnf-ci-gpg/dnf-ci-gpg-public,file://{context.dnf.fixturesdir}/gpgkeys/keys/dnf-ci-gpg-subkey/dnf-ci-gpg-subkey-public |
   When I execute dnf with args "repolist"
   Then the exit code is 0
    And stdout contains "dnf-ci-gpg\s+dnf-ci-gpg"
      # At the start of each test, there is only one (default) key
   When I execute rpm with args "-q gpg-pubkey"
   Then the exit code is 0
    And stdout matches line by line
        """
        gpg-pubkey-*
        """


@dnf5daemon
Scenario Outline: Install <offline> masterkey signed package and check GPG key was imported
   When I execute dnf with args "install <offline> setup"
   Then the exit code is 0
    And <transaction> is following
        | Action        | Package                             |
        | install       | setup-0:2.12.1-1.fc29.noarch        |
      # There is now one imported gpg key in RPM db
      # (the braces are doubled because there is .format() used for the string)
   When I execute rpm with args "-q gpg-pubkey --qf 'gpg(%{{packager}})\n'"
   Then the exit code is 0
    And stdout contains "gpg\(dnf-ci-gpg\)"

Examples:
    | offline       | transaction       |
    |               | Transaction       |
    | --offline     | DNF Transaction   |


@dnf5daemon
Scenario Outline: Install <offline> subkey signed package with masterkey signed dependency
   When I execute dnf with args "install <offline> filesystem"
   Then the exit code is 0
    And <transaction> is following
        | Action        | Package                             |
        | install       | filesystem-0:3.9-2.fc29.x86_64      |
        | install-dep   | setup-0:2.12.1-1.fc29.noarch        |
   When I execute rpm with args "-q gpg-pubkey --qf 'gpg(%{{packager}})\n'"
   Then the exit code is 0
    And stdout contains "gpg\(dnf-ci-gpg\)"
    And stdout contains "gpg\(dnf-ci-gpg-subkey\)"

Examples:
    | offline       | transaction       |
    |               | Transaction       |
    | --offline     | DNF Transaction   |


# XXX stderr @dnf5daemon
Scenario Outline: Fail to <offline> install signed package with incorrectly signed dependency (with key from different repository)
   When I execute dnf with args "install <offline> glibc"
   Then the exit code is 1
    And DNF Transaction is following
        | Action        | Package                                   |
        | install       | glibc-0:2.28-9.fc29.x86_64                |
        | install-dep   | setup-0:2.12.1-1.fc29.noarch              |
        | install-dep   | filesystem-0:3.9-2.fc29.x86_64            |
        | install-dep   | basesystem-0:11-6.fc29.noarch             |
        | install-dep   | glibc-common-0:2.28-9.fc29.x86_64         |
        | install-dep   | glibc-all-langpacks-0:2.28-9.fc29.x86_64  |
    And RPMDB Transaction is empty
    And stderr contains lines matching
    """
    Transaction failed: Signature verification failed\.
    OpenPGP check for package "basesystem-11-6\.fc29\.noarch" \(.*/basesystem-11-6\.fc29\.noarch\.rpm\) from repo "dnf-ci-gpg" has failed: Public key is not installed\.
    """

Examples:
    | offline       |
    |               |
    | --offline     |


# XXX stderr @dnf5daemon
Scenario Outline: Fail to <offline> install signed package with incorrect checksum
   When I execute dnf with args "install <offline> broken-package"
   Then the exit code is 1
    And DNF Transaction is following
        | Action        | Package                               |
        | install       | broken-package-0:0.2.4-1.fc29.noarch  |
    And RPMDB Transaction is empty
    And stderr contains lines matching
    """
    Transaction failed: Signature verification failed\.
    OpenPGP check for package "broken-package-0\.2\.4-1\.fc29\.noarch" \(.*/broken-package-0\.2\.4-1\.fc29\.noarch\.rpm\) from repo "dnf-ci-gpg" has failed: Problem occurred when opening the package\.
    """

Examples:
    | offline       |
    |               |
    | --offline     |


@dnf5daemon
Scenario Outline: Install <offline> masterkey signed, unsigned and masterkey signed with unknown key packages from repo with gpgcheck=0 in repofile
  Given I configure repository "dnf-ci-gpg" with
        | key      | value                                                                      |
        | gpgcheck | 0                                                                          |
        | gpgkey   | file://{context.dnf.fixturesdir}/gpgkeys/keys/dnf-ci-gpg/dnf-ci-gpg-public |
   # install masterkey signed package
   When I execute dnf with args "install <offline> setup"
   Then the exit code is 0
   # install unsigned package
   When I execute dnf with args "install <offline> flac"
   Then the exit code is 0
   # install master signed with unknown key package
   When I execute dnf with args "install <offline> basesystem"
   Then the exit code is 0

Examples:
    | offline       |
    |               |
    | --offline     |


# XXX stderr @dnf5daemon
Scenario Outline: Attempt to <offline> install unsigned package from repo with gpgcheck=1
   When I execute dnf with args "install <offline> flac"
   Then the exit code is 1
    And stderr contains lines matching
    """
    Transaction failed: Signature verification failed\.
    OpenPGP check for package "flac-1\.3\.2-8\.fc29\.x86_64" \(.*/flac-1\.3\.2-8\.fc29\.x86_64\.rpm\) from repo "dnf-ci-gpg" has failed: The package is not signed\.
    """

Examples:
    | offline       |
    |               |
    | --offline     |


Scenario Outline: Install <offline> unsigned package from repository without gpgcheck set using option --no-gpgchecks
   When I execute dnf with args "install <offline> flac --no-gpgchecks"
   Then the exit code is 0
    And <transaction> is following
        | Action        | Package                             |
        | install       | flac-0:1.3.2-8.fc29.x86_64          |

Examples:
    | offline       | transaction       |
    |               | Transaction       |
    | --offline     | DNF Transaction   |


@bz1314405
Scenario Outline: Fail to <offline> install package with incorrect checksum with --no-gpgchecks
  Given I configure repository "dnf-ci-gpg" with
        | key      | value |
        | gpgcheck |       |
        | gpgkey   |       |
   When I execute dnf with args "install <offline> broken-package --no-gpgchecks"
   Then the exit code is 1
    And DNF Transaction is following
        | Action        | Package                               |
        | install       | broken-package-0:0.2.4-1.fc29.noarch  |
    And RPMDB Transaction is empty
    And stderr contains "Transaction failed: Rpm transaction failed."

Examples:
    | offline       |
    |               |
    | --offline     |


@bz1915990
@bz1932079
@bz1932089
@bz1932090
Scenario Outline: Refuse to <offline> install a package with broken gpg signature
  Given I drop repository "dnf-ci-gpg"
    And I use repository "dnf-ci-broken-rpm-signature" generated with exit code "2"
    And I configure repository "dnf-ci-broken-rpm-signature" with
        | key      | value                                                                                                                                                               |
        | gpgcheck | 1                                                                                                                                                                   |
        | gpgkey   | file://{context.dnf.fixturesdir}/gpgkeys/keys/dnf-ci-gpg/dnf-ci-gpg-public,file://{context.dnf.fixturesdir}/gpgkeys/keys/dnf-ci-gpg-subkey/dnf-ci-gpg-subkey-public |
   When I execute dnf with args "install <offline> setup"
   Then the exit code is 1
   # dnf must not extract any files from the broken package
   Then file "/usr/share/doc/setup/README" does not exist

Examples:
    | offline       |
    |               |
    | --offline     |


@xfail
# Reported as https://github.com/rpm-software-management/dnf5/issues/1790
@1941959
Scenario: Expire repo when failed to install package with incorrect checksum
  Given I drop repository "dnf-ci-gpg"
    And I use repository "dnf-ci-gpg" as http
    And I configure repository "dnf-ci-gpg" with
        | key      | value |
        | gpgcheck | 0     |
        | gpgkey   |       |
   When I execute dnf with args "install broken-package"
   Then the exit code is 1
    And DNF Transaction is following
        | Action        | Package                               |
        | install       | broken-package-0:0.2.4-1.fc29.noarch  |
    And RPMDB Transaction is empty
   # Check that repository dnf-ci-gpg is refreshed on the next run
   When I execute dnf with args "makecache"
   Then stderr matches line by line
   """
   Updating and loading repositories:
    dnf-ci-gpg test repository             100% |   .*
   Repositories loaded.
   """
