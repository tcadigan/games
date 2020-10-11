// Emacs style mode select -*- C++ -*-
// -----------------------------------------------------------------------------
//
// $Id:$
//
// Copyright (C) 1993-1996 by id Software, Inc.
//
// This source is available for distribution and/or modification
// only under the terms of the DOOM Source Code License as
// published by id Software. All rights reserved.
//
// The source is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// FITNESS FOR A PARTICULAR PURPOSE. See the DOOM Source Code License
// for more details.
//
// DESCRIPTION:
//     Head up display
//
// -----------------------------------------------------------------------------

#ifndef __HU_STUFF_H__
#define __HU_STUFF_H__

#include "d_event.h"

// Globally visible constants.
#define HU_FONTSTART '!' // The first font characters
#define HU_FONTEND '_' // The last font characters

// Calculate number of glyphs in front.
#define HU_FONTSIZE (HU_FONTEND - HU_FONTSTART + 1)

#define HU_BROADCAST 5

#define HU_MSGREFRESH KEY_ENTER
#define HU_MSGX 0
#define HU_MSGY 0
#define HU_MSGWIDTH 64 // In characters
#define HU_MSGHEIGHT 1 // In lines

#define HU_MSGTIMEOUT (4 * TICRATE)

// HEADS UP TEXT

void HU_Init(void);
void HU_Start(void);

boolean HU_Responder(event_t *ev);

void HU_Ticker(void);
void HU_Drawer(void);
void HU_dequeueChatChar(void);
void HU_Erase(void);

#endif

// -----------------------------------------------------------------------------
//
// $Log:$
//
// -----------------------------------------------------------------------------
