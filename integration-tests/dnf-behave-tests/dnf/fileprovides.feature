Feature: Adding file provides tests

Scenario: Run repoclosure with already created cache without filelists
  Given I use repository "fileprovides"
    # We run repoquery --whatprovides to trigger generation of file provides (calling make_provides_ready())
    # This command doesn't require filelists.xml
    And I successfully execute dnf with args "repoquery --whatprovides htop"
    # This command requires filelists.xml
   When I execute dnf with args "repoclosure"
   Then the exit code is 0
    And stderr is
        """
        <REPOSYNC>
        """
    And stdout is empty
