# destructive because it can create a new user on the system
@destructive
Feature: Testing functionality related to sharing root metadata cache to users

Background:
  # unprivileged user will need access to enter installroot and read files there
  Given I successfully execute "chmod go+rwx {context.dnf.installroot}"
    # prepare a directory for the user's cache
    And I create directory "/{context.dnf.installroot}/var/cache/dnf-user"
    And I successfully execute "chmod 777 {context.dnf.installroot}/var/cache/dnf-user"


Scenario: Root cache is shared when user metadata are empty
  Given I use repository "dnf-ci-fedora"
    And I successfully execute dnf with args "makecache"
   Then stderr matches line by line
        """
        Updating and loading repositories:
         dnf-ci-fedora test repository .*
        Repositories loaded.
        """
    And stdout is
        """
        Metadata cache created.
        """
   When I execute dnf with args "makecache --setopt=system_cachedir={context.dnf.installroot}/var/cache/dnf --setopt=cachedir={context.dnf.installroot}/var/cache/dnf-user" as an unprivileged user
   Then stderr matches line by line
        """
        Updating and loading repositories:
        Repositories loaded.
        """
    And stdout is
        """
        Metadata cache created.
        """


Scenario: Root cache is not shared when the user doesn't have permissions
  Given I use repository "dnf-ci-fedora"
    And I successfully execute dnf with args "makecache"
   Then stderr matches line by line
        """
        Updating and loading repositories:
         dnf-ci-fedora test repository .*
        Repositories loaded.
        """
    And stdout is
        """
        Metadata cache created.
        """
   When I successfully execute "chmod 700 {context.dnf.installroot}/var/cache/dnf"
    And I execute dnf with args "makecache --setopt=system_cachedir={context.dnf.installroot}/var/cache/dnf --setopt=cachedir={context.dnf.installroot}/var/cache/dnf-user" as an unprivileged user
   Then stderr matches line by line
        """
        Updating and loading repositories:
         dnf-ci-fedora test repository .*
        Repositories loaded.
        """
    And stdout is
        """
        Metadata cache created.
        """


@bz2299337
Scenario: When root cache is shared metalink is copied as well and unprivileged user sees mirrors
  Given I copy repository "simple-base" for modification
    And I use repository "simple-base" as http
    And I set up metalink for repository "simple-base"
    And I successfully execute dnf with args "makecache"
   When I execute dnf with args "download labirinto.x86_64 --url --setopt=system_cachedir={context.dnf.installroot}/var/cache/dnf --setopt=cachedir={context.dnf.installroot}/var/cache/dnf-user" as an unprivileged user
   Then stdout is
        """
        http://localhost:{context.dnf.ports[simple-base]}/x86_64/labirinto-1.0-1.fc29.x86_64.rpm
        """
   Then stderr is
        """
        Updating and loading repositories:
        Repositories loaded.
        """
   When I execute "find | sort" in "{context.dnf.installroot}/var/cache/dnf-user"
   Then stdout matches line by line
        """
        \.
        \./simple-base-[0-9a-f]{16}
        \./simple-base-[0-9a-f]{16}/metalink.xml
        \./simple-base-[0-9a-f]{16}/repodata
        \./simple-base-[0-9a-f]{16}/repodata/primary\.xml\.*
        \./simple-base-[0-9a-f]{16}/repodata/repomd\.xml
        \./simple-base-[0-9a-f]{16}/solv
        \./simple-base-[0-9a-f]{16}/solv/simple-base\.solv
        """


Scenario: When root cache is shared mirrorlist is copied as well and unprivileged user sees mirrors
  Given I use repository "simple-base" as http
    And I create and substitute file "/tmp/mirrorlist-file" with
        """
        http://localhost:{context.dnf.ports[simple-base]}
        """
    And I configure a new repository "simple-base" with
        | key        | value                                         |
        | mirrorlist | {context.dnf.installroot}/tmp/mirrorlist-file |
    And I successfully execute dnf with args "makecache"
   When I execute dnf with args "download labirinto.x86_64 --url --setopt=system_cachedir={context.dnf.installroot}/var/cache/dnf --setopt=cachedir={context.dnf.installroot}/var/cache/dnf-user" as an unprivileged user
   Then stdout is
        """
        http://localhost:{context.dnf.ports[simple-base]}/x86_64/labirinto-1.0-1.fc29.x86_64.rpm
        """
    Then stderr is
        """
        Updating and loading repositories:
        Repositories loaded.
        """
   When I execute "find | sort" in "{context.dnf.installroot}/var/cache/dnf-user"
   Then stdout matches line by line
        """
        \.
        \./simple-base-[0-9a-f]{16}
        \./simple-base-[0-9a-f]{16}/mirrorlist
        \./simple-base-[0-9a-f]{16}/repodata
        \./simple-base-[0-9a-f]{16}/repodata/primary\.xml\.*
        \./simple-base-[0-9a-f]{16}/repodata/repomd\.xml
        \./simple-base-[0-9a-f]{16}/solv
        \./simple-base-[0-9a-f]{16}/solv/simple-base\.solv
        """
