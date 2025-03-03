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


%{
    // Define ExceptionWrap class.
    // It is used to deliver and work with C++ exception including nested exceptions in the target language.
    #include "libdnf5/common/exception.hpp"

    namespace libdnf5::common {

    class ExceptionWrap {
    public:
        ExceptionWrap() : except_ptr{std::current_exception()} {}

        /// @return The basic error message.
        const char * what() const noexcept;

        /// @return The exception class name.
        const char * get_name() const noexcept;

        /// @return The domain name (namespace and enclosing class names) of the exception.
        const char * get_domain_name() const noexcept;

        /// Formats the error message of an exception.
        /// If the exception is nested, recurses to format the message of the exception it holds.
        ///
        /// @param detail Defines the detail of the message.
        /// @return Error message including messages from nested exceptions.
        std::string format(libdnf5::FormatDetailLevel detail) const;

        /// Formats the error message of an exception to string with domain and name.
        /// If the exception is nested, recurses to format the message of the exception it holds.
        ///
        /// @return Detailed error message including messages from nested exceptions.
        std::string to_string() const;

        /// Rethrows the original exception.
        void rethrow_original() const {
            if (!except_ptr) {
                return;
            }
            std::rethrow_exception(except_ptr);
        }

        /// If a nested exception is contained, it is rethrown.
        void rethrow_if_nested_original() const {
            try {
                rethrow_original();
            } catch (const std::exception & ex) {
                std::rethrow_if_nested(ex);
            }
        }

        /// If a nested exception is contained, it is thrown in a new ExceptionWrap exception.
        void rethrow_if_nested() const;

    private:
        std::exception_ptr except_ptr;
    };

    }  // namespace libdnf5::common
%}
