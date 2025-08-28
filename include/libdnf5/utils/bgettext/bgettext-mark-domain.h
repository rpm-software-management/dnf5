// Copyright Contributors to the DNF5 project.
// Copyright Contributors to the libdnf project.
// SPDX-License-Identifier: LGPL-2.1-or-later

#ifndef _BGETTEXT_MARK_DOMAIN_H_
#define _BGETTEXT_MARK_DOMAIN_H_

#ifndef GETTEXT_DOMAIN
#error You must define GETTEXT_DOMAIN before including bgettext-mark.h.
#endif

#include "bgettext-mark-common.h"

// clang-format off
// Use EMPTY_MESSAGE instead of M_(""). Otherwise, xgettext breaks catalogs.
#define EMPTY_MESSAGE \
    { .bgettextMsg = "\004" GETTEXT_DOMAIN "\00" "" }
#define M_(msgId) \
    { .bgettextMsg = "\004" GETTEXT_DOMAIN "\00" msgId }
#define MP_(msgId, msgIdPlural) \
    { .bgettextMsg = "\005" GETTEXT_DOMAIN "\00" msgId "\00" msgIdPlural }
#define MC_(context, msgId) \
    { .bgettextMsg = "\006" GETTEXT_DOMAIN "\00" context "\004" msgId }
#define MCP_(context, msgId, msgIdPlural) \
    { .bgettextMsg = "\007" GETTEXT_DOMAIN "\00" context "\004" msgId "\00" msgIdPlural }
//clang-format on

#endif /* _BGETTEXT_MARK_DOMAIN_H_ */
