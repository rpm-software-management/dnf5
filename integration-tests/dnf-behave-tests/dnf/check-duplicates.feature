Feature: Check when there are duplicate packages


Background: Force installation of two different versions of an RPM
  Given I use repository "dnf-ci-fedora-updates"
   When I execute dnf with args "install flac-1.3.3-1.fc29"
   Then the exit code is 0
    And Transaction is following
        | Action        | Package                          |
        | install       | flac-0:1.3.3-1.fc29.x86_64       |
   When I execute rpm with args "-i {context.dnf.fixturesdir}/repos/dnf-ci-fedora-updates/x86_64/flac-1.3.3-3.fc29.x86_64.rpm"
   Then the exit code is 0


Scenario Outline: Check <option>
   When I execute dnf with args "check <option>"
   Then the exit code is 1
    And stdout is
        """
        flac-0:1.3.3-1.fc29.x86_64
         duplicate with "flac-0:1.3.3-3.fc29.x86_64"
        flac-0:1.3.3-3.fc29.x86_64
         duplicate with "flac-0:1.3.3-1.fc29.x86_64"
        """
    And stderr is
        """
        Check discovered 2 problem(s) in 2 package(s)
        """

Examples:
        | option             |
        # no option defaults to "all"
        |                    |
        | --duplicates       |


Scenario Outline: Check <option>
   When I execute dnf with args "check <option>"
   Then the exit code is 0
    And stdout is empty
    And stderr is empty

Examples:
        | option             |
        | --dependencies     |
        | --obsoleted        |
