# destructive because it creates/overwrites /usr/share/locale/de/LC_MESSAGES/libdnf5.mo file
@destructive
Feature: Upgrade packages already downloaded to the cache

Background: Install some RPMs from one repository
  Given I use repository "dnf-ci-fedora"
   When I execute dnf with args "install wget"
   Then the exit code is 0
    And Transaction is following
        | Action        | Package                                   |
        | install       | wget-0:1.19.5-5.fc29.x86_64               |

@bz2024527
@bz2070966
@bz2070967
Scenario: Upgrade works correctly with non-english locale when packages were already downloaded to the cache
  Given I use repository "dnf-ci-fedora-updates" as http
    # pre-download updates to the cache
    And I successfully execute dnf with args "upgrade --downloadonly"
    # In UBI containers rpm does not install translation files (see /etc/rpm/macros.image-language-conf).
    # We need to copy german dnf.mo to proper location in order to be able to run dnf in german locales.
    And I copy file "{context.dnf.fixturesdir}/data/dnf-translations/libdnf5.mo" to "//usr/share/locale/de/LC_MESSAGES/libdnf5.mo"
    And I set LC_ALL to "de_DE.utf-8"
   When I execute dnf with args "upgrade"
   Then the exit code is 0
    # make sure dnf was running in german (stdout contains translated "Already downloaded"
    And stderr contains ">>> Bereits heruntergeladen"
    # cannot do `And Transaction is following` because changed locales break transaction table parsing
    And RPMDB Transaction is following
        | Action        | Package                                   |
        | upgrade       | wget-0:1.19.6-5.fc29.x86_64               |
