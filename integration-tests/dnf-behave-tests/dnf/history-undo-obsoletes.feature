Feature: Transaction history undo with obsoletes

# Package glibc-all-langpacks requires glibc
# Package glibc obsoletes glibc-profile < 2.4

Scenario: Undo with obsoletes
  Given I use repository "dnf-ci-thirdparty"

   When I execute dnf with args "install glibc-profile"
   Then the exit code is 0
    And Transaction is following
        | Action        | Package                                   |
        | install       | glibc-profile-0:2.3.1-10.x86_64           |

   When I use repository "dnf-ci-fedora"
    And I execute dnf with args "install glibc-all-langpacks"
   Then the exit code is 0
    And Transaction is following
        | Action        | Package                                   |
        | install       | glibc-0:2.28-9.fc29.x86_64                |
        | install       | glibc-all-langpacks-0:2.28-9.fc29.x86_64  |
        | install-dep   | setup-0:2.12.1-1.fc29.noarch              |
        | install-dep   | glibc-common-0:2.28-9.fc29.x86_64         |
        | install-dep   | filesystem-0:3.9-2.fc29.x86_64            |
        | install-dep   | basesystem-0:11-6.fc29.noarch             |
        | obsoleted     | glibc-profile-0:2.3.1-10.x86_64           |

   When I execute dnf with args "history undo last"
   Then the exit code is 0
    And Transaction is following
        | Action        | Package                                   |
        | remove        | glibc-all-langpacks-0:2.28-9.fc29.x86_64  |
        | remove-unused | setup-0:2.12.1-1.fc29.noarch              |
        | remove        | glibc-0:2.28-9.fc29.x86_64                |
        | remove-unused | glibc-common-0:2.28-9.fc29.x86_64         |
        | remove-unused | filesystem-0:3.9-2.fc29.x86_64            |
        | remove-unused | basesystem-0:11-6.fc29.noarch             |
        | install       | glibc-profile-0:2.3.1-10.x86_64           |
