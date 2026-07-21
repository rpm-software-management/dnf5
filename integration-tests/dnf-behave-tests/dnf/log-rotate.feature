Feature: Log rotation


@xfail
# https://github.com/rpm-software-management/dnf5/issues/1818
@bz1702690
@bz1816573
Scenario: Size and number of log files respects log_size and log_rotate options
  Given I use repository "dnf-ci-fedora"
    And I execute dnf with args "--setopt=log_size=1024 --setopt=log_rotate=2 install glibc"
    And I execute dnf with args "--setopt=log_size=1024 --setopt=log_rotate=2 remove glibc"
    And I execute dnf with args "--setopt=log_size=1024 --setopt=log_rotate=2 install glibc"
    And I execute dnf with args "--setopt=log_size=1024 --setopt=log_rotate=2 remove glibc"
    And I execute dnf with args "--setopt=log_size=1024 --setopt=log_rotate=2 install glibc"
    And I execute dnf with args "--setopt=log_size=1024 --setopt=log_rotate=2 remove glibc"
    And I execute dnf with args "--setopt=log_size=1024 --setopt=log_rotate=2 install glibc"
    And I execute dnf with args "--setopt=log_size=1024 --setopt=log_rotate=2 remove glibc"

   When I execute "ls {context.dnf.installroot}/var/log | grep "dnf5\.log""
   Then stdout is
        """
        dnf5.log
        dnf5.log.1
        dnf5.log.2
        """
   Then size of file "var/log/dnf5.log" is at most "1024"
   Then size of file "var/log/dnf5.log.1" is at most "1024"
   Then size of file "var/log/dnf5.log.2" is at most "1024"


# Log rotation cannot work within installroot
@no_installroot
# https://github.com/rpm-software-management/dnf5/issues/1820
@bz1910084
Scenario: Log rotation keeps file permissions
Given I use repository "dnf-ci-fedora-updates"
  And I successfully execute dnf with args "install flac"
    # Set permissions to 600
  And I successfully execute "chmod 600 /var/log/dnf5.log"
    # Run dnf again, so that files are rotated
 When I execute dnf with args "--setopt=log_size=1 --setopt=log_rotate=2 remove flac"
 Then the exit code is 0
  And file "/var/log/dnf5.log" has mode "600"
  And file "/var/log/dnf5.log.1" has mode "600"


# Log rotation cannot work within installroot
@no_installroot
# https://github.com/rpm-software-management/dnf5/issues/2487
Scenario: Log rotation keeps ACL
Given I use repository "dnf-ci-fedora-updates"
  And I successfully execute dnf with args "install flac"
    # Set non-default ACL
  And I successfully execute "setfacl -m user:root:r /var/log/dnf5.log"
    # Run dnf again, so that files are rotated
 When I execute dnf with args "--setopt=log_size=1 --setopt=log_rotate=2 remove flac"
 Then the exit code is 0
  And file "/var/log/dnf5.log" has ACL entry "user:root:r--"
  And file "/var/log/dnf5.log.1" has ACL entry "user:root:r--"
