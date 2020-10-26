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
//     The status bar widget code.
//
// -----------------------------------------------------------------------------

#ifndef __STLIB__
#define __STLIB__

// We are referring to patches.
#include "r_defs.h"

// Background and foreground screen numbers
#define BG 4
#define FG 0

// Typedefs of widgets

// Number widget
typedef struct {
  // Upper right-hand corner of the number (right-justified)
  int x;
  int y;
  // Max # of digits in number
  int width;
  // Last number value
  int oldnum;
  // Pointer to current value
  int *num;
  // Pointer to boolean stating whether to update number
  boolean *on;
  // List of patches for 0-9
  patch_t **p;
  // User data
  int data;
} st_number_t;

// Percent widget ("child" of number widget, or, more precisely, contains a
// number widget.)
typedef struct {
  // Number information
  st_number_t n;
  // Percent sign graphic
  patch_t *p;
} st_percent_t;

// Multiple Icon widget
typedef struct {
  // Center-justified location of icons
  int x;
  int y;
  // Last icon number
  int oldinum;
  // Pointer to current icon
  int *inum;
  // Pointer to boolean stating whether to update icon
  boolean *on;
  // List of icons
  patch_t **p;
  // User data
  int data;
} st_multicon_t;

// Binary Icon widget
typedef struct {
  // Center-justified location of icon
  int x;
  int y;
  // Last icon value
  int oldval;
  // Pointer to current icon status
  boolean *val;
  // Pointer to boolean stating whether to update icon
  boolean *on;
  patch_t *p; // Icon
  int data; // User data
} st_binicon_t;

// Widget creation, access, and update routines

// Initializes widget library. More precisely, initialize STMINUS, everything
// else is done somewhere else.
void STlib_init(void);

// Number widet routines
void STlib_initNum(st_number_t *n, int x, int y, patch_t **pl, int *num,
		   boolean *on, int width);

void STlib_updateNum(st_number_t *n, boolean refresh);

// Percent widget routines
void STlib_initPercent(st_percent_t *p, int x, int y, patch_t **pl, int *num,
		       boolean *on, patch_t *percent);

void STlib_updatePercent(st_percent_t *per, int refresh);

// Multiple Icon widget routines
void STlib_initMultIcon(st_multicon_t *mi, int x, int y, patch_t **il, int *inum,
			boolean *on);

void STlib_updateMulIcon(st_multicon_t *mi, boolean refresh);

// Binary Icon widget routines
void STlib_initBinIcon(st_binicon_t *b, int x, int y, patch_t i, boolean *val,
		       boolean *on);

void STlib_updateBinIcon(st_binicon_t *bi, boolean refresh);

#endif

// -----------------------------------------------------------------------------
//
// $Log:$
//
// -----------------------------------------------------------------------------
