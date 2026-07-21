Feature: dnf repoquery command on packages with xml:base

# https://github.com/rpm-software-management/dnf/issues/2130
Scenario: Repoquery --location for repository that have packages with xml:base
Given I copy repository "simple-base" for modification
  And I copy repository "dnf-ci-thirdparty" for modification
  And I use repository "simple-base"
  And I generate repodata for repository "simple-base" with extra arguments "--baseurl https://the.xml.base.location/repo/"
  And I execute "mergerepo_c --omit-baseurl --repo file://{context.dnf.repos[simple-base].path} --repo file://{context.dnf.repos[dnf-ci-thirdparty].path}" in "{context.dnf.installroot}"
  And I configure a new repository "merged-repo" with
      | key     | value                                        |
      | baseurl | file://{context.dnf.installroot}/merged_repo |
  And I drop repository "simple-base"
  And I drop repository "dnf-ci-thirdparty"
  # packages from merged-repo that originated from dnf-ci-thirdparty do not have xml:base
 When I execute dnf with args "repoquery --location solveigs-song"
 Then the exit code is 0
  And stdout is
  """
  file://{context.dnf.installroot}/merged_repo/src/solveigs-song-1.0-1.src.rpm
  file://{context.dnf.installroot}/merged_repo/x86_64/solveigs-song-1.0-1.x86_64.rpm
  """
  # packages from merged-repo that originated from simple-base repo have xml:base
 When I execute dnf with args "repoquery --location labirinto"
 Then the exit code is 0
  And stdout is
  """
  https://the.xml.base.location/repo/src/labirinto-1.0-1.fc29.src.rpm
  https://the.xml.base.location/repo/x86_64/labirinto-1.0-1.fc29.x86_64.rpm
  """
