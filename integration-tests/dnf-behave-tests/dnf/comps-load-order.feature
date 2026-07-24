@bz1928181
Feature: Comps are merged based on repoconf load order


Scenario: Comps merging and repoconf load order of dnf-ci-fedora and dnf-ci-fedora-updates repo files
  Given I create and substitute file "/etc/yum.repos.d/dnf-ci-fedora.repo" with
      """
      [dnf-ci-fedora]
      name=dnf-ci-fedora
      baseurl={context.scenario.repos_location}/comps-upgrade-1
      enabled=1
      gpgcheck=0
      """
    # the following step runs createrepo on that repo
    And I use repository "comps-upgrade-1"
    And I create and substitute file "/etc/yum.repos.d/dnf-ci-fedora-updates.repo" with
      """
      [dnf-ci-fedora-updates]
      name=dnf-ci-fedora-updates
      baseurl={context.scenario.repos_location}/comps-upgrade-2
      enabled=1
      gpgcheck=0
      """
    # the following step runs createrepo on that repo
    And I use repository "comps-upgrade-2"
   When I execute dnf with args "group info a-group"
   Then the exit code is 0
    And stdout contains "Name *: A-group - repo#2"
    And stdout contains "Description *: Testgroup for DNF CI testing - repo#2"


Scenario: Comps merging and repoconf load order of dnf-ci-abc and dnf-ci-fedora repo files
  Given I create and substitute file "/etc/yum.repos.d/dnf-ci-fedora.repo" with
      """
      [dnf-ci-fedora]
      name=dnf-ci-fedora
      baseurl={context.scenario.repos_location}/comps-upgrade-1
      enabled=1
      gpgcheck=0
      """
    # the following step runs createrepo on that repo
    And I use repository "comps-upgrade-1"
    And I create and substitute file "/etc/yum.repos.d/dnf-ci-abc.repo" with
      """
      [dnf-ci-abc]
      name=dnf-ci-abc
      baseurl={context.scenario.repos_location}/comps-upgrade-2
      enabled=1
      gpgcheck=0
      """
    # the following step runs createrepo on that repo
    And I use repository "comps-upgrade-2"
   When I execute dnf with args "group info a-group"
   Then the exit code is 0
    And stdout contains "Name *: A-group - repo#1"
    And stdout contains "Description *: Testgroup for DNF CI testing - repo#1"
