// Copyright Contributors to the DNF5 project.
// Copyright Contributors to the libdnf project.
// SPDX-License-Identifier: LGPL-2.1-or-later

#ifndef _BGETTEXT_H_
#define _BGETTEXT_H_

#include "bgettext-common.h"

#define _(msgId)                  ((const char *)gettext(msgId))
#define P_(msgId, msgIdPlural, n) ((const char *)ngettext(msgId, msgIdPlural, n))
#define C_(context, msgId)        b_dpgettext2(NULL, context "\004" msgId, sizeof(context))
#define CP_(context, msgId, msgIdPlural, n) \
    ((const char *)b_dnpgettext2(NULL, context "\004" msgId, sizeof(context), msgIdPlural, n))

#define TM_(markedMsg, n) b_dmgettext(NULL, markedMsg, n)

#endif /* _BGETTEXT_H_ */
