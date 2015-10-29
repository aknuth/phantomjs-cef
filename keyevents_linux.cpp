// Copyright (c) 2015 Klaralvdalens Datakonsult AB (KDAB).
// All rights reserved. Use of this source code is governed by a BSD-style
// license that can be found in the LICENSE file.

#include "keyevents_linux.h"

#include "WindowsKeyboardCodes.h"

#include <X11/keysym.h>
#include <X11/XF86keysym.h>

#include "debug.h"

int vkToNative(int vk)
{
  switch (vk) {
    case VK_UNKNOWN: return 0;
    case VK_BACK: return XF86XK_Back;
    case VK_TAB: return XK_Tab;
    case VK_CLEAR: return XK_Clear;
    case VK_RETURN: return XK_Return;
    case VK_SHIFT: return XK_Shift_L;
    case VK_CONTROL: return XK_Control_L; // CTRL key
    case VK_MENU: return XK_Menu; // ALT key
    case VK_PAUSE: return XK_Pause; // PAUSE key
    case VK_CAPITAL: return XK_Caps_Lock; // CAPS LOCK key
    case VK_KANA: return XK_Katakana; // Input Method Editor (IME) Kana mode
//     case VK_HANGUL: return XK_Hangul; // IME Hangul mode
//     case VK_JUNJA: return XK_JUNJA; // IME Junja mode
//     case VK_FINAL: return XK_FINAL; // IME final mode
//     case VK_HANJA: return XK_HANJA; // IME Hanja mode
    case VK_KANJI: return XK_Kanji; // IME Kanji mode
    case VK_ESCAPE: return XK_Escape; // ESC key
//     case VK_CONVERT: return XK_CONVERT; // IME convert
//     case VK_NONCONVERT: return XK_NONCONVERT; // IME nonconvert
//     case VK_ACCEPT: return XK_ACCEPT; // IME accept
//     case VK_MODECHANGE: return XK_MODECHANGE; // IME mode change request
    case VK_SPACE: return XK_space; // SPACE key
    case VK_PRIOR: return XK_Prior; // PAGE UP key
    case VK_NEXT: return XK_Next; // PAGE DOWN key
    case VK_END: return XK_End; // END key
    case VK_HOME: return XK_Home; // HOME key
    case VK_LEFT: return XK_Left; // LEFT ARROW key
    case VK_UP: return XK_Up; // UP ARROW key
    case VK_RIGHT: return XK_Right; // RIGHT ARROW key
    case VK_DOWN: return XK_Down; // DOWN ARROW key
    case VK_SELECT: return XK_Select; // SELECT key
    case VK_PRINT: return XK_Print; // PRINT key
    case VK_EXECUTE: return XK_Execute; // EXECUTE key
    case VK_SNAPSHOT: return XK_Print; // PRINT SCREEN key
    case VK_INSERT: return XK_Insert; // INS key
    case VK_DELETE: return XK_Delete; // DEL key
    case VK_HELP: return XK_Help; // HELP key

    case VK_0: return XK_0;
    case VK_1: return XK_1;
    case VK_2: return XK_2;
    case VK_3: return XK_3;
    case VK_4: return XK_4;
    case VK_5: return XK_5;
    case VK_6: return XK_6;
    case VK_7: return XK_7;
    case VK_8: return XK_8;
    case VK_9: return XK_9;
    case VK_A: return XK_A;
    case VK_B: return XK_B;
    case VK_C: return XK_C;
    case VK_D: return XK_D;
    case VK_E: return XK_E;
    case VK_F: return XK_F;
    case VK_G: return XK_G;
    case VK_H: return XK_H;
    case VK_I: return XK_I;
    case VK_J: return XK_J;
    case VK_K: return XK_K;
    case VK_L: return XK_L;
    case VK_M: return XK_M;
    case VK_N: return XK_N;
    case VK_O: return XK_O;
    case VK_P: return XK_P;
    case VK_Q: return XK_Q;
    case VK_R: return XK_R;
    case VK_S: return XK_S;
    case VK_T: return XK_T;
    case VK_U: return XK_U;
    case VK_V: return XK_V;
    case VK_W: return XK_W;
    case VK_X: return XK_X;
    case VK_Y: return XK_Y;
    case VK_Z: return XK_Z;

    case VK_LWIN: return XK_Meta_L; // Left Windows key (Microsoft Natural keyboard)

    case VK_RWIN: return XK_Meta_R; // Right Windows key (Natural keyboard)

//     case VK_APPS: return XK_APPS; // Applications key (Natural keyboard)

    case VK_SLEEP: return XF86XK_Sleep; // Computer Sleep key

    // Num pad keys
    case VK_NUMPAD0: return XK_KP_0;
    case VK_NUMPAD1: return XK_KP_1;
    case VK_NUMPAD2: return XK_KP_2;
    case VK_NUMPAD3: return XK_KP_3;
    case VK_NUMPAD4: return XK_KP_4;
    case VK_NUMPAD5: return XK_KP_5;
    case VK_NUMPAD6: return XK_KP_6;
    case VK_NUMPAD7: return XK_KP_7;
    case VK_NUMPAD8: return XK_KP_8;
    case VK_NUMPAD9: return XK_KP_9;
    case VK_MULTIPLY: return XK_KP_Multiply;
    case VK_ADD: return XK_KP_Add;
    case VK_SEPARATOR: return XK_KP_Separator;
    case VK_SUBTRACT: return XK_KP_Subtract;
    case VK_DECIMAL: return XK_KP_Decimal;
    case VK_DIVIDE: return XK_KP_Divide;

    case VK_F1: return XK_F1;
    case VK_F2: return XK_F2;
    case VK_F3: return XK_F3;
    case VK_F4: return XK_F4;
    case VK_F5: return XK_F5;
    case VK_F6: return XK_F6;
    case VK_F7: return XK_F7;
    case VK_F8: return XK_F8;
    case VK_F9: return XK_F9;
    case VK_F10: return XK_F10;
    case VK_F11: return XK_F11;
    case VK_F12: return XK_F12;
    case VK_F13: return XK_F13;
    case VK_F14: return XK_F14;
    case VK_F15: return XK_F15;
    case VK_F16: return XK_F16;
    case VK_F17: return XK_F17;
    case VK_F18: return XK_F18;
    case VK_F19: return XK_F19;
    case VK_F20: return XK_F20;
    case VK_F21: return XK_F21;
    case VK_F22: return XK_F22;
    case VK_F23: return XK_F23;
    case VK_F24: return XK_F24;

    case VK_NUMLOCK: return XK_Num_Lock;
    case VK_SCROLL: return XK_Scroll_Lock;
    case VK_LSHIFT: return XK_Shift_L;
    case VK_RSHIFT: return XK_Shift_R;
    case VK_LCONTROL: return XK_Control_L;
    case VK_RCONTROL: return XK_Control_R;
    case VK_LMENU: return XK_Menu;
    case VK_RMENU: return XK_Menu;

    case VK_BROWSER_BACK: return XF86XK_Back; // Windows 2000/XP: Browser Back key
    case VK_BROWSER_FORWARD: return XF86XK_Forward; // Windows 2000/XP: Browser Forward key
    case VK_BROWSER_REFRESH: return XF86XK_Refresh; // Windows 2000/XP: Browser Refresh key
    case VK_BROWSER_STOP: return XF86XK_Stop; // Windows 2000/XP: Browser Stop key
    case VK_BROWSER_SEARCH: return XF86XK_Search; // Windows 2000/XP: Browser Search key
    case VK_BROWSER_FAVORITES: return XF86XK_Favorites; // Windows 2000/XP: Browser Favorites key
    case VK_BROWSER_HOME: return XF86XK_HomePage; // Windows 2000/XP: Browser Start and Home key
    case VK_VOLUME_MUTE: return XF86XK_AudioMute; // Windows 2000/XP: Volume Mute key
    case VK_VOLUME_DOWN: return XF86XK_AudioLowerVolume; // Windows 2000/XP: Volume Down key
    case VK_VOLUME_UP: return XF86XK_AudioRaiseVolume; // Windows 2000/XP: Volume Up key
    case VK_MEDIA_NEXT_TRACK: return XF86XK_AudioNext; // Windows 2000/XP: Next Track key
    case VK_MEDIA_PREV_TRACK: return XF86XK_AudioPrev; // Windows 2000/XP: Previous Track key
    case VK_MEDIA_STOP: return XF86XK_AudioStop; // Windows 2000/XP: Stop Media key
    case VK_MEDIA_PLAY_PAUSE: return XF86XK_AudioPause; // Windows 2000/XP: Play/Pause Media key
    case VK_MEDIA_LAUNCH_MAIL: return XF86XK_Mail; // Windows 2000/XP: Start Mail key
    case VK_MEDIA_LAUNCH_MEDIA_SELECT: return XF86XK_AudioMedia; // Windows 2000/XP: Select Media key
    case VK_MEDIA_LAUNCH_APP1: return XF86XK_Launch0; // VK_LAUNCH_APP1 (B6) Windows 2000/XP: Start Application 1 key
    case VK_MEDIA_LAUNCH_APP2: return XF86XK_Launch1; // VK_LAUNCH_APP2 (B7) Windows 2000/XP: Start Application 2 key

    // Windows 2000/XP: For any country/region, the '+' key
    case VK_OEM_PLUS: return  XK_plus;

    // Windows 2000/XP: For any country/region, the ',' key
    case VK_OEM_COMMA: return XK_comma;

    // Windows 2000/XP: For any country/region, the '-' key
    case VK_OEM_MINUS: return XK_minus;

    // Windows 2000/XP: For any country/region, the '.' key
    case VK_OEM_PERIOD: return XK_period;

    // VK_OEM_1 (BA) Used for miscellaneous characters; it can vary by keyboard. Windows 2000/XP: For the US standard keyboard, the ';:' key
//     case VK_OEM_1:
    // VK_OEM_2 (BF) Used for miscellaneous characters; it can vary by keyboard. Windows 2000/XP: For the US standard keyboard, the '/?' key
//     case VK_OEM_2:
    // VK_OEM_3 (C0) Used for miscellaneous characters; it can vary by keyboard. Windows 2000/XP: For the US standard keyboard, the '`~' key
//     case VK_OEM_3:
    // VK_OEM_4 (DB) Used for miscellaneous characters; it can vary by keyboard. Windows 2000/XP: For the US standard keyboard, the '[{' key
//     case VK_OEM_4:
    // VK_OEM_5 (DC) Used for miscellaneous characters; it can vary by keyboard. Windows 2000/XP: For the US standard keyboard, the '\|' key
//     case VK_OEM_5:
    // VK_OEM_6 (DD) Used for miscellaneous characters; it can vary by keyboard. Windows 2000/XP: For the US standard keyboard, the ']}' key
//     case VK_OEM_6:
    // VK_OEM_7 (DE) Used for miscellaneous characters; it can vary by keyboard. Windows 2000/XP: For the US standard keyboard, the 'single-quote/double-quote' key
//     case VK_OEM_7:
    // VK_OEM_8 (DF) Used for miscellaneous characters; it can vary by keyboard.
//     case VK_OEM_8:
    // VK_OEM_102 (E2) Windows 2000/XP: Either the angle bracket key or the backslash key on the RT 102-key keyboard
//     case VK_OEM_102:

    // Windows 95/98/Me, Windows NT 4.0, Windows 2000/XP: IME PROCESS key
//     case VK_PROCESSKEY: return XK_PROCESSKEY;

    // Windows 2000/XP: Used to pass Unicode characters as if they were keystrokes. The VK_PACKET key is the low word of a 32-bit Virtual Key value used for non-keyboard input methods. For more information, see Remark in KEYBDINPUT,SendInput, WM_KEYDOWN, and WM_KEYUP
//     case VK_PACKET: return XK_PACKET;

//     case VK_ATTN: return XK_3270_Attn; // Attn key
//     case VK_CRSEL: // CrSel key
//     case VK_EXSEL: // ExSel key
//     case VK_EREOF: return XK_3270_EraseEOF; // Erase EOF key
    case VK_PLAY: return XF86XK_AudioPlay; // Play key
    case VK_ZOOM: return XF86XK_ZoomIn; // Zoom key

//     case VK_NONAME: // Reserved for future use

//     case VK_PA1: // VK_PA1 (FD) PA1 key

    case VK_OEM_CLEAR: return XK_Clear; // Clear key
  }
  qCWarning(keyevents) << "Failed to map VK=" << vk << "to native X11 key sym.";
  return 0;
}

