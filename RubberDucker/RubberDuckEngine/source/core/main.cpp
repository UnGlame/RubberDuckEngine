#include "precompiled/pch.hpp"
#include "main.hpp"

std::unique_ptr<RDE::Engine> g_engine;

int main()
{
    // Enable run-time memory check for debug builds
#if defined(RDE_DEBUG)
    _CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
#endif

    g_engine = std::make_unique<RDE::Engine>();

    try {
        g_engine->run();
    }
    catch (const std::exception& e) {
        RDE_LOG_CRITICAL("Exception thrown: {}", e.what());
        return EXIT_FAILURE;
    }
    catch (...) {
        RDE_LOG_CRITICAL("Exception thrown!");
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}