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

#include "html.hpp"

#include "download_file.hpp"

#include <libdnf5/utils/fs/temp.hpp>

#include <fstream>
#include <iostream>

Html::Html(libdnf5::Base & base, const std::string & html_url) {
    auto temp_file = libdnf5::utils::fs::TempFile("/tmp", "dnf5-obs-plugin");
    download_file(base, html_url, temp_file.get_path());
    doc = (xmlDoc *)htmlReadFile(temp_file.get_path().string().c_str(), NULL, HTML_PARSE_NOERROR);
    if (doc)
        ctx = xmlXPathNewContext(doc);
    if (ctx)
        obj = xmlXPathEvalExpression((const xmlChar *)"//a[@title='Go to download repository']", ctx);
    if (obj && obj->nodesetval && obj->nodesetval->nodeNr)
        download_url = xmlGetProp(obj->nodesetval->nodeTab[0], (const xmlChar *)"href");
}

std::string Html::get_download_url() {
    if (!download_url)
        return "";
    std::string value((const char *)download_url);
    return value;
}

Html::~Html() {
    if (download_url)
        xmlFree(download_url);
    if (obj)
        xmlXPathFreeObject(obj);
    if (ctx)
        xmlXPathFreeContext(ctx);
    if (doc)
        xmlFreeDoc(doc);
}
