// Copyright Contributors to the DNF5 project.
// Copyright Contributors to the libdnf project.
// SPDX-License-Identifier: LGPL-2.1-or-later

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
