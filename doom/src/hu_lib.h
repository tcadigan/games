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
//     none
//
// -----------------------------------------------------------------------------

#ifndef __HULIB__
#define __HULIB__

// We are referring to patches.
#include "r_defs.h"

// Background and foreground screen numbers. Different from other modules.
#define BG 1
#define FG 0

// Font stuff
#define HU_CHARERASE KEY_BACKSPACE

#define HU_MAXLINES 4
#define HU_MAXLINELENGTH 80

// Typedefs of widgets

// Text line widget (parent of Scrolling Text and Input Text widgets)
typedef struct {
  // Left-justified position of scrolling text window
  int x;
  int y;

  patch_t **f;                  // Font
  int sc;                       // Start character
  char l[HU_MAXLINELENGTH + 1]; // Line of text
  int len;                      // Current line length

  // Whether this line needs to be updated
  int needsupdate;
} hu_textline_t;

// Scrolling Text window widget (child of Text Line widget)
typedef struct {
  hu_textline_t l[HU_MAXLINES]; // Text lines to draw
  int h; // Height in lines
  int cl; // Current line number

  // Pointer to boolean stating whether to update window
  boolean *on;
  boolean laston; // Last value of *->on
} hu_stext_t;

// Input Text Line widget (child of Text Line widget)
typedef struct {
  hu_textline_t l; // Text line to input on

  // Left margin past which I am not to delete characters
  int lm;

  // Pointer to boolean stating whether to upate window
  boolean *on;
  boolean laston; // Last value of *->on
} hu_itext_t;

// Widget creation, access, and update routines

// Initializes heads-up widget library
void HUlib_init(void);

// Textline code

// Clear a line of text
void HUlib_clearTextLine(hu_textline_t *t);

void HUlib_initTextLine(hu_textline_t *t, int x, int y, patch_t **f, int sc);

// Returns success
boolean HUlib_addCharToTextLine(hu_textline_t *t, char ch);

// Returns success
boolean HUlib_delCharFromTextLine(hu_textline_t *t);

// Draws tline
void HUlib_drawTextLine(hu_textline_t *l, boolean drawcursor);

// Erases text line
void HUlib_eraseTextLine(hu_textline_t *l);

// Scrolling Text window widget routines

// ?
void HUlib_initSText(hu_stext_t *s, int x, int y, int h, patch_t **font,
		     int startchar, boolean *on);

// Add a new line
void HUlib_addLineToSText(hu_stext_t *s);

// ?
void HUlib_addMessageToSText(hu_stext_t *s, char *prefix, char *msg);

// Draws stext
void HUlib_drawSText(hu_stext_t *s);

// Erases all stext lines
void HUlib_eraseSText(hu_stext_t *s);

// Input Text Line widget routines
void HUlib_initIText(hu_itext_t *it, int x, int y, patch_t **font,
		     int startchar, boolean *on);

// Enforces left margin
void HUlib_delCharFromIText(hu_itext_t *it);

// Enforces left margin
void HUlib_eraseLineFromIText(hu_itext_t *it);

// Reset line and left margin
void HUlib_resetIText(hu_itext_t *it);

// Left of left-margin
void HUlib_addPrefixToIText(hu_itext_t *it, char *str);

// Whether eaten
boolean HUlib_keyInIText(hu_itext_t *it);

// Erases all itext lines
void HUlib_eraseIText(hu_itext_t *it);

#endif

// -----------------------------------------------------------------------------
//
// $Log:$
//
// -----------------------------------------------------------------------------
