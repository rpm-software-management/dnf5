==============
Best practices
==============

* All code changes should be covered with unit tests
* Run ``clang-format`` and ``clang-tidy`` to check coding style before submitting a pull request
* If clang-format produces sub-optimal output, consider using ``// clang-format off|on`` where appropriate
* Use templates and lambdas only if really necessary
* Avoid ``shared_ptr`` because it isn't supported in all SWIG bindings
* Set symbol visibility - ABI

We follow the following coding style:


.. toctree::
    :maxdepth: 1

    coding_style_cpp
    coding_style_py
    documentation_strings
    project_layout
    symbol_visibility
