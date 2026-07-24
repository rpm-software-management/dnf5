Feature: api: query repos


Background:
Given I use repository "simple-base"
  And I use repository "dnf-ci-fedora"
  And I use repository "dnf-ci-fedora-updates"


Scenario: Construct query and filter dnf-ci-fedora repo
 When I execute python libdnf5 api script with setup
      """
      query = libdnf5.repo.RepoQuery(base)
      query.filter_id("dnf-ci-fedora")
      for repo in query:
          print(repo.get_id())
      """
 Then the exit code is 0
  And stdout is
      """
      dnf-ci-fedora
      """


Scenario: Construct query and filter fails due to bad argument type
 When I execute python libdnf5 api script with setup
      """
      query = libdnf5.repo.RepoQuery(base)
      query.filter_id(123)
      for repo in query:
          print(repo.get_id())
      """
 Then the exit code is 1
  And stdout is empty
  And stderr contains "TypeError: Wrong number or type of arguments for overloaded function 'RepoQuery_filter_id'."


Scenario: Construct query and filter using QueryCmp_ICONTAINS
 When I execute python libdnf5 api script with setup
      """
      from libdnf5.common import QueryCmp_ICONTAINS
      query = libdnf5.repo.RepoQuery(base)
      query.filter_id("CI", QueryCmp_ICONTAINS)
      repo_ids = [repo.get_id() for repo in query]
      for repo_id in sorted(repo_ids):
          print(repo_id)
      """
 Then the exit code is 0
  And stdout is
      """
      dnf-ci-fedora
      dnf-ci-fedora-updates
      """


Scenario: Construct query and filter using QueryCmp_IGLOB
 When I execute python libdnf5 api script with setup
      """
      from libdnf5.common import QueryCmp_IGLOB
      query = libdnf5.repo.RepoQuery(base)
      query.filter_id(["*BASE*", "*updates*"], QueryCmp_IGLOB)
      repo_ids = [repo.get_id() for repo in query]
      for repo_id in sorted(repo_ids):
          print(repo_id)
      """
 Then the exit code is 0
  And stdout is
      """
      dnf-ci-fedora-updates
      simple-base
      """


Scenario: Construct query and filter using QueryCmp_NOT_IGLOB
 When I execute python libdnf5 api script with setup
      """
      from libdnf5.common import QueryCmp_NOT_IGLOB
      query = libdnf5.repo.RepoQuery(base)
      query.filter_id("*UPDATES*", QueryCmp_NOT_IGLOB)
      query.filter_id("*system*", QueryCmp_NOT_IGLOB)
      repo_ids = [repo.get_id() for repo in query]
      for repo_id in sorted(repo_ids):
          print(repo_id)
      """
 Then the exit code is 0
  And stdout is
      """
      dnf-ci-fedora
      simple-base
      """
