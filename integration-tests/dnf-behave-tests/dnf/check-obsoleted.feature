Feature: Check when there is a package that obsoletes another installed package


Background: Force installation of an obsoleted RPM
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
   When I execute rpm with args "-i --nodeps {context.dnf.fixturesdir}/repos/dnf-ci-thirdparty/x86_64/glibc-profile-2.3.1-10.x86_64.rpm"
   Then the exit code is 0


Scenario Outline: Check <option>
   When I execute dnf with args "check <option>"
   Then the exit code is 1
    And stdout is
        """
        glibc-profile-0:2.3.1-10.x86_64
         obsoleted by "glibc-profile < 2.4" from "glibc-0:2.28-9.fc29.x86_64"
        """
    And stderr is
        """
        Check discovered 1 problem(s) in 1 package(s)
        """

Examples:
        | option             |
        # no option defaults to "all"
        |                    |
        | --obsoleted        |


Scenario Outline: Check <option>
   When I execute dnf with args "check <option>"
   Then the exit code is 0
    And stdout is empty
    And stderr is empty

Examples:
        | option             |
        | --dependencies     |
        | --duplicates       |
