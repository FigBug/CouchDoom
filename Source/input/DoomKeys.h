#pragma once

//==============================================================================
// The subset of Doom KEY_* codes CouchDoom sends via DoomHost::postKey (which
// forwards them straight to the game, no remapping). Values mirror
// doomgeneric/doomkeys.h. Weapons are the plain ASCII digits '1'..'7'.
//==============================================================================
namespace dk
{
    constexpr int RIGHTARROW = 0xae;   // turn right
    constexpr int LEFTARROW  = 0xac;   // turn left
    constexpr int UPARROW    = 0xad;   // forward
    constexpr int DOWNARROW  = 0xaf;   // back
    constexpr int STRAFE_L   = 0xa0;   // strafe left
    constexpr int STRAFE_R   = 0xa1;   // strafe right
    constexpr int USE        = 0xa2;   // use / open
    constexpr int FIRE       = 0xa3;   // fire
    constexpr int RSHIFT     = 0xb6;   // run / speed
    constexpr int ESCAPE     = 27;     // menu (key_menu_activate)
    constexpr int ENTER      = 13;     // menu select (key_menu_forward)
    constexpr int BACKSPACE  = 0x7f;   // menu back (key_menu_back)
    // Menu yes/no dialogs use the plain ASCII 'y'/'n' (key_menu_confirm/abort).
}
