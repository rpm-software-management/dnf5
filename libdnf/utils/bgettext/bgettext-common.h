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

#ifndef _BGETTEXT_COMMON_H_
#define _BGETTEXT_COMMON_H_

#include <libintl.h>
#include <stddef.h>

// Marks messages for translation
#define M_(msgId) ("\001" msgId)
#define MP_(msgId, msgIdPlural) ("\003" msgId "\00" msgIdPlural)
#define MC_(context, msgId) ("\005" context "\004" msgId)
#define MCP_(context, msgId, msgIdPlural) ("\007" context "\004" msgId "\00" msgIdPlural)

#ifdef __cplusplus
extern "C" {
#endif

// Preferred is use of C_() and CP_() macros. But these macros don't support non-string-literals
// as context and msgid arguments. Next functions are intended for this case.
const char * b_dpgettext(const char * domain, const char * context, const char * msgId);
const char * b_dnpgettext(const char * domain, const char * context, const char * msgId, const char * msgIdPlural, unsigned long int n);

const char * b_dmgettext(const char * domain, const char * markedText, unsigned long int n);

// Applications should normally not use this function directly, but use the C_() and CP() macros.
const char * b_dpgettext2(const char * domain, const char * ctxMsgId, size_t msgIdOffset);
const char * b_dnpgettext2(const char * domain, const char * ctxMsgId, size_t msgIdOffset, const char * msgIdPlural, unsigned long int n);

#ifdef __cplusplus
}
#endif

#endif /* _BGETTEXT_COMMON_H_ */
