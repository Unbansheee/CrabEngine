//
// Created by Vinnie on 24/03/2025.
//

export module Engine.Assert;
import std;

export namespace Assert {
    using AssertHandler = std::function<void(
    const std::string& condition,
    const std::string& message,
    const std::source_location& location
)>;
}

namespace Internal {
    inline static Assert::AssertHandler handler = nullptr;
    // Separate initialization function
    inline void initialize_handler() {
        handler = [](const auto& condition, const auto& message, const auto& loc) {
            std::cerr << "Assertion failed!\n"
                      << "File: " << loc.file_name() << ":" << loc.line() << "\n"
                      << "Function: " << loc.function_name() << "\n"
                      << "Condition: " << condition << "\n"
                      << "Message: " << message << "\n";

            #if defined(_MSC_VER)
                        __debugbreak();
            #elif defined(__GNUC__) || defined(__clang__)
                        __builtin_trap();
            #endif

                        //std::abort();
            };
        }
}




export namespace Assert {
    void SetHandler(AssertHandler newHandler) {
        Internal::handler = newHandler;
    }

    void Fail(
        const std::string& message = "",
        const std::source_location& loc = std::source_location::current()
    ) {
        if (!Internal::handler) Internal::initialize_handler();
        Internal::handler("FAIL()", message, loc);
    }

    void Check(
        bool condition,
        const std::string& expression = "",
        const std::string& message = "",
        const std::source_location& loc = std::source_location::current()
    ) {
#ifdef ENGINE_ENABLE_ASSERTS
        if (!condition) {
            if (!Internal::handler) Internal::initialize_handler();
            Internal::handler(expression, message, loc);
        }
#endif
    }

    // For expressions that should always be checked (even in release builds)
    void Verify(
        bool condition,
        const std::string& expression = "",
        const std::string& message = "",
        const std::source_location& loc = std::source_location::current()
    ) {
        if (!condition) {
            if (!Internal::handler) Internal::initialize_handler();
            Internal::handler(expression, message, loc);
        }
    }
}

