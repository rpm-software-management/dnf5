Documentation strings
=====================

We use `doxygen <https://www.doxygen.nl/manual/commands.html>`_ + sphinx + breathe to render libdnf5 documentation.

A typical docstring contains the following:

* Description (mandatory; we don't use @brief as it doesn't seem to impact the rendered output)
* a blank line
* @param - argument name and description (only if applicable, can be specified multiple times)
* @return - description of the returned value (only if applicable)
* @exception - exception type and description (only if applicable, can be specified multiple times)
* @since - version an entity become available (mandatory)
* @deprecated - reason why an entity is deprecated (only if applicable)
* @note - note text (optional)
* @warning - warning text (optional)

We highlight identifiers with ```backticks```.


Example::

    /// Split a string by a delimiter into a vector of strings
    ///
    /// @param text             Input text.
    /// @param delimiter        A delimiter we're using to split the text.
    /// @param max_items        Limit number of splits to produce a vector containing up to `max_items` items.
    /// @return Split text.
    ///         A continuation line for the return value description.
    /// @exception std::out_of_range    Value of the `max_items` argument is out of expected range.
    /// @since 5.0
    std::vector<std::string> split(std::string text, std::string delimiter, int max_items);


We also reference DNF 4 and libdnf5 0.x API classes, methods, attributes or functions that the subject of the documentation replaces
by using @replaces command followed with a description of the replaced entity:

  * <project>:<path>:class:<ClassName>
  * <project>:<path>:method:<ClassName>.<method_name>(args)
  * <project>:<path>:attribute:<ClassName>.<attribute_name>
  * <project>:<path>:function:<function_name>

We keep these in regular comments rather than docstrings to avoid spamming the rendered docs.

Example::

    /// <docstring>
    //
    // @replaces dnf:dnf/package.py:attribute:Package.name
    // @replaces libdnf5:libdnf5/hy-package.h:function:dnf_package_get_name(DnfPackage *pkg);
    std::string get_name() const;
