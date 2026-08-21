Feature: install command on packages and their docs


Scenario: Install a documentation package from local repodata
Given I use repository "microdnf-install-nodocs"
 When I execute dnf with args "install man-pages"
 Then the exit code is 0
  And Transaction is following
      | Action        | Package                                   |
      | install       | man-pages-0:4.16-3.fc29.x86_64            |
  And file "/usr/share/doc/man-pages/README" exists


@bz1769831
Scenario: Install package with option from local repodata with local packages
Given I use repository "microdnf-install-nodocs"
 When I execute dnf with args "--setopt=tsflags=nodocs install man-pages"
 Then the exit code is 0
  And Transaction is following
      | Action        | Package                                   |
      | install       | man-pages-0:4.16-3.fc29.x86_64            |
  And file "/usr/share/doc/man-pages/README" does not exist


@bz1771012
Scenario: Install package with --nodocs option from local repodata with local packages
Given I use repository "microdnf-install-nodocs"
  And I execute dnf with args "--nodocs install man-pages"
 Then the exit code is 0
  And Transaction is following
      | Action        | Package                                   |
      | install       | man-pages-0:4.16-3.fc29.x86_64            |
  And file "/usr/share/doc/man-pages/README" does not exist
