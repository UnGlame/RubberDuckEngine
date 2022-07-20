#pragma once
#include <glm/glm.hpp>

#include "window/window.hpp"

namespace RDE
{

enum class KeyCode : uint32_t;
enum class MouseCode : uint32_t;

class InputHandler
{
  public:
    static constexpr size_t k_largestKeyCode = GLFW_KEY_MENU;
    static constexpr size_t k_largestMouseCode = GLFW_MOUSE_BUTTON_8;

    using KeyBitset = std::bitset<k_largestKeyCode>;
    using MouseBitset = std::bitset<k_largestMouseCode>;

    // Callbacks to retrieve input from glfw
    static void keyInputCallback(GLFWwindow *window, int key, int scancode,
                                 int action, int mods);
    static void mouseInputCallback(GLFWwindow *window, int button, int action,
                                   int mods);
    static void mousePositionCallback(GLFWwindow *window, double xpos,
                                      double ypos);

    inline bool isKeyDown(KeyCode key) const
    {
        return s_keyDown.test(static_cast<size_t>(key));
    }
    inline bool isKeyPressed(KeyCode key) const
    {
        return s_keyPressed.test(static_cast<size_t>(key));
    }

    inline bool isModifierDown() const {} // TODO

    inline bool isMouseKeyDown(MouseCode mouse) const
    {
        return s_mouseDown.test(static_cast<size_t>(mouse));
    }
    inline bool isMouseKeyPressed(MouseCode mouse) const
    {
        return s_mousePressed.test(static_cast<size_t>(mouse));
    }
    inline int32_t rawMouseDeltaX() const { return m_rawMouseDelta.x; }
    inline int32_t rawMouseDeltaY() const { return m_rawMouseDelta.y; }
    inline glm::ivec2 rawMouseDelta() const { return m_rawMouseDelta; }

    void computeRawMouseDelta();

    void resetInput();

  private:
    static KeyBitset s_keyDown;
    static KeyBitset s_keyPressed;

    static MouseBitset s_mouseDown;
    static MouseBitset s_mousePressed;

    static glm::vec2 s_mousePos;

    glm::vec2 m_prevMousePos;
    glm::ivec2 m_rawMouseDelta;
};

enum class KeyCode : uint32_t {
    // Printable keys
    Space = GLFW_KEY_SPACE,
    Apostrophe = GLFW_KEY_APOSTROPHE, // '
    Comma = GLFW_KEY_COMMA,           // ,
    Minus = GLFW_KEY_MINUS,           // -
    Period = GLFW_KEY_PERIOD,         // .
    Slash = GLFW_KEY_SLASH,           // /
    Zero = GLFW_KEY_0,
    One = GLFW_KEY_1,
    Two = GLFW_KEY_2,
    Three = GLFW_KEY_3,
    Four = GLFW_KEY_4,
    Five = GLFW_KEY_5,
    Six = GLFW_KEY_6,
    Seven = GLFW_KEY_7,
    Eight = GLFW_KEY_8,
    Nine = GLFW_KEY_9,
    Semicolon = GLFW_KEY_SEMICOLON, // ;
    Equal = GLFW_KEY_EQUAL,         // =
    A = GLFW_KEY_A,
    B = GLFW_KEY_B,
    C = GLFW_KEY_C,
    D = GLFW_KEY_D,
    E = GLFW_KEY_E,
    F = GLFW_KEY_F,
    G = GLFW_KEY_G,
    H = GLFW_KEY_H,
    I = GLFW_KEY_I,
    J = GLFW_KEY_J,
    K = GLFW_KEY_K,
    L = GLFW_KEY_L,
    M = GLFW_KEY_M,
    N = GLFW_KEY_N,
    O = GLFW_KEY_O,
    P = GLFW_KEY_P,
    Q = GLFW_KEY_Q,
    R = GLFW_KEY_R,
    S = GLFW_KEY_S,
    T = GLFW_KEY_T,
    U = GLFW_KEY_U,
    V = GLFW_KEY_V,
    W = GLFW_KEY_W,
    X = GLFW_KEY_X,
    Y = GLFW_KEY_Y,
    Z = GLFW_KEY_Z,
    LeftBracket = GLFW_KEY_LEFT_BRACKET,   // [
    Backslash = GLFW_KEY_BACKSLASH,        // \  /**/
    RightBracket = GLFW_KEY_RIGHT_BRACKET, // ]
    Tilde = GLFW_KEY_GRAVE_ACCENT,         // `
    World1 = GLFW_KEY_WORLD_1,             // non-US #1
    World2 = GLFW_KEY_WORLD_2,             // non-US #2

    // Function keys
    Escape = GLFW_KEY_ESCAPE,
    Enter = GLFW_KEY_ENTER,
    Tab = GLFW_KEY_TAB,
    Backspace = GLFW_KEY_BACKSPACE,
    Insert = GLFW_KEY_INSERT,
    Delete = GLFW_KEY_DELETE,
    Right = GLFW_KEY_RIGHT,
    Left = GLFW_KEY_LEFT,
    Down = GLFW_KEY_DOWN,
    Up = GLFW_KEY_UP,
    PageUp = GLFW_KEY_PAGE_UP,
    PageDown = GLFW_KEY_PAGE_DOWN,
    Home = GLFW_KEY_HOME,
    End = GLFW_KEY_END,
    CapsLock = GLFW_KEY_CAPS_LOCK,
    ScrollLock = GLFW_KEY_SCROLL_LOCK,
    NumLock = GLFW_KEY_NUM_LOCK,
    PrintScreen = GLFW_KEY_PRINT_SCREEN,
    Pause = GLFW_KEY_PAUSE,
    F1 = GLFW_KEY_F1,
    F2 = GLFW_KEY_F2,
    F3 = GLFW_KEY_F3,
    F4 = GLFW_KEY_F4,
    F5 = GLFW_KEY_F5,
    F6 = GLFW_KEY_F6,
    F7 = GLFW_KEY_F7,
    F8 = GLFW_KEY_F8,
    F9 = GLFW_KEY_F9,
    F10 = GLFW_KEY_F10,
    F11 = GLFW_KEY_F11,
    F12 = GLFW_KEY_F12,
    F13 = GLFW_KEY_F13,
    F14 = GLFW_KEY_F14,
    F15 = GLFW_KEY_F15,
    F16 = GLFW_KEY_F16,
    F17 = GLFW_KEY_F17,
    F18 = GLFW_KEY_F18,
    F19 = GLFW_KEY_F19,
    F20 = GLFW_KEY_F20,
    F21 = GLFW_KEY_F21,
    F22 = GLFW_KEY_F22,
    F23 = GLFW_KEY_F23,
    F24 = GLFW_KEY_F24,
    F25 = GLFW_KEY_F25,
    Keypad0 = GLFW_KEY_KP_0,
    Keypad1 = GLFW_KEY_KP_1,
    Keypad2 = GLFW_KEY_KP_2,
    Keypad3 = GLFW_KEY_KP_3,
    Keypad4 = GLFW_KEY_KP_4,
    Keypad5 = GLFW_KEY_KP_5,
    Keypad6 = GLFW_KEY_KP_6,
    Keypad7 = GLFW_KEY_KP_7,
    Keypad8 = GLFW_KEY_KP_8,
    Keypad9 = GLFW_KEY_KP_9,
    KeypadDecimal = GLFW_KEY_KP_DECIMAL,
    KeypadDivide = GLFW_KEY_KP_DIVIDE,
    KeypadMultiply = GLFW_KEY_KP_MULTIPLY,
    KeypadSubtract = GLFW_KEY_KP_SUBTRACT,
    KeypadAdd = GLFW_KEY_KP_ADD,
    KeypadEnter = GLFW_KEY_KP_ENTER,
    KeypadEqual = GLFW_KEY_KP_EQUAL,
    LeftShift = GLFW_KEY_LEFT_SHIFT,
    LeftControl = GLFW_KEY_LEFT_CONTROL,
    LeftAlt = GLFW_KEY_LEFT_ALT,
    LeftSuper = GLFW_KEY_LEFT_SUPER,
    RightShift = GLFW_KEY_RIGHT_SHIFT,
    RightControl = GLFW_KEY_RIGHT_CONTROL,
    RightAlt = GLFW_KEY_RIGHT_ALT,
    RightSuper = GLFW_KEY_RIGHT_SUPER,
    Menu = GLFW_KEY_MENU,

    Count
};

enum class MouseCode : uint32_t {
    Mouse1 = GLFW_MOUSE_BUTTON_1,
    Mouse2 = GLFW_MOUSE_BUTTON_2,
    Mouse3 = GLFW_MOUSE_BUTTON_3,
    Mouse4 = GLFW_MOUSE_BUTTON_4,
    Mouse5 = GLFW_MOUSE_BUTTON_5,
    Mouse6 = GLFW_MOUSE_BUTTON_6,
    Mouse7 = GLFW_MOUSE_BUTTON_7,
    Mouse8 = GLFW_MOUSE_BUTTON_8,

    Count
};
} // namespace RDE
