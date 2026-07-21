Feature: Test for dnf provides


Background: use dnf-ci-fedora repository
  Given I use repository "dnf-ci-fedora"


Scenario: dnf provides webclient - installed package wget provides webclient
   When I execute dnf with args "install wget glibc"
   Then the exit code is 0
  Given I drop repository "dnf-ci-fedora"
   When I execute dnf with args "provides webclient"
   Then stdout contains "wget-1.19.5-5.fc29[^R]+Repo[ \t]+: @System"
    And stdout does not contain "(glibc)|(setup)"


Scenario: dnf provides webclient - installed and in repo package wget provides webclient
   When I execute dnf with args "install wget glibc"
   Then the exit code is 0
   When I execute dnf with args "provides webclient"
   Then stdout contains "wget-1.19.5-5.fc29[^R]+Repo[ \t]+: @System"
   Then stdout contains "wget-1.19.5-5.fc29[^R]+Repo[ \t]+: dnf-ci-fedora"
    And stdout does not contain "(glibc)|(setup)"


Scenario: dnf provides webclient - installed and in repos package wget provides webclient
   When I execute dnf with args "install wget glibc"
   Then the exit code is 0
  Given I use repository "dnf-ci-fedora-updates"
   When I execute dnf with args "provides webclient"
   Then stdout contains "wget-1.19.5-5.fc29[^R]+Repo[ \t]+: @System"
   Then stdout contains "wget-1.19.5-5.fc29[^R]+Repo[ \t]+: dnf-ci-fedora"
   Then stdout contains "wget-1.19.6-5.fc29[^R]+Repo[ \t]+: dnf-ci-fedora-updates"
    And stdout does not contain "(glibc)|(setup)"

Scenario: dnf provides nonexistentprovide
   When I execute dnf with args "provides nonexistentprovde"
   Then the exit code is 1
    And stderr is
       """
       <REPOSYNC>
       No matches found. If searching for a file, try specifying the full path or using a wildcard prefix ("*/") at the beginning.
       """

Scenario: test order of provides "dnf provides webclient A B C"
   When I execute dnf with args "provides webclient A B C"
   Then stdout contains "wget-1.19.5-5.fc29[^R]+Repo[ \t]+: dnf-ci-fedora"
    And stdout does not contain "(glibc)|(setup)"
    And the exit code is 1
    And stderr is
        """
        <REPOSYNC>
        No matches found for A.
        No matches found for B.
        No matches found for C.
        If searching for a file, try specifying the full path or using a wildcard prefix ("*/") at the beginning.
        """

Scenario: test order of provides "dnf provides A webclient B C"
   When I execute dnf with args "provides A webclient B C"
   Then stdout contains "wget-1.19.5-5.fc29[^R]+Repo[ \t]+: dnf-ci-fedora"
    And stdout does not contain "(glibc)|(setup)"
    And the exit code is 1
    And stderr is
        """
        <REPOSYNC>
        No matches found for A.
        No matches found for B.
        No matches found for C.
        If searching for a file, try specifying the full path or using a wildcard prefix ("*/") at the beginning.
        """

Scenario: test order of provides "dnf provides A B C webclient"
   When I execute dnf with args "provides A B C webclient"
   Then stdout contains "wget-1.19.5-5.fc29[^R]+Repo[ \t]+: dnf-ci-fedora"
    And stdout does not contain "(glibc)|(setup)"
    And the exit code is 1
    And stderr is
        """
        <REPOSYNC>
        No matches found for A.
        No matches found for B.
        No matches found for C.
        If searching for a file, try specifying the full path or using a wildcard prefix ("*/") at the beginning.
        """

Scenario: try to match a package by binary if other provides do not match
  Given I use repository "dnf-ci-provides"
   When I execute dnf with args "provides bar"
   Then stdout contains "foo-1.0.0-0.fc29[^R]+Repo[ \t]+: dnf-ci-provides"
   Then the exit code is 0
