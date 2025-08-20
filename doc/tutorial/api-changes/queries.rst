Changes in creating queries
===========================

To get a list of packages based on given criteria, they can be queried using ``dnf.query.Query`` class in DNF4 and ``libdnf5::rpm::PackageQuery`` class in DNF5.

In DNF4, the ``filter`` method doesn't actually modify the query, but rather return a new query limiting it according to the given arguments. In DNF5, the original query is modifed by the filters.

DNF4 Python:

.. code-block:: python
  :linenos:

  # Create a package query.
  packages = base.sack.query()

  # Filter the packages, the filters can be stacked one after another.
  packages = packages.filter(name="one")

  # Iterate over the filtered packages in the query.
  for pkg in packages:
      print(pkg.name)

DNF5 Python:

.. literalinclude:: ../tests/bindings/python3/api_changes_from_dnf4/package_query.py
    :language: py
    :linenos:

DNF5 C++:

.. literalinclude:: ../tests/api_changes_from_dnf4/package_query.cpp
    :language: c++
    :linenos:
    :lines: 2,4-

The groups and environments are in DNF4 stored in the ``dnf.comps.Comps`` class, whih is different from a query object, though it also provides some filtering. In DNF5, the groups and environments are queried using the ``libdnf5::comps::GroupQuery`` and ``libdnf5::comps::EnvironmentQuery`` classes respectively, similarly to the ``libdnf5::rpm::PackageQuery`` class. The example shows querying groups, but the environments are analogous.

DNF4 Python:

.. code-block:: python
  :linenos:

  # Get all groups.
  groups = base.comps.groups
  # Or get groups with a given pattern in id or name.
  groups = groups_by_pattern("group-id")

  # Iterate over the groups.
  for group in groups:
      print(group.ui_name)

DNF5 Python:

.. literalinclude:: ../tests/bindings/python3/api_changes_from_dnf4/group_query.py
    :language: py
    :linenos:

DNF5 C++:

.. literalinclude:: ../tests/api_changes_from_dnf4/group_query.cpp
    :language: c++
    :linenos:
    :lines: 2,4-
