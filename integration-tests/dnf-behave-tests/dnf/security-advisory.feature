Feature: Test upgrade, upgrade-minimal, and check-ugrade with advisory, cve, secseverity


Background: Use repository with advisories
  Given I use repository "dnf-ci-security"
   When I execute dnf with args "install advisory_A-1.0-1 advisory_B-1.0-1"
   Then the exit code is 0
    And Transaction is following
        | Action        | Package                   |
        | install       | advisory_A-0:1.0-1.x86_64 |
        | install       | advisory_B-0:1.0-1.x86_64 |


Scenario: upgrade-minimal cve and advisory
   When I execute dnf with args "upgrade-minimal --cve CVE-001 --advisory DNF-BUGFIX-001"
   Then the exit code is 0
    And Transaction is following
        | Action        | Package                   |
        | upgrade       | advisory_A-0:1.0-2.x86_64 |
        | upgrade       | advisory_B-0:1.0-2.x86_64 |

Scenario: check-upgrade --minimal cve and advisory
   When I execute dnf with args "check-upgrade --minimal --cves CVE-001 --advisories DNF-BUGFIX-001"
   Then the exit code is 100
    And stderr is
        """
        <REPOSYNC>
        """
    And stdout matches line by line
        """
        Upgrades
        advisory_A.x86_64 +1.0-2 +dnf-ci-security
        advisory_B.x86_64 +1.0-2 +dnf-ci-security
        """

Scenario: upgrade-minimal with pkgs specified cve and advisory
   When I execute dnf with args "upgrade-minimal advisory_A advisory_B --cve CVE-001 --advisory DNF-BUGFIX-001"
   Then the exit code is 0
    And Transaction is following
        | Action        | Package                   |
        | upgrade       | advisory_A-0:1.0-2.x86_64 |
        | upgrade       | advisory_B-0:1.0-2.x86_64 |

Scenario: check-upgrade --minimal with pkgs specified cve and advisory
   When I execute dnf with args "check-upgrade --minimal advisory_A advisory_B --cves CVE-001 --advisories DNF-BUGFIX-001"
   Then the exit code is 100
    And stderr is
        """
        <REPOSYNC>
        """
    And stdout matches line by line
        """
        Upgrades
        advisory_A.x86_64 +1.0-2 +dnf-ci-security
        advisory_B.x86_64 +1.0-2 +dnf-ci-security
        """

Scenario: upgrade advisories
   When I execute dnf with args "upgrade --advisories=DNF-BUGFIX-001 --advisories=DNF-SECURITY-004"
   Then the exit code is 0
    And Transaction is following
        | Action        | Package                   |
        | upgrade       | advisory_A-0:1.0-4.x86_64 |
        | upgrade       | advisory_B-0:1.0-4.x86_64 |

Scenario: upgrade --minimal advisories
   When I execute dnf with args "upgrade --minimal --advisories=DNF-BUGFIX-001 --advisories=DNF-SECURITY-004"
   Then the exit code is 0
    And Transaction is following
        | Action        | Package                   |
        | upgrade       | advisory_A-0:1.0-2.x86_64 |
        | upgrade       | advisory_B-0:1.0-4.x86_64 |

Scenario: check-upgrade --minimal advisories
   When I execute dnf with args "check-upgrade --minimal --advisories=DNF-BUGFIX-001 --advisories=DNF-SECURITY-004"
   Then the exit code is 100
    And stderr is
        """
        <REPOSYNC>
        """
    And stdout matches line by line
        """
        Upgrades
        advisory_A.x86_64 +1.0-2 +dnf-ci-security
        advisory_B.x86_64 +1.0-4 +dnf-ci-security
        """

Scenario: upgrade cves
   When I execute dnf with args "upgrade --cve CVE-001 --cve CVE-002"
   Then the exit code is 0
    And Transaction is following
        | Action        | Package                   |
        | upgrade       | advisory_B-0:1.0-4.x86_64 |

Scenario: check-upgrade cves
   When I execute dnf with args "check-upgrade --cves CVE-001 --cves CVE-002"
   Then the exit code is 100
    And stderr is
        """
        <REPOSYNC>
        """
    And stdout matches line by line
        """
        Upgrades
        advisory_B.x86_64 +1.0-4 +dnf-ci-security
        """

Scenario: upgrade-minimal sec-severity
   When I execute dnf with args "upgrade-minimal --advisory-severities=Moderate"
   Then the exit code is 0
    And Transaction is following
        | Action        | Package                   |
        | upgrade       | advisory_B-0:1.0-2.x86_64 |

Scenario: check-upgrade --minimal with sec-severity
   When I execute dnf with args "check-upgrade --minimal --advisory-severities=Moderate"
   Then the exit code is 100
    And stderr is
        """
        <REPOSYNC>
        """
    And stdout matches line by line
        """
        Upgrades
        advisory_B.x86_64 +1.0-2 +dnf-ci-security
        """

Scenario: upgrade-minimal with pkgs specified sec-severity
   When I execute dnf with args "upgrade-minimal advisory_B --advisory-severities=Moderate"
   Then the exit code is 0
    And Transaction is following
        | Action        | Package                   |
        | upgrade       | advisory_B-0:1.0-2.x86_64 |

Scenario: check-upgrade --minimal with pkgs specified sec-severity
   When I execute dnf with args "check-upgrade --minimal advisory_B --advisory-severities=Moderate"
   Then the exit code is 100
    And stderr is
        """
        <REPOSYNC>
        """
    And stdout matches line by line
        """
        Upgrades
        advisory_B.x86_64 +1.0-2 +dnf-ci-security
        """

Scenario: upgrade secseverity
   When I execute dnf with args "upgrade --advisory-severities Critical"
   Then the exit code is 0
    And Transaction is following
        | Action        | Package                   |
        | upgrade       | advisory_B-0:1.0-4.x86_64 |


Scenario: upgrade-minimal security plus bugfix
   When I execute dnf with args "upgrade-minimal --security --bugfix"
   Then the exit code is 0
    And Transaction is following
        | Action        | Package                   |
        | upgrade       | advisory_A-0:1.0-3.x86_64 |
        | upgrade       | advisory_B-0:1.0-4.x86_64 |

Scenario: check-upgrade --minimal security plus bugfix
   When I execute dnf with args "check-upgrade --minimal --security --bugfix"
   Then the exit code is 100
    And stderr is
        """
        <REPOSYNC>
        """
    And stdout matches line by line
        """
        Upgrades
        advisory_A.x86_64 +1.0-3 +dnf-ci-security
        advisory_B.x86_64 +1.0-4 +dnf-ci-security
        """
