#include "Event.h"
#include <SDL.h>
#include <vector>
#include <tuple>
#include <string>
#include <map>
#include <cstdint>

using std::vector;
using std::tuple;
using std::make_tuple;
using std::get;
using std::string;
using std::map;
using std::uint8_t;

namespace mi
{

map<SDL_Keycode, Key>     EventHandler::sdlToKey;
map<Key, string>          EventHandler::keyToStr;
map<uint8_t, MouseButton> EventHandler::sdlToButton;
map<MouseButton, string>  EventHandler::buttonToStr;

void EventHandler::Init()
{
    sdlToKey.clear();
    keyToStr.clear();
    sdlToButton.clear();
    buttonToStr.clear();

    vector<tuple<string, SDL_Keycode, Key>> keyMappingData =
        {
            make_tuple("0",                 SDLK_0,                  Key::KB_0              ),
            make_tuple("1",                 SDLK_1,                  Key::KB_1              ),
            make_tuple("2",                 SDLK_2,                  Key::KB_2              ),
            make_tuple("3",                 SDLK_3,                  Key::KB_3              ),
            make_tuple("4",                 SDLK_4,                  Key::KB_4              ),
            make_tuple("5",                 SDLK_5,                  Key::KB_5              ),
            make_tuple("6",                 SDLK_6,                  Key::KB_6              ),
            make_tuple("7",                 SDLK_7,                  Key::KB_7              ),
            make_tuple("8",                 SDLK_8,                  Key::KB_8              ),
            make_tuple("9",                 SDLK_9,                  Key::KB_9              ),
            make_tuple("A",                 SDLK_a,                  Key::A                 ),
            make_tuple("B",                 SDLK_b,                  Key::B                 ),
            make_tuple("C",                 SDLK_c,                  Key::C                 ),
            make_tuple("D",                 SDLK_d,                  Key::D                 ),
            make_tuple("E",                 SDLK_e,                  Key::E                 ),
            make_tuple("F",                 SDLK_f,                  Key::F                 ),
            make_tuple("G",                 SDLK_g,                  Key::G                 ),
            make_tuple("H",                 SDLK_h,                  Key::H                 ),
            make_tuple("I",                 SDLK_i,                  Key::I                 ),
            make_tuple("J",                 SDLK_j,                  Key::J                 ),
            make_tuple("K",                 SDLK_k,                  Key::K                 ),
            make_tuple("L",                 SDLK_l,                  Key::L                 ),
            make_tuple("M",                 SDLK_m,                  Key::M                 ),
            make_tuple("N",                 SDLK_n,                  Key::N                 ),
            make_tuple("O",                 SDLK_o,                  Key::O                 ),
            make_tuple("P",                 SDLK_p,                  Key::P                 ),
            make_tuple("Q",                 SDLK_q,                  Key::Q                 ),
            make_tuple("R",                 SDLK_r,                  Key::R                 ),
            make_tuple("S",                 SDLK_s,                  Key::S                 ),
            make_tuple("T",                 SDLK_t,                  Key::T                 ),
            make_tuple("U",                 SDLK_u,                  Key::U                 ),
            make_tuple("V",                 SDLK_v,                  Key::V                 ),
            make_tuple("W",                 SDLK_w,                  Key::W                 ),
            make_tuple("X",                 SDLK_x,                  Key::X                 ),
            make_tuple("Y",                 SDLK_y,                  Key::Y                 ),
            make_tuple("Z",                 SDLK_z,                  Key::Z                 ),
            make_tuple("F1",                SDLK_F1,                 Key::F1                ),
            make_tuple("F2",                SDLK_F2,                 Key::F2                ),
            make_tuple("F3",                SDLK_F3,                 Key::F3                ),
            make_tuple("F4",                SDLK_F4,                 Key::F4                ),
            make_tuple("F5",                SDLK_F5,                 Key::F5                ),
            make_tuple("F6",                SDLK_F6,                 Key::F6                ),
            make_tuple("F7",                SDLK_F7,                 Key::F7                ),
            make_tuple("F8",                SDLK_F8,                 Key::F8                ),
            make_tuple("F9",                SDLK_F9,                 Key::F9                ),
            make_tuple("F10",               SDLK_F10,                Key::F10               ),
            make_tuple("F11",               SDLK_F11,                Key::F11               ),
            make_tuple("F12",               SDLK_F12,                Key::F12               ),
            make_tuple("F13",               SDLK_F13,                Key::F13               ),
            make_tuple("F14",               SDLK_F14,                Key::F14               ),
            make_tuple("F15",               SDLK_F15,                Key::F15               ),
            make_tuple("F16",               SDLK_F16,                Key::F16               ),
            make_tuple("F17",               SDLK_F17,                Key::F17               ),
            make_tuple("F18",               SDLK_F18,                Key::F18               ),
            make_tuple("F19",               SDLK_F19,                Key::F19               ),
            make_tuple("F20",               SDLK_F20,                Key::F20               ),
            make_tuple("F21",               SDLK_F21,                Key::F21               ),
            make_tuple("F22",               SDLK_F22,                Key::F22               ),
            make_tuple("F23",               SDLK_F23,                Key::F23               ),
            make_tuple("F24",               SDLK_F24,                Key::F24               ),
            make_tuple("0 (Num)",           SDLK_KP_0,               Key::NUM_0             ),
            make_tuple("1 (Num)",           SDLK_KP_1,               Key::NUM_1             ),
            make_tuple("2 (Num)",           SDLK_KP_2,               Key::NUM_2             ),
            make_tuple("3 (Num)",           SDLK_KP_3,               Key::NUM_3             ),
            make_tuple("4 (Num)",           SDLK_KP_4,               Key::NUM_4             ),
            make_tuple("5 (Num)",           SDLK_KP_5,               Key::NUM_5             ),
            make_tuple("6 (Num)",           SDLK_KP_6,               Key::NUM_6             ),
            make_tuple("7 (Num)",           SDLK_KP_7,               Key::NUM_7             ),
            make_tuple("8 (Num)",           SDLK_KP_8,               Key::NUM_8             ),
            make_tuple("9 (Num)",           SDLK_KP_9,               Key::NUM_9             ),
            make_tuple("00 (Num)",          SDLK_KP_00,              Key::NUM_00            ),
            make_tuple("000 (Num)",         SDLK_KP_000,             Key::NUM_000           ),
            make_tuple("A (Num)",           SDLK_KP_A,               Key::NUM_A             ),
            make_tuple("B (Num)",           SDLK_KP_B,               Key::NUM_B             ),
            make_tuple("C (Num)",           SDLK_KP_C,               Key::NUM_C             ),
            make_tuple("D (Num)",           SDLK_KP_D,               Key::NUM_D             ),
            make_tuple("E (Num)",           SDLK_KP_E,               Key::NUM_E             ),
            make_tuple("F (Num)",           SDLK_KP_F,               Key::NUM_F             ),
            make_tuple("Binary (Num)",      SDLK_KP_BINARY,          Key::NUM_BINARY        ),
            make_tuple("Octal (Num)",       SDLK_KP_OCTAL,           Key::NUM_OCTAL         ),
            make_tuple("Decimal (Num)",     SDLK_KP_DECIMAL,         Key::NUM_DECIMAL       ),
            make_tuple("Hexadecimal (Num)", SDLK_KP_HEXADECIMAL,     Key::NUM_HEXADECIMAL   ),
            make_tuple("Backspace (Num)",   SDLK_KP_BACKSPACE,       Key::NUM_BACKSPACE     ),
            make_tuple("Clear (Num)",       SDLK_KP_CLEAR,           Key::NUM_CLEAR         ),
            make_tuple("Clear Entry (Num)", SDLK_KP_CLEARENTRY,      Key::NUM_CLEARENTRY    ),
            make_tuple("Enter (Num)",       SDLK_KP_ENTER,           Key::NUM_ENTER         ),
            make_tuple("Space (Num)",       SDLK_KP_SPACE,           Key::NUM_SPACE         ),
            make_tuple("Tab (Num)",         SDLK_KP_TAB,             Key::NUM_TAB           ),
            make_tuple("+ (Num)",           SDLK_KP_PLUS,            Key::NUM_PLUS          ),
            make_tuple("- (Num)",           SDLK_KP_MINUS,           Key::NUM_MINUS         ),
            make_tuple("* (Num)",           SDLK_KP_MULTIPLY,        Key::NUM_MULTIPLY      ),
            make_tuple("/ (Num)",           SDLK_KP_DIVIDE,          Key::NUM_DIVIDE        ),
            make_tuple("% (Num)",           SDLK_KP_PERCENT,         Key::NUM_PERCENT       ),
            make_tuple(". (Num)",           SDLK_KP_PERIOD,          Key::NUM_PERIOD        ),
            make_tuple("+/- (Num)",         SDLK_KP_PLUSMINUS,       Key::NUM_PLUSMINUS     ),
            make_tuple("^ (Num)",           SDLK_KP_POWER,           Key::NUM_POWER         ),
            make_tuple("& (Num)",           SDLK_KP_AMPERSAND,       Key::NUM_AMPERSAND     ),
            make_tuple("| (Num)",           SDLK_KP_VERTICALBAR,     Key::NUM_VERTICALBAR   ),
            make_tuple("! (Num)",           SDLK_KP_EXCLAM,          Key::NUM_EXCLAM        ),
            make_tuple("# (Num)",           SDLK_KP_HASH,            Key::NUM_HASH          ),
            make_tuple("@ (Num)",           SDLK_KP_AT,              Key::NUM_AT            ),
            make_tuple(": (Num)",           SDLK_KP_COLON,           Key::NUM_COLON         ),
            make_tuple(", (Num)",           SDLK_KP_COMMA,           Key::NUM_COMMA         ),
            make_tuple("< (Num)",           SDLK_KP_LESS,            Key::NUM_LESS          ),
            make_tuple("> (Num)",           SDLK_KP_GREATER,         Key::NUM_GREATER       ),
            make_tuple("{ (Num)",           SDLK_KP_LEFTBRACE,       Key::NUM_LEFTBRACE     ),
            make_tuple("} (Num)",           SDLK_KP_RIGHTBRACE,      Key::NUM_RIGHTBRACE    ),
            make_tuple("( (Num)",           SDLK_KP_LEFTPAREN,       Key::NUM_LEFTPAREN     ),
            make_tuple(") (Num)",           SDLK_KP_RIGHTPAREN,      Key::NUM_RIGHTPAREN    ),
            make_tuple("&& (Num)",          SDLK_KP_DBLAMPERSAND,    Key::NUM_DBLAMPERSAND  ),
            make_tuple("|| (Num)",          SDLK_KP_DBLVERTICALBAR,  Key::NUM_DBLVERTICALBAR),
            make_tuple("XOR (Num)",         SDLK_KP_XOR,             Key::NUM_XOR           ),
            make_tuple("=AS400 (Num)",      SDLK_KP_EQUALSAS400,     Key::NUM_EQUALSAS400   ),
            make_tuple("= (Num)",           SDLK_KP_EQUALS,          Key::NUM_EQUALS        ),
            make_tuple("M+ (Num)",          SDLK_KP_MEMADD,          Key::NUM_MEMADD        ),
            make_tuple("M- (Num)",          SDLK_KP_MEMSUBTRACT,     Key::NUM_MEMSUBTRACT   ),
            make_tuple("M* (Num)",          SDLK_KP_MEMMULTIPLY,     Key::NUM_MEMMULTIPLY   ),
            make_tuple("M/ (Num)",          SDLK_KP_MEMDIVIDE,       Key::NUM_MEMDIVIDE     ),
            make_tuple("MR (Num)",          SDLK_KP_MEMRECALL,       Key::NUM_MEMRECALL     ),
            make_tuple("MS (Num)",          SDLK_KP_MEMSTORE,        Key::NUM_MEMSTORE      ),
            make_tuple("MC (Num)",          SDLK_KP_MEMCLEAR,        Key::NUM_MEMCLEAR      ),
            make_tuple("Home (App)",        SDLK_AC_HOME,            Key::AC_HOME           ),
            make_tuple("Back (App)",        SDLK_AC_BACK,            Key::AC_BACK           ),
            make_tuple("Forward (App)",     SDLK_AC_FORWARD,         Key::AC_FORWARD        ),
            make_tuple("Search (App)",      SDLK_AC_SEARCH,          Key::AC_SEARCH         ),
            make_tuple("Bookmarks (App)",   SDLK_AC_BOOKMARKS,       Key::AC_BOOKMARKS      ),
            make_tuple("Refresh (App)",     SDLK_AC_REFRESH,         Key::AC_REFRESH        ),
            make_tuple("Stop (App)",        SDLK_AC_STOP,            Key::AC_STOP           ),
            make_tuple("Play (Media)",      SDLK_AUDIOPLAY,          Key::AUDIOPLAY         ),
            make_tuple("Stop (Media)",      SDLK_AUDIOSTOP,          Key::AUDIOSTOP         ),
            make_tuple("Mute (Media)",      SDLK_AUDIOMUTE,          Key::AUDIOMUTE         ),
            make_tuple("Next (Media)",      SDLK_AUDIONEXT,          Key::AUDIONEXT         ),
            make_tuple("Prev (Media)",      SDLK_AUDIOPREV,          Key::AUDIOPREV         ),
            make_tuple("VolDown (Media)",   SDLK_VOLUMEDOWN,         Key::VOLUMEDOWN        ),
            make_tuple("VolUp (Media)",     SDLK_VOLUMEUP,           Key::VOLUMEUP          ),
            make_tuple("Left Shift",        SDLK_LSHIFT,             Key::LSHIFT            ),
            make_tuple("Right Shift",       SDLK_RSHIFT,             Key::RSHIFT            ),
            make_tuple("Left Ctrl",         SDLK_LCTRL,              Key::LCTRL             ),
            make_tuple("Right Ctrl",        SDLK_RCTRL,              Key::RCTRL             ),
            make_tuple("Left Alt",          SDLK_LALT,               Key::LALT              ),
            make_tuple("Right Alt",         SDLK_RALT,               Key::RALT              ),
            make_tuple("Left GUI",          SDLK_LGUI,               Key::LGUI              ),
            make_tuple("Right GUI",         SDLK_RGUI,               Key::RGUI              ),
            make_tuple("ScrollLock",        SDLK_SCROLLLOCK,         Key::SCROLLLOCK        ),
            make_tuple("CapsLock",          SDLK_CAPSLOCK,           Key::CAPSLOCK          ),
            make_tuple("Numlock",           SDLK_NUMLOCKCLEAR,       Key::NUMLOCKCLEAR      ),
            make_tuple("Up",                SDLK_UP,                 Key::UP                ),
            make_tuple("Down",              SDLK_DOWN,               Key::DOWN              ),
            make_tuple("Left",              SDLK_LEFT,               Key::LEFT              ),
            make_tuple("Right",             SDLK_RIGHT,              Key::RIGHT             ),
            make_tuple("Space",             SDLK_SPACE,              Key::SPACE             ),
            make_tuple("Backspace",         SDLK_BACKSPACE,          Key::BACKSPACE         ),
            make_tuple("Delete",            SDLK_DELETE,             Key::DELETE            ),
            make_tuple("Tab",               SDLK_TAB,                Key::TAB               ),
            make_tuple("Return",            SDLK_RETURN,             Key::RETURN            ),
            make_tuple("Return(2)",         SDLK_RETURN2,            Key::RETURN2           ),
            make_tuple("PageDown",          SDLK_PAGEDOWN,           Key::PAGEDOWN          ),
            make_tuple("PageUp",            SDLK_PAGEUP,             Key::PAGEUP            ),
            make_tuple("Home",              SDLK_HOME,               Key::HOME              ),
            make_tuple("End",               SDLK_END,                Key::END               ),
            make_tuple("Insert",            SDLK_INSERT,             Key::INSERT            ),
            make_tuple("PrintScreen",       SDLK_PRINTSCREEN,        Key::PRINTSCREEN       ),
            make_tuple("Escape",            SDLK_ESCAPE,             Key::ESCAPE            ),
            make_tuple("Copy",              SDLK_COPY,               Key::COPY              ),
            make_tuple("Cut",               SDLK_CUT,                Key::CUT               ),
            make_tuple("Paste",             SDLK_PASTE,              Key::PASTE             ),
            make_tuple("&",                 SDLK_AMPERSAND,          Key::AMPERSAND         ),
            make_tuple("@",                 SDLK_AT,                 Key::AT                ),
            make_tuple("^",                 SDLK_CARET,              Key::CARET             ),
            make_tuple(".",                 SDLK_PERIOD,             Key::PERIOD            ),
            make_tuple(",",                 SDLK_COMMA,              Key::COMMA             ),
            make_tuple(";",                 SDLK_SEMICOLON,          Key::SEMICOLON         ),
            make_tuple(":",                 SDLK_COLON,              Key::COLON             ),
            make_tuple("$",                 SDLK_DOLLAR,             Key::DOLLAR            ),
            make_tuple("!",                 SDLK_EXCLAIM,            Key::EXCLAIM           ),
            make_tuple("#",                 SDLK_HASH,               Key::HASH              ),
            make_tuple("?",                 SDLK_QUESTION,           Key::QUESTION          ),
            make_tuple("`",                 SDLK_BACKQUOTE,          Key::BACKQUOTE         ),
            make_tuple("'",                 SDLK_QUOTE,              Key::QUOTE             ),
            make_tuple("\"",                SDLK_QUOTEDBL,           Key::QUOTEDBL          ),
            make_tuple("_",                 SDLK_UNDERSCORE,         Key::UNDERSCORE        ),
            make_tuple("=",                 SDLK_EQUALS,             Key::EQUALS            ),
            make_tuple("+",                 SDLK_PLUS,               Key::PLUS              ),
            make_tuple("-",                 SDLK_MINUS,              Key::MINUS             ),
            make_tuple("*",                 SDLK_ASTERISK,           Key::ASTERISK          ),
            make_tuple("%",                 SDLK_PERCENT,            Key::PERCENT           ),
            make_tuple("/",                 SDLK_SLASH,              Key::SLASH             ),
            make_tuple("\\",                SDLK_BACKSLASH,          Key::BACKSLASH         ),
            make_tuple("(",                 SDLK_LEFTPAREN,          Key::LEFTPAREN         ),
            make_tuple(")",                 SDLK_RIGHTPAREN,         Key::RIGHTPAREN        ),
            make_tuple("<",                 SDLK_LESS,               Key::LESS              ),
            make_tuple(">",                 SDLK_GREATER,            Key::GREATER           ),
            make_tuple("[",                 SDLK_LEFTBRACKET,        Key::LEFTBRACKET       ),
            make_tuple("]",                 SDLK_RIGHTBRACKET,       Key::RIGHTBRACKET      ),
            make_tuple("WWW",               SDLK_WWW,                Key::WWW               ),
            make_tuple("Undo",              SDLK_UNDO,               Key::UNDO              ),
            make_tuple("Redo",              SDLK_AGAIN,              Key::AGAIN             ),
            make_tuple("Stop",              SDLK_STOP,               Key::STOP              ),
            make_tuple("Select",            SDLK_SELECT,             Key::SELECT            ),
            make_tuple("Key Lights Up",     SDLK_KBDILLUMUP,         Key::KBDILLUMUP        ),
            make_tuple("Key Lights Down",   SDLK_KBDILLUMDOWN,       Key::KBDILLUMDOWN      ),
            make_tuple("Key Lights Toggle", SDLK_KBDILLUMTOGGLE,     Key::KBDILLUMTOGGLE    ),
            make_tuple("BrightnessDown",    SDLK_BRIGHTNESSDOWN,     Key::BRIGHTNESSDOWN    ),
            make_tuple("BrightnessUp",      SDLK_BRIGHTNESSUP,       Key::BRIGHTNESSUP      ),
            make_tuple("MediaSelect",       SDLK_MEDIASELECT,        Key::MEDIASELECT       ),
            make_tuple("Mail",              SDLK_MAIL,               Key::MAIL              ),
            make_tuple("Calculator",        SDLK_CALCULATOR,         Key::CALCULATOR        ),
            make_tuple("Separator",         SDLK_SEPARATOR,          Key::SEPARATOR         ),
            make_tuple("Decimal Sep",       SDLK_DECIMALSEPARATOR,   Key::DECIMALSEPARATOR  ),
            make_tuple("Thousands Sep",     SDLK_THOUSANDSSEPARATOR, Key::THOUSANDSSEPARATOR),
            make_tuple("Currency Sub",      SDLK_CURRENCYSUBUNIT,    Key::CURRENCYSUBUNIT   ),
            make_tuple("Currency",          SDLK_CURRENCYUNIT,       Key::CURRENCYUNIT      ),
            make_tuple("Prior",             SDLK_PRIOR,              Key::PRIOR             ),
            make_tuple("Execute",           SDLK_EXECUTE,            Key::EXECUTE           ),
            make_tuple("ExSel",             SDLK_EXSEL,              Key::EXSEL             ),
            make_tuple("Find",              SDLK_FIND,               Key::FIND              ),
            make_tuple("Help",              SDLK_HELP,               Key::HELP              ),
            make_tuple("Menu",              SDLK_MENU,               Key::MENU              ),
            make_tuple("Eject",             SDLK_EJECT,              Key::EJECT             ),
            make_tuple("Mute",              SDLK_MUTE,               Key::MUTE              ),
            make_tuple("Pause",             SDLK_PAUSE,              Key::PAUSE             ),
            make_tuple("Oper",              SDLK_OPER,               Key::OPER              ),
            make_tuple("Out",               SDLK_OUT,                Key::OUT               ),
            make_tuple("Mode Switch",       SDLK_MODE,               Key::MODE              ),
            make_tuple("Display Switch",    SDLK_DISPLAYSWITCH,      Key::DISPLAYSWITCH     ),
            make_tuple("Power",             SDLK_POWER,              Key::POWER             ),
            make_tuple("Sleep",             SDLK_SLEEP,              Key::SLEEP             ),
            make_tuple("SysReq",            SDLK_SYSREQ,             Key::SYSREQ            ),
            make_tuple("Alt Erase",         SDLK_ALTERASE,           Key::ALTERASE          ),
            make_tuple("Application",       SDLK_APPLICATION,        Key::APPLICATION       ),
            make_tuple("Cancel",            SDLK_CANCEL,             Key::CANCEL            ),
            make_tuple("Clear",             SDLK_CLEAR,              Key::CLEAR             ),
            make_tuple("Clear/Again",       SDLK_CLEARAGAIN,         Key::CLEARAGAIN        ),
            make_tuple("Computer",          SDLK_COMPUTER,           Key::COMPUTER          ),
            make_tuple("CrSel",             SDLK_CRSEL,              Key::CRSEL             ),
            make_tuple("Unknown",           SDLK_UNKNOWN,            Key::UNKNOWN           )
        };

    vector<tuple<string, uint8_t, MouseButton>> buttonMappingData =
        {
            make_tuple("Mouse Button Left",    SDL_BUTTON_LEFT,   MouseButton::LEFT),
            make_tuple("Mouse Button Middle",  SDL_BUTTON_MIDDLE, MouseButton::MIDDLE),
            make_tuple("Mouse Button Right",   SDL_BUTTON_RIGHT,  MouseButton::RIGHT)
        };

    for (size_t i=0; i<keyMappingData.size(); i++)
    {
        sdlToKey[get<1>(keyMappingData[i])] = get<2>(keyMappingData[i]);
        keyToStr[get<2>(keyMappingData[i])] = get<0>(keyMappingData[i]);
    }

    for (size_t i=0; i<buttonMappingData.size(); i++)
    {
        sdlToButton[get<1>(buttonMappingData[i])] = get<2>(buttonMappingData[i]);
        buttonToStr[get<2>(buttonMappingData[i])] = get<0>(buttonMappingData[i]);
    }

    return;
}

void EventHandler::Update()
{
    SDL_Event sdlEvent;
    Event     event;
    while(SDL_PollEvent(&sdlEvent) != 0)
    {
        if (sdlEvent.type == SDL_QUIT)
        {
            event.type = EventType::QUIT;
            events.push_back(event);
            continue;
        }

        if (sdlEvent.type == SDL_KEYUP || sdlEvent.type == SDL_KEYDOWN)
        {
            if (sdlEvent.key.repeat)
                continue; // Ignore repeated key presses.

            auto it = sdlToKey.find(sdlEvent.key.keysym.sym);
            if (it == sdlToKey.end())
                continue;

            event.type = (sdlEvent.type == SDL_KEYUP)?EventType::KEY_UP:EventType::KEY_DOWN;
            event.key  = it->second;
            events.push_back(event);
            continue;
        }

        if (sdlEvent.type == SDL_TEXTINPUT)
        {
            event.type = EventType::TEXT;
            for (long i=0; i<5; i++)
                event.utf8[i] = 0;

            if ((static_cast<uint8_t>(sdlEvent.text.text[0]) & 0x80) == 0)
            {
                event.utf8[0] = static_cast<uint8_t>(sdlEvent.text.text[0]);
            }
            else
            if (   (static_cast<uint8_t>(sdlEvent.text.text[0]) & 0xe0) == 0xc0
                && (static_cast<uint8_t>(sdlEvent.text.text[1]) & 0xc0) == 0x80)
            {
                event.utf8[0] = static_cast<uint8_t>(sdlEvent.text.text[0]);
                event.utf8[1] = static_cast<uint8_t>(sdlEvent.text.text[1]);
            }
            else
            if (   (static_cast<uint8_t>(sdlEvent.text.text[0]) & 0xf0) == 0xe0
                && (static_cast<uint8_t>(sdlEvent.text.text[1]) & 0xc0) == 0x80
                && (static_cast<uint8_t>(sdlEvent.text.text[2]) & 0xc0) == 0x80)
            {
                event.utf8[0] = static_cast<uint8_t>(sdlEvent.text.text[0]);
                event.utf8[1] = static_cast<uint8_t>(sdlEvent.text.text[1]);
                event.utf8[2] = static_cast<uint8_t>(sdlEvent.text.text[2]);
            }
            else
            if (   (static_cast<uint8_t>(sdlEvent.text.text[0]) & 0xf8) == 0xf0
                && (static_cast<uint8_t>(sdlEvent.text.text[1]) & 0xc0) == 0x80
                && (static_cast<uint8_t>(sdlEvent.text.text[2]) & 0xc0) == 0x80
                && (static_cast<uint8_t>(sdlEvent.text.text[3]) & 0xc0) == 0x80)
            {
                event.utf8[0] = static_cast<uint8_t>(sdlEvent.text.text[0]);
                event.utf8[1] = static_cast<uint8_t>(sdlEvent.text.text[1]);
                event.utf8[2] = static_cast<uint8_t>(sdlEvent.text.text[2]);
                event.utf8[3] = static_cast<uint8_t>(sdlEvent.text.text[3]);
            }
            else
                continue;

            events.push_back(event);
            continue;
        }

        if (sdlEvent.type == SDL_MOUSEBUTTONDOWN || sdlEvent.type == SDL_MOUSEBUTTONUP)
        {
            if (sdlEvent.type == SDL_MOUSEBUTTONDOWN)
                event.type = EventType::MOUSE_DOWN;
            else 
                event.type = EventType::MOUSE_UP;

            auto it = sdlToButton.find(sdlEvent.button.button);
            if (it == sdlToButton.end())
                continue;

            event.button = it->second;
            event.x      = sdlEvent.button.x;
            event.y      = sdlEvent.button.y;

            events.push_back(event);
            continue;
        }

        if (sdlEvent.type == SDL_MOUSEMOTION)
        {
            event.type = EventType::MOUSE_MOVED;
            event.x    = sdlEvent.motion.x;
            event.y    = sdlEvent.motion.y;
            events.push_back(event);
            continue;
        }

        if (sdlEvent.type == SDL_WINDOWEVENT)
        {
            if (sdlEvent.window.event == SDL_WINDOWEVENT_MINIMIZED)
            {
                event.type = EventType::WINDOW_MINIMIZED;
                events.push_back(event);
                continue;
            }

            if (sdlEvent.window.event == SDL_WINDOWEVENT_MAXIMIZED)
            {
                event.type = EventType::WINDOW_MAXIMIZED;
                events.push_back(event);
                continue;
            }

            if (sdlEvent.window.event == SDL_WINDOWEVENT_RESTORED)
            {
                event.type = EventType::WINDOW_RESTORED;
                events.push_back(event);
                continue;
            }
        }
    }

    return;
}

string EventHandler::KeyToString(Key key)
{
    auto it = keyToStr.find(key);

    if (it == keyToStr.end())
        return "Unknown";

    return it->second;
}

string EventHandler::ButtonToString(MouseButton button)
{
    auto it = buttonToStr.find(button);

    if (it == buttonToStr.end())
        return "Unknown";

    return it->second;
}

void EventHandler::GetMouseState(std::int32_t& buttonsPressed, std::int32_t& x, std::int32_t& y)
{
    int xPos;
    int yPos;

    std::uint32_t state = SDL_GetMouseState(&xPos, &yPos);
    
    x = static_cast<std::int32_t>(xPos);
    y = static_cast<std::int32_t>(yPos);
    
    buttonsPressed = 0;
    for (const auto& button : sdlToButton)
    {
        if (SDL_BUTTON(button.first) & state)
            buttonsPressed |= static_cast<std::int32_t>(button.second);
    }

    return;
}

} // End of namespace mi.