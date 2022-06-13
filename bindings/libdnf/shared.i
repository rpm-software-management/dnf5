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
%}
