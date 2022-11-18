#pragma once
#include "mono/jit/jit.h"
#include "mono/metadata/assembly.h"

namespace RDE
{
class MonoHandler
{
    MonoDomain* m_root = nullptr;
    MonoImage* m_APIImage = nullptr;
    MonoImage* m_ScriptImage = nullptr;
    MonoAssembly* m_APIAssembly = nullptr;
    MonoAssembly* m_ScriptAssembly = nullptr;

    void GenerateDLL();
    bool LoadDLLImage(const char* filename, MonoImage*& image,
                      MonoAssembly*& assembly);

  public:
    MonoHandler() = default;

    [[nodiscard]] inline MonoImage* GetAPIImage() { return m_APIImage; }
    [[nodiscard]] inline MonoImage* GetScriptImage() { return m_ScriptImage; }

    void init();
    void cleanup();
    void StartMono();
    void StopMono();
};
} // namespace RDE
