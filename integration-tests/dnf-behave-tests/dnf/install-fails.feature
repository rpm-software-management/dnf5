Feature: Installing attemps fail

@bz1568965
Scenario: Report all missing dependencies
   Given I use repository "dnf-ci-thirdparty"
    When I execute dnf with args "install SuperRipper anitras-dance"
    Then stderr contains "nothing provides abcde needed by SuperRipper-1.0-1.x86_64 from dnf-ci-thirdparty"
    Then stderr contains "nothing provides nodejs needed by anitras-dance-1.0-1.x86_64 from dnf-ci-thirdparty"

@bz1599774
Scenario: Report error when installing empty file
   Given I set working directory to "{context.dnf.installroot}"
     And I execute "touch empty.rpm"
    When I execute dnf with args "install empty.rpm"
    Then the exit code is 1
     And stderr is
         """
         <REPOSYNC>
         Failed to load RPM "empty.rpm": empty.rpm: not a rpm
         """

@bz1616321
Scenario: Report error when installing text file
   Given I set working directory to "{context.dnf.installroot}"
     And I create file "invalid.rpm" with
         """
         this is not rpm
         """
    When I execute dnf with args "install invalid.rpm"
    Then the exit code is 1
     And stderr is
         """
         <REPOSYNC>
         Failed to load RPM "invalid.rpm": invalid.rpm: not a rpm
         """

@bz1599774
Scenario: Report error when installing non-existing RPM file
    When I execute dnf with args "install no_such_file.rpm"
    Then the exit code is 1
     And stderr is
         """
         <REPOSYNC>
         Failed to access RPM "no_such_file.rpm": No such file or directory
         """

Scenario: Cannot install source rpm
   Given I use repository "simple-base"
    When I execute dnf with args "install vagare.src"
    Then the exit code is 1
     And stderr is
         """
         <REPOSYNC>
         Failed to resolve the transaction:
         Argument 'vagare.src' matches only source packages.
         """
