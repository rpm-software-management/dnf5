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

#ifndef _BGETTEXT_MARK_H_
#define _BGETTEXT_MARK_H_

#include "bgettext-mark-common.h"

// clang-format off
// Use EMPTY_MESSAGE instead of M_(""). Otherwise, xgettext breaks catalogs.
#define EMPTY_MESSAGE \
    { .bgettextMsg = "\000" "" }
#define M_(msgId) \
    { .bgettextMsg = "\000" msgId }
#define MP_(msgId, msgIdPlural) \
    { .bgettextMsg = "\001" msgId "\00" msgIdPlural }
#define MC_(context, msgId) \
    { .bgettextMsg = "\002" context "\004" msgId }
#define MCP_(context, msgId, msgIdPlural) \
    { .bgettextMsg = "\003" context "\004" msgId "\00" msgIdPlural }
// clang-format on

#endif /* _BGETTEXT_MARK_H_ */
