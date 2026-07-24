Feature: Tests for the basic repoquery dependencies functionality:
    --requires, --provides, --conflicts, --obsoletes, --requires-pre
    --whatrequires, --whatprovides, --whatconflicts, --whatobsoletes

Background:
 Given I use repository "repoquery-deps"


# --requires
Scenario: repoquery --requires NAME
 When I execute dnf with args "repoquery --requires middle1"
 Then the exit code is 0
  And stderr is
      """
      <REPOSYNC>
      """
  And stdout is
      """
      /a/bottom4-file
      bottom1-prov1
      bottom1-prov2 = 1.0
      bottom1-prov2 >= 2.0
      bottom2 = 1:1.0-1
      bottom3
      """

Scenario: repoquery --requires NAME-VERSION
 When I execute dnf with args "repoquery --requires middle1-1.0"
 Then the exit code is 0
  And stderr is
      """
      <REPOSYNC>
      """
  And stdout is
      """
      bottom1-prov1
      bottom1-prov2 = 1.0
      bottom2 = 1:1.0-1
      bottom3
      """

Scenario: repoquery --providers-of=requires NAME
 When I execute dnf with args "repoquery --setopt=optional_metadata_types=filelists --providers-of=requires middle1"
 Then the exit code is 0
  And stderr is
      """
      <REPOSYNC>
      """
  And stdout is
      """
      bottom1-1:2.0-1.x86_64
      bottom2-1:1.0-1.x86_64
      bottom3-1:2.0-1.x86_64
      bottom4-1:1.0-1.x86_64
      """

Scenario: repoquery --providers-of=requires NAMEGLOB
 When I execute dnf with args "repoquery --setopt=optional_metadata_types=filelists --providers-of=requires middle[1]"
 Then the exit code is 0
  And stderr is
      """
      <REPOSYNC>
      """
  And stdout is
      """
      bottom1-1:2.0-1.x86_64
      bottom2-1:1.0-1.x86_64
      bottom3-1:2.0-1.x86_64
      bottom4-1:1.0-1.x86_64
      """

Scenario: repoquery --providers-of=requires --recursive NAME
 When I execute dnf with args "repoquery --setopt=optional_metadata_types=filelists --providers-of=requires --recursive top1"
 Then the exit code is 0
  And stderr is
      """
      <REPOSYNC>
      """
  And stdout is
      """
      bottom1-1:2.0-1.x86_64
      bottom2-1:1.0-1.x86_64
      bottom3-1:2.0-1.x86_64
      bottom4-1:1.0-1.x86_64
      middle1-1:2.0-1.x86_64
      middle2-1:2.0-1.x86_64
      """

Scenario: repoquery --providers-of=requires --recursive NAME-VERSION
 When I execute dnf with args "repoquery --setopt=optional_metadata_types=filelists --providers-of=requires --recursive top1-2.0"
 Then the exit code is 0
  And stderr is
      """
      <REPOSYNC>
      """
  And stdout is
      """
      bottom1-1:2.0-1.x86_64
      bottom2-1:1.0-1.x86_64
      bottom4-1:1.0-1.x86_64
      middle1-1:2.0-1.x86_64
      middle2-1:2.0-1.x86_64
      """

Scenario: repoquery --providers-of=requires --installed NAME when nothing is installed
 When I execute dnf with args "repoquery --providers-of=requires --installed middle3"
 Then the exit code is 0
  And stdout is empty

Scenario: repoquery --providers-of=requires --installed NAME
Given I successfully execute dnf with args "install middle3"
 When I execute dnf with args "repoquery --providers-of=requires middle3 --installed "
 Then the exit code is 0
  And stdout is
      """
      bottom1-1:1.0-1.x86_64
      bottom3-1:2.0-1.x86_64
      bottom4-1:1.0-1.x86_64
      bottom5-1:1.0-1.x86_64
      """

# missing --tree option: https://github.com/rpm-software-management/dnf5/issues/913
@xfail
Scenario: repoquery --requires --resolve --recursive --tree NAME-VERSION
 When I execute dnf with args "repoquery --requires --resolve --recursive --tree top1-2.0"
 Then the exit code is 0
  And stderr is
      """
      <REPOSYNC>
      """
  And stdout is
      """
      top1-1:2.0-1.src
      top1-1:2.0-1.x86_64
       \_ middle1-1:2.0-1.x86_64 [3: /a/bottom4-file, bottom2 = 1:1.0-1, bottom1-prov2 >= 2.0]
       |   \_ bottom1-1:2.0-1.x86_64 [0: ]
       |   \_ bottom2-1:1.0-1.x86_64 [0: ]
       |   \_ bottom4-1:1.0-1.x86_64 [0: ]
       \_ middle2-1:2.0-1.x86_64 [1: bottom1-prov3 <= 1.2]
       |   \_ bottom1-1:2.0-1.x86_64 [0: ]
      """

Scenario: repoquery --requires NAME-VERSION NAME
 When I execute dnf with args "repoquery --requires middle1-1.0 middle2"
 Then the exit code is 0
  And stderr is
      """
      <REPOSYNC>
      """
  And stdout is
      """
      bottom1-prov1
      bottom1-prov2 = 1.0
      bottom1-prov3 <= 1.2
      bottom1-prov3 > 1.5
      bottom2 = 1:1.0-1
      bottom3
      bottom3-prov1
      """

Scenario: repoquery --requires NAMEGLOB-VERSION NAME
 When I execute dnf with args "repoquery --requires middle[1]-1.0 middle2"
 Then the exit code is 0
  And stderr is
      """
      <REPOSYNC>
      """
  And stdout is
      """
      bottom1-prov1
      bottom1-prov2 = 1.0
      bottom1-prov3 <= 1.2
      bottom1-prov3 > 1.5
      bottom2 = 1:1.0-1
      bottom3
      bottom3-prov1
      """

Scenario: repoquery --requires with requires(pre/post/preun/postun)
 When I execute dnf with args "repoquery --requires middle3-1.0-1"
 Then the exit code is 0
  And stderr is
      """
      <REPOSYNC>
      """
  And stdout is
      """
      bottom1-prov1
      bottom3-prov1
      bottom4-prov1
      bottom5-prov1
      """

# Using --qf .. is a diffrent code path in repoquery
Scenario: repoquery --requires with requires(pre/post/preun/postun)
 When I execute dnf with args "repoquery --qf '%{{requires}}' middle3-1.0-1"
 Then the exit code is 0
  And stderr is
      """
      <REPOSYNC>
      """
  And stdout is
      """
      bottom5-prov1
      bottom4-prov1
      bottom3-prov1
      bottom1-prov1
      """


# --provides
Scenario: repoquery --provides NAME
 When I execute dnf with args "repoquery --provides bottom1"
 Then the exit code is 0
  And stderr is
      """
      <REPOSYNC>
      """
  And stdout is
      """
      a-common-provide
      bottom1 = 1:1.0-1
      bottom1 = 1:2.0-1
      bottom1(x86-64) = 1:1.0-1
      bottom1(x86-64) = 1:2.0-1
      bottom1-prov1
      bottom1-prov2 = 1.0
      bottom1-prov2 = 2.0
      bottom1-prov3 <= 1.3
      """

Scenario: repoquery --provides NAME-VERSION
 When I execute dnf with args "repoquery --provides bottom1-1.0"
 Then the exit code is 0
  And stderr is
      """
      <REPOSYNC>
      """
  And stdout is
      """
      a-common-provide
      bottom1 = 1:1.0-1
      bottom1(x86-64) = 1:1.0-1
      bottom1-prov1
      bottom1-prov2 = 1.0
      """

Scenario: repoquery --provides NAMEGLOB-VERSIONGLOB
 When I execute dnf with args "repoquery --provides bottom[2]-*"
 Then the exit code is 0
  And stderr is
      """
      <REPOSYNC>
      """
  And stdout is
      """
      a-common-provide
      bottom2 = 1:1.0-1
      bottom2 = 1:2.0-1
      bottom2(x86-64) = 1:1.0-1
      bottom2(x86-64) = 1:2.0-1
      bottom2-prov1
      """


# --conflicts
Scenario: repoquery --conflicts NAME
 When I execute dnf with args "repoquery --conflicts bottom2"
 Then the exit code is 0
  And stderr is
      """
      <REPOSYNC>
      """
  And stdout is
      """
      bottom1
      bottom3 < 2.0
      bottom3 = 1:1.0-1
      """

Scenario: repoquery --conflicts NAME-VERSION
 When I execute dnf with args "repoquery --conflicts bottom2-1.0"
 Then the exit code is 0
  And stderr is
      """
      <REPOSYNC>
      """
  And stdout is
      """
      bottom1
      bottom3 = 1:1.0-1
      """

Scenario: repoquery --conflicts NAMEGLOB-VERSIONGLOB
 When I execute dnf with args "repoquery --conflicts 'bottom*-?.0'"
 Then the exit code is 0
  And stderr is
      """
      <REPOSYNC>
      """
  And stdout is
      """
      bottom1
      bottom3 < 2.0
      bottom3 = 1:1.0-1
      """


# --obsoletes
Scenario: repoquery --obsoletes NAME
 When I execute dnf with args "repoquery --obsoletes bottom3"
 Then the exit code is 0
  And stderr is
      """
      <REPOSYNC>
      """
  And stdout is
      """
      bottom1 < 7
      bottom2
      bottom2 = 1:2.0-1
      """

Scenario: repoquery --obsoletes NAME-VERSION
 When I execute dnf with args "repoquery --obsoletes bottom3-1.0"
 Then the exit code is 0
  And stderr is
      """
      <REPOSYNC>
      """
  And stdout is
      """
      bottom1 < 7
      bottom2
      """

Scenario: repoquery --obsoletes NAMEGLOB-VERSION
 When I execute dnf with args "repoquery --obsoletes bottom[3]-1.0"
 Then the exit code is 0
  And stderr is
      """
      <REPOSYNC>
      """
  And stdout is
      """
      bottom1 < 7
      bottom2
      """

@bz1782906
# --whatrequires
Scenario: repoquery --whatrequires NAME
 When I execute dnf with args "repoquery --whatrequires bottom1"
 Then the exit code is 0
  And stderr is
      """
      <REPOSYNC>
      """
  And stdout is
      """
      middle1-1:1.0-1.x86_64
      middle1-1:2.0-1.x86_64
      middle2-1:2.0-1.x86_64
      middle3-1:1.0-1.x86_64
      """

@bz1782906
Scenario: repoquery --whatrequires NAME-VERSION
 When I execute dnf with args "repoquery --whatrequires bottom1-2.0"
 Then the exit code is 0
  And stderr is
      """
      <REPOSYNC>
      """
  And stdout is
      """
      middle1-1:2.0-1.x86_64
      middle2-1:2.0-1.x86_64
      """

@bz1782906
Scenario: repoquery --whatrequires NEVRA
 When I execute dnf with args "repoquery --whatrequires bottom2-1:1.0-1.x86_64"
 Then the exit code is 0
  And stderr is
      """
      <REPOSYNC>
      """
  And stdout is
      """
      middle1-1:1.0-1.x86_64
      middle1-1:2.0-1.x86_64
      """

@bz1782906
Scenario: repoquery --whatrequires NAME (file provide)
        When I execute dnf with args "repoquery --whatrequires bottom4 --setopt=optional_metadata_types=filelists"
 Then the exit code is 0
  And stderr is
      """
      <REPOSYNC>
      """
  And stdout is
      """
      middle1-1:2.0-1.x86_64
      middle3-1:1.0-1.x86_64
      """

Scenario: repoquery --whatrequires PROVIDE_NAME
 When I execute dnf with args "repoquery --whatrequires bottom1-prov2"
 Then the exit code is 0
  And stderr is
      """
      <REPOSYNC>
      """
  And stdout is
      """
      middle1-1:1.0-1.x86_64
      middle1-1:2.0-1.x86_64
      """

Scenario: repoquery --whatrequires PROVIDE_NAME = VERSION
 When I execute dnf with args "repoquery --whatrequires 'bottom1-prov2 = 2.0'"
 Then the exit code is 0
  And stderr is
      """
      <REPOSYNC>
      """
  And stdout is
      """
      middle1-1:2.0-1.x86_64
      """

Scenario: repoquery --whatrequires --recursive PROVIDE_NAME
 When I execute dnf with args "repoquery --recursive --whatrequires bottom1-prov2"
 Then the exit code is 0
  And stderr is
      """
      <REPOSYNC>
      """
  And stdout is
      """
      middle1-1:1.0-1.x86_64
      middle1-1:2.0-1.x86_64
      top1-1:1.0-1.x86_64
      top1-1:2.0-1.x86_64
      """

Scenario: repoquery --exactdeps --whatrequires PROVIDE_NAME
 When I execute dnf with args "repoquery --exactdeps --whatrequires bottom1-prov3"
 Then the exit code is 0
  And stderr is
      """
      <REPOSYNC>
      """
  And stdout is
      """
      middle2-1:1.0-1.x86_64
      middle2-1:2.0-1.x86_64
      """

Scenario: repoquery --exactdeps --whatrequires PROVIDE_NAMEGLOB
 When I execute dnf with args "repoquery --exactdeps --whatrequires bottom1-prov[23]"
 Then the exit code is 0
  And stderr is
      """
      <REPOSYNC>
      """
  And stdout is
      """
      middle1-1:1.0-1.x86_64
      middle1-1:2.0-1.x86_64
      middle2-1:1.0-1.x86_64
      middle2-1:2.0-1.x86_64
      """

Scenario: repoquery --exactdeps --whatrequires PROVIDE_NAME >= VERSION (matches)
 When I execute dnf with args "repoquery --exactdeps --whatrequires 'bottom1-prov3 >= 11'"
 Then the exit code is 0
  And stderr is
      """
      <REPOSYNC>
      """
  And stdout is
      """
      middle2-1:1.0-1.x86_64
      """

Scenario: repoquery --exactdeps --whatrequires PROVIDE_NAME < VERSION (doesn't match)
 When I execute dnf with args "repoquery --exactdeps --whatrequires 'bottom1-prov3 < 1'"
 Then the exit code is 0
  And stderr is
      """
      <REPOSYNC>
      """
  And stdout is
      """
      middle2-1:2.0-1.x86_64
      """

@bz1667898
Scenario: repoquery --whatrequires NAME --whatrequires NAME
  When I execute dnf with args "-q repoquery --whatrequires 'bottom1-prov3 > 30' --whatrequires bottom2"
 Then the exit code is 0
  And stderr is
      """
      <REPOSYNC>
      """
  And stdout is
      """
      middle1-1:1.0-1.x86_64
      middle1-1:2.0-1.x86_64
      middle2-1:1.0-1.x86_64
      """

Scenario: repoquery --whatrequires NAME (buildrequires, srpm)
 When I execute dnf with args "repoquery --whatrequires bottom5-prov1"
 Then the exit code is 0
  And stderr is
      """
      <REPOSYNC>
      """
  And stdout is
      """
      middle3-1:1.0-1.x86_64
      middle4-1:1.0-1.src
      """

@bz1812596
Scenario: repoquery --whatrequires NEVRA (buildrequires, srpm)
 When I execute dnf with args "repoquery --whatrequires bottom5"
 Then the exit code is 0
  And stderr is
      """
      <REPOSYNC>
      """
  And stdout is
      """
      middle3-1:1.0-1.x86_64
      middle4-1:1.0-1.src
      middle5-1:1.0-1.x86_64
      """


# --whatprovides
Scenario: repoquery --whatprovides NAME
 When I execute dnf with args "repoquery --whatprovides bottom1-prov1"
 Then the exit code is 0
  And stderr is
      """
      <REPOSYNC>
      """
  And stdout is
      """
      bottom1-1:1.0-1.x86_64
      """

Scenario: repoquery --whatprovides NAME (versioned provide in multiple packages)
 When I execute dnf with args "repoquery --whatprovides bottom1-prov2"
 Then the exit code is 0
  And stderr is
      """
      <REPOSYNC>
      """
  And stdout is
      """
      bottom1-1:1.0-1.x86_64
      bottom1-1:2.0-1.x86_64
      """

Scenario: repoquery --whatprovides NAME (a provide present in different packages)
 When I execute dnf with args "repoquery --whatprovides a-common-provide"
 Then the exit code is 0
  And stderr is
      """
      <REPOSYNC>
      """
  And stdout is
      """
      bottom1-1:1.0-1.x86_64
      bottom1-1:2.0-1.x86_64
      bottom2-1:1.0-1.x86_64
      """

Scenario: repoquery --whatprovides NAME = VERSION
 When I execute dnf with args "repoquery --whatprovides 'bottom1-prov2 = 2.0'"
 Then the exit code is 0
  And stderr is
      """
      <REPOSYNC>
      """
  And stdout is
      """
      bottom1-1:2.0-1.x86_64
      """

Scenario: repoquery --whatprovides NAME = VERSION (non-versioned provide)
 When I execute dnf with args "repoquery --whatprovides 'bottom2-prov1 = 1.3'"
 Then the exit code is 0
  And stderr is
      """
      <REPOSYNC>
      """
  And stdout is
      """
      bottom2-1:1.0-1.x86_64
      bottom2-1:2.0-1.x86_64
      """

Scenario: repoquery --whatprovides NAME <= VERSION (matches)
 When I execute dnf with args "repoquery --whatprovides 'bottom1-prov3 <= 1.2'"
 Then the exit code is 0
  And stderr is
      """
      <REPOSYNC>
      """
  And stdout is
      """
      bottom1-1:2.0-1.x86_64
      """

Scenario: repoquery --whatprovides NAME > VERSION (doesn't match)
 When I execute dnf with args "repoquery --whatprovides 'bottom1-prov3 > 1.3'"
 Then the exit code is 0
  And stderr is
      """
      <REPOSYNC>
      """
  And stdout is empty

Scenario: repoquery --whatprovides NAMEGLOB <= VERSION
 When I execute dnf with args "repoquery --whatprovides 'bottom[1-3]-prov? >= 1.2'"
 Then the exit code is 0
  And stderr is
      """
      <REPOSYNC>
      """
  And stdout is
      """
      bottom1-1:1.0-1.x86_64
      bottom1-1:2.0-1.x86_64
      bottom2-1:1.0-1.x86_64
      bottom2-1:2.0-1.x86_64
      bottom3-1:1.0-1.x86_64
      """

@bz1667898
Scenario: repoquery --whatprovides NAME = VERSION --whatprovides NAME
 When I execute dnf with args "repoquery --whatprovides 'bottom1-prov2 = 2.0' --whatprovides bottom3-prov1"
 Then the exit code is 0
  And stderr is
      """
      <REPOSYNC>
      """
  And stdout is
      """
      bottom1-1:2.0-1.x86_64
      bottom3-1:1.0-1.x86_64
      bottom3-1:2.0-1.x86_64
      """

@bz1667898
Scenario: repoquery --whatprovides NAME = version, NAME
 When I execute dnf with args "repoquery --whatprovides 'bottom1-prov2 = 2.0, bottom3-prov1'"
 Then the exit code is 0
  And stderr is
      """
      <REPOSYNC>
      """
  And stdout is
      """
      bottom1-1:2.0-1.x86_64
      bottom3-1:1.0-1.x86_64
      bottom3-1:2.0-1.x86_64
      """


# --whatconflicts
Scenario: repoquery --whatconflicts NAME
 When I execute dnf with args "repoquery --whatconflicts bottom1"
 Then the exit code is 0
  And stderr is
      """
      <REPOSYNC>
      """
  And stdout is
      """
      bottom2-1:1.0-1.x86_64
      bottom2-1:2.0-1.x86_64
      """

Scenario: repoquery --whatconflicts NAME (versioned conflict)
 When I execute dnf with args "repoquery --whatconflicts bottom3"
 Then the exit code is 0
  And stderr is
      """
      <REPOSYNC>
      """
  And stdout is
      """
      bottom2-1:1.0-1.x86_64
      bottom2-1:2.0-1.x86_64
      """

Scenario: repoquery --whatconflicts NAME = VERSION
 When I execute dnf with args "repoquery --whatconflicts 'bottom3 = 1:1.0-1'"
 Then the exit code is 0
  And stderr is
      """
      <REPOSYNC>
      """
  And stdout is
      """
      bottom2-1:1.0-1.x86_64
      """

Scenario: repoquery --whatconflicts NAME < VERSION (one match)
 When I execute dnf with args "repoquery --whatconflicts 'bottom3 < 0.5'"
 Then the exit code is 0
  And stderr is
      """
      <REPOSYNC>
      """
  And stdout is
      """
      bottom2-1:2.0-1.x86_64
      """

Scenario: repoquery --whatconflicts NAME < VERSION (two matches)
 When I execute dnf with args "repoquery --whatconflicts 'bottom3 < 1:1.5'"
 Then the exit code is 0
  And stderr is
      """
      <REPOSYNC>
      """
  And stdout is
      """
      bottom2-1:1.0-1.x86_64
      bottom2-1:2.0-1.x86_64
      """

Scenario: repoquery --whatconflicts NAMEGLOB < VERSIONGLOB (two matches)
 When I execute dnf with args "repoquery --whatconflicts 'bottom[3] < 1:1.5'"
 Then the exit code is 0
  And stderr is
      """
      <REPOSYNC>
      """
  And stdout is
      """
      bottom2-1:1.0-1.x86_64
      bottom2-1:2.0-1.x86_64
      """

@bz1667898
Scenario: repoquery --whatconflicts NAME --whatconflicts NAME = VERSION
 When I execute dnf with args "repoquery --whatconflicts middle2 --whatconflicts 'bottom3 = 1:1.0-1'"
 Then the exit code is 0
  And stderr is
      """
      <REPOSYNC>
      """
  And stdout is
      """
      bottom2-1:1.0-1.x86_64
      middle1-1:2.0-1.x86_64
      """


# --whatobsoletes
Scenario: repoquery --whatobsoletes NAME
 When I execute dnf with args "repoquery --whatobsoletes bottom2"
 Then the exit code is 0
  And stderr is
      """
      <REPOSYNC>
      """
  And stdout is
      """
      bottom3-1:1.0-1.x86_64
      bottom3-1:2.0-1.x86_64
      """

Scenario: repoquery --whatobsoletes NAME = VERSION (version obsolete exists)
 When I execute dnf with args "repoquery --whatobsoletes 'bottom2 = 1:2.0-1'"
 Then the exit code is 0
  And stderr is
      """
      <REPOSYNC>
      """
  And stdout is
      """
      bottom3-1:1.0-1.x86_64
      bottom3-1:2.0-1.x86_64
      """

Scenario: repoquery --whatobsoletes NAME = VERSION (version obsolete doesn't exist)
 When I execute dnf with args "repoquery --whatobsoletes 'bottom2 = 2.0'"
 Then the exit code is 0
  And stderr is
      """
      <REPOSYNC>
      """
  And stdout is
      """
      bottom3-1:1.0-1.x86_64
      """

Scenario: repoquery --whatobsoletes NAME = VERSION (matches a range)
 When I execute dnf with args "repoquery --whatobsoletes 'bottom1 = 4'"
 Then the exit code is 0
  And stderr is
      """
      <REPOSYNC>
      """
  And stdout is
      """
      bottom3-1:1.0-1.x86_64
      """

Scenario: repoquery --whatobsoletes NAME = VERSION (doesn't match a range)
 When I execute dnf with args "repoquery --whatobsoletes 'bottom1 = 8'"
 Then the exit code is 0
  And stderr is
      """
      <REPOSYNC>
      """
  And stdout is empty

Scenario: repoquery --whatobsoletes NAME <= VERSION (matches)
 When I execute dnf with args "repoquery --whatobsoletes 'bottom1 <= 4'"
 Then the exit code is 0
  And stderr is
      """
      <REPOSYNC>
      """
  And stdout is
      """
      bottom3-1:1.0-1.x86_64
      """

Scenario: repoquery --whatobsoletes NAMEGLOB <= VERSION (matches)
 When I execute dnf with args "repoquery --whatobsoletes 'bottom[1] <= 4'"
 Then the exit code is 0
  And stderr is
      """
      <REPOSYNC>
      """
  And stdout is
      """
      bottom3-1:1.0-1.x86_64
      """

@bz1667898
Scenario: repoquery --whatobsoletes NAME, NAME = VERSION
 When I execute dnf with args "repoquery --whatobsoletes 'middle1, bottom2 = 2.0'"
 Then the exit code is 0
  And stderr is
      """
      <REPOSYNC>
      """
  And stdout is
      """
      bottom3-1:1.0-1.x86_64
      middle2-1:1.0-1.x86_64
      middle2-1:2.0-1.x86_64
      """


# --requires-pre
Scenario: repoquery --requires-pre
 When I execute dnf with args "repoquery --requires-pre middle3"
 Then the exit code is 0
  And stderr is
      """
      <REPOSYNC>
      """
  And stdout is
      """
      bottom1-prov1
      bottom3-prov1
      """

Scenario: repoquery --requires-pre with installed pkg show all %pre, %post, %preun, %postun deps
Given I successfully execute dnf with args "install middle3"
  And I drop repository "repoquery-deps"
 When I execute dnf with args "repoquery --installed --requires-pre middle3"
 Then the exit code is 0
  And stderr is
      """
      <REPOSYNC>
      """
  And stdout is
      """
      bottom1-prov1
      bottom3-prov1
      bottom4-prov1
      bottom5-prov1
      """
