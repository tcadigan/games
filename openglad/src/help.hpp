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
#ifndef __HELP_HPP__
#define __HELP_HPP__

#include <SDL2/SDL.h>

#include "video_screen.hpp"

Sint16 fill_help_array(Uint8 somearray[HELP_WIDTH][MAX_LINES], SDL_RWops *infile);
Sint16 read_campaign_intro(VideoScreen *myscreen);
Sint16 read_scenario(VideoScreen *myscreen);
Uint8 *read_one_line(SDL_RWops *infiles, Sint16 length);

#endif
