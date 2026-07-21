Feature: Reporting broken dependencies with various strict, best options


@bz2088422
Scenario: Broken dependencies block the transaction when strict mode is on
  Given I use repository "dnf-ci-fedora"
   When I execute dnf with args "install postgresql --exclude=postgresql-libs --setopt=strict=true"
   Then the exit code is 1


@bz2088422
Scenario: Broken dependencies are reported when strict and best options are off
  Given I use repository "dnf-ci-fedora"
   When I execute dnf with args "install postgresql --exclude=postgresql-libs --setopt=strict=false --setopt=best=false"
   Then the exit code is 0
    And stderr is
    """
    <REPOSYNC>
    Problem: package postgresql-9.6.5-1.fc29.x86_64 from dnf-ci-fedora requires libpq.so.5()(64bit), but none of the providers can be installed
      - package postgresql-9.6.5-1.fc29.x86_64 from dnf-ci-fedora requires postgresql-libs(x86-64) = 9.6.5-1.fc29, but none of the providers can be installed
      - conflicting requests
      - package postgresql-libs-9.6.5-1.fc29.x86_64 from dnf-ci-fedora is filtered out by exclude filtering
    """
    And Transaction is following
        | Action                | Package                           |
        | broken                | postgresql-9.6.5-1.fc29.x86_64    |


@bz2088422
Scenario: Broken dependencies are reported when strict option is off and best option is on
  Given I use repository "dnf-ci-fedora"
   When I execute dnf with args "install postgresql --exclude=postgresql-libs --setopt=strict=false --setopt=best=true"
   Then the exit code is 0
    And stderr is
    """
    <REPOSYNC>
    Problem: package postgresql-9.6.5-1.fc29.x86_64 from dnf-ci-fedora requires libpq.so.5()(64bit), but none of the providers can be installed
      - package postgresql-9.6.5-1.fc29.x86_64 from dnf-ci-fedora requires postgresql-libs(x86-64) = 9.6.5-1.fc29, but none of the providers can be installed
      - conflicting requests
      - package postgresql-libs-9.6.5-1.fc29.x86_64 from dnf-ci-fedora is filtered out by exclude filtering
    """
    And Transaction is following
        | Action                | Package                           |
        | broken                | postgresql-9.6.5-1.fc29.x86_64    |


@bz2088422
Scenario: Broken dependencies are reported when skip-broken and best options are on
  Given I use repository "dnf-ci-fedora"
   When I execute dnf with args "install postgresql --exclude=postgresql-libs --skip-broken --setopt=best=true"
   Then the exit code is 0
    And stderr is
    """
    <REPOSYNC>
    Problem: package postgresql-9.6.5-1.fc29.x86_64 from dnf-ci-fedora requires libpq.so.5()(64bit), but none of the providers can be installed
      - package postgresql-9.6.5-1.fc29.x86_64 from dnf-ci-fedora requires postgresql-libs(x86-64) = 9.6.5-1.fc29, but none of the providers can be installed
      - conflicting requests
      - package postgresql-libs-9.6.5-1.fc29.x86_64 from dnf-ci-fedora is filtered out by exclude filtering
    """
    And Transaction is following
        | Action                | Package                           |
        | broken                | postgresql-9.6.5-1.fc29.x86_64    |
