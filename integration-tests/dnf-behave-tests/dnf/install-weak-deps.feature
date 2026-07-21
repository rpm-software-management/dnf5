Feature: Tests --setopt=install_weak_deps=


Background: Prepare environment
 Given I use repository "dnf-ci-fedora"


Scenario: Install "abcde" without weak dependencies
   When I execute dnf with args "install --setopt=install_weak_deps=0 abcde"
   Then the exit code is 0
    And Transaction is following
        | Action        | Package                                   |
        | install       | abcde-0:2.9.2-1.fc29.noarch               |
        | install-dep   | wget-0:1.19.5-5.fc29.x86_64               |


Scenario: Install "abcde" with weak dependencies
   When I execute dnf with args "install --setopt=install_weak_deps=1 abcde"
   Then the exit code is 0
    And Transaction is following
        | Action        | Package                                   |
        | install       | abcde-0:2.9.2-1.fc29.noarch               |
        | install-weak  | flac-0:1.3.2-8.fc29.x86_64                |
        | install-dep   | wget-0:1.19.5-5.fc29.x86_64               |
