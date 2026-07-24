# librepo is built without zchunk support on RHEL.
@not.with_os=rhel__ge__11
Feature: zchunk tests


Scenario: I can install an RPM from local mirror with zchunk repo and enabled zchunk
Given I copy repository "simple-base" for modification
  And I generate repodata for repository "simple-base" with extra arguments "--zck"
  And I use repository "simple-base"
  And I configure dnf with
      | key    | value |
      | zchunk | True  |
 When I execute dnf with args "install labirinto"
 Then the exit code is 0
  And Transaction is following
      | Action        | Package                       |
      | install       | labirinto-0:1.0-1.fc29.x86_64 |


Scenario: download zchunk metadata, enabled by default
Given I copy repository "simple-base" for modification
  And I generate repodata for repository "simple-base" with extra arguments "--zck"
  And I use repository "simple-base" as http
  And I start capturing outbound HTTP requests
 When I execute dnf with args "install labirinto"
 Then the exit code is 0
  And exactly 2 HTTP GET requests should match
      | path                      |
      | /repodata/primary.xml.zck |


@bz1851841
@bz1779104
Scenario: ignore zchunk metadata if disabled
Given I copy repository "simple-base" for modification
  And I generate repodata for repository "simple-base" with extra arguments "--zck"
  And I use repository "simple-base" as http
  And I start capturing outbound HTTP requests
  And I configure dnf with
      | key    | value |
      | zchunk | False |
 When I execute dnf with args "install labirinto"
 Then the exit code is 0
  And exactly 1 HTTP GET requests should match
      | path                      |
      | /repodata/primary.xml.zst |


@bz1886706
Scenario: I can install an RPM from FTP mirror with zchunk repo and enabled zchunk
Given I copy repository "simple-base" for modification
  And I generate repodata for repository "simple-base" with extra arguments "--zck"
  And I use repository "simple-base" as ftp
  And I configure dnf with
      | key    | value |
      | zchunk | True  |
 When I execute dnf with args "install labirinto"
 Then the exit code is 0
  And Transaction is following
      | Action        | Package                       |
      | install       | labirinto-0:1.0-1.fc29.x86_64 |


Scenario: I can install an RPM from FTP mirror with zchunk repo and disabled zchunk
Given I copy repository "simple-base" for modification
  And I generate repodata for repository "simple-base" with extra arguments "--zck"
  And I use repository "simple-base" as ftp
  And I configure dnf with
      | key    | value |
      | zchunk | False |
 When I execute dnf with args "install labirinto"
 Then the exit code is 0
  And Transaction is following
      | Action        | Package                       |
      | install       | labirinto-0:1.0-1.fc29.x86_64 |


Scenario: when zchunk is enabled, prefer HTTP over FTP
Given I copy repository "simple-base" for modification
  And I generate repodata for repository "simple-base" with extra arguments "--zck"
  And I start http server "http_server" at "/{context.dnf.repos[simple-base].path}"
  And I start ftp server "ftp_server" at "/{context.dnf.repos[simple-base].path}"
  And I create and substitute file "/tmp/mirrorlist" with
      """
      ftp://localhost:{context.dnf.ports[ftp_server]}/
      http://localhost:{context.dnf.ports[http_server]}/
      """
  And I configure a new repository "testrepo" with
      | key        | value                                    |
      | mirrorlist | {context.dnf.installroot}/tmp/mirrorlist |
  And I configure dnf with
      | key    | value |
      | zchunk | True  |
  And I start capturing outbound HTTP requests
 When I execute dnf with args "install labirinto"
 Then the exit code is 0
  And Transaction is following
      | Action        | Package                       |
      | install       | labirinto-0:1.0-1.fc29.x86_64 |
  And exactly 2 HTTP GET requests should match
      | path                      |
      | /repodata/primary.xml.zck |


Scenario: when zchunk is enabled, prefer HTTP over FTP (reversed)
Given I copy repository "simple-base" for modification
  And I generate repodata for repository "simple-base" with extra arguments "--zck"
  And I start http server "http_server" at "/{context.dnf.repos[simple-base].path}"
  And I start ftp server "ftp_server" at "/{context.dnf.repos[simple-base].path}"
  And I create and substitute file "/tmp/mirrorlist" with
      """
      http://localhost:{context.dnf.ports[http_server]}/
      ftp://localhost:{context.dnf.ports[ftp_server]}/
      """
  And I configure a new repository "testrepo" with
      | key        | value                                    |
      | mirrorlist | {context.dnf.installroot}/tmp/mirrorlist |
  And I configure dnf with
      | key    | value |
      | zchunk | True  |
  And I start capturing outbound HTTP requests
 When I execute dnf with args "install labirinto"
 Then the exit code is 0
  And Transaction is following
      | Action        | Package                       |
      | install       | labirinto-0:1.0-1.fc29.x86_64 |
  And exactly 2 HTTP GET requests should match
      | path                      |
      | /repodata/primary.xml.zck |


Scenario: using mirror wihtout ranges supports and zchunk results in only two GET requests for primary (the first try is with range specified)
Given I copy repository "simple-base" for modification
  And I generate repodata for repository "simple-base" with extra arguments "--zck"
  And I use repository "simple-base" as http
  And I configure dnf with
      | key    | value |
      | zchunk | True |
  And I start capturing outbound HTTP requests
 When I execute dnf with args "install labirinto"
 Then the exit code is 0
  And Transaction is following
      | Action        | Package                       |
      | install       | labirinto-0:1.0-1.fc29.x86_64 |
  And exactly 2 HTTP GET requests should match
      | path                      |
      | /repodata/primary.xml.zck |
