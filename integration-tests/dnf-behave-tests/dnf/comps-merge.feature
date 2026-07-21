Feature: Packagelists, grouplists and optionlists are merged

# https://github.com/rpm-software-management/dnf5/issues/183
Scenario: The group packagelist is a union of this group's lists across all repos
  Given I use repository "comps-upgrade-1"
    And I use repository "comps-upgrade-2"
   When I execute dnf with args "group info AB-group"
   Then the exit code is 0
    And stdout is
        """
        Id                   : AB-group
        Name                 : AB-group
        Description          : Testgroup for DNF CI testing
        Installed            : no
        Order                : 1024
        Langonly             :
        Uservisible          : yes
        Repositories         : comps-upgrade-1, comps-upgrade-2
        Mandatory packages   : A-mandatory
                             : B-mandatory
        Default packages     : A-default
                             : B-default
        Optional packages    : A-optional
                             : B-optional
        Conditional packages : A-conditional-false
                             : A-conditional-true
                             : B-conditional-false
                             : B-conditional-true
        """


# https://github.com/rpm-software-management/dnf5/issues/881
Scenario: The environment grouplist and optionlist is a union of this environment's lists across all repos
  Given I use repository "comps-upgrade-1"
    And I use repository "comps-upgrade-2"
   When I execute dnf with args "environment info ABC-environment"
   Then the exit code is 0
    And stdout is
        """
        Id                   : ABC-environment
        Name                 : ABC-environment
        Description          : Testenvironment for DNF CI testing
        Order                : 1024
        Installed            : False
        Repositories         : comps-upgrade-1, comps-upgrade-2
        Required groups      : B-group
                             : a-group
        Optional groups      : AB-group
                             : C-group
        """


Scenario: When a package changes type within group, it is considered as both types
  Given I use repository "comps-upgrade-1"
    And I use repository "comps-upgrade-2"
   When I execute dnf with args "group info change-package-type"
   Then the exit code is 0
    And stdout is
        """
        Id                   : change-package-type
        Name                 : change-package-type
        Description          : Testgroup for DNF CI testing
        Installed            : no
        Order                : 1024
        Langonly             :
        Uservisible          : yes
        Repositories         : comps-upgrade-1, comps-upgrade-2
        Mandatory packages   : A-mandatory
                             : dummy
        Default packages     : A-mandatory
                             : B-mandatory
        Optional packages    : B-mandatory
                             : C-mandatory
        Conditional packages : C-mandatory
                             : dummy
        """
