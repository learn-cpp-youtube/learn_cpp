#pragma once

#include <SDL.h>
#include <map>
#include <string>
#include <deque>
#include <cstdint>

namespace mi
{

enum class EventType
{
    QUIT,
    WINDOW_MINIMIZED,
    WINDOW_MAXIMIZED,
    WINDOW_RESTORED,
    TEXT,
    KEY_DOWN,
    KEY_UP,
    MOUSE_DOWN,
    MOUSE_UP,
    MOUSE_MOVED
};

enum class MouseButton
{
    LEFT   = 1 << 0,
    MIDDLE = 1 << 1,
    RIGHT  = 1 << 2
};

enum class Key
{
    KB_0 = 0,
    KB_1,
    KB_2,
    KB_3,
    KB_4,
    KB_5,
    KB_6,
    KB_7,
    KB_8,
    KB_9,
    A,
    B,
    C,
    D,
    E,
    F,
    G,
    H,
    I,
    J,
    K,
    L,
    M,
    N,
    O,
    P,
    Q,
    R,
    S,
    T,
    U,
    V,
    W,
    X,
    Y,
    Z,
    F1,
    F2,
    F3,
    F4,
    F5,
    F6,
    F7,
    F8,
    F9,
    F10,
    F11,
    F12,
    F13,
    F14,
    F15,
    F16,
    F17,
    F18,
    F19,
    F20,
    F21,
    F22,
    F23,
    F24,
    NUM_0,
    NUM_1,
    NUM_2,
    NUM_3,
    NUM_4,
    NUM_5,
    NUM_6,
    NUM_7,
    NUM_8,
    NUM_9,
    NUM_00,
    NUM_000,
    NUM_A,
    NUM_B,
    NUM_C,
    NUM_D,
    NUM_E,
    NUM_F,
    NUM_BINARY,
    NUM_OCTAL,
    NUM_DECIMAL,
    NUM_HEXADECIMAL,
    NUM_BACKSPACE,
    NUM_CLEAR,
    NUM_CLEARENTRY,
    NUM_ENTER,
    NUM_SPACE,
    NUM_TAB,
    NUM_PLUS,
    NUM_MINUS,
    NUM_MULTIPLY,
    NUM_DIVIDE,
    NUM_PERCENT,
    NUM_PERIOD,
    NUM_PLUSMINUS,
    NUM_POWER,
    NUM_AMPERSAND,
    NUM_VERTICALBAR,
    NUM_EXCLAM,
    NUM_HASH,
    NUM_AT,
    NUM_COLON,
    NUM_COMMA,
    NUM_LESS,
    NUM_GREATER,
    NUM_LEFTBRACE,
    NUM_RIGHTBRACE,
    NUM_LEFTPAREN,
    NUM_RIGHTPAREN,
    NUM_DBLAMPERSAND,
    NUM_DBLVERTICALBAR,
    NUM_XOR,
    NUM_EQUALSAS400,
    NUM_EQUALS,
    NUM_MEMADD,
    NUM_MEMSUBTRACT,
    NUM_MEMMULTIPLY,
    NUM_MEMDIVIDE,
    NUM_MEMRECALL,
    NUM_MEMSTORE,
    NUM_MEMCLEAR,
    AC_HOME,
    AC_BACK,
    AC_FORWARD,
    AC_SEARCH,
    AC_BOOKMARKS,
    AC_REFRESH,
    AC_STOP,
    AUDIOPLAY,
    AUDIOSTOP,
    AUDIOMUTE,
    AUDIONEXT,
    AUDIOPREV,
    VOLUMEDOWN,
    VOLUMEUP,
    LSHIFT,
    RSHIFT,
    LCTRL,
    RCTRL,
    LALT,
    RALT,
    LGUI,
    RGUI,
    SCROLLLOCK,
    CAPSLOCK,
    NUMLOCKCLEAR,
    UP,
    DOWN,
    LEFT,
    RIGHT,
    SPACE,
    BACKSPACE,
    DELETE,
    TAB,
    RETURN,
    RETURN2,
    PAGEDOWN,
    PAGEUP,
    HOME,
    END,
    INSERT,
    PRINTSCREEN,
    ESCAPE,
    COPY,
    CUT,
    PASTE,
    AMPERSAND,
    AT,
    CARET,
    PERIOD,
    COMMA,
    SEMICOLON,
    COLON,
    DOLLAR,
    EXCLAIM,
    HASH,
    QUESTION,
    BACKQUOTE,
    QUOTE,
    QUOTEDBL,
    UNDERSCORE,
    EQUALS,
    PLUS,
    MINUS,
    ASTERISK,
    PERCENT,
    SLASH,
    BACKSLASH,
    LEFTPAREN,
    RIGHTPAREN,
    LESS,
    GREATER,
    LEFTBRACKET,
    RIGHTBRACKET,
    WWW,
    UNDO,
    AGAIN,
    STOP,
    SELECT,
    KBDILLUMUP,
    KBDILLUMDOWN,
    KBDILLUMTOGGLE,
    BRIGHTNESSDOWN,
    BRIGHTNESSUP,
    MEDIASELECT,
    MAIL,
    CALCULATOR,
    SEPARATOR,
    DECIMALSEPARATOR,
    THOUSANDSSEPARATOR,
    CURRENCYSUBUNIT,
    CURRENCYUNIT,
    PRIOR,
    EXECUTE,
    EXSEL,
    FIND,
    HELP,
    MENU,
    EJECT,
    MUTE,
    PAUSE,
    OPER,
    OUT,
    MODE,
    DISPLAYSWITCH,
    POWER,
    SLEEP,
    SYSREQ,
    ALTERASE,
    APPLICATION,
    CANCEL,
    CLEAR,
    CLEARAGAIN,
    COMPUTER,
    CRSEL,
    UNKNOWN
};

class Event
{
public:
    EventType    type;
    Key          key;     // Only used by KEY_UP and KEY_DOWN.
    std::uint8_t utf8[5]; // Only used by TEXT, NULL terminated utf-8 character.
    MouseButton  button;  // Only used by MOUSE_UP and MOUSE_DOWN.
    std::int32_t x;       // Only used by MOUSE events.
    std::int32_t y;       // Only used by MOUSE events.
};

class EventHandler
{
public:
    static void Init();
    void Update();
    
    static std::string KeyToString(Key key);
    static std::string ButtonToString(MouseButton button);

    static void GetMouseState(std::int32_t& buttonsPressed, std::int32_t& x, std::int32_t& y);

    std::deque<Event> events;

private:
    static std::map<SDL_Keycode, Key>          sdlToKey;
    static std::map<Key, std::string>          keyToStr;

    static std::map<std::uint8_t, MouseButton> sdlToButton;
    static std::map<MouseButton, std::string>  buttonToStr;
};

} // End of namespace mi.