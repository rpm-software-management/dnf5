Feature: Testing DNF metadata handling


@bz1644283
Scenario: update expired metadata on first dnf update
Given I create directory "/temp-repos/temp-repo"
  And I configure a new repository "testrepo" with
      | key             | value                                          |
      | baseurl         | {context.dnf.installroot}/temp-repos/temp-repo |
      | metadata_expire | 1s                                             |
  And I execute "createrepo_c --update ." in "{context.dnf.installroot}/temp-repos/temp-repo"
 Then the exit code is 0
 When I execute dnf with args "list"
 Then the exit code is 0
  And stderr contains "testrepo"
Given I copy directory "{context.scenario.repos_location}/dnf-ci-fedora" to "/temp-repos/temp-repo/dnf-ci-fedora"
 Then the exit code is 0
  And I execute "createrepo_c --update ." in "{context.dnf.installroot}/temp-repos/temp-repo"
 Then the exit code is 0
 #Ensure metadata are expired
  And I execute "sleep 2s"
 Then I execute dnf with args "upgrade"
 Then the exit code is 0
 Then I execute dnf with args "--cacheonly list"
 Then the exit code is 0
  And stdout contains "\s+wget.src\s+1.19.5-5.fc29\s+testrepo"


@bz1771147
Scenario: dnf respects metadata_expire (with default of 48 hours)
Given I use repository "simple-base" as http
 When I execute dnf with args "repoquery labirinto"
 Then the exit code is 0
  And stdout is
  """
  labirinto-0:1.0-1.fc29.src
  labirinto-0:1.0-1.fc29.x86_64
  """
 # since libdnf measures the metadata file age in whole seconds, wait till it changes
 When I sleep for "1" seconds
  And I execute dnf with args "repoquery labirinto"
 Then the exit code is 0
  And stderr is
  """
  Updating and loading repositories:
  Repositories loaded.
  """
  And stdout is
  """
  labirinto-0:1.0-1.fc29.src
  labirinto-0:1.0-1.fc29.x86_64
  """
 When I execute dnf with args "--refresh repoquery labirinto"
 Then the exit code is 0
  And stdout is
  """
  labirinto-0:1.0-1.fc29.src
  labirinto-0:1.0-1.fc29.x86_64
  """
  And stderr matches line by line
  """
  Updating and loading repositories:
  simple-base test repository .*
  Repositories loaded.
  """


@bz1866505
Scenario: I cannot create/overwrite a file in /etc from local repository
# This directory structure is needed at the repo source so that it can be matched on the system running dnf
# the path where to donwload the file ends up looking something like this:
# /var/cache/dnf/test-622efad968597580/tmpdir.2fwp3B/../../../../../etc/malicious.file -> /etc/malicious.file
Given I create file "/a/etc/malicious.file" with
      """
      my evil config
      """
  And I create file "/a/b/c/d/e/repodata/repomd.xml" with
      """
      <?xml version="1.0" encoding="UTF-8"?>
      <repomd>
          <data type="primary">
              <location href="../../../../../etc/malicious.file"/>
          </data>
      </repomd>
      """
 When I execute dnf with args "--repofrompath=test,{context.dnf.installroot}/a/b/c/d/e/ --repo test --refresh --nogpgcheck install htop"
 Then file "/etc/malicious.file" does not exist


@bz1866505
Scenario: I cannot create/overwrite a file in /etc from remote repository
Given I create file "/a/etc/malicious.file" with
      """
      my evil config
      """
  And I create file "/a/b/c/d/e/f/g/repodata/repomd.xml" with
      """
      <?xml version="1.0" encoding="UTF-8"?>
      <repomd>
          <data type="primary">
              <location href="../../../../../etc/malicious.file"/>
          </data>
      </repomd>
      """
  And I start http server "malicious_server" at "{context.dnf.installroot}/a"
  And I configure a new repository "test" with
        | key      | value                                                                                                                                                               |
        | gpgcheck | 0    |
        | baseurl  | http://0.0.0.0:{context.dnf.ports[malicious_server]}/b/c/d/e/f/g/ |
 When I execute dnf with args "--refresh install htop"
 Then file "/etc/malicious.file" does not exist


Scenario: present user understandable message when there is a mismatch between available repodata and packages
    Given I copy repository "simple-base" for modification
    And I use repository "simple-base" as http
    And I execute "echo \"checksum mismatch\" >> /{context.dnf.repos[simple-base].path}/x86_64/labirinto-1.0-1.fc29.x86_64.rpm"
    When I execute dnf with args "install labirinto"
    Then the exit code is 1
    And stderr contains "Interrupted by header callback: Inconsistent server data"
    And file "/var/log/dnf5.log" contains lines
        """
        .* INFO \[librepo\] Error during transfer: Interrupted by header callback: Inconsistent server data, reported file Content-Length: .*, repository metadata states file length: .* \(please report to repository maintainer\)
        """


Scenario: Identical metalink checksums match, no repo redownload needed
Given I copy repository "simple-base" for modification
  And I use repository "simple-base" as http
  And I set up metalink for repository "simple-base"
  And I execute dnf with args "makecache"
  And I start capturing outbound HTTP requests
 When I execute dnf with args "makecache --refresh"
 Then the exit code is 0
 And stderr is
      """
      <REPOSYNC>
      """
  And HTTP log is
      """
      GET simple-base /metalink.xml?releasever=29
      """


Scenario: Updated metalink with updated metadata causes the whole repo to redownload
Given I copy repository "simple-base" for modification
  And I use repository "simple-base" as http
  And I set up metalink for repository "simple-base"
  And I execute dnf with args "makecache"
  And I generate repodata for repository "simple-base" with extra arguments "--baseurl update-metadata"
  # We need to run this step so that we can regenerate the metalink
  And I use repository "simple-base" as http
  And I set up metalink for repository "simple-base"
  And I start capturing outbound HTTP requests
 When I execute dnf with args "makecache --refresh"
 Then the exit code is 0
 And stderr is
      """
      <REPOSYNC>
      """
  And HTTP log is
      """
      GET simple-base /metalink.xml?releasever=29
      GET simple-base /repodata/repomd.xml
      GET simple-base /repodata/primary.xml.zst
      """


Scenario: Identical repomd checksums match, no repo redownload needed
Given I copy repository "simple-base" for modification
  And I use repository "simple-base" as http
  And I execute dnf with args "makecache"
  And I start capturing outbound HTTP requests
 When I execute dnf with args "makecache --refresh"
 Then the exit code is 0
 And stderr is
      """
      <REPOSYNC>
      """
  And HTTP log is
      """
      GET simple-base /repodata/repomd.xml
      """


Scenario: Updated repomd with updated metadata causes the whole repo to redownload
Given I copy repository "simple-base" for modification
  And I use repository "simple-base" as http
  And I execute dnf with args "makecache"
  And I generate repodata for repository "simple-base" with extra arguments "--baseurl update-metadata"
  And I start capturing outbound HTTP requests
 When I execute dnf with args "makecache --refresh"
 Then the exit code is 0
 And stderr is
      """
      <REPOSYNC>
      """
  And HTTP log is
      """
      GET simple-base /repodata/repomd.xml
      GET simple-base /repodata/primary.xml.zst
      """


Scenario: Checksum from metalink is verified agains downloaded repomd.xml checksum, fail if they don't match
Given I copy repository "simple-base" for modification
  And I use repository "simple-base" as http
  And I set up metalink for repository "simple-base"
  # Change repomd.xml from simple-base repo so that it doesn't match with metalink checksum
  And I generate repodata for repository "simple-base" with extra arguments "--baseurl update-metadata"
  And I start capturing outbound HTTP requests
 When I execute dnf with args "makecache --refresh"
 Then the exit code is 1
  And stderr matches line by line
      """
      Updating and loading repositories:
       simple-base test repository .*
      >>> Downloading successful, but checksum doesn't match. Calculated: .*
      >>> Downloading successful, but checksum doesn't match. Calculated: .*
      >>> Downloading successful, but checksum doesn't match. Calculated: .*
      >>> Downloading successful, but checksum doesn't match. Calculated: .*
      >>> Usable URL not found
      Failed to download metadata \(metalink: "http://.*/metalink.xml\?releasever=29"\) for repository "simple-base": Usable URL not found
      """
  And HTTP log is
      """
      GET simple-base /metalink.xml?releasever=29
      GET simple-base /repodata/repomd.xml
      GET simple-base /repodata/repomd.xml
      GET simple-base /repodata/repomd.xml
      GET simple-base /repodata/repomd.xml
      """
