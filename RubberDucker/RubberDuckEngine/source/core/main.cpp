#include "pch.hpp"
#include "application.hpp"

int main()
{
    // Enable run-time memory check for debug builds
#if defined(RDE_DEBUG)
    _CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
#endif

    RDE::Application app;

    try {
        app.run();
    }
    catch (const std::exception& e) {
        RDE_LOG_CRITICAL("Exception thrown: {}", e.what());
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}