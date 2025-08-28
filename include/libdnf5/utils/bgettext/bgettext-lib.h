// Copyright Contributors to the libdnf project.
// SPDX-License-Identifier: LGPL-2.1-or-later

#ifndef _BGETTEXT_LIB_H_
#define _BGETTEXT_LIB_H_

// TODO(jrohel) separate bgettext into a library
#ifndef GETTEXT_DOMAIN
#error You must define GETTEXT_DOMAIN before including bgettext-lib.h.
#endif

#include "bgettext-common.h"

#define _(msgId)                  ((const char *)dgettext(GETTEXT_DOMAIN, msgId))
#define P_(msgId, msgIdPlural, n) ((const char *)dngettext(GETTEXT_DOMAIN, msgId, msgIdPlural, n))
#define C_(context, msgId)        b_dpgettext2(GETTEXT_DOMAIN, context "\004" msgId, sizeof(context))
#define CP_(context, msgId, msgIdPlural, n) \
    ((const char *)b_dnpgettext2(GETTEXT_DOMAIN, context "\004" msgId, sizeof(context), msgIdPlural, n))

#define TM_(markedMsg, n) b_dmgettext(GETTEXT_DOMAIN, markedMsg, n)

#endif /* _BGETTEXT_LIB_H_ */
