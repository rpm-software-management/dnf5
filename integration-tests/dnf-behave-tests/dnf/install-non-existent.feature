Feature: Test for installation of non-existent rpm or package


@bz1578369
Scenario: Try to install a non-existent rpm
  Given I use repository "dnf-ci-fedora"
   When I execute dnf with args "install non-existent.rpm"
   Then the exit code is 1
    And stderr is
        """
        <REPOSYNC>
        Failed to access RPM "non-existent.rpm": No such file or directory
        """


Scenario: Try to install a non-existent package
  Given I use repository "dnf-ci-fedora"
   When I execute dnf with args "install non-existent-package"
   Then the exit code is 1
    And stderr is
        """
        <REPOSYNC>
        Failed to resolve the transaction:
        No match for argument: non-existent-package
        You can try to add to command line:
          --skip-unavailable to skip unavailable packages
        """


@bz1717429
Scenario: Install an existent and an non-existent package
  Given I use repository "dnf-ci-fedora"
   When I execute dnf with args "install setup non-existent-package"
   Then the exit code is 1
    And stderr is
        """
        <REPOSYNC>
        Failed to resolve the transaction:
        No match for argument: non-existent-package
        You can try to add to command line:
          --skip-unavailable to skip unavailable packages
        """


@bz1717429
Scenario: Install an existent and an non-existent package with --skip-unavailable
  Given I use repository "dnf-ci-fedora"
   When I execute dnf with args "install setup non-existent-package --skip-unavailable"
   Then the exit code is 0
    And stderr contains lines
    """
    No match for argument: non-existent-package
    """
    And Transaction is following
        | Action        | Package                                   |
        | install       | setup-0:2.12.1-1.fc29.noarch              |
