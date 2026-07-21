Feature: Tests for different package download sources


# dnf5 does not fall-back to baseurl if mirrorlist fails.
# https://github.com/rpm-software-management/dnf5/issues/1763
@xfail
@bz1775184
Scenario: baseurl is used if all mirrors from mirrorlist fail
Given I create directory "/baseurlrepo"
  And I execute "createrepo_c {context.dnf.installroot}/baseurlrepo"
  And I create file "/tmp/mirrorlist" with
      """
      file:///nonexistent.repo
      http://127.0.0.1:5000/nonexistent
      """
  And I configure a new repository "testrepo" with
      | key        | value                                    |
      | baseurl    | {context.dnf.installroot}/baseurlrepo    |
      | mirrorlist | {context.dnf.installroot}/tmp/mirrorlist |
 When I execute dnf with args "makecache"
 Then the exit code is 0
  And stderr is
  """
  <REPOSYNC>
  """


# https://github.com/rpm-software-management/dnf5/issues/1763
@xfail
@bz1775184
Scenario: baseurl is used if mirrorlist file cannot be found
Given I create directory "/baseurlrepo"
  And I execute "createrepo_c {context.dnf.installroot}/baseurlrepo"
  And I configure a new repository "testrepo" with
      | key        | value                                    |
      | baseurl    | {context.dnf.installroot}/baseurlrepo    |
      | mirrorlist | {context.dnf.installroot}/tmp/mirrorlist |
 When I execute dnf with args "makecache"
 Then the exit code is 0
  And stderr is
  """
  <REPOSYNC>
  """


# https://github.com/rpm-software-management/dnf5/issues/1763
@xfail
@bz1775184
Scenario: baseurl is used if mirrorlist file is empty
Given I create directory "/baseurlrepo"
  And I execute "createrepo_c {context.dnf.installroot}/baseurlrepo"
  And I create file "/tmp/mirrorlist" with
      """
      """
  And I configure a new repository "testrepo" with
      | key        | value                                    |
      | baseurl    | {context.dnf.installroot}/baseurlrepo    |
      | mirrorlist | {context.dnf.installroot}/tmp/mirrorlist |
 When I execute dnf with args "makecache"
 Then the exit code is 0
  And stderr is
  """
  <REPOSYNC>
  """


# https://github.com/rpm-software-management/dnf5/issues/1763
@xfail
Scenario: no working donwload source result in an error
Given I create directory "/baseurlrepo"
  And I execute "createrepo_c {context.dnf.installroot}/baseurlrepo"
  And I create file "/tmp/mirrorlist" with
      """
      file:///nonexistent.repo
      http://127.0.0.1:5000/nonexistent
      """
  And I configure a new repository "testrepo" with
      | key        | value                                    |
      | baseurl    | {context.dnf.installroot}/I_dont_exist   |
      | mirrorlist | {context.dnf.installroot}/tmp/mirrorlist |
 When I execute dnf with args "makecache"
 Then the exit code is 1
  And stderr contains "Errors during downloading metadata for repository 'testrepo':"
  And stderr contains "- Curl error \(37\): Couldn't read a file:// file for file:///nonexistent.repo/repodata/repomd.xml \[Couldn't open file /nonexistent.repo/repodata/repomd.xml\]"
  And stderr contains "- Curl error \(7\): Couldn't connect to server for http://127.0.0.1:5000/nonexistent/repodata/repomd.xml \[Failed to connect to 127.0.0.1 port 5000 after 0 ms: Connection refused\]"
  And stderr contains "- Curl error \(37\): Couldn't read a file:// file for file:///tmp/dnf_ci_installroot_.*/I_dont_exist/repodata/repomd.xml \[Couldn't open file /tmp/dnf_ci_installroot_.*/I_dont_exist/repodata/repomd.xml\]"
  And stderr contains "Error: Failed to download metadata for repo 'testrepo': Cannot download repomd.xml: Cannot download repodata/repomd.xml: All mirrors were tried"


Scenario: mirrorlist is prefered over baseurl
Given I create directory "/baseurlrepo"
  And I execute "createrepo_c {context.dnf.installroot}/baseurlrepo"
  And I create directory "/mirrorlistrepo"
  And I copy file "{context.dnf.fixturesdir}/repos/dnf-ci-fedora/noarch/setup-2.12.1-1.fc29.noarch.rpm" to "/mirrorlistrepo/setup-2.12.1-1.fc29.noarch.rpm"
  And I execute "createrepo_c {context.dnf.installroot}/mirrorlistrepo"
  And I create and substitute file "/tmp/mirrorlist" with
      """
      file://{context.dnf.installroot}/mirrorlistrepo
      """
  And I configure a new repository "testrepo" with
      | key        | value                                    |
      | baseurl    | {context.dnf.installroot}/baseurlrepo    |
      | mirrorlist | {context.dnf.installroot}/tmp/mirrorlist |
 When I execute dnf with args "install setup"
 Then the exit code is 0
  And Transaction is following
      | Action        | Package                      |
      | install       | setup-0:2.12.1-1.fc29.noarch |


Scenario: Install from local repodata with locations pointing to remote packages
Given I make packages from repository "dnf-ci-fedora" accessible via http
  And I copy repository "dnf-ci-fedora" for modification
  And I generate repodata for repository "dnf-ci-fedora" with extra arguments "--location-prefix http://localhost:{context.dnf.ports[dnf-ci-fedora]}"
  And I use repository "dnf-ci-fedora"
  # delete packages from the repo copied for modification so they cannot be accidentally used
  And I delete directory "/{context.dnf.repos[dnf-ci-fedora].path}/noarch"
 When I execute dnf with args "--setopt=keepcache=true install setup"
 Then the exit code is 0
  And Transaction is following
      | Action        | Package                                  |
      | install       | setup-0:2.12.1-1.fc29.noarch             |
  And file "/var/cache/dnf/dnf-ci-fedora*/packages/setup*" exists


Scenario: Install from remote repodata with locations pointing to packages on different HTTP servers
Given I make packages from repository "dnf-ci-fedora" accessible via http
  And I copy repository "dnf-ci-fedora" for modification
  And I generate repodata for repository "dnf-ci-fedora" with extra arguments "--location-prefix http://localhost:{context.dnf.ports[dnf-ci-fedora]}"
  And I use repository "dnf-ci-fedora" as http
  # delete packages from the repo copied for modification so they cannot be accidentally used
  And I delete directory "/{context.dnf.repos[dnf-ci-fedora].path}/noarch"
 When I execute dnf with args "install setup"
 Then the exit code is 0
  And Transaction is following
      | Action        | Package                                  |
      | install       | setup-0:2.12.1-1.fc29.noarch             |


# https://github.com/rpm-software-management/dnf5/issues/1321
@bz1817130
Scenario: Download a package that contains special URL characters that need to be encoded (e.g. a +)
Given I use repository "download-sources" as http
  And I start capturing outbound HTTP requests
 When I execute dnf with args "install special-c++-package"
 Then the exit code is 0
  And HTTP log contains
      """
      GET /noarch/special-c%2b%2b-package-1.0-1.noarch.rpm
      """


@bz1817130
@xfail
# For packages with full URL in their location we can't encode the package name.
# The URL would need to come encoded in the repo metadata from createrepo_c.
Scenario: Download a package that contains special URL characters with full URL in location
Given I make packages from repository "download-sources" accessible via http
  And I copy repository "download-sources" for modification
  And I generate repodata for repository "download-sources" with extra arguments "--location-prefix http://localhost:{context.dnf.ports[download-sources]}"
  And I use repository "download-sources" as http
  # delete packages from the repo copied for modification so they cannot be accidentally used
  And I delete directory "/{context.dnf.repos[download-sources].path}/noarch"
  And I start capturing outbound HTTP requests
 When I execute dnf with args "install special-c++-package"
 Then the exit code is 0
  And HTTP log contains
      """
      GET /noarch/special-c%2b%2b-package-1.0-1.noarch.rpm
      """


@bz2103015
Scenario: Download a package that contains special URL characters by passing an encoded URL
Given I use repository "download-sources" as http
  And I start capturing outbound HTTP requests
 When I execute dnf with args "install http://localhost:{context.dnf.ports[download-sources]}/noarch/special-c%2b%2b-package-1.0-1.noarch.rpm"
 Then the exit code is 0
  And HTTP log contains
      """
      GET /noarch/special-c%2b%2b-package-1.0-1.noarch.rpm
      """
  And Transaction is following
      | Action        | Package                          |
      | install       | special-c++-package-1.0-1.noarch |


@bz2381859
# We don't actually check which mirror is picked, only verify it runs
Scenario: Verify we can run upgrade --refresh with fastest mirror enabled with multiple repositories and mirrors
Given I use repository "dnf-ci-fedora" as http
  And I successfully execute dnf with args "install flac"
  And I use repository "dnf-ci-thirdparty" as http
  And I use repository "dnf-ci-fedora-updates" as http
  And I create directory "/mirror"
  And I copy directory "{context.dnf.fixturesdir}/repos/dnf-ci-fedora-updates/x86_64" to "/mirror/x86_64"
  And I execute "createrepo_c {context.dnf.installroot}/mirror"
  And I start http server "mirror" at "{context.dnf.installroot}/mirror"
  And I create and substitute file "/mirrorlist" with
      """
      http://localhost:{context.dnf.ports[dnf-ci-fedora-updates]}/
      http://localhost:{context.dnf.ports[mirror]}/
      """
  And I configure a new repository "testrepo" with
      | key        | value                                |
      | mirrorlist | {context.dnf.installroot}/mirrorlist |
  And I configure dnf with
      | key           | value |
      | fastestmirror | True  |
 When I execute dnf with args "update --refresh"
 Then the exit code is 0
  And Transaction is following
      | Action      | Package                    |
      | upgrade     | flac-0:1.3.3-3.fc29.x86_64 |
