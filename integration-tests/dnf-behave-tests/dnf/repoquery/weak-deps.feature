Feature: Tests for repoquery weak deps related functionality:
 --recommends, --supplements, --suggests, --enhances,
 --whatrecommends, --whatsupplements, --whatsuggests, --whatenhances
 --whatdepends

# all the packages a1, a2, b1, b2, c1, c2, d1, d2 and top follow
# this scheme in various forms (see .specs for details):
#
#  top
#   |
#   | requires
#   |
#   +---------------+---------------+---------------+
#   |               |               |               |
#   |               |               |               |
#   v  recommends   v  supplements  v   suggests    v
#   a ------------> b ------------> c ------------> d
#   ^                                               |
#   |                   enhances                    |
#   +-----------------------------------------------+

Background:
Given I use repository "repoquery-weak-deps"


# --recommends
Scenario: repoquery --recommends NAME
 When I execute dnf with args "repoquery --recommends a2"
 Then the exit code is 0
  And stderr is
      """
      <REPOSYNC>
      """
  And stdout is
      """
      b1
      b1 > 1.0
      b2 = 2.0
      """

Scenario: repoquery --recommends NAME-VERSION
 When I execute dnf with args "repoquery --recommends a2-1.0"
 Then the exit code is 0
  And stderr is
      """
      <REPOSYNC>
      """
  And stdout is
      """
      b1
      """

Scenario: repoquery --recommends NAMEGLOB-VERSION
 When I execute dnf with args "repoquery --recommends a?-2.0"
 Then the exit code is 0
  And stderr is
      """
      <REPOSYNC>
      """
  And stdout is
      """
      b1 > 1.0
      b2 = 2.0
      """

Scenario: repoquery --recommends NAME (nonexisting package)
 When I execute dnf with args "repoquery --recommends dummy"
 Then the exit code is 0
  And stderr is
      """
      <REPOSYNC>
      """
  And stdout is empty


# --supplements
Scenario: repoquery --supplements NAME
 When I execute dnf with args "repoquery --supplements b2"
 Then the exit code is 0
  And stderr is
      """
      <REPOSYNC>
      """
  And stdout is
      """
      c1
      c1 >= 1.0
      c2 = 2.0
      """

Scenario: repoquery --supplements NAME-VERSION
 When I execute dnf with args "repoquery --supplements b2-1.0"
 Then the exit code is 0
  And stderr is
      """
      <REPOSYNC>
      """
  And stdout is
      """
      c1
      """

Scenario: repoquery --supplements NAMEGLOB-VERSIONGLOB
 When I execute dnf with args "repoquery --supplements b?-?.0"
 Then the exit code is 0
  And stderr is
      """
      <REPOSYNC>
      """
  And stdout is
      """
      c1
      c1 >= 1.0
      c2 = 1.0
      c2 = 2.0
      """

Scenario: repoquery --supplements NAME (nonexisting package)
 When I execute dnf with args "repoquery --supplements dummy"
 Then the exit code is 0
  And stderr is
      """
      <REPOSYNC>
      """
  And stdout is empty


# --suggests
Scenario: repoquery --suggests NAME
 When I execute dnf with args "repoquery --suggests c2"
 Then the exit code is 0
  And stderr is
      """
      <REPOSYNC>
      """
  And stdout is
      """
      d1
      d1 < 1.0
      d2 = 2.0
      """

Scenario: repoquery --suggests NAME-VERSION
 When I execute dnf with args "repoquery --suggests c2-1.0"
 Then the exit code is 0
  And stderr is
      """
      <REPOSYNC>
      """
  And stdout is
      """
      d1
      """

Scenario: repoquery --suggests NAMEGLOB-VERSION
 When I execute dnf with args "repoquery --suggests c[2]-1.0"
 Then the exit code is 0
  And stderr is
      """
      <REPOSYNC>
      """
  And stdout is
      """
      d1
      """

Scenario: repoquery --suggests NAME (nonexisting package)
 When I execute dnf with args "repoquery --suggests dummy"
 Then the exit code is 0
  And stderr is
      """
      <REPOSYNC>
      """
  And stdout is empty


# --enhances
Scenario: repoquery --enhances NAME
 When I execute dnf with args "repoquery --enhances d2"
 Then the exit code is 0
  And stderr is
      """
      <REPOSYNC>
      """
  And stdout is
      """
      a1
      a1 <= 1.0
      a2 = 2.0
      """

Scenario: repoquery --enhances NAME-VERSION
 When I execute dnf with args "repoquery --enhances d2-1.0"
 Then the exit code is 0
  And stderr is
      """
      <REPOSYNC>
      """
  And stdout is
      """
      a1
      """

Scenario: repoquery --enhances NAMEGLOB-VERSIONGLOB
 When I execute dnf with args "repoquery --enhances d[2]-[1].0"
 Then the exit code is 0
  And stderr is
      """
      <REPOSYNC>
      """
  And stdout is
      """
      a1
      """

Scenario: repoquery --enhances NAME (nonexisting package)
 When I execute dnf with args "repoquery --enhances dummy"
 Then the exit code is 0
  And stderr is
      """
      <REPOSYNC>
      """
  And stdout is empty


# --whatrecommends
Scenario: repoquery --whatrecommends NAME
 When I execute dnf with args "repoquery --whatrecommends b2"
 Then the exit code is 0
  And stderr is
      """
      <REPOSYNC>
      """
  And stdout is
      """
      a1-1:1.0-1.x86_64
      a2-1:2.0-1.x86_64
      """

Scenario: repoquery --whatrecommends NAMEGLOB
 When I execute dnf with args "repoquery --whatrecommends b?"
 Then the exit code is 0
  And stderr is
      """
      <REPOSYNC>
      """
  And stdout is
      """
      a1-1:1.0-1.x86_64
      a2-1:1.0-1.x86_64
      a2-1:2.0-1.x86_64
      """

Scenario: repoquery --whatrecommends NAMEGLOB > VERSION
 When I execute dnf with args "repoquery --whatrecommends 'b? < 1.0'"
 Then the exit code is 0
  And stderr is
      """
      <REPOSYNC>
      """
  And stdout is
      """
      a1-1:1.0-1.x86_64
      a2-1:1.0-1.x86_64
      """

Scenario: repoquery --whatrecommends NAME (nonexisting package)
 When I execute dnf with args "repoquery --whatrecommends dummy"
 Then the exit code is 0
  And stderr is
      """
      <REPOSYNC>
      """
  And stdout is empty


# --whatsupplements
Scenario: repoquery --whatsupplements NAME
 When I execute dnf with args "repoquery --whatsupplements c2"
 Then the exit code is 0
  And stderr is
      """
      <REPOSYNC>
      """
  And stdout is
      """
      b1-1:1.0-1.x86_64
      b2-1:2.0-1.x86_64
      """

Scenario: repoquery --whatsupplements NAMEGLOB
 When I execute dnf with args "repoquery --whatsupplements c[2]"
 Then the exit code is 0
  And stderr is
      """
      <REPOSYNC>
      """
  And stdout is
      """
      b1-1:1.0-1.x86_64
      b2-1:2.0-1.x86_64
      """

Scenario: repoquery --whatsupplements NAMEGLOB >= VERSION
 When I execute dnf with args "repoquery --whatsupplements 'c[2] >= 1.5'"
 Then the exit code is 0
  And stderr is
      """
      <REPOSYNC>
      """
  And stdout is
      """
      b2-1:2.0-1.x86_64
      """

Scenario: repoquery --whatsupplements NAME (nonexisting package)
 When I execute dnf with args "repoquery --whatsupplements dummy"
 Then the exit code is 0
  And stderr is
      """
      <REPOSYNC>
      """
  And stdout is empty


# --whatsuggests
Scenario: repoquery --whatsuggests NAME
 When I execute dnf with args "repoquery --whatsuggests d2"
 Then the exit code is 0
  And stderr is
      """
      <REPOSYNC>
      """
  And stdout is
      """
      c1-1:1.0-1.x86_64
      c2-1:2.0-1.x86_64
      """

Scenario: repoquery --whatsuggests NAMEGLOB
 When I execute dnf with args "repoquery --whatsuggests d[12]"
 Then the exit code is 0
  And stderr is
      """
      <REPOSYNC>
      """
  And stdout is
      """
      c1-1:1.0-1.x86_64
      c2-1:1.0-1.x86_64
      c2-1:2.0-1.x86_64
      """

Scenario: repoquery --whatsuggests NAME >= VERSION
 When I execute dnf with args "repoquery --whatsuggests 'd1 >= 3.14'"
 Then the exit code is 0
  And stderr is
      """
      <REPOSYNC>
      """
  And stdout is
      """
      c1-1:1.0-1.x86_64
      c2-1:1.0-1.x86_64
      """

Scenario: repoquery --whatsuggests NAME (nonexisting package)
 When I execute dnf with args "repoquery --whatsuggests dummy"
 Then the exit code is 0
  And stderr is
      """
      <REPOSYNC>
      """
  And stdout is empty


# --whatenhances
Scenario: repoquery --whatenhances NAME
 When I execute dnf with args "repoquery --whatenhances a2"
 Then the exit code is 0
  And stderr is
      """
      <REPOSYNC>
      """
  And stdout is
      """
      d1-1:1.0-1.x86_64
      d2-1:2.0-1.x86_64
      """

Scenario: repoquery --whatenhances NAMEGLOB
 When I execute dnf with args "repoquery --whatenhances a[2]"
 Then the exit code is 0
  And stderr is
      """
      <REPOSYNC>
      """
  And stdout is
      """
      d1-1:1.0-1.x86_64
      d2-1:2.0-1.x86_64
      """

Scenario: repoquery --whatenhances NAMEGLOB >= VERSION
 When I execute dnf with args "repoquery --whatenhances 'a? >= 3.0'"
 Then the exit code is 0
  And stderr is
      """
      <REPOSYNC>
      """
  And stdout is
      """
      d1-1:1.0-1.x86_64
      d2-1:1.0-1.x86_64
      """

Scenario: repoquery --whatenhances NAME (nonexisting package)
 When I execute dnf with args "repoquery --whatenhances dummy"
 Then the exit code is 0
  And stderr is
      """
      <REPOSYNC>
      """
  And stdout is empty


# --whatdepends
Scenario: repoquery --whatdepends NAME (requires + enhances)
 When I execute dnf with args "repoquery --whatdepends a1"
 Then the exit code is 0
  And stderr is
      """
      <REPOSYNC>
      """
  And stdout is
      """
      d1-1:1.0-1.x86_64
      d2-1:1.0-1.x86_64
      d2-1:2.0-1.x86_64
      top-1:1.0-1.x86_64
      """

Scenario: repoquery --whatdepends NAME (requires + recommends)
 When I execute dnf with args "repoquery --whatdepends b1"
 Then the exit code is 0
  And stderr is
      """
      <REPOSYNC>
      """
  And stdout is
      """
      a1-1:1.0-1.x86_64
      a2-1:1.0-1.x86_64
      a2-1:2.0-1.x86_64
      top-1:1.0-1.x86_64
      """

Scenario: repoquery --whatdepends NAME (requires + supplements)
 When I execute dnf with args "repoquery --whatdepends c1"
 Then the exit code is 0
  And stderr is
      """
      <REPOSYNC>
      """
  And stdout is
      """
      b1-1:1.0-1.x86_64
      b2-1:1.0-1.x86_64
      b2-1:2.0-1.x86_64
      top-1:1.0-1.x86_64
      """

Scenario: repoquery --whatdepends NAME (requires + suggests)
 When I execute dnf with args "repoquery --whatdepends d1"
 Then the exit code is 0
  And stderr is
      """
      <REPOSYNC>
      """
  And stdout is
      """
      c1-1:1.0-1.x86_64
      c2-1:1.0-1.x86_64
      c2-1:2.0-1.x86_64
      top-1:1.0-1.x86_64
      """

Scenario: repoquery --whatdepends NAME > VERSION
 When I execute dnf with args "repoquery --whatdepends 'd2 > 1.4'"
 Then the exit code is 0
  And stderr is
      """
      <REPOSYNC>
      """
  And stdout is
      """
      c2-1:2.0-1.x86_64
      """

Scenario: repoquery --whatdepends NAMEGLOB < VERSION
 When I execute dnf with args "repoquery --whatdepends '[abc]2 < 1.5'"
 Then the exit code is 0
  And stderr is
      """
      <REPOSYNC>
      """
  And stdout is
      """
      a1-1:1.0-1.x86_64
      b1-1:1.0-1.x86_64
      d1-1:1.0-1.x86_64
      """

Scenario: repoquery --whatdepends NAMEGLOB >= VERSION (ranged matching)
When I execute dnf with args "repoquery --whatdepends '[abc]1 <= 1.0'"
 Then the exit code is 0
  And stderr is
      """
      <REPOSYNC>
      """
  And stdout is
      """
      a1-1:1.0-1.x86_64
      a2-1:1.0-1.x86_64
      b1-1:1.0-1.x86_64
      b2-1:1.0-1.x86_64
      b2-1:2.0-1.x86_64
      d1-1:1.0-1.x86_64
      d2-1:1.0-1.x86_64
      d2-1:2.0-1.x86_64
      top-1:1.0-1.x86_64
      """

Scenario: repoquery --exactdeps --whatdepends NAME
 When I execute dnf with args "repoquery --exactdeps --whatdepends c1"
 Then the exit code is 0
  And stderr is
      """
      <REPOSYNC>
      """
  And stdout is
      """
      b1-1:1.0-1.x86_64
      b2-1:1.0-1.x86_64
      b2-1:2.0-1.x86_64
      """

Scenario: repoquery --exactdeps --whatdepends NAME (no nonexact deps)
 When I execute dnf with args "repoquery --exactdeps --whatdepends d1"
 Then the exit code is 0
  And stderr is
      """
      <REPOSYNC>
      """
  And stdout is
      """
      c1-1:1.0-1.x86_64
      c2-1:1.0-1.x86_64
      c2-1:2.0-1.x86_64
      top-1:1.0-1.x86_64
      """

Scenario: repoquery --whatdepends NAME (nonexisting package)
 When I execute dnf with args "repoquery --whatdepends dummy"
 Then the exit code is 0
  And stderr is
      """
      <REPOSYNC>
      """
  And stdout is empty
