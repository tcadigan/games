/*
 * Copyright (C) 1995-2002 FSGames. Ported by Sean Ford and Yan Shosh
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */
#ifndef __INPUT_HPP__
#define __INPUT_HPP__

//
// input.hpp
//
// Input code
//

#include <ctype.h>
#include <string>

#include <SDL2/SDL.h>

#define SDLKey SDL_Keycode

#define KEYSTATE_UNKNOWN SDL_SCANCODE_UNKNOWN
#define KEYSTATE_a SDL_SCANCODE_A
#define KEYSTATE_b SDL_SCANCODE_B
#define KEYSTATE_c SDL_SCANCODE_C
#define KEYSTATE_d SDL_SCANCODE_D
#define KEYSTATE_e SDL_SCANCODE_E
#define KEYSTATE_f SDL_SCANCODE_F
#define KEYSTATE_g SDL_SCANCODE_G
#define KEYSTATE_h SDL_SCANCODE_H
#define KEYSTATE_i SDL_SCANCODE_I
#define KEYSTATE_j SDL_SCANCODE_J
#define KEYSTATE_k SDL_SCANCODE_K
#define KEYSTATE_l SDL_SCANCODE_L
#define KEYSTATE_m SDL_SCANCODE_M
#define KEYSTATE_n SDL_SCANCODE_N
#define KEYSTATE_o SDL_SCANCODE_O
#define KEYSTATE_p SDL_SCANCODE_P
#define KEYSTATE_q SDL_SCANCODE_Q
#define KEYSTATE_r SDL_SCANCODE_R
#define KEYSTATE_s SDL_SCANCODE_S
#define KEYSTATE_t SDL_SCANCODE_T
#define KEYSTATE_u SDL_SCANCODE_U
#define KEYSTATE_v SDL_SCANCODE_V
#define KEYSTATE_w SDL_SCANCODE_W
#define KEYSTATE_x SDL_SCANCODE_X
#define KEYSTATE_y SDL_SCANCODE_Y
#define KEYSTATE_z SDL_SCANCODE_Z
#define KEYSTATE_0 SDL_SCANCODE_0
#define KEYSTATE_1 SDL_SCANCODE_1
#define KEYSTATE_2 SDL_SCANCODE_2
#define KEYSTATE_3 SDL_SCANCODE_3
#define KEYSTATE_4 SDL_SCANCODE_4
#define KEYSTATE_5 SDL_SCANCODE_5
#define KEYSTATE_6 SDL_SCANCODE_6
#define KEYSTATE_7 SDL_SCANCODE_7
#define KEYSTATE_8 SDL_SCANCODE_8
#define KEYSTATE_9 SDL_SCANCODE_9
#define KEYSTATE_COMMA SDL_SCANCODE_COMMA
#define KEYSTATE_PERIOD SDL_SCANCODE_PERIOD
#define KEYSTATE_DELETE SDL_SCANCODE_DELETE
#define KEYSTATE_UP SDL_SCANCODE_UP
#define KEYSTATE_DOWN SDL_SCANCODE_DOWN
#define KEYSTATE_LEFT SDL_SCANCODE_LEFT
#define KEYSTATE_RIGHT SDL_SCANCODE_RIGHT
#define KEYSTATE_PAGEDOWN SDL_SCANCODE_PAGEDOWN
#define KEYSTATE_PAGEUP SDL_SCANCODE_PAGEUP
#define KEYSTATE_RETURN SDL_SCANCODE_RETURN
#define KEYSTATE_ESCAPE SDL_SCANCODE_ESCAPE
#define KEYSTATE_SPACE SDL_SCANCODE_SPACE
#define KEYSTATE_SLASH SDL_SCANCODE_SLASH
#define KEYSTATE_LCTRL SDL_SCANCODE_LCTRL
#define KEYSTATE_RCTRL SDL_SCANCODE_RCTRL
#define KEYSTATE_LSHIFT SDL_SCANCODE_LSHIFT
#define KEYSTATE_RSHIFT SDL_SCANCODE_RSHIFT
#define KEYSTATE_RIGHTBRACKET SDL_SCANCODE_RIGHTBRACKET
#define KEYSTATE_LEFTBRACKET SDL_SCANCODE_LEFTBRACKET
#define KEYSTATE_KP_PLUS SDL_SCANCODE_KP_PLUS
#define KEYSTATE_KP_MINUS SDL_SCANCODE_KP_MINUS
#define KEYSTATE_KP_MULTIPLY SDL_SCANCODE_KP_MULTIPLY
#define KEYSTATE_KP_0 SDL_SCANCODE_KP_0
#define KEYSTATE_KP_1 SDL_SCANCODE_KP_1
#define KEYSTATE_KP_2 SDL_SCANCODE_KP_2
#define KEYSTATE_KP_3 SDL_SCANCODE_KP_3
#define KEYSTATE_KP_4 SDL_SCANCODE_KP_4
#define KEYSTATE_KP_5 SDL_SCANCODE_KP_5
#define KEYSTATE_KP_6 SDL_SCANCODE_KP_6
#define KEYSTATE_KP_7 SDL_SCANCODE_KP_7
#define KEYSTATE_KP_8 SDL_SCANCODE_KP_8
#define KEYSTATE_KP_9 SDL_SCANCODE_KP_9
#define KEYSTATE_F1 SDL_SCANCODE_F1
#define KEYSTATE_F5 SDL_SCANCODE_F5
#define KEYSTATE_F9 SDL_SCANCODE_F9
#define KEYSTATE_F10 SDL_SCANCODE_F10

// Zardus: Defines for event getting method
#define POLL 0
#define WAIT 1

// Keyboard defines
#define MAXKEYS 320

bool isPlayerHoldingKey(Sint32 player_index, Sint32 key_enum);
bool didPlayerPressKey(Sint32 player_index, Sint32 key_enum, SDL_Event const &event);
bool didPlayerReleaseKey(Sint32 player_index, Sint32 key_enum, SDL_Event const &event);

// buffers: Added prototype
void get_input_events(bool type);
void handle_events(SDL_Event const &event);

// Specific event handling
void handle_window_event(SDL_Event const &event);
void handle_key_event(SDL_Event const &event);
void handle_text_event(SDL_Event const &event);

// Takes SDLK (SDL_Keycode) values
void sendFakeKeyDownEvent(Sint32 keycode);
void sendFakeKeyUpEvent(Sint32 keycode);

// Return last keypress
SDL_Keycode query_key();

// Return last text input
std::string query_text_input();

bool query_input_continue();
Sint16 get_and_reset_scroll_amount();

bool query_key_event(Sint32 key, SDL_Event const &event);
bool isAnyPlayerKey(SDLKey key);
bool isPlayerKey(Sint32 player_num, SDLKey key);

SDL_Event wait_for_key_event();
void quit_if_quit_event(SDL_Event const &event);

bool isKeyboardEvent(SDL_Event const &event);

// Clears the SDL event queue
void clear_events();

void assignKeyFromWaitEvent(Sint32 player_num, Sint32 key_enum);

// Set keyboard to none pressed
void clear_keyboard();

// Wait for some key
void wait_for_key(Sint32 somekey);

// query_ & clear_key_press_event
Sint16 query_key_press_event();

// Detect a key press :)
void clear_key_press_event();

// query_ & dlear_key_press_event
Sint16 query_text_input_event();

// Detect a key press :)
void clear_text_input_event();

// OBSOLETE, use query_keyboard
bool query_key_code(Sint32 code);

void clear_key_code(Sint32 code);
void enable_keyrepeat();
void disable_keyrepeat();
void init_input();

Uint8 convert_to_ascii(Sint32 scancode);

extern Uint8 const *keystates;

// In window coords
extern Sint32 viewport_offset_x;
extern Sint32 viewport_offset_y;
extern Sint32 window_w;
extern Sint32 window_h;
extern Sint32 viewport_w;
extern Sint32 viewport_h;

extern Sint16 scroll_amount; // For scrolling up and down text popups
extern Sint16 key_press_event;

#endif
