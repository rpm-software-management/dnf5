# bgettext - Better gettext

This is an extension of the GNU gettext.

At first I tried to use the glib extension of gettext. But glib is incomplete.
* There are macros `_()`, `C_()`. But there are missing macros for plural forms. And there is not support of plural forms with context.
* There are macros `N_()`, `NC_()` witch only marks string for translation. There is the same problem with plural. Moreover these macros are not useable as I needed.
* I do not want to depend on glib just for translation.

## So, I wrote my extension that offers everything I need.

### Extensions
* Added macros for translation `_()`, `P_()`, `C_()`, `CP_()`.
* Added macros for marking and encode text `M_()`, `MP_()`, `MC_()`, `MCP_()`.
* Added macro `TM_()` for translating text encoded by `M*_()` macros.
* Added functions for work with non-string-literals.

## Description of macros for translation

#### `_(msgId)`
This macro wraps gettext/dgettext. The macro attempts to translate a text string into the user's native language, by looking up the translation in a message catalog.
The msgId argument identifies the message to be translated. By convention, it is the English version of the message, with non-ASCII characters replaced by ASCII approximationsUses gettext to get the translation for msgId. 
If you are using the `_()` macro, you need to make sure that you pass `--keyword=_` to xgettext when extracting messages. Note that this only works with GNU gettext >= 0.15.
##### Parameters
`msgId` - a message id, must be a string literal
##### Returns
If a translation was found in one of the specified catalogs, it is converted to the locale's codeset and returned. The resulting string is statically allocated and must not be modified or freed. Otherwise msgid is returned. 
##### Example
`label = _("This is text for translation")`
 
#### `P_(msgId, msgIdPlural, n)`
This macro wraps ngettext/dngettext. It is used for translate message and choose plural form.
Plural forms are grammatical variants depending on the a number. Some languages have two forms, called singular and plural. Other languages have three or more forms.
If you are using the `P_()` macro, you need to make sure that you pass `--keyword=P_:1,2` to xgettext when extracting messages. Note that this only works with GNU gettext >= 0.15.
##### Parameters
`msgId` - a message id, must be a string 
`msgIdPlural` - plural form of the message
`n` - the quantity for which translation is needed
##### Returns
If a translation was found in one of the specified catalogs, the appropriate plural form is converted to the locale's codeset and returned. The resulting string is statically allocated and must not be modified or freed. In the "C" locale, or if none of the used catalogs contain a translation for msgid, the ngettext, dngettext and dcngettext functions return msgId if n == 1, or msgIdPlural if n != 1.
##### Example
`label = P_("There is one cat.", "There are %d cats.", n);`

#### `C_(context, msgId)`
This macro encodes context and msgId. The dgettext is used to get the translation for msgId for given context. This is useful for short strings which may need different translations, depending on the context in which they are used.
If you are using the `C_()` macro, you need to make sure that you pass `--keyword=C_:1c,2` to xgettext when extracting messages. Note that this only works with GNU gettext >= 0.15.
##### Parameters
`context` - a message context, must be a string literal
`msgId` - a message id, must be a string literal
##### Returns
If a translation was found in one of the specified catalogs, it is converted to the locale's codeset and returned. The resulting string is statically allocated and must not be modified or freed. Otherwise msgid is returned.
##### Example
`label1 = C_("Error", "Bug");`
`label2 = C_("Insects", "Bug");`

#### `CP_(context, msgId, msgIdPlural, n)`
This is the most powerfull macro. It supports translation of message with context and plural forms. The macro encodes context and msgId and a dngettext is used internaly. See `P_()` and `C_()` macros for more informations about plural forms and context.
If you are using the `CP_()` macro, you need to make sure that you pass `--keyword=CP_:1c,2,3` to xgettext when extracting messages. Note that this only works with GNU gettext >= 0.15.
##### Parameters
`context` - a message context, must be a string literal
`msgId` - a message id, must be a string 
`msgIdPlural` - plural form of the message, must be a string literal
`n` - the quantity for which translation is needed
##### Returns
If a translation was found in one of the specified catalogs, the appropriate plural form is converted to the locale's codeset and returned. The resulting string is statically allocated and must not be modified or freed. In the "C" locale, or if none of the used catalogs contain a translation for msgid, the ngettext, dngettext and dcngettext functions return msgId if n == 1, or msgIdPlural if n != 1.
##### Example
`label1 = CP_("Vehicles", "Bus", "Buses", n);`
`label2 = CP_("Signal", "Bus", "Buses", n);`

## Macros for text marking
Marks a message for translation and encode it for later usage. This is useful in situations where the translated strings can't be directly used, e.g. in string array initializers. To get the translated string, use `TM_()` macro.
If you are using these macros, you need to make sure that you pass `--keyword=M_`, `--keyword=MP_:1,2`, `--keyword=MC_:1c,2`, `--keyword=MCP_:1c,2,3` to xgettext when extracting messages. Note that this only works with GNU gettext >= 0.15.
#### `M_(msgId)`
#### `MP_(msgId, msgIdPlural)`
#### `MC_(context, msgId)`
#### `MCP_(context, msgId, msgIdPlural)`

##### Example
```
{
  static const char *messages[] = {
    M_("message1"),
    M_("message2")
  };
  const char *msg;
  ...
  msg = TM_(messages[index], 1);
  ...
}
```
