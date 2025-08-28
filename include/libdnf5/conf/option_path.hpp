// Copyright Contributors to the libdnf project.
// SPDX-License-Identifier: LGPL-2.1-or-later

#ifndef LIBDNF5_CONF_OPTION_PATH_HPP
#define LIBDNF5_CONF_OPTION_PATH_HPP

#include "option_path_errors.hpp"
#include "option_string.hpp"

#include "libdnf5/defs.h"


namespace libdnf5 {

/// Option that stores file/directory path.
/// Support default value, and path verification (absolute, existence).
// @replaces libdnf:conf/OptionPath.hpp:class:OptionPath
class LIBDNF_API OptionPath : public OptionString {
public:
    /// Constructor sets default value and conditions.
    // @replaces libdnf:conf/OptionPath.hpp:ctor:OptionPath.OptionPath(const std::string & defaultValue, bool exists = false, bool absPath = false)
    explicit OptionPath(std::string default_value, bool exists = false, bool abs_path = false);

    /// Constructor sets default value and conditions.
    // @replaces libdnf:conf/OptionPath.hpp:ctor:OptionPath.OptionPath(const char * defaultValue, bool exists = false, bool absPath = false)
    explicit OptionPath(const char * default_value, bool exists = false, bool abs_path = false);

    /// Constructor sets default value and conditions.
    // @replaces libdnf:conf/OptionPath.hpp:ctor:OptionPath.OptionPath(const std::string & defaultValue, const std::string & regex, bool icase, bool exists = false, bool absPath = false)
    OptionPath(std::string default_value, std::string regex, bool icase, bool exists = false, bool abs_path = false);

    ~OptionPath();

    OptionPath(const OptionPath & src);

    /// Constructor sets default value and conditions.
    // @replaces libdnf:conf/OptionPath.hpp:ctor:OptionPath.OptionPath(const char * defaultValue, const std::string & regex, bool icase, bool exists = false, bool absPath = false)
    OptionPath(const char * default_value, std::string regex, bool icase, bool exists = false, bool abs_path = false);

    /// Makes copy (clone) of this object.
    // @replaces libdnf:conf/OptionPath.hpp:method:OptionPath.clone()
    OptionPath * clone() const override;

    /// Parses input string and sets new value and priority.
    /// According setting passed in constructor it can verify that the path is absolute, exists and match regex.
    /// The value and priority are stored only if the new priority is equal to or higher than the stored priority.
    // @replaces libdnf:conf/OptionPath.hpp:method:OptionPath.set(Priority priority, const std::string & value)
    void set(Priority priority, const std::string & value) override;

    /// Parses input string and sets new value and runtime priority.
    void set(const std::string & value) override;

    /// Tests input value and throws exception if the value is not allowed.
    // @replaces libdnf:conf/OptionPath.hpp:method:OptionPath.test(const std::string & value)
    void test(const std::string & value) const;

private:
    class LIBDNF_LOCAL Impl;
    ImplPtr<Impl> p_impl;
};

}  // namespace libdnf5

#endif
