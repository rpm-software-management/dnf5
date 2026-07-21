@dnf5daemon
Feature: Tests for installing RPM from paths

Scenario Outline: I can install an RPM from path, where path is <path type>
  Given I set working directory to "{context.dnf.fixturesdir}/repos/dnf-ci-fedora"
   When I execute dnf with args "install <path>"
   Then the exit code is 0
    And Transaction is following
        | Action        | Package                                   |
        | install       | setup-0:2.12.1-1.fc29.noarch              |

Examples:
        | path type                   | path                                                                                       |
        | absolute                    | {context.dnf.fixturesdir}/repos/dnf-ci-fedora/noarch/setup-2.12.1-1.fc29.noarch.rpm        |
        | relative                    | noarch/setup-2.12.1-1.fc29.noarch.rpm                                                      |
        | relative with wildcards     | noarch/setup-*.fc29.noarch.rpm                                                             |
        | file://                     | file://{context.dnf.fixturesdir}/repos/dnf-ci-fedora/noarch/setup-2.12.1-1.fc29.noarch.rpm |
        | case insensitive URL scheme | FiLe://{context.dnf.fixturesdir}/repos/dnf-ci-fedora/noarch/setup-2.12.1-1.fc29.noarch.rpm |


Scenario: I can install an RPM from path, when specifying the RPM multiple times
   When I execute dnf with args "install {context.dnf.fixturesdir}/repos/dnf-ci-fedora/noarch/setup-2.12.1-1.fc29.noarch.rpm {context.dnf.fixturesdir}/repos/dnf-ci-fedora/noarch/setup-2.12.1-1.fc29.noarch.rpm"
   Then the exit code is 0
    And Transaction is following
        | Action        | Package                                   |
        | install       | setup-0:2.12.1-1.fc29.noarch              |


Scenario: I can install an RPM from url, where url is http address
  Given I use repository "dnf-ci-fedora" as http
  And I execute dnf with args "install http://localhost:{context.dnf.ports[dnf-ci-fedora]}/noarch/setup-2.12.1-1.fc29.noarch.rpm"
   Then the exit code is 0
    And Transaction is following
        | Action        | Package                                   |
        | install       | setup-0:2.12.1-1.fc29.noarch              |


Scenario: I can install an RPM from url, where url is http address - test case insensitivity of the URL scheme
  Given I use repository "dnf-ci-fedora" as http
  And I execute dnf with args "install hTtP://localhost:{context.dnf.ports[dnf-ci-fedora]}/noarch/setup-2.12.1-1.fc29.noarch.rpm"
   Then the exit code is 0
    And Transaction is following
        | Action        | Package                                   |
        | install       | setup-0:2.12.1-1.fc29.noarch              |


Scenario: I can install an RPM from url, where url is ftp address
  Given I use repository "dnf-ci-fedora" as ftp
  And I execute dnf with args "install ftp://localhost:{context.dnf.ports[dnf-ci-fedora]}/noarch/setup-2.12.1-1.fc29.noarch.rpm"
   Then the exit code is 0
    And Transaction is following
        | Action        | Package                                   |
        | install       | setup-0:2.12.1-1.fc29.noarch              |


Scenario: I can install an RPM from url, where url is ftp address - test case insensitivity of the URL scheme
  Given I use repository "dnf-ci-fedora" as ftp
  And I execute dnf with args "install fTP://localhost:{context.dnf.ports[dnf-ci-fedora]}/noarch/setup-2.12.1-1.fc29.noarch.rpm"
   Then the exit code is 0
    And Transaction is following
        | Action        | Package                                   |
        | install       | setup-0:2.12.1-1.fc29.noarch              |
