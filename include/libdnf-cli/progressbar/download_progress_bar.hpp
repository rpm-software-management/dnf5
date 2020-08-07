/*
Copyright (C) 2020 Red Hat, Inc.

This file is part of libdnf: https://github.com/rpm-software-management/libdnf/

Libdnf is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation, either version 2 of the License, or
(at your option) any later version.

Libdnf is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with libdnf.  If not, see <https://www.gnu.org/licenses/>.
*/


#ifndef LIBDNF_CLI_PROGRESSBAR_DOWNLOAD_PROGRESS_BAR_HPP
#define LIBDNF_CLI_PROGRESSBAR_DOWNLOAD_PROGRESS_BAR_HPP


#include "progress_bar.hpp"
#include "widgets/description.hpp"
#include "widgets/number.hpp"
#include "widgets/percent.hpp"
#include "widgets/progress.hpp"
#include "widgets/size.hpp"
#include "widgets/speed.hpp"
#include "widgets/time.hpp"


namespace libdnf::cli::progressbar {


class DownloadProgressBar : public ProgressBar {
public:
    explicit DownloadProgressBar(int64_t download_size, const std::string & description);

    using ProgressBar::get_messages;
    using ProgressBar::set_state;
    using ProgressBar::set_ticks;
    using ProgressBar::set_total_ticks;

    // TODO(dmach): add print() method

protected:
    void to_stream(std::ostream & stream) override;

private:
    // TODO(dmach): fix inconsistency - MultiProgresBar::operator<< erases previously printed lines, DownloadProgressBar::operator<< does not
    friend std::ostream & operator<<(std::ostream & stream, DownloadProgressBar & bar);

    // widgets
    libdnf::cli::progressbar::NumberWidget number_widget;
    libdnf::cli::progressbar::DescriptionWidget description_widget;
    libdnf::cli::progressbar::PercentWidget percent_widget;
    libdnf::cli::progressbar::ProgressWidget progress_widget;
    libdnf::cli::progressbar::SpeedWidget speed_widget;
    libdnf::cli::progressbar::SizeWidget size_widget;
    libdnf::cli::progressbar::TimeWidget time_widget;
};


}  // namespace libdnf::cli::progressbar


#endif  // LIBDNF_CLI_PROGRESSBAR_DOWNLOAD_PROGRESS_BAR_HPP
