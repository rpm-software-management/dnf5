// Copyright Contributors to the DNF5 project.
// Copyright Contributors to the libdnf project.
// SPDX-License-Identifier: LGPL-2.1-or-later

#ifndef _BGETTEXT_MARK_COMMON_H_
#define _BGETTEXT_MARK_COMMON_H_

struct BgettextMessage {
    const char * bgettextMsg;
};

#ifdef __cplusplus
extern "C" {
#endif

/// Returns name of the message domain encoded in the message.
/// @param message message encoded for translation
/// @return name of message domain or NULL if the domain is not present in the encoded message
const char * b_gettextmsg_get_domain(struct BgettextMessage message);

/// Returns message id from encoded message.
/// @param message message encoded for translation
/// @return message id
const char * b_gettextmsg_get_id(struct BgettextMessage message);

/// Returns message plural id from encoded message.
/// @param message message encoded for translation
/// @return plural message id or NULL if it is not present in the encoded message
const char * b_gettextmsg_get_plural_id(struct BgettextMessage message);

/// Attempts to translate the 'message' into the user's language by searching for the translation in a message catalog.
/// @param domain message domain used for translation, argument is used only if the domain is not present in encoded message
/// @param message message encoded for translation
/// @param n defines plural form to be use (returns the base form if encoded message does not define plural form)
/// @return translated message (or message id if translation was not found)
const char * b_dmgettext(const char * domain, struct BgettextMessage message, unsigned long int n);

#ifdef __cplusplus
}
#endif

#endif /* _BGETTEXT_MARK_COMMON_H_ */
