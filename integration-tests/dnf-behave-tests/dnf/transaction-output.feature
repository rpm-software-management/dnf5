Feature: Test transasction output


@bz1794856
Scenario: Check whitespace between columns with long values in transaction table
Given I use repository "dnf-ci-thirdparty"
 When I execute dnf with args "clean all"
 # Piping to grep is forcing DNF to print into non standard terminal which is by default limited to 80 columns.
  And I execute "eval dnf5 -y --releasever={context.dnf.releasever} --installroot={context.dnf.installroot} --setopt=module_platform_id={context.dnf.module_platform_id} --disableplugin='*' install forTestingPurposesWeEvenHaveReallyLongVersions | grep -v xxxxxx" in "{context.dnf.installroot}"
 Then the exit code is 0
  And Transaction is following
      | Action        | Package                                                                                    |
      | install       | forTestingPurposesWeEvenHaveReallyLongVersions-0:1435347658326856238756823658aaaa-1.x86_64 |
  And stdout contains "forTestingPurposesWeEvenHaveReallyLongVersions\s+x86_64\s+0:1435347658326856238756823658aaaa-1\s+dnf-ci-thirdparty\s+.*"


@bz1773436
Scenario: Packages in transaction are sorted by NEVRA
  Given I use repository "dnf-ci-fedora"
    And I use repository "dnf-ci-thirdparty"
   When I execute dnf with args "install wget glibc flac SuperRipper"
   Then the exit code is 0
    And stderr matches line by line
      """
      <REPOSYNC>
      Total size of inbound packages is .. KiB. Need to download .. KiB.
      After this operation, . B extra will be used \(install . B, remove . B\).
      [ 1/11] wget-0:1.19.5-5.fc29.x86_64     100% |  .\d\.\d   B/s |  .\d\.\d KiB |  \d\dm\d\ds
      [ 2/11] flac-0:1.3.2-8.fc29.x86_64      100% |  .\d\.\d   B/s |  .\d\.\d KiB |  \d\dm\d\ds
      [ 3/11] SuperRipper-0:1.0-1.x86_64      100% |  .\d\.\d   B/s |  .\d\.\d KiB |  \d\dm\d\ds
      [ 4/11] glibc-0:2.28-9.fc29.x86_64      100% |  .\d\.\d   B/s |  .\d\.\d KiB |  \d\dm\d\ds
      [ 5/11] abcde-0:2.9.2-1.fc29.noarch     100% |  .\d\.\d   B/s |  .\d\.\d KiB |  \d\dm\d\ds
      [ 6/11] basesystem-0:11-6.fc29.noarch   100% |  .\d\.\d   B/s |  .\d\.\d KiB |  \d\dm\d\ds
      [ 7/11] glibc-common-0:2.28-9.fc29.x86_ 100% |  .\d\.\d   B/s |  .\d\.\d KiB |  \d\dm\d\ds
      [ 8/11] filesystem-0:3.9-2.fc29.x86_64  100% |  .\d\.\d   B/s |  .\d\.\d KiB |  \d\dm\d\ds
      [ 9/11] setup-0:2.12.1-1.fc29.noarch    100% |  .\d\.\d   B/s |  .\d\.\d KiB |  \d\dm\d\ds
      [10/11] glibc-all-langpacks-0:2.28-9.fc 100% |  .\d\.\d   B/s |  .\d\.\d KiB |  \d\dm\d\ds
      [11/11] FlacBetterEncoder-0:1.0-1.x86_6 100% |  .\d\.\d   B/s |  .\d\.\d KiB |  \d\dm\d\ds
      --------------------------------------------------------------------------------
      [11/11] Total                           100% |  .... MiB/s |  .... KiB |  00m00s
      Running transaction
      [ 1/13] Verify package files            100% |  .\d\.\d   B/s | ..\d\.\d   B |  \d\dm\d\ds
      [ 2/13] Prepare transaction             100% |  .\d\.\d   B/s | ..\d\.\d   B |  \d\dm\d\ds
      [ 3/13] Installing setup-0:2.12.1-1.fc2 100% |  .\d\.\d   B/s | ..\d\.\d   B |  \d\dm\d\ds
      [ 4/13] Installing filesystem-0:3.9-2.f 100% |  .\d\.\d   B/s | ..\d\.\d   B |  \d\dm\d\ds
      [ 5/13] Installing basesystem-0:11-6.fc 100% |  .\d\.\d   B/s | ..\d\.\d   B |  \d\dm\d\ds
      [ 6/13] Installing glibc-common-0:2.28- 100% |  .\d\.\d   B/s | ..\d\.\d   B |  \d\dm\d\ds
      [ 7/13] Installing glibc-all-langpacks- 100% |  .\d\.\d   B/s | ..\d\.\d   B |  \d\dm\d\ds
      [ 8/13] Installing glibc-0:2.28-9.fc29. 100% |  .\d\.\d   B/s | ..\d\.\d   B |  \d\dm\d\ds
      [ 9/13] Installing wget-0:1.19.5-5.fc29 100% |  .\d\.\d   B/s | ..\d\.\d   B |  \d\dm\d\ds
      [10/13] Installing abcde-0:2.9.2-1.fc29 100% |  .\d\.\d   B/s | ..\d\.\d   B |  \d\dm\d\ds
      [11/13] Installing SuperRipper-0:1.0-1. 100% |  .\d\.\d   B/s | ..\d\.\d   B |  \d\dm\d\ds
      [12/13] Installing FlacBetterEncoder-0: 100% |  .\d\.\d   B/s | ..\d\.\d   B |  \d\dm\d\ds
      [13/13] Installing flac-0:1.3.2-8.fc29. 100% |  .\d\.\d KiB/s | ..\d\.\d   B |  \d\dm\d\ds
      Complete!
      """
    And stdout matches line by line
      """
      Package              Arch   Version         Repository             Size
      Installing:
       SuperRipper         x86_64 0:1.0-1         dnf-ci-thirdparty   \d\.\d   B
       flac                x86_64 0:1.3.2-8.fc29  dnf-ci-fedora       \d\.\d   B
       glibc               x86_64 0:2.28-9.fc29   dnf-ci-fedora       \d\.\d   B
       wget                x86_64 0:1.19.5-5.fc29 dnf-ci-fedora       \d\.\d   B
      Installing dependencies:
       abcde               noarch 0:2.9.2-1.fc29  dnf-ci-fedora       \d\.\d   B
       basesystem          noarch 0:11-6.fc29     dnf-ci-fedora       \d\.\d   B
       filesystem          x86_64 0:3.9-2.fc29    dnf-ci-fedora       \d\.\d   B
       glibc-all-langpacks x86_64 0:2.28-9.fc29   dnf-ci-fedora       \d\.\d   B
       glibc-common        x86_64 0:2.28-9.fc29   dnf-ci-fedora       \d\.\d   B
       setup               noarch 0:2.12.1-1.fc29 dnf-ci-fedora       \d\.\d   B
      Installing weak dependencies:
       FlacBetterEncoder   x86_64 0:1.0-1         dnf-ci-thirdparty   \d\.\d   B

      Transaction Summary:
       Installing:        11 packages
      """
