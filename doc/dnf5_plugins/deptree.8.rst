..
    Copyright Contributors to the DNF5 project.
    SPDX-License-Identifier: GPL-2.0-or-later

.. _deptree_plugin_ref-label:

#################
 Deptree Command
#################

Synopsis
========

``dnf5 [GLOBAL OPTIONS] deptree [OPTIONS] <package-spec>...``


Description
===========

``deptree`` displays a package dependency graph using enabled repository metadata
by default. With ``--installed``, it uses packages from the local RPMDB instead.
It does not invoke the dependency solver.

By default, dependencies of type ``Requires`` are followed. Requirements with a
single provider link directly to that provider. Requirements with multiple
providers are displayed below a ``choice:`` node. ``rpmlib(*)`` requirements are
ignored. Requirements with no provider are displayed as ``unsatisfied:`` nodes.

A package already displayed in the same tree is not expanded again and is marked
with ``[already shown]``. This prevents cycles and repeated subtrees from growing without
bound.

Package specifications accept package names, NEVRAs, and glob patterns. For an
ambiguous specification without an architecture, the command selects the latest
EVR for each matching architecture. Dependency providers and reverse dependents
follow the same policy. ``--arch`` limits roots and traversal to the specified
architectures. Use ``--showduplicates`` to retain all matching versions. An
architecture-qualified NEVRA is honored as written.

In repository mode, repository selection follows the standard global options,
including ``--repo``, ``--enable-repo``, and ``--disable-repo``.


Options
=======

``--reverse``
    | Show packages that depend on each selected package. Reverse traversal uses
      the latest EVR of each package name and architecture by default.

``--flat``
    | Print a sorted, deduplicated list of package NEVRAs instead of a tree.

``--installed``
    | Display dependency trees for installed packages only.

``--arch=ARCH[,ARCH...]``
    | Limit roots, provider candidates, and reverse dependents to the specified
      architectures. This option can be specified multiple times.

``--depth=N``
    | Restrict traversal to ``N`` dependency levels. ``0`` prints only roots.

``--show-requires``
    | Render explicit ``requirement:`` nodes between packages and their providers.

``--types=LIST``
    | Process the comma-separated dependency types in ``LIST``. Accepted values
      are ``requires``, ``recommends``, ``suggests``, ``supplements``, and
      ``enhances``. The default is ``requires``.

``--showduplicates``
    | Include every matching package EVR. Without this option, ambiguous roots
      and traversal candidates use the latest EVR per name/architecture.

``--json``
    | Print machine-readable JSON. Normal mode emits a graph with ``roots``,
      ``nodes``, and ``edges`` arrays. Combined with ``--flat``, it emits a
      sorted JSON array of NEVRA strings.


JSON Output
===========

``dnf5 deptree --json <package-spec>``

Without ``--flat``, the command returns a graph object with these fields:

- ``roots`` (array of strings): IDs of root package nodes.
- ``nodes`` (array of objects): each graph node appears once. Every node has an
  ``id`` and ``type``.

    - ``package`` nodes contain ``name``, ``epoch``, ``version``, ``release``,
      and ``arch`` (all strings). Their IDs begin with ``pkg:``.
    - ``choice`` nodes contain ``capability`` (string). Their IDs begin with
      ``choice:``.
    - ``conditional`` nodes contain ``condition`` (string). Their IDs begin
      with ``cond:``.
    - ``else`` nodes contain ``condition`` (string). Their IDs begin with
      ``else:``.
    - ``requirement`` nodes, produced by ``--show-requires``, contain
      ``capability`` (string). Their IDs begin with ``requirement:``.
    - ``unsatisfied`` nodes contain the unresolved ``capability`` (string). Their
      IDs begin with ``unsatisfied:``.

- ``edges`` (array of objects): graph links with ``source`` and ``target`` node
  IDs. Every edge contains ``requirements``, an array of capability strings. It
  is empty for graph-structure links and provider links.

``dnf5 deptree --flat --json <package-spec>``

With both ``--flat`` and ``--json``, the command returns a sorted JSON array of
unique package NEVRA strings. Intermediate choice, conditional, else, and
requirement nodes are omitted.


Examples
========

``dnf5 deptree libsolv``
    | Show dependencies of the newest matching ``libsolv`` package builds.

``dnf5 deptree --reverse dnf5``
    | Show packages in enabled repositories that depend on matching ``dnf5``
      package builds.

``dnf5 deptree --showduplicates --depth=1 'libsolv*'``
    | Show every matching ``libsolv`` build and one dependency level.

``dnf5 deptree --types=requires,recommends --json bash``
    | Emit a JSON dependency graph that includes both hard and recommended
      dependencies.
