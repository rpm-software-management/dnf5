// Copyright Contributors to the libdnf project.
// SPDX-License-Identifier: LGPL-2.1-or-later

#ifndef LIBDNF5_LOGGER_STREAM_LOGGER_HPP
#define LIBDNF5_LOGGER_STREAM_LOGGER_HPP

#include "logger.hpp"

#include "libdnf5/common/impl_ptr.hpp"
#include "libdnf5/defs.h"

#include <memory>
#include <ostream>


namespace libdnf5 {

/// StreamLogger is an implementation of logging class that writes messages into a stream.
class LIBDNF_API StreamLogger : public StringLogger {
public:
    explicit StreamLogger(std::unique_ptr<std::ostream> && log_stream);
    ~StreamLogger() override;

    void write(const char * line) noexcept override;

private:
    class LIBDNF_LOCAL Impl;
    ImplPtr<Impl> p_impl;
};

/// Logger that logs to a stream stored as a reference, meant to be used with std::cerr and std::cout.
class LIBDNF_API StdCStreamLogger : public StringLogger {
public:
    explicit StdCStreamLogger(std::ostream & log_stream);
    ~StdCStreamLogger() override;

    void write(const char * line) noexcept override;

private:
    class LIBDNF_LOCAL Impl;
    ImplPtr<Impl> p_impl;
};

}  // namespace libdnf5

#endif
