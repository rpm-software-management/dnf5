Feature: Check when there are multiple problems


Background: Force installation of an RPM that will cause problems with dependencies, duplicates and obsoletes
  Given I use repository "dnf-ci-fedora"
   When I execute dnf with args "install glibc"
   Then the exit code is 0
    And Transaction is following
        | Action        | Package                                   |
        | install       | glibc-0:2.28-9.fc29.x86_64                |
        | install-dep   | setup-0:2.12.1-1.fc29.noarch              |
        | install-dep   | filesystem-0:3.9-2.fc29.x86_64            |
        | install-dep   | basesystem-0:11-6.fc29.noarch             |
        | install-dep   | glibc-common-0:2.28-9.fc29.x86_64         |
        | install-dep   | glibc-all-langpacks-0:2.28-9.fc29.x86_64  |
   When I execute rpm with args "-i --nodeps {context.dnf.fixturesdir}/repos/dnf-ci-fedora-updates/x86_64/glibc-2.28-26.fc29.x86_64.rpm"
   Then the exit code is 0


Scenario: Check
   When I execute dnf with args "check"
   Then the exit code is 1
    And stdout is
        """
        glibc-0:2.28-26.fc29.x86_64
         missing require "glibc-common = 2.28-26.fc29"
         missing require "glibc-langpack = 2.28-26.fc29"
         duplicate with "glibc-0:2.28-9.fc29.x86_64"
        glibc-0:2.28-9.fc29.x86_64
         obsoleted by "glibc < 2.28-10.fc29" from "glibc-0:2.28-26.fc29.x86_64"
         duplicate with "glibc-0:2.28-26.fc29.x86_64"
        """
    And stderr is
        """
        Check discovered 5 problem(s) in 2 package(s)
        """


Scenario: Check --dependencies
   When I execute dnf with args "check --dependencies"
   Then the exit code is 1
    And stdout is
        """
        glibc-0:2.28-26.fc29.x86_64
         missing require "glibc-common = 2.28-26.fc29"
         missing require "glibc-langpack = 2.28-26.fc29"
        """
    And stderr is
        """
        Check discovered 2 problem(s) in 1 package(s)
        """


Scenario: Check --duplicates
   When I execute dnf with args "check --duplicates"
   Then the exit code is 1
    And stdout is
        """
        glibc-0:2.28-26.fc29.x86_64
         duplicate with "glibc-0:2.28-9.fc29.x86_64"
        glibc-0:2.28-9.fc29.x86_64
         duplicate with "glibc-0:2.28-26.fc29.x86_64"
        """
    And stderr is
        """
        Check discovered 2 problem(s) in 2 package(s)
        """


Scenario: Check --obsoleted
   When I execute dnf with args "check --obsoleted"
   Then the exit code is 1
    And stdout is
        """
        glibc-0:2.28-9.fc29.x86_64
         obsoleted by "glibc < 2.28-10.fc29" from "glibc-0:2.28-26.fc29.x86_64"
        """
    And stderr is
        """
        Check discovered 1 problem(s) in 1 package(s)
        """
