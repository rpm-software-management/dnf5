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
