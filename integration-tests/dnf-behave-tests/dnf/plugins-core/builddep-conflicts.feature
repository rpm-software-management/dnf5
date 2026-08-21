@not.with_dnf=4
Feature: Tests for BuildConflicts support in builddep command


# `with-build-conflict` package BuildConflicts with `weak-dependency` and
# BuildRequires `build-requirement-a` which in turn Recommends `weak-dependency`
# `weak-dependency` package should not get installed
Scenario Outline: Builddep command does not install conflicting weak dependency using <type>
  Given I use repository "dnf-ci-builddep"
   When I execute dnf with args "builddep <specification>"
   Then the exit code is 0
    And Transaction is following
        | Action                | Package                               |
        | install               | build-requirement-a-0:1.0-1.x86_64    |

Examples:
    | type      | specification     |
    | spec      | {context.dnf.fixturesdir}/specs/dnf-ci-builddep/with-build-conflict-1.0-1.spec |
    | srpm      | {context.dnf.fixturesdir}/repos/dnf-ci-builddep/src/with-build-conflict-1.0-1.src.rpm |
    | package   | with-build-conflict |



Scenario Outline: Builddep command remove conflicting package using <type>
  Given I use repository "dnf-ci-builddep"
    And I successfully execute dnf with args "install weak-dependency"
   When I execute dnf with args "builddep <specification>"
   Then the exit code is 0
    And Transaction is following
        | Action                | Package                               |
        | install               | build-requirement-a-0:1.0-1.x86_64    |
        | remove                | weak-dependency-0:1.0-1.x86_64        |

Examples:
    | type      | specification     |
    | spec      | {context.dnf.fixturesdir}/specs/dnf-ci-builddep/with-build-conflict-1.0-1.spec |
    | srpm      | {context.dnf.fixturesdir}/repos/dnf-ci-builddep/src/with-build-conflict-1.0-1.src.rpm |
    | package   | with-build-conflict |
