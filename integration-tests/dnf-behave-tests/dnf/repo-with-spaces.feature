Feature: Tests handling a repository with spaces in baseurl


Background:
# just generate repodata and then configure the repo manually, since the ID with spaces is invalid
Given I use repository "repo with spaces"
  And I drop repository "repo with spaces"


@bz1853349
Scenario: Install an rpm from a repo with spaces in baseurl
Given I create and substitute file "/etc/yum.repos.d/test-repo.repo" with
      """
      [test-repo]
      name=repo with spaces test repository
      enabled=1
      gpgcheck=0
      baseurl={context.scenario.repos_location}/repo%20with%20spaces
      """
 When I execute dnf with args "install test-package"
 Then the exit code is 0
  And Transaction is following
      | Action  | Package                     |
      | install | test-package-0:1.0-1.x86_64 |


@bz1853349
Scenario: Install an rpm with spaces in its baseurl (the xml:base attribute of the package)
Given I copy repository "repo with spaces" for modification
  And I generate repodata for repository "repo with spaces" with extra arguments "--baseurl file://{context.scenario.repos_location}/repo%20with%20spaces"
  And I create and substitute file "/etc/yum.repos.d/test-repo.repo" with
      """
      [test-repo]
      name=repo with spaces test repository
      enabled=1
      gpgcheck=0
      baseurl=file://{context.dnf.tempdir}/repos/repo%20with%20spaces
      """
 When I execute dnf with args "install test-package"
 Then the exit code is 0
  And Transaction is following
      | Action  | Package                     |
      | install | test-package-0:1.0-1.x86_64 |


@bz1853349
Scenario: Download an rpm with spaces in its baseurl (the xml:base attribute of the package) to a destdir
Given I copy repository "repo with spaces" for modification
  And I generate repodata for repository "repo with spaces" with extra arguments "--baseurl file://{context.scenario.repos_location}/repo%20with%20spaces"
  And I create and substitute file "/etc/yum.repos.d/test-repo.repo" with
      """
      [test-repo]
      name=repo with spaces test repository
      enabled=1
      gpgcheck=0
      baseurl=file://{context.dnf.tempdir}/repos/repo%20with%20spaces
      """
 When I execute dnf with args "download test-package --destdir={context.dnf.tempdir}"
 Then the exit code is 0
  And file "/{context.dnf.tempdir}/test-package-1.0-1.x86_64.rpm" exists
