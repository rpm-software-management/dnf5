Feature: Test removal packages along with group

# Self-dependent package is a package that requires and provides the same capability.
# Such requirements are removed by createrepo_c so they are not in repository
# metadata, but they are still present in the rpm package and rpmdb.
@bz2173927
Scenario: Remove group with self dependent package
  Given I use repository "comps-group-pkgremove"
    And I successfully execute dnf with args "group install group-one"
   Then Transaction is following
        | Action        | Package                                       |
        | install-group | self-dependent-package-0:1.0-1.fc29.noarch    |
        | group-install | Test Group One                                |
   When I execute dnf with args "group remove group-one"
   Then the exit code is 0
    And Transaction is following
        | Action        | Package                                       |
        | remove        | self-dependent-package-0:1.0-1.fc29.noarch    |
        | group-remove  | Test Group One                                |


# test-package -> requires test-package-dep -> requires test-package-dep-dep
# group-two: test-package
# group-three: test-package-dep
@bz2173927
Scenario: Remove also group packages required only by another removed group package
  Given I use repository "comps-group-pkgremove"
    And I successfully execute dnf with args "group install group-two group-three"
   Then Transaction is following
        | Action        | Package                                       |
        | install-group | test-package-0:1.0-1.fc29.noarch              |
        | install-group | test-package-dep-0:1.0-1.fc29.noarch          |
        | install-dep   | test-package-dep-dep-0:1.0-1.fc29.noarch      |
        | group-install | Test Group Two                                |
        | group-install | Test Group Three                              |
   When I execute dnf with args "group remove group-two group-three"
   Then the exit code is 0
   Then Transaction is following
        | Action        | Package                                       |
        | remove        | test-package-0:1.0-1.fc29.noarch              |
        | remove        | test-package-dep-0:1.0-1.fc29.noarch          |
        | remove-unused | test-package-dep-dep-0:1.0-1.fc29.noarch      |
        | group-remove  | Test Group Two                                |
        | group-remove  | Test Group Three                              |


# test-package -> requires test-package-dep -> requires test-package-dep-dep
# group-two: test-package
# group-four: test-package-dep-dep
@bz2173927
Scenario: Remove also group packages required only by unused dependency of another removed group package
  Given I use repository "comps-group-pkgremove"
    And I successfully execute dnf with args "group install group-two group-four"
   Then Transaction is following
        | Action        | Package                                       |
        | install-group | test-package-0:1.0-1.fc29.noarch              |
        | install-group | test-package-dep-dep-0:1.0-1.fc29.noarch      |
        | install-dep   | test-package-dep-0:1.0-1.fc29.noarch          |
        | group-install | Test Group Two                                |
        | group-install | Test Group Four                               |
   When I execute dnf with args "group remove group-two group-four"
   Then the exit code is 0
   Then Transaction is following
        | Action        | Package                                       |
        | remove        | test-package-0:1.0-1.fc29.noarch              |
        | remove        | test-package-dep-dep-0:1.0-1.fc29.noarch      |
        | remove-unused | test-package-dep-0:1.0-1.fc29.noarch          |
        | group-remove  | Test Group Two                                |
        | group-remove  | Test Group Four                               |
