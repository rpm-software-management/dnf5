// Copyright Contributors to the DNF5 project.
// Copyright Contributors to the libdnf project.
// SPDX-License-Identifier: LGPL-2.1-or-later


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


namespace libdnf5::cli::progressbar {


class LIBDNF_CLI_API DownloadProgressBar : public ProgressBar {
public:
    explicit DownloadProgressBar(int64_t download_size, const std::string & description);

    using ProgressBar::get_messages;
    using ProgressBar::set_state;
    using ProgressBar::set_ticks;
    using ProgressBar::set_total_ticks;

    // TODO(dmach): add print() method

    bool get_number_widget_visible() const noexcept { return number_widget.get_visible(); }
    void set_number_widget_visible(bool value) noexcept { number_widget.set_visible(value); }

protected:
    void to_stream(std::ostream & stream) override;

private:
    // TODO(dmach): fix inconsistency - MultiProgresBar::operator<< erases previously printed lines, DownloadProgressBar::operator<< does not
    LIBDNF_CLI_API friend std::ostream & operator<<(std::ostream & stream, DownloadProgressBar & bar);

    // widgets
    libdnf5::cli::progressbar::NumberWidget number_widget;
    libdnf5::cli::progressbar::DescriptionWidget description_widget;
    libdnf5::cli::progressbar::PercentWidget percent_widget;
    libdnf5::cli::progressbar::ProgressWidget progress_widget;
    libdnf5::cli::progressbar::SpeedWidget speed_widget;
    libdnf5::cli::progressbar::SizeWidget size_widget;
    libdnf5::cli::progressbar::TimeWidget time_widget;
};


}  // namespace libdnf5::cli::progressbar


#endif  // LIBDNF5_CLI_PROGRESSBAR_DOWNLOAD_PROGRESS_BAR_HPP
