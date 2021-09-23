%{
    // Undefine macros from ruby.h polluting the global namespace and
    // conflicting with C++ definitions (in particular from the fmt/format.h
    // header).
    //
    // These are unlikely to be needed in the bindings and in case they are, it
    // should cause a compile-time error (meaning no silent breakage).
    #undef isfinite
    #undef int128_t
    #undef uint128_t
%}
