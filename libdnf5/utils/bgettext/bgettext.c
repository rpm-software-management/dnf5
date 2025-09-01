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

#include "libdnf5/defs.h"
#include "libdnf5/utils/bgettext/bgettext-common.h"

#include <string.h>

enum { BGETTEXT_PLURAL = 1 << 0, BGETTEXT_CONTEXT = 1 << 1, BGETTEXT_DOMAIN = 1 << 2 };

LIBDNF_API const char * b_dpgettext(const char * domain, const char * context, const char * msgId) {
    size_t context_len = strlen(context) + 1;
    size_t msgId_len = strlen(msgId) + 1;
    char ctxMsgId[context_len + msgId_len];

    memcpy(ctxMsgId, context, context_len - 1);
    ctxMsgId[context_len - 1] = '\004';
    memcpy(ctxMsgId + context_len, msgId, msgId_len);

    const char * const translation = dgettext(domain, ctxMsgId);

    if (translation == ctxMsgId)
        return msgId;

    return translation;
}

LIBDNF_API const char * b_dpgettext2(const char * domain, const char * ctxMsgId, size_t msgIdOffset) {
    const char * const translation = dgettext(domain, ctxMsgId);
    if (translation == ctxMsgId)
        return ctxMsgId + msgIdOffset;
    return translation;
}

LIBDNF_API const char * b_dnpgettext(
    const char * domain, const char * context, const char * msgId, const char * msgIdPlural, unsigned long int n) {
    size_t context_len = strlen(context) + 1;
    size_t msgId_len = strlen(msgId) + 1;
    char ctxMsgId[context_len + msgId_len];

    memcpy(ctxMsgId, context, context_len - 1);
    ctxMsgId[context_len - 1] = '\004';
    memcpy(ctxMsgId + context_len, msgId, msgId_len);

    const char * const translation = dngettext(domain, ctxMsgId, msgIdPlural, n);

    if (translation == ctxMsgId)
        return msgId;

    return translation;
}

LIBDNF_API const char * b_dnpgettext2(
    const char * domain, const char * ctxMsgId, size_t msgIdOffset, const char * msgIdPlural, unsigned long int n) {
    const char * const translation = dngettext(domain, ctxMsgId, msgIdPlural, n);
    if (translation == ctxMsgId)
        return ctxMsgId + msgIdOffset;
    return translation;
}

LIBDNF_API const char * b_gettextmsg_get_domain(struct BgettextMessage message) {
    return *message.bgettextMsg & BGETTEXT_DOMAIN ? message.bgettextMsg + 1 : NULL;
}

LIBDNF_API const char * b_gettextmsg_get_id(struct BgettextMessage message) {
    const char * msgId = message.bgettextMsg + 1;
    if (*message.bgettextMsg & BGETTEXT_DOMAIN) {
        msgId = strchr(msgId, 0) + 1;
    }
    if (*message.bgettextMsg & BGETTEXT_CONTEXT) {
        msgId = strchr(msgId, 0x04) + 1;
    }
    return msgId;
}

LIBDNF_API const char * b_gettextmsg_get_plural_id(struct BgettextMessage message) {
    if (!(*message.bgettextMsg & BGETTEXT_PLURAL)) {
        return NULL;
    }
    const char * msgId = message.bgettextMsg + 1;
    if (*message.bgettextMsg & BGETTEXT_DOMAIN) {
        msgId = strchr(msgId, 0) + 1;
    }
    msgId = strchr(msgId, 0) + 1;  // skip message id
    return msgId;
}

LIBDNF_API const char * b_dmgettext(const char * domain, struct BgettextMessage message, unsigned long int n) {
    const char * markedMsg = message.bgettextMsg;
    if ((*markedMsg & ~(BGETTEXT_PLURAL | BGETTEXT_CONTEXT | BGETTEXT_DOMAIN)) == 0) {
        const char * msgId;
        if (*markedMsg & BGETTEXT_DOMAIN) {
            domain = markedMsg + 1;
            msgId = strchr(domain, 0) + 1;
        } else {
            msgId = markedMsg + 1;
        }
        if (*markedMsg & BGETTEXT_PLURAL) {
            const char * const msgIdPlural = strchr(msgId, 0) + 1;
            const char * const translation = dngettext(domain, msgId, msgIdPlural, n);
            if ((*markedMsg & BGETTEXT_CONTEXT) && n == 1 && translation == msgId)
                return strchr(msgId, 4) + 1;
            return translation;
        } else {
            const char * const translation = dgettext(domain, msgId);
            if ((*markedMsg & BGETTEXT_CONTEXT) && (translation == msgId))
                return strchr(msgId, 4) + 1;
            return translation;
        }
    }
    return markedMsg;
}
