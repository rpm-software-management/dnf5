%{
    // Undefine macros polluting the global namespace and conflicting
    // with C++ definitions.
    //
    // These are unlikely to be needed in the bindings and in case they are, it
    // should cause a compile-time error (meaning no silent breakage).

    // From ruby - ruby.h: conflicts with fmt/format.h header
    #undef isfinite
    #undef int128_t
    #undef uint128_t

    // From perl5 - utf8.h: conflicts with fmt/format.h header
    #undef utf8_to_utf16

    // Recent versions of Perl (5.8.0) have namespace conflict problems.
    // Perl defines a bunch of short macros to make the Perl API function names shorter.
    // For example, in /usr/lib64/perl5/CORE/embed.h there is:
    // #define get_context Perl_get_context
    // We have to undefine that since we don't want our AdvisoryModule::get_context to
    // be renamed to Perl_get_context
    #undef get_context
%}


// Define empty macros. They are used to define the visibility of symbols.
#define LIBDNF_API
#define LIBDNF_LOCAL
#define LIBDNF_PLUGIN_API
