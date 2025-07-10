// Copyright Contributors to the DNF5 project.
// Copyright Contributors to the libdnf project.
// SPDX-License-Identifier: LGPL-2.1-or-later
//
// This file is part of libdnf: https://github.com/rpm-software-management/libdnf/
//
// Libdnf is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, either version 2.1 of the License, or
// (at your option) any later version.
//
// Libdnf is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with libdnf.  If not, see <https://www.gnu.org/licenses/>.


#ifndef LIBDNF5_CLI_PROGRESSBAR_DOWNLOAD_PROGRESS_BAR_HPP
#define LIBDNF5_CLI_PROGRESSBAR_DOWNLOAD_PROGRESS_BAR_HPP


#include "progress_bar.hpp"
#include "widgets/description.hpp"
#include "widgets/number.hpp"
#include "widgets/percent.hpp"
#include "widgets/progress.hpp"
#include "widgets/size.hpp"
#include "widgets/speed.hpp"
#include "widgets/time.hpp"

#include "libdnf5-cli/defs.h"

#include "libdnf5/common/impl_ptr.hpp"


namespace libdnf5::cli::progressbar {


class LIBDNF_CLI_API DownloadProgressBar : public ProgressBar {
public:
    explicit DownloadProgressBar(int64_t download_size, const std::string & description);
    ~DownloadProgressBar();
    DownloadProgressBar(const DownloadProgressBar & src);
    DownloadProgressBar & operator=(const DownloadProgressBar & src);
    DownloadProgressBar(DownloadProgressBar && src) noexcept;
    DownloadProgressBar & operator=(DownloadProgressBar && src) noexcept;

    using ProgressBar::get_messages;
    using ProgressBar::set_state;
    using ProgressBar::set_ticks;
    using ProgressBar::set_total_ticks;

    // TODO(dmach): add print() method

    bool get_number_widget_visible() const noexcept;
    void set_number_widget_visible(bool value) noexcept;

protected:
    void to_stream(std::ostream & stream) override;

private:
    // TODO(dmach): fix inconsistency - MultiProgresBar::operator<< erases previously printed lines, DownloadProgressBar::operator<< does not
    LIBDNF_CLI_API friend std::ostream & operator<<(std::ostream & stream, DownloadProgressBar & bar);

    class LIBDNF_CLI_LOCAL Impl;
    ImplPtr<Impl> p_impl;
};


}  // namespace libdnf5::cli::progressbar


#endif  // LIBDNF5_CLI_PROGRESSBAR_DOWNLOAD_PROGRESS_BAR_HPP
