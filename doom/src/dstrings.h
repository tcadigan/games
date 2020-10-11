// Emacs style mode select -*- C++ -*-
// -----------------------------------------------------------------------------
//
// $Id:$
//
// Copyright (C) 1993-1996 by id Software, Inc.
//
// This source is available for distribution and/or modification
// only under the terms of the DOOM Source Code License as
// published by id software. All rights reserved.
//
// The source is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// FITNESS FOR A PARTICULAR PURPOSE. See the DOOM Source Code License
// for more details.
//
// DESCRIPTION:
//     DOOM strings, by language.
//
// -----------------------------------------------------------------------------

#ifndef __DSTRINGS__
#define __DSTRINGS__

// All important printed strings. Language selection (message strings).
// Use -DFRENCH etc.

#ifdef FRENCH
#include "d_french.h"

#else

#include "d_englsh.h"
#endif

// Misc. other strings.
#define SAVEGAME "doomsav"

// File location, relative to current position. Paths names are OS-sensitive.
#define DEVMAPS "devmaps"
#define DEVDATA "devdata"

// Not done in French?

// QuitDOOM messages
#define NUM_QUITMESSAGES 22

extern char *endmsg[];

#endif

// -----------------------------------------------------------------------------
//
// $Log:$
//
// -----------------------------------------------------------------------------
