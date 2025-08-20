Changes in loading repositories
===============================

The repositories are loaded in order to have information about installed and available packages, comps, modules and excludes.

In DNF5, the repositories must be loaded only after the ``libdnf5::base::Base::setup()`` call and they cannot be loaded repeatedly. If a new set of repositories is needed, a new Base object should be created.

DNF4 Python:

.. code-block:: python
  :linenos:

  # Optionally, read repositories from system configuration files.
  base.read_all_repos()

  # Optionally, create and configure a new repository.
  repo = dnf.repo.Repo("my_new_repo_id", base.conf)
  repo.baseurl = [baseurl]
  base.repos.add(repo)

  # Load repositories. To limit which repositories are loaded, pass
  # load_system_repo=False or load_available_repos=False.
  base.fill_sack()

DNF5 Python:

.. literalinclude:: ../tests/bindings/python3/api_changes_from_dnf4/load_repos.py
    :language: py
    :linenos:

DNF5 C++:

.. literalinclude:: ../tests/api_changes_from_dnf4/load_repos.cpp
    :language: c++
    :linenos:
