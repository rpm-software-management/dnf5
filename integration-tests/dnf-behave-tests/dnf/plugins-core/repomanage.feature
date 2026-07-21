Feature: Tests for repomanage command


Scenario: basic functionality of repomanage --new
Given I copy repository "dnf-ci-thirdparty-updates" for modification
 When I execute dnf with args "repomanage --new {context.dnf.repos[dnf-ci-thirdparty-updates].path}"
 Then the exit code is 0
  And stdout is
      """
      {context.dnf.tempdir}/repos/dnf-ci-thirdparty-updates/src/CQRlib-extension-1.6-2.src.rpm
      {context.dnf.tempdir}/repos/dnf-ci-thirdparty-updates/src/SuperRipper-1.3-1.src.rpm
      {context.dnf.tempdir}/repos/dnf-ci-thirdparty-updates/x86_64/CQRlib-extension-1.6-2.x86_64.rpm
      {context.dnf.tempdir}/repos/dnf-ci-thirdparty-updates/x86_64/SuperRipper-1.3-1.x86_64.rpm
      """


Scenario: basic functionality of repomanage --new works without repodata
Given I copy repository "dnf-ci-thirdparty-updates" for modification
  And I delete directory "/{context.dnf.repos[dnf-ci-thirdparty-updates].path}/repodata"
 When I execute dnf with args "repomanage --new {context.dnf.repos[dnf-ci-thirdparty-updates].path}"
 Then the exit code is 0
  And stdout is
      """
      {context.dnf.tempdir}/repos/dnf-ci-thirdparty-updates/src/CQRlib-extension-1.6-2.src.rpm
      {context.dnf.tempdir}/repos/dnf-ci-thirdparty-updates/src/SuperRipper-1.3-1.src.rpm
      {context.dnf.tempdir}/repos/dnf-ci-thirdparty-updates/x86_64/CQRlib-extension-1.6-2.x86_64.rpm
      {context.dnf.tempdir}/repos/dnf-ci-thirdparty-updates/x86_64/SuperRipper-1.3-1.x86_64.rpm
      """


Scenario: basic functionality of repomanage --old
Given I copy repository "dnf-ci-thirdparty-updates" for modification
 When I execute dnf with args "repomanage --old {context.dnf.repos[dnf-ci-thirdparty-updates].path}"
 Then the exit code is 0
  And stdout is
      """
      {context.dnf.tempdir}/repos/dnf-ci-thirdparty-updates/src/SuperRipper-1.2-1.src.rpm
      {context.dnf.tempdir}/repos/dnf-ci-thirdparty-updates/x86_64/SuperRipper-1.2-1.x86_64.rpm
      """


Scenario: basic functionality of repomanage --new with -s
Given I copy repository "dnf-ci-thirdparty-updates" for modification
 When I execute dnf with args "repomanage --new -s {context.dnf.repos[dnf-ci-thirdparty-updates].path}"
 Then the exit code is 0
  And stdout is
      """
      {context.dnf.tempdir}/repos/dnf-ci-thirdparty-updates/src/CQRlib-extension-1.6-2.src.rpm {context.dnf.tempdir}/repos/dnf-ci-thirdparty-updates/src/SuperRipper-1.3-1.src.rpm {context.dnf.tempdir}/repos/dnf-ci-thirdparty-updates/x86_64/CQRlib-extension-1.6-2.x86_64.rpm {context.dnf.tempdir}/repos/dnf-ci-thirdparty-updates/x86_64/SuperRipper-1.3-1.x86_64.rpm
      """


Scenario: basic functionality of repomanage --new with -k
Given I copy repository "dnf-ci-thirdparty-updates" for modification
 When I execute dnf with args "repomanage --new -k 2 {context.dnf.repos[dnf-ci-thirdparty-updates].path}"
 Then the exit code is 0
  And stdout is
      """
      {context.dnf.tempdir}/repos/dnf-ci-thirdparty-updates/src/CQRlib-extension-1.6-2.src.rpm
      {context.dnf.tempdir}/repos/dnf-ci-thirdparty-updates/src/SuperRipper-1.2-1.src.rpm
      {context.dnf.tempdir}/repos/dnf-ci-thirdparty-updates/src/SuperRipper-1.3-1.src.rpm
      {context.dnf.tempdir}/repos/dnf-ci-thirdparty-updates/x86_64/CQRlib-extension-1.6-2.x86_64.rpm
      {context.dnf.tempdir}/repos/dnf-ci-thirdparty-updates/x86_64/SuperRipper-1.2-1.x86_64.rpm
      {context.dnf.tempdir}/repos/dnf-ci-thirdparty-updates/x86_64/SuperRipper-1.3-1.x86_64.rpm
      """


Scenario: basic functionality of repomanage --new with duplicate packages
Given I copy repository "dnf-ci-thirdparty-updates" for modification
  And I create directory "/{context.dnf.repos[dnf-ci-thirdparty-updates].path}/duplicate"
  And I copy file "/{context.dnf.repos[dnf-ci-thirdparty-updates].path}/x86_64/SuperRipper-1.3-1.x86_64.rpm" to "/{context.dnf.repos[dnf-ci-thirdparty-updates].path}/duplicate/SuperRipper-1.3-1.x86_64.rpm"
  And I delete directory "/{context.dnf.repos[dnf-ci-thirdparty-updates].path}/repodata"
 When I execute dnf with args "repomanage --new {context.dnf.repos[dnf-ci-thirdparty-updates].path}"
 Then the exit code is 0
  And stdout is
      """
      {context.dnf.tempdir}/repos/dnf-ci-thirdparty-updates/duplicate/SuperRipper-1.3-1.x86_64.rpm
      {context.dnf.tempdir}/repos/dnf-ci-thirdparty-updates/src/CQRlib-extension-1.6-2.src.rpm
      {context.dnf.tempdir}/repos/dnf-ci-thirdparty-updates/src/SuperRipper-1.3-1.src.rpm
      {context.dnf.tempdir}/repos/dnf-ci-thirdparty-updates/x86_64/CQRlib-extension-1.6-2.x86_64.rpm
      {context.dnf.tempdir}/repos/dnf-ci-thirdparty-updates/x86_64/SuperRipper-1.3-1.x86_64.rpm
      """


Scenario: basic functionality of repomanage --new with duplicate packages and -k 2
Given I copy repository "dnf-ci-thirdparty-updates" for modification
  And I create directory "/{context.dnf.repos[dnf-ci-thirdparty-updates].path}/duplicate"
  And I copy file "/{context.dnf.repos[dnf-ci-thirdparty-updates].path}/x86_64/SuperRipper-1.3-1.x86_64.rpm" to "/{context.dnf.repos[dnf-ci-thirdparty-updates].path}/duplicate/SuperRipper-1.3-1.x86_64.rpm"
  And I delete directory "/{context.dnf.repos[dnf-ci-thirdparty-updates].path}/repodata"
 When I execute dnf with args "repomanage -k 2 --new {context.dnf.repos[dnf-ci-thirdparty-updates].path}"
 Then the exit code is 0
  And stdout is
      """
      {context.dnf.tempdir}/repos/dnf-ci-thirdparty-updates/duplicate/SuperRipper-1.3-1.x86_64.rpm
      {context.dnf.tempdir}/repos/dnf-ci-thirdparty-updates/src/CQRlib-extension-1.6-2.src.rpm
      {context.dnf.tempdir}/repos/dnf-ci-thirdparty-updates/src/SuperRipper-1.2-1.src.rpm
      {context.dnf.tempdir}/repos/dnf-ci-thirdparty-updates/src/SuperRipper-1.3-1.src.rpm
      {context.dnf.tempdir}/repos/dnf-ci-thirdparty-updates/x86_64/CQRlib-extension-1.6-2.x86_64.rpm
      {context.dnf.tempdir}/repos/dnf-ci-thirdparty-updates/x86_64/SuperRipper-1.2-1.x86_64.rpm
      {context.dnf.tempdir}/repos/dnf-ci-thirdparty-updates/x86_64/SuperRipper-1.3-1.x86_64.rpm
      """


@xfail
# Missing modulariry support: https://github.com/rpm-software-management/dnf5/issues/2388
@bz1804720
Scenario: don't list packages as old if they are in different stream
Given I copy repository "dnf-ci-fedora-modular-updates" for modification
 When I execute dnf with args "repomanage --old -k 1 {context.dnf.repos[dnf-ci-fedora-modular-updates].path}"
 Then the exit code is 0
  And stdout is empty


@xfail
# Missing modulariry support: https://github.com/rpm-software-management/dnf5/issues/2388
@bz1804720
Scenario: list packages as new if they are in different stream or are non modular
Given I copy repository "dnf-ci-fedora-modular-updates" for modification
 When I execute dnf with args "repomanage --new {context.dnf.repos[dnf-ci-fedora-modular-updates].path}"
 Then the exit code is 0
  And stdout is
      """
      {context.dnf.tempdir}/repos/dnf-ci-fedora-modular-updates/noarch/nodejs-docs-10.14.1-1.module_2533+7361f245.noarch.rpm
      {context.dnf.tempdir}/repos/dnf-ci-fedora-modular-updates/noarch/nodejs-docs-11.1.0-1.module_2379+8d497405.noarch.rpm
      {context.dnf.tempdir}/repos/dnf-ci-fedora-modular-updates/noarch/nodejs-docs-12.1.0-1.module_2379+8d497405.noarch.rpm
      {context.dnf.tempdir}/repos/dnf-ci-fedora-modular-updates/noarch/nodejs-docs-8.11.4-1.module_2030+42747d40.noarch.rpm
      {context.dnf.tempdir}/repos/dnf-ci-fedora-modular-updates/src/http-parser-2.9.0-1.module_2672+97d6a5e9.src.rpm
      {context.dnf.tempdir}/repos/dnf-ci-fedora-modular-updates/src/nodejs-10.14.1-1.module_2533+7361f245.src.rpm
      {context.dnf.tempdir}/repos/dnf-ci-fedora-modular-updates/src/nodejs-11.1.0-1.module_2379+8d497405.src.rpm
      {context.dnf.tempdir}/repos/dnf-ci-fedora-modular-updates/src/nodejs-12.1.0-1.module_2379+8d497405.src.rpm
      {context.dnf.tempdir}/repos/dnf-ci-fedora-modular-updates/src/nodejs-8.11.4-1.module_2030+42747d40.src.rpm
      {context.dnf.tempdir}/repos/dnf-ci-fedora-modular-updates/src/nodejs-8.14.0-1.src.rpm
      {context.dnf.tempdir}/repos/dnf-ci-fedora-modular-updates/src/npm-8.14.0-1.module_2030+42747d41.src.rpm
      {context.dnf.tempdir}/repos/dnf-ci-fedora-modular-updates/src/postgresql-10.6-1.module_2594+0c9aadc5.src.rpm
      {context.dnf.tempdir}/repos/dnf-ci-fedora-modular-updates/src/postgresql-11.1-2.module_2597+e45c4cc9.src.rpm
      {context.dnf.tempdir}/repos/dnf-ci-fedora-modular-updates/src/postgresql-9.6.11-1.module_2689+ea8f147f.src.rpm
      {context.dnf.tempdir}/repos/dnf-ci-fedora-modular-updates/x86_64/http-parser-2.9.0-1.module_2672+97d6a5e9.x86_64.rpm
      {context.dnf.tempdir}/repos/dnf-ci-fedora-modular-updates/x86_64/nodejs-10.14.1-1.module_2533+7361f245.x86_64.rpm
      {context.dnf.tempdir}/repos/dnf-ci-fedora-modular-updates/x86_64/nodejs-11.1.0-1.module_2379+8d497405.x86_64.rpm
      {context.dnf.tempdir}/repos/dnf-ci-fedora-modular-updates/x86_64/nodejs-12.1.0-1.module_2379+8d497405.x86_64.rpm
      {context.dnf.tempdir}/repos/dnf-ci-fedora-modular-updates/x86_64/nodejs-8.11.4-1.module_2030+42747d40.x86_64.rpm
      {context.dnf.tempdir}/repos/dnf-ci-fedora-modular-updates/x86_64/nodejs-8.14.0-1.x86_64.rpm
      {context.dnf.tempdir}/repos/dnf-ci-fedora-modular-updates/x86_64/nodejs-devel-10.14.1-1.module_2533+7361f245.x86_64.rpm
      {context.dnf.tempdir}/repos/dnf-ci-fedora-modular-updates/x86_64/nodejs-devel-11.1.0-1.module_2379+8d497405.x86_64.rpm
      {context.dnf.tempdir}/repos/dnf-ci-fedora-modular-updates/x86_64/nodejs-devel-12.1.0-1.module_2379+8d497405.x86_64.rpm
      {context.dnf.tempdir}/repos/dnf-ci-fedora-modular-updates/x86_64/nodejs-devel-8.11.4-1.module_2030+42747d40.x86_64.rpm
      {context.dnf.tempdir}/repos/dnf-ci-fedora-modular-updates/x86_64/npm-10.14.1-1.module_2533+7361f245.x86_64.rpm
      {context.dnf.tempdir}/repos/dnf-ci-fedora-modular-updates/x86_64/npm-11.1.0-1.module_2379+8d497405.x86_64.rpm
      {context.dnf.tempdir}/repos/dnf-ci-fedora-modular-updates/x86_64/npm-12.1.0-1.module_2379+8d497405.x86_64.rpm
      {context.dnf.tempdir}/repos/dnf-ci-fedora-modular-updates/x86_64/npm-8.14.0-1.module_2030+42747d41.x86_64.rpm
      {context.dnf.tempdir}/repos/dnf-ci-fedora-modular-updates/x86_64/postgresql-10.6-1.module_2594+0c9aadc5.x86_64.rpm
      {context.dnf.tempdir}/repos/dnf-ci-fedora-modular-updates/x86_64/postgresql-11.1-2.module_2597+e45c4cc9.x86_64.rpm
      {context.dnf.tempdir}/repos/dnf-ci-fedora-modular-updates/x86_64/postgresql-9.6.11-1.module_2689+ea8f147f.x86_64.rpm
      {context.dnf.tempdir}/repos/dnf-ci-fedora-modular-updates/x86_64/postgresql-libs-10.6-1.module_2594+0c9aadc5.x86_64.rpm
      {context.dnf.tempdir}/repos/dnf-ci-fedora-modular-updates/x86_64/postgresql-libs-11.1-2.module_2597+e45c4cc9.x86_64.rpm
      {context.dnf.tempdir}/repos/dnf-ci-fedora-modular-updates/x86_64/postgresql-libs-9.6.11-1.module_2689+ea8f147f.x86_64.rpm
      {context.dnf.tempdir}/repos/dnf-ci-fedora-modular-updates/x86_64/postgresql-server-10.6-1.module_2594+0c9aadc5.x86_64.rpm
      {context.dnf.tempdir}/repos/dnf-ci-fedora-modular-updates/x86_64/postgresql-server-11.1-2.module_2597+e45c4cc9.x86_64.rpm
      {context.dnf.tempdir}/repos/dnf-ci-fedora-modular-updates/x86_64/postgresql-server-9.6.11-1.module_2689+ea8f147f.x86_64.rpm
      {context.dnf.tempdir}/repos/dnf-ci-fedora-modular-updates/x86_64/postgresql-test-10.6-1.module_2594+0c9aadc5.x86_64.rpm
      {context.dnf.tempdir}/repos/dnf-ci-fedora-modular-updates/x86_64/postgresql-test-11.1-2.module_2597+e45c4cc9.x86_64.rpm
      {context.dnf.tempdir}/repos/dnf-ci-fedora-modular-updates/x86_64/postgresql-test-9.6.11-1.module_2689+ea8f147f.x86_64.rpm
      """


@xfail
# Missing modulariry support: https://github.com/rpm-software-management/dnf5/issues/2388
@bz1804720
Scenario: packages get listed as old if there are newer version within the same stream
Given I copy repository "dnf-ci-multicontext-hybrid-multiversion-modular" for modification
 When I execute dnf with args "repomanage --old -k 1 {context.dnf.repos[dnf-ci-multicontext-hybrid-multiversion-modular].path}"
 Then the exit code is 0
  And stdout is
      """
      {context.dnf.tempdir}/repos/dnf-ci-multicontext-hybrid-multiversion-modular/noarch/nodejs-docs-5.12.1-1.fc29.noarch.rpm
      {context.dnf.tempdir}/repos/dnf-ci-multicontext-hybrid-multiversion-modular/src/nodejs-5.12.1-1.fc29.src.rpm
      {context.dnf.tempdir}/repos/dnf-ci-multicontext-hybrid-multiversion-modular/src/nodejs-5.3.1-1.module_2011+41787af0.src.rpm
      {context.dnf.tempdir}/repos/dnf-ci-multicontext-hybrid-multiversion-modular/src/nodejs-5.3.1-1.module_3012+41787ba3.src.rpm
      {context.dnf.tempdir}/repos/dnf-ci-multicontext-hybrid-multiversion-modular/src/nodejs-5.4.1-2.module_2011+41787af1.src.rpm
      {context.dnf.tempdir}/repos/dnf-ci-multicontext-hybrid-multiversion-modular/src/nodejs-5.4.1-2.module_3012+41787ba4.src.rpm
      {context.dnf.tempdir}/repos/dnf-ci-multicontext-hybrid-multiversion-modular/src/postgresql-9.6.8-1.module_1710+b535a823.src.rpm
      {context.dnf.tempdir}/repos/dnf-ci-multicontext-hybrid-multiversion-modular/x86_64/nodejs-5.12.1-1.fc29.x86_64.rpm
      {context.dnf.tempdir}/repos/dnf-ci-multicontext-hybrid-multiversion-modular/x86_64/nodejs-5.3.1-1.module_2011+41787af0.x86_64.rpm
      {context.dnf.tempdir}/repos/dnf-ci-multicontext-hybrid-multiversion-modular/x86_64/nodejs-5.3.1-1.module_3012+41787ba3.x86_64.rpm
      {context.dnf.tempdir}/repos/dnf-ci-multicontext-hybrid-multiversion-modular/x86_64/nodejs-devel-5.12.1-1.fc29.x86_64.rpm
      {context.dnf.tempdir}/repos/dnf-ci-multicontext-hybrid-multiversion-modular/x86_64/npm-5.12.1-1.fc29.x86_64.rpm
      """


@xfail
# Missing modulariry support: https://github.com/rpm-software-management/dnf5/issues/2388
@bz1804720
Scenario: only the newest modular packages get listed as new
Given I copy repository "dnf-ci-multicontext-hybrid-multiversion-modular" for modification
 When I execute dnf with args "repomanage --new {context.dnf.repos[dnf-ci-multicontext-hybrid-multiversion-modular].path}"
 Then the exit code is 0
  And stdout is
      """
      {context.dnf.tempdir}/repos/dnf-ci-multicontext-hybrid-multiversion-modular/noarch/nodejs-docs-5.12.2-3.fc29.noarch.rpm
      {context.dnf.tempdir}/repos/dnf-ci-multicontext-hybrid-multiversion-modular/src/nodejs-5.12.2-3.fc29.src.rpm
      {context.dnf.tempdir}/repos/dnf-ci-multicontext-hybrid-multiversion-modular/src/postgresql-9.8.1-1.module_9790+c535b823.src.rpm
      {context.dnf.tempdir}/repos/dnf-ci-multicontext-hybrid-multiversion-modular/x86_64/nodejs-5.12.2-3.fc29.x86_64.rpm
      {context.dnf.tempdir}/repos/dnf-ci-multicontext-hybrid-multiversion-modular/x86_64/nodejs-5.4.1-2.module_2011+41787af1.x86_64.rpm
      {context.dnf.tempdir}/repos/dnf-ci-multicontext-hybrid-multiversion-modular/x86_64/nodejs-5.4.1-2.module_3012+41787ba4.x86_64.rpm
      {context.dnf.tempdir}/repos/dnf-ci-multicontext-hybrid-multiversion-modular/x86_64/nodejs-devel-5.12.2-3.fc29.x86_64.rpm
      {context.dnf.tempdir}/repos/dnf-ci-multicontext-hybrid-multiversion-modular/x86_64/npm-5.12.2-3.fc29.x86_64.rpm
      {context.dnf.tempdir}/repos/dnf-ci-multicontext-hybrid-multiversion-modular/x86_64/postgresql-9.6.8-1.module_1710+b535a823.x86_64.rpm
      {context.dnf.tempdir}/repos/dnf-ci-multicontext-hybrid-multiversion-modular/x86_64/postgresql-9.8.1-1.module_9790+c535b823.x86_64.rpm
      """


@xfail
# Missing modulariry support: https://github.com/rpm-software-management/dnf5/issues/2388
@bz1804720
Scenario: all packages get listed as new if --keep is sufficiently big
Given I copy repository "dnf-ci-multicontext-hybrid-multiversion-modular" for modification
 When I execute dnf with args "repomanage --new --keep 100 {context.dnf.repos[dnf-ci-multicontext-hybrid-multiversion-modular].path}"
 Then the exit code is 0
  And stdout is
      """
      {context.dnf.tempdir}/repos/dnf-ci-multicontext-hybrid-multiversion-modular/noarch/nodejs-docs-5.12.1-1.fc29.noarch.rpm
      {context.dnf.tempdir}/repos/dnf-ci-multicontext-hybrid-multiversion-modular/noarch/nodejs-docs-5.12.2-3.fc29.noarch.rpm
      {context.dnf.tempdir}/repos/dnf-ci-multicontext-hybrid-multiversion-modular/src/nodejs-5.12.1-1.fc29.src.rpm
      {context.dnf.tempdir}/repos/dnf-ci-multicontext-hybrid-multiversion-modular/src/nodejs-5.12.2-3.fc29.src.rpm
      {context.dnf.tempdir}/repos/dnf-ci-multicontext-hybrid-multiversion-modular/src/nodejs-5.3.1-1.module_2011+41787af0.src.rpm
      {context.dnf.tempdir}/repos/dnf-ci-multicontext-hybrid-multiversion-modular/src/nodejs-5.3.1-1.module_3012+41787ba3.src.rpm
      {context.dnf.tempdir}/repos/dnf-ci-multicontext-hybrid-multiversion-modular/src/nodejs-5.4.1-2.module_2011+41787af1.src.rpm
      {context.dnf.tempdir}/repos/dnf-ci-multicontext-hybrid-multiversion-modular/src/nodejs-5.4.1-2.module_3012+41787ba4.src.rpm
      {context.dnf.tempdir}/repos/dnf-ci-multicontext-hybrid-multiversion-modular/src/postgresql-9.6.8-1.module_1710+b535a823.src.rpm
      {context.dnf.tempdir}/repos/dnf-ci-multicontext-hybrid-multiversion-modular/src/postgresql-9.8.1-1.module_9790+c535b823.src.rpm
      {context.dnf.tempdir}/repos/dnf-ci-multicontext-hybrid-multiversion-modular/x86_64/nodejs-5.12.1-1.fc29.x86_64.rpm
      {context.dnf.tempdir}/repos/dnf-ci-multicontext-hybrid-multiversion-modular/x86_64/nodejs-5.12.2-3.fc29.x86_64.rpm
      {context.dnf.tempdir}/repos/dnf-ci-multicontext-hybrid-multiversion-modular/x86_64/nodejs-5.3.1-1.module_2011+41787af0.x86_64.rpm
      {context.dnf.tempdir}/repos/dnf-ci-multicontext-hybrid-multiversion-modular/x86_64/nodejs-5.3.1-1.module_3012+41787ba3.x86_64.rpm
      {context.dnf.tempdir}/repos/dnf-ci-multicontext-hybrid-multiversion-modular/x86_64/nodejs-5.4.1-2.module_2011+41787af1.x86_64.rpm
      {context.dnf.tempdir}/repos/dnf-ci-multicontext-hybrid-multiversion-modular/x86_64/nodejs-5.4.1-2.module_3012+41787ba4.x86_64.rpm
      {context.dnf.tempdir}/repos/dnf-ci-multicontext-hybrid-multiversion-modular/x86_64/nodejs-devel-5.12.1-1.fc29.x86_64.rpm
      {context.dnf.tempdir}/repos/dnf-ci-multicontext-hybrid-multiversion-modular/x86_64/nodejs-devel-5.12.2-3.fc29.x86_64.rpm
      {context.dnf.tempdir}/repos/dnf-ci-multicontext-hybrid-multiversion-modular/x86_64/npm-5.12.1-1.fc29.x86_64.rpm
      {context.dnf.tempdir}/repos/dnf-ci-multicontext-hybrid-multiversion-modular/x86_64/npm-5.12.2-3.fc29.x86_64.rpm
      {context.dnf.tempdir}/repos/dnf-ci-multicontext-hybrid-multiversion-modular/x86_64/postgresql-9.6.8-1.module_1710+b535a823.x86_64.rpm
      {context.dnf.tempdir}/repos/dnf-ci-multicontext-hybrid-multiversion-modular/x86_64/postgresql-9.8.1-1.module_9790+c535b823.x86_64.rpm
      """


@xfail
# Missing modulariry support: https://github.com/rpm-software-management/dnf5/issues/2388
Scenario: multiple runs of repomanage don't use cached metadata
Given I copy repository "dnf-ci-multicontext-hybrid-multiversion-modular" for modification
  And I execute dnf with args "repomanage --new --keep 100 {context.dnf.repos[dnf-ci-multicontext-hybrid-multiversion-modular].path}"
  And I delete file "//{context.dnf.tempdir}/repos/dnf-ci-multicontext-hybrid-multiversion-modular/*/nodejs*" with globs
  And I generate repodata for repository "dnf-ci-multicontext-hybrid-multiversion-modular"
  And I execute dnf with args "repomanage --new --keep 100 {context.dnf.repos[dnf-ci-multicontext-hybrid-multiversion-modular].path}"
 Then the exit code is 0
  And stdout is
      """
      {context.dnf.tempdir}/repos/dnf-ci-multicontext-hybrid-multiversion-modular/src/postgresql-9.6.8-1.module_1710+b535a823.src.rpm
      {context.dnf.tempdir}/repos/dnf-ci-multicontext-hybrid-multiversion-modular/src/postgresql-9.8.1-1.module_9790+c535b823.src.rpm
      {context.dnf.tempdir}/repos/dnf-ci-multicontext-hybrid-multiversion-modular/x86_64/npm-5.12.1-1.fc29.x86_64.rpm
      {context.dnf.tempdir}/repos/dnf-ci-multicontext-hybrid-multiversion-modular/x86_64/npm-5.12.2-3.fc29.x86_64.rpm
      {context.dnf.tempdir}/repos/dnf-ci-multicontext-hybrid-multiversion-modular/x86_64/postgresql-9.6.8-1.module_1710+b535a823.x86_64.rpm
      {context.dnf.tempdir}/repos/dnf-ci-multicontext-hybrid-multiversion-modular/x86_64/postgresql-9.8.1-1.module_9790+c535b823.x86_64.rpm
      """


@xfail
# Missing modulariry support: https://github.com/rpm-software-management/dnf5/issues/2388
@bz2034736
@bz2058676
Scenario: --oldonly doesn't print and rpm if it is contained in both new and old version of a stream
Given I copy repository "repomanage-modular" for modification
 When I execute dnf with args "repomanage --new {context.dnf.repos[repomanage-modular].path}"
 Then the exit code is 0
  And stdout is
      """
      {context.dnf.tempdir}/repos/repomanage-modular/noarch/xsom-0-19.20110809svn.module+el8.1.0+3366+6dfb954c.noarch.rpm
      {context.dnf.tempdir}/repos/repomanage-modular/src/xsom-0-19.20110809svn.module+el8.1.0+3366+6dfb954c.src.rpm
      """
 When I execute dnf with args "repomanage --old {context.dnf.repos[repomanage-modular].path}"
 Then the exit code is 0
  And stdout is
      """
      {context.dnf.tempdir}/repos/repomanage-modular/noarch/meson-0.47.1-5.module_1993+7c0a4d1e.noarch.rpm
      {context.dnf.tempdir}/repos/repomanage-modular/noarch/xsom-0-19.20110809svn.module+el8.1.0+3366+6dfb954c.noarch.rpm
      {context.dnf.tempdir}/repos/repomanage-modular/src/meson-0.47.1-5.module_1993+7c0a4d1e.src.rpm
      {context.dnf.tempdir}/repos/repomanage-modular/src/xsom-0-19.20110809svn.module+el8.1.0+3366+6dfb954c.src.rpm
      """
 When I execute dnf with args "repomanage --oldonly {context.dnf.repos[repomanage-modular].path}"
 Then the exit code is 0
  And stdout is
      """
      {context.dnf.tempdir}/repos/repomanage-modular/noarch/meson-0.47.1-5.module_1993+7c0a4d1e.noarch.rpm
      {context.dnf.tempdir}/repos/repomanage-modular/src/meson-0.47.1-5.module_1993+7c0a4d1e.src.rpm
      """


@xfail
# Missing modulariry support: https://github.com/rpm-software-management/dnf5/issues/2388
@bz1804720
Scenario: Show rpms from a newest module stream version from repo even if even newer stream version is enabled from a differente repo
Given I use repository "repomanage-8.5"
  And I execute dnf with args "module enable python36"
  And I drop repository "repomanage-8.5"
  And I copy repository "repomanage-8.3" for modification
 When I execute dnf with args "repomanage {context.dnf.repos[repomanage-8.3].path}"
 Then the exit code is 0
  And stdout is
      """
      {context.dnf.tempdir}/repos/repomanage-8.3/src/python36-3.6.8-2.module+el8.1.0+3334+5cb623d7.src.rpm
      {context.dnf.tempdir}/repos/repomanage-8.3/x86_64/python36-3.6.8-2.module+el8.1.0+3334+5cb623d7.x86_64.rpm
      """


@xfail
# Missing modulariry support: https://github.com/rpm-software-management/dnf5/issues/2388
Scenario: Show rpms from a newest module stream from repo even if there is enabled repo with even newer stream version
Given I copy repository "repomanage-8.3" for modification
  And I use repository "repomanage-8.5"
 When I execute dnf with args "repomanage {context.dnf.repos[repomanage-8.3].path}"
 Then the exit code is 0
  And stdout is
      """
      {context.dnf.tempdir}/repos/repomanage-8.3/src/python36-3.6.8-2.module+el8.1.0+3334+5cb623d7.src.rpm
      {context.dnf.tempdir}/repos/repomanage-8.3/x86_64/python36-3.6.8-2.module+el8.1.0+3334+5cb623d7.x86_64.rpm
      """
