/*
   Copyright (C) 2017 Jaroslav Rohel <jrohel@redhat.com>

   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU Lesser General Public License as published by
   the Free Software Foundation; either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.  */

#include "bgettext-common.h"

#include <string.h>

const char *
b_dpgettext(const char * domain, const char * context, const char * msgId)
{
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

const char *
b_dpgettext2(const char * domain, const char * ctxMsgId, size_t msgIdOffset)
{
    const char * const translation = dgettext(domain, ctxMsgId);
    if (translation == ctxMsgId)
        return ctxMsgId + msgIdOffset;
    return translation;
}

const char *
b_dnpgettext(const char * domain, const char * context, const char * msgId, const char * msgIdPlural, unsigned long int n)
{
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

const char *
b_dnpgettext2(const char * domain, const char * ctxMsgId, size_t msgIdOffset, const char * msgIdPlural, unsigned long int n)
{
    const char * const translation = dngettext(domain, ctxMsgId, msgIdPlural, n);
    if (translation == ctxMsgId)
        return ctxMsgId + msgIdOffset;
    return translation;
}

const char *
b_dmgettext(const char * domain, const char * markedMsg, unsigned long int n)
{
    if (*markedMsg & 0x01) {
        const char * const msgId = markedMsg + 1;
        if (*markedMsg & 0x02) {
            const char * const msgIdPlural = strchr(msgId, 0) + 1;
            const char * const translation = dngettext(domain, msgId, msgIdPlural, n);
            if ((*markedMsg & 0x04) && n == 1 && translation == msgId)
                return strchr(msgId, 4) + 1;
            return translation;
        } else {
            const char * const translation = dgettext(domain, msgId);
            if ((*markedMsg & 0x04) && (translation == msgId))
                return strchr(msgId, 4) + 1;
            return translation;
        }
    }
    return markedMsg;
}
