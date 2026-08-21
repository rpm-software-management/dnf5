Feature: Tests related to keepcache functionality


Background:
  Given I use repository "simple-base"


Scenario: Keepcache set to false removes installed packages from cache
   When I execute dnf with args "install labirinto"
   Then the exit code is 0
    And Transaction is following
        | Action        | Package                                   |
        | install       | labirinto-0:1.0-1.fc29.x86_64             |
   When I execute "find -type f -name '*.rpm'" in "{context.dnf.installroot}/var/cache/dnf"
   Then stdout is empty


Scenario: Keepcache set to true keeps installed packages in cache
   When I execute dnf with args "--setopt=keepcache=true install labirinto"
   Then the exit code is 0
    And Transaction is following
        | Action        | Package                                   |
        | install       | labirinto-0:1.0-1.fc29.x86_64             |
   When I execute "find -type f -name '*.rpm'" in "{context.dnf.installroot}/var/cache/dnf"
   Then stdout matches line by line
   """
   \./simple-base-[0-9a-f]{16}/packages/labirinto-1\.0-1\.fc29\.x86_64\.rpm
   """


Scenario: Keepcache set to false does not remove command-line packages
   When I execute dnf with args "install file://{context.dnf.fixturesdir}/repos/simple-base/x86_64/labirinto-1.0-1.fc29.x86_64.rpm"
   Then the exit code is 0
    And Transaction is following
        | Action        | Package                                   |
        | install       | labirinto-0:1.0-1.fc29.x86_64             |
   When I execute "find -type f -name '*.rpm'" in "{context.dnf.installroot}/var/cache/dnf"
   Then stdout matches line by line
   """
   \./@commandline-[0-9a-f]{16}/packages/[0-9a-f]{16}-labirinto-1\.0-1\.fc29\.x86_64\.rpm
   """


Scenario: Keepcache set to false does not remove packages from download command after the following successful transaction
  Given I set working directory to "{context.dnf.tempdir}"
   When I execute dnf with args "download labirinto"
   Then the exit code is 0
    And file "/{context.dnf.tempdir}/labirinto-1.0-1.fc29.x86_64.rpm" exists
   When I execute dnf with args "install labirinto"
   Then the exit code is 0
    And Transaction is following
        | Action        | Package                                   |
        | install       | labirinto-0:1.0-1.fc29.x86_64             |
    And file "/{context.dnf.tempdir}/labirinto-1.0-1.fc29.x86_64.rpm" exists


Scenario: Reseting keepcache does not remove previously kept packages from cache
   When I execute dnf with args "--setopt=keepcache=true install labirinto vagare"
   Then the exit code is 0
    And Transaction is following
        | Action        | Package                                   |
        | install       | labirinto-0:1.0-1.fc29.x86_64             |
        | install       | vagare-0:1.0-1.fc29.x86_64                |
   When I set LC_ALL to "C.UTF-8"
    And I execute "find -type f -name '*.rpm' | sort" in "{context.dnf.installroot}/var/cache/dnf"
   Then stdout matches line by line
   """
   \./simple-base-[0-9a-f]{16}/packages/labirinto-1\.0-1\.fc29\.x86_64\.rpm
   \./simple-base-[0-9a-f]{16}/packages/vagare-1\.0-1\.fc29\.x86_64\.rpm
   """
   When I execute dnf with args "--setopt=keepcache=false install dedalo"
   Then the exit code is 0
    And Transaction is following
        | Action        | Package                            |
        | install       | dedalo-0:1.0-1.fc29.x86_64         |
   When I set LC_ALL to "C.UTF-8"
    And I execute "find -type f -name '*.rpm' | sort" in "{context.dnf.installroot}/var/cache/dnf"
   Then stdout matches line by line
   """
   \./simple-base-[0-9a-f]{16}/packages/labirinto-1\.0-1\.fc29\.x86_64\.rpm
   \./simple-base-[0-9a-f]{16}/packages/vagare-1\.0-1\.fc29\.x86_64\.rpm
   """


# Test that a package that was cached in the previous transaction (the 'vagare' package)
# will be left untouched by the following successful remove operation (the 'labirinto' package)
@bz2237883
Scenario: Cached packages are not removed after remove command with keepcache set to false
  Given I successfully execute dnf with args "install labirinto"
   Then Transaction is following
        | Action        | Package                                   |
        | install       | labirinto-0:1.0-1.fc29.x86_64             |
   When I execute "find -type f -name '*.rpm'" in "{context.dnf.installroot}/var/cache/dnf"
   Then stdout is empty
  Given I successfully execute dnf with args "install vagare --downloadonly"
   When I execute "find -type f -name '*.rpm'" in "{context.dnf.installroot}/var/cache/dnf"
   Then stdout matches line by line
   """
   \./simple-base-[0-9a-f]{16}/packages/vagare-1\.0-1\.fc29\.x86_64\.rpm
   """
  Given I successfully execute dnf with args "remove labirinto"
   Then Transaction is following
        | Action        | Package                                   |
        | remove        | labirinto-0:1.0-1.fc29.x86_64             |
   When I execute "find -type f -name '*.rpm'" in "{context.dnf.installroot}/var/cache/dnf"
   Then stdout matches line by line
   """
   \./simple-base-[0-9a-f]{16}/packages/vagare-1\.0-1\.fc29\.x86_64\.rpm
   """
