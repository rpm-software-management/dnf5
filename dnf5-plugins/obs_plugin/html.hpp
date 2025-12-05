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

#ifndef DNF5_COMMANDS_OBS_HTML_HPP
#define DNF5_COMMANDS_OBS_HTML_HPP

#include <libdnf5/base/base.hpp>
#include <libxml/HTMLparser.h>
#include <libxml/parser.h>
#include <libxml/tree.h>
#include <libxml/xpath.h>


class Html {
private:
    xmlChar * download_url{nullptr};
    xmlXPathObject * obj{nullptr};
    xmlXPathContext * ctx{nullptr};
    xmlDoc * doc{nullptr};

public:
    Html(libdnf5::Base & base, const std::string & html_url);
    std::string get_download_url();
    ~Html();
};

#endif  // DNF5_COMMANDS_OBS_HTML_HPP
