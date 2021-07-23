/*
Copyright Contributors to the libdnf project.

This file is part of libdnf: https://github.com/rpm-software-management/libdnf/

Libdnf is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation, either version 2.1 of the License, or
(at your option) any later version.

Libdnf is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with libdnf.  If not, see <https://www.gnu.org/licenses/>.
*/

#ifndef LIBDNF_LOGGER_NULL_LOGGER_HPP
#define LIBDNF_LOGGER_NULL_LOGGER_HPP

#include "logger.hpp"


namespace libdnf {

/// NullLogger is an implementation of logging class that discards all incoming logging messages.
/// It can be used in case when no logs are needed.
///
/// @replaces libdnf:utils/logger.hpp:class:NullLogger
class NullLogger : public Logger {
public:
    /// @replaces libdnf:utils/logger.hpp:method:NullLogger.write(int , libdnf::Logger::Level , const std::string & )
    void log(Level /*level*/, const std::string & /*message*/) noexcept override {}

    /// @replaces libdnf:utils/logger.hpp:method:NullLogger.write(int , time_t , pid_t , libdnf::Logger::Level , const std::string & )
    void write(time_t /*time*/, pid_t /*pid*/, Level /*level*/, const std::string & /*message*/) noexcept override {}
};

}  // namespace libdnf

#endif
