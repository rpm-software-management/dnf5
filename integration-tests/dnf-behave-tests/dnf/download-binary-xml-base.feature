Feature: dnf download command on packages with xml:base


Scenario: Download packages from local repository with local xml:base
Given I copy repository "dnf-ci-fedora" for modification
  And I use repository "dnf-ci-fedora"
  And I generate repodata for repository "dnf-ci-fedora" with extra arguments "--baseurl file://{context.dnf.installroot}/xml_base/dnf-ci-fedora"
  # setup data to which the base:xml (createrepo_c --baseurl argument) is pointing
  And I copy directory "{context.dnf.repos[dnf-ci-fedora].path}" to "/xml_base/dnf-ci-fedora"
 When I execute dnf with args "download setup --destdir={context.dnf.tempdir}"
 Then the exit code is 0
 And file "/xml_base/dnf-ci-fedora/noarch/setup-2.12.1-1.fc29.noarch.rpm" exists
  And file sha256 checksums are following
      | Path                                                 | sha256                                                                                     |
      | {context.dnf.tempdir}/setup-2.12.1-1.fc29.noarch.rpm | file://{context.dnf.fixturesdir}/repos/dnf-ci-fedora/noarch/setup-2.12.1-1.fc29.noarch.rpm |


Scenario: Download from local repodata with xml:base pointing to remote packages
Given I make packages from repository "dnf-ci-fedora" accessible via http
  And I copy repository "dnf-ci-fedora" for modification
  And I generate repodata for repository "dnf-ci-fedora" with extra arguments "--baseurl http://localhost:{context.dnf.ports[dnf-ci-fedora]}"
  And I use repository "dnf-ci-fedora"
 When I execute dnf with args "download setup --destdir={context.dnf.tempdir}"
 Then stderr is
      """
      <REPOSYNC>
      """
 Then the exit code is 0
  And file sha256 checksums are following
      | Path                                                 | sha256                                                                                     |
      | {context.dnf.tempdir}/setup-2.12.1-1.fc29.noarch.rpm | file://{context.dnf.fixturesdir}/repos/dnf-ci-fedora/noarch/setup-2.12.1-1.fc29.noarch.rpm |


Scenario: Download from remote repodata with xml:base pointing to packages on different remote
Given I make packages from repository "dnf-ci-fedora" accessible via http
  And I copy repository "dnf-ci-fedora" for modification
  And I generate repodata for repository "dnf-ci-fedora" with extra arguments "--baseurl http://localhost:{context.dnf.ports[dnf-ci-fedora]}"
  And I use repository "dnf-ci-fedora" as http
 When I execute dnf with args "download setup --destdir={context.dnf.tempdir}"
 Then the exit code is 0
 Then stderr is
      """
      <REPOSYNC>
      """
  And file sha256 checksums are following
      | Path                                                 | sha256                                                                                     |
      | {context.dnf.tempdir}/setup-2.12.1-1.fc29.noarch.rpm | file://{context.dnf.fixturesdir}/repos/dnf-ci-fedora/noarch/setup-2.12.1-1.fc29.noarch.rpm |


Scenario: Download to destdir from local repodata that have packages with xml:base pointing to a remote as well as local packages
Given I make packages from repository "dnf-ci-fedora" accessible via http
  And I copy repository "dnf-ci-fedora" for modification
  And I copy repository "dnf-ci-thirdparty" for modification
  And I generate repodata for repository "dnf-ci-fedora" with extra arguments "--baseurl http://localhost:{context.dnf.ports[dnf-ci-fedora]}"
  And I execute "mergerepo_c --repo file://{context.dnf.repos[dnf-ci-fedora].path} --repo file://{context.dnf.repos[dnf-ci-thirdparty].path}" in "{context.dnf.installroot}"
  And I configure a new repository "merged-repo" with
      | key     | value                                        |
      | baseurl | file://{context.dnf.installroot}/merged_repo |
 When I execute dnf with args "download setup alternator --destdir={context.dnf.tempdir}"
 Then the exit code is 0
  And stderr is
      """
      <REPOSYNC>
      """
  And file sha256 checksums are following
      | Path                                                 | sha256                                                                                      |
      | {context.dnf.tempdir}/setup-2.12.1-1.fc29.noarch.rpm | file://{context.dnf.fixturesdir}/repos/dnf-ci-fedora/noarch/setup-2.12.1-1.fc29.noarch.rpm  |
      | {context.dnf.tempdir}/alternator-1.1-1.x86_64.rpm    | file://{context.dnf.fixturesdir}/repos/dnf-ci-thirdparty/x86_64/alternator-1.1-1.x86_64.rpm |
