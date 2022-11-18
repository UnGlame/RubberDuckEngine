#pragma once

namespace RDE
{

class Editor
{
  public:
    void init();
    void update();

    [[nodiscard]] _forceinline bool renderingEnabled() const
    {
<<<<<<< HEAD
    public:
        void init();
        void update();
        void mainMenuUpdate();
=======
        return m_renderingEnabled;
    }
    __forceinline void enableRendering(bool enable)
    {
        m_renderingEnabled = enable;
    }
    inline void toggle() { m_renderingEnabled = !m_renderingEnabled; }
>>>>>>> main

  private:
    void newFrame() const;
    void debugInfo();

    float m_dtTimer = 0.0f;
    float m_dtToDisplay = 1.0f;
    bool m_renderingEnabled = true;
};
} // namespace RDE
