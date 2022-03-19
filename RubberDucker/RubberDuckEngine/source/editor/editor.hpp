#pragma once

namespace RDE {

    class Editor
    {
    public:
        void init();
        void update();

    private:
        void newFrame() const;
        void debugInfo();

        float m_dtTimer = 0.0f;
        float m_dtToDisplay = 1.0f;
    };
}