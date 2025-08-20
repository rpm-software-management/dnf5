Changes in resolving dependencies and running a transaction
===========================================================

In both DNF4 and DNF5, there is a ``Goal`` class which allows to add items for resolving into a transaction. Once all the items are added, the ``Goal`` can be resolved, which produces a ``Transaction`` object to run.

DNF4 also has methods in the ``dnf.base.Base`` class to modify the ``Goal``, such as ``dnf.base.Base.install``, ``dnf.base.Base.remove`` etc., ``dnf.base.Base.resolve`` to resolve the goal and ``dnf.base.Base.do_transaction`` to run the transation. In DNF5, the ``Goal`` class is used directly. The ``libdnf5::Goal::resolve`` returns the resulting transaction which can be run with ``libdnf5::base::Transaction::run``.

In case of an error in resolving dependencies, in DNF4, the ``dnf.base.Base.resolve`` raises ``dnf.exceptions.DepsolveError`` and the problems may be acquired using methods of the ``dnf.goal.Goal`` class, while in DNF5, the ``libdnf5::Goal::resolve`` doesn't raise any exception and the problems are stored in the returned ``libdnf5::base::Transaction`` object.

DNF4 Python:

.. code-block:: python
  :linenos:

  # Add an RPM package named "one" for installation into the goal.
  base.install("one")

  try:
      # Resolve the goal, create a transaction object.
      base.resolve()
  except dnf.exceptions.DepsolveError as e:
      print(e)
      raise

  # Download the packages.
  progress = dnf.cli.progress.MultiFileProgressMeter()
  base.download_packages(base.transaction.install_set, progress)

  # Run the transaction.
  base.do_transaction()

DNF5 Python:

.. literalinclude:: ../tests/bindings/python3/api_changes_from_dnf4/transaction.py
    :language: py
    :linenos:

DNF5 C++:

.. literalinclude:: ../tests/api_changes_from_dnf4/transaction.cpp
    :language: c++
    :linenos:
    :lines: 2,4-
