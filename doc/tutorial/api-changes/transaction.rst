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

Both DNF4 and DNF5 provide information about the progress of downloads using callbacks. This is implemented by inheriting from a callbacks base class and overriding its methods. The ``dnf.callback.DownloadProgress`` class in DNF4 is in DNF5 replaced by the ``libdnf5::repo::DownloadCallbacks`` class and the methods are slightly different.

In DNF5, there is a callback method ``add_new_download`` for each item to be downloaded instead of the ``start`` method for the whole download. The ``add_new_download`` returns a pointer to user data (for other language bindings, it's an integer instead, due to issues with ownership of the data) that are then passed as arguments to the other callback methods, so that they can be linked together. The DNF5 examples show, how it can be used.

DNF4 Python:

.. code-block:: python
    :linenos:

    class PackageDownloadCallbacks(dnf.callback.DownloadProgress):
        def start(self, total_files, total_size):
            print(f"Started downloading {total_files} files, total size: {total_size}")

        def end(self, payload, status, msg):
            if status is dnf.callback.STATUS_OK:
                print(f"Downloaded '{payload}'")
            elif status is dnf.callback.STATUS_ALREADY_EXISTS:
                print(f"Skipped to download '{payload}': {msg}")
            else:
                print(f"Failed to download '{payload}': {msg}")

    # Download the packages.
    base.download_packages(base.transaction.install_set, PackageDownloadCallbacks())

DNF5 Python:

.. literalinclude:: ../tests/bindings/python3/api_changes_from_dnf4/download_callbacks.py
    :language: py
    :linenos:

DNF5 C++:

.. literalinclude:: ../tests/api_changes_from_dnf4/download_callbacks.cpp
    :language: c++
    :linenos:
    :lines: 2,4-

For the transaction callbacks, DNF4 class ``dnf.callback.TransactionProgress`` mainly uses its ``progress`` method to report almost all events, differentiating them with the ``action`` argument. In DNF5, the ``libdnf5::rpm::TransactionCallbacks`` class has multiple methods that can be overridden, differentiated by possible events that can happen during a transaction.

DNF4 Python:

.. code-block:: python
    :linenos:

    class TransactionCallbacks(dnf.callback.TransactionProgress):
        def progress(self, package, action, ti_done, ti_total, ts_done, ts_total):
            if action == dnf.transaction.PKG_INSTALL and ti_done == ti_total:
                print(f"Installed: {package.name}-{package.evr}.{package.arch}")
            elif action == dnf.transaction.PKG_SCRIPTLET:
                print(f"Running scriptlet for: {package.name}-{package.evr}.{package.arch}")

        def error(self, message):
            print(f"Transaction error: {message}")

    # Run the transaction.
    base.do_transaction(TransactionCallbacks())

DNF5 Python:

.. literalinclude:: ../tests/bindings/python3/api_changes_from_dnf4/transaction_callbacks.py
    :language: py
    :linenos:

DNF5 C++:

.. literalinclude:: ../tests/api_changes_from_dnf4/transaction_callbacks.cpp
    :language: c++
    :linenos:
    :lines: 2-5,7-
