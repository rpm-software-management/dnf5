Feature: Install packages with base:xml set


@xfail
# https://github.com/rpm-software-management/ci-dnf-stack/issues/1576
Scenario: Installed packages from local repository with local xml:base are not cached
Given I copy repository "dnf-ci-fedora" for modification
  And I use repository "dnf-ci-fedora"
  And I generate repodata for repository "dnf-ci-fedora" with extra arguments "--baseurl file://{context.dnf.installroot}/temp-repo/xml_base/dnf-ci-fedora"
  # setup data to which the base:xml (createrepo_c --baseurl argument) is pointing
  And I copy directory "{context.dnf.repos[dnf-ci-fedora].path}" to "/temp-repo/xml_base/dnf-ci-fedora"
 When I execute dnf with args "--setopt=keepcache=true install setup"
 Then the exit code is 0
  And Transaction is following
      | Action        | Package                                  |
      | install       | setup-0:2.12.1-1.fc29.noarch             |
  And file "/var/cache/dnf/dnf-ci-fedora*/packages/setup*" does not exist


@dnf5daemon
Scenario: Install from local repodata with xml:base pointing to remote packages
Given I make packages from repository "dnf-ci-fedora" accessible via http
  And I copy repository "dnf-ci-fedora" for modification
  And I generate repodata for repository "dnf-ci-fedora" with extra arguments "--baseurl http://localhost:{context.dnf.ports[dnf-ci-fedora]}"
  And I use repository "dnf-ci-fedora"
 When I execute dnf with args "--setopt=keepcache=true install setup"
 Then the exit code is 0
  And Transaction is following
      | Action        | Package                                  |
      | install       | setup-0:2.12.1-1.fc29.noarch             |
  And file "/var/cache/dnf/dnf-ci-fedora*/packages/setup*" exists


@dnf5daemon
Scenario: Install from remote repodata with xml:base pointing to packages on different HTTP servers
Given I make packages from repository "dnf-ci-fedora" accessible via http
  And I copy repository "dnf-ci-fedora" for modification
  And I generate repodata for repository "dnf-ci-fedora" with extra arguments "--baseurl http://localhost:{context.dnf.ports[dnf-ci-fedora]}"
  And I use repository "dnf-ci-fedora" as http
 When I execute dnf with args "install setup"
 Then the exit code is 0
  And Transaction is following
      | Action        | Package                                  |
      | install       | setup-0:2.12.1-1.fc29.noarch             |


@dnf5daemon
Scenario: Install from local repodata with xml:base pointing to remote packages doesn't delete unused local packages
Given I make packages from repository "dnf-ci-fedora" accessible via http
  And I copy repository "dnf-ci-fedora" for modification
  And I generate repodata for repository "dnf-ci-fedora" with extra arguments "--baseurl http://localhost:{context.dnf.ports[dnf-ci-fedora]}"
  And I use repository "dnf-ci-fedora"
  And file "/{context.dnf.repos[dnf-ci-fedora].path}/noarch/setup-2.12.1-1.fc29.noarch.rpm" exists
 When I execute dnf with args "install setup"
 Then file "/{context.dnf.repos[dnf-ci-fedora].path}/noarch/setup-2.12.1-1.fc29.noarch.rpm" exists
  And the exit code is 0
  And Transaction is following
      | Action        | Package                                  |
      | install       | setup-0:2.12.1-1.fc29.noarch             |


@xfail
# https://github.com/rpm-software-management/ci-dnf-stack/issues/1576
Scenario: Install from local repodata that have packages with xml:base pointing to a remote as well as local packages
Given I make packages from repository "dnf-ci-fedora" accessible via http
  And I copy repository "dnf-ci-fedora" for modification
  And I copy repository "dnf-ci-thirdparty" for modification
  And I generate repodata for repository "dnf-ci-fedora" with extra arguments "--baseurl http://localhost:{context.dnf.ports[dnf-ci-fedora]}"
  And I execute "mergerepo_c --repo file://{context.dnf.repos[dnf-ci-fedora].path} --repo file://{context.dnf.repos[dnf-ci-thirdparty].path}" in "{context.dnf.installroot}"
  And I configure a new repository "merged-repo" with
      | key     | value                                        |
      | baseurl | file://{context.dnf.installroot}/merged_repo |
 When I execute dnf with args "--setopt=keepcache=true install setup alternator"
 Then the exit code is 0
  And Transaction is following
      | Action  | Package                      |
      | install | setup-0:2.12.1-1.fc29.noarch |
      | install | alternator-0:1.1-1.x86_64    |
 # We don't want to cache local packages
  And file "/var/cache/dnf/merged-repo*/packages/alternator*" does not exist
 # We want to cache remote packages
  And file "/var/cache/dnf/merged-repo*/packages/setup*" exists
