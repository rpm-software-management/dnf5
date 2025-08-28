// Copyright Contributors to the libdnf project.
// SPDX-License-Identifier: LGPL-2.1-or-later

#ifndef _BGETTEXT_COMMON_H_
#define _BGETTEXT_COMMON_H_

#include "bgettext-mark-common.h"

#include <libintl.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

/// Attempts to translate the 'msgId' into the user's language by searching for the translation in a message catalog.
/// The use of C_() macro is preferred. But this macro don't support non-string-literals as 'context' and 'msgId' arguments.
/// This function is intended for this case.
///
/// @param domain message domain used for translation, non-empty string or NULL
///               NULL means to use the domain name specified through a preceding 'textdomain' call.
/// @param context message context to distinguish between multiple translations of the same msgId
/// @param msgId message for translation
/// @return translated message (or msgId if translation was not found)
const char * b_dpgettext(const char * domain, const char * context, const char * msgId);

/// Attempts to translate the 'msgId' into the user's language by searching for the translation in a message catalog.
/// The use of CP_() macro is preferred. But this macro don't support non-string-literals as 'context' and 'msgId' arguments.
/// This function is intended for this case.
///
/// @param domain message domain used for translation, non-empty string or NULL
///               NULL means to use the domain name specified through a preceding textdomain call.
/// @param context message context to distinguish between multiple translations of the same msgId
/// @param msgId message for translation
/// @param msgIdPlural plural form of message for translation
/// @param n defines plural form to be use
/// @return translated message (or msgId if translation was not found)
const char * b_dnpgettext(
    const char * domain, const char * context, const char * msgId, const char * msgIdPlural, unsigned long int n);

// Applications should normally not use this function directly, but use the C_() and CP_() macros.
const char * b_dpgettext2(const char * domain, const char * ctxMsgId, size_t msgIdOffset);
// Applications should normally not use this function directly, but use the C_() and CP_() macros.
const char * b_dnpgettext2(
    const char * domain, const char * ctxMsgId, size_t msgIdOffset, const char * msgIdPlural, unsigned long int n);

#ifdef __cplusplus
}
#endif

#endif /* _BGETTEXT_COMMON_H_ */
