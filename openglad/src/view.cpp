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
// view.cpp

/*
 * ChangeLog:
 *     buffers: 07/31/02: *include cleanup
 */

#include "view.hpp"

#include "colors.hpp"
#include "guy.hpp"
#include "help.hpp"
#include "input.hpp"
#include "joy_data.hpp"
#include "options.hpp"
#include "pal32.hpp"
#include "radar.hpp"
#include "util.hpp"
#include "version.hpp"
#include "video_screen.hpp"
#include "view_sizes.hpp"
#include "walker.hpp"

#include <algorithm>
#include <iomanip>
#include <sstream>

// These are for Chad's team info page
#define VIEW_TEAM_TOP 2
#define VIEW_TEAM_LEFT 20
#define VIEW_TEAM_BOTTOM 198
#define VIEW_TEAM_RIGHT 280

#define LEFT_OPS 49
#define TOP_OPS 44
#define TEXT_HEIGHT 5
#define OPLINES(y) (TOP_OPS + ((y) * (TEXT_HEIGHT + 3)))
#define PANEL_COLOR 13

// This is for saving/loading the key preferences
Sint32 save_key_prefs();
Sint32 load_key_pregs();

// Zardus: No longer unsigned
Sint32 get_keypress();

/*
 * *****************************************************************
 * VIEWSCREEN -- It's nothing like viewscreen, it just looks like it
 * *****************************************************************
 *
 * viewscreen(Uint8, Sint16, Sint16, Screen) - Initializes the viewscreen data (pix = char)
 * Sint16 draw()
 */

// Viewscreen -- This initializes the gprahics data for the viewscreen, as well
// as its graphics x and y size. In addition, it informs the viewscreen of the
// screen object it is linked to.
ViewScreen::ViewScreen(Sint16 x, Sint16 y, Sint16 width, Sint16 height, Sint16 whatnum)
{
    Sint32 i;

    xview = width;
    yview = height;
    topy = 0;
    topx = topy;

    // Where to display on the physical screen
    xloc = x;
    yloc = y;
    endx = xloc + width;
    endy = yloc + height;
    control = nullptr;
    gamma = 0;
    prefsob = theprefs;

    // Key entries...
    // What viewscreen am I?
    mynum = whatnum;
    // Assign keyboard mappings
    mykeys = allkeys[mynum];

    // Load key prefs, if present
    prefsob->load(mynum, prefs, mykeys);

    myradar = new Radar(mynum);
    // This radar has not yet been started
    radarstart = 0;

    for (i = 0; i < MAX_MESSAGES; ++i) {
        textcycles[i] = 0;
        // Null message;
        textlist[i] = nullptr;
    }

    // Properly resize the viewscreen
    resize(prefs[PREF_VIEW]);
}

// Destruct the viewscreen and its variables
ViewScreen::~ViewScreen()
{
    if (myradar) {
        delete myradar;
    }

    myradar = nullptr;
}

void ViewScreen::clear()
{
    Uint16 i;

    for (i = 0; i < 64000; ++i) {
        myscreen->videobuffer[i] = 0;
    }
}

bool ViewScreen::redraw()
{
    Sint16 i;
    Sint16 j;
    Sint16 xneg = 0;
    Sint16 yneg = 0;
    Walker *controlob = control;
    PixieN **backp = myscreen->level_data.back;
    PixieData *gridp = myscreen->level_data.grid;
    Uint16 maxx = gridp->w;
    Uint16 maxy = gridp->h;

    // Check if we are partially into a grid square and require extra row
    if (controlob) {
        topx = controlob->xpos - ((xview - controlob->sizex) / 2);
        topy = controlob->ypos - ((yview - controlob->sizey) / 2);
    } else {
        // No control object now...
        topx = myscreen->level_data.topx;
        topy = myscreen->level_data.topy;
    }

    if (topx < 0) {
        xneg = 1;
    }

    if (topy < 0) {
        yneg = 1;
    }

    // Note: >> 4 is equivalent to /16 but faster, since it doesn't divide
    // likewise << 4 is equivalent to *16, but faster

    for (j = ((topy / GRID_SIZE) - yneg); j < (((topy + yview) / GRID_SIZE) + 1); ++j) {
        for (i = ((topx / GRID_SIZE) - xneg); i < (((topx + xview) / GRID_SIZE) + 1); ++i) {
            // Note: back is a PixieN.
            // Background graphic [grid(x, y)] -> Put in buffer
            if ((i < 0) || (j < 0) || (i >= maxx) || (j >= maxy)) {
                // Show side of wall
                if ((j == -1) && (i > -1) && (i < maxx)) {
                    backp[PIX_WALLSIDE1]->setxy(i * GRID_SIZE, j * GRID_SIZE);
                    backp[PIX_WALLSIDE1]->draw(topx, topy, xloc, yloc, endx, endy);
                } else if ((j == -2) && (i > -1) && (i < maxx)) {
                    // Show top side of wall
                    backp[PIX_H_WALL1]->setxy(i * GRID_SIZE, j * GRID_SIZE);
                    backp[PIX_H_WALL1]->draw(topx, topy, xloc, yloc, endx, endy);
                } else {
                    backp[PIX_WALLTOP_H]->setxy(i * GRID_SIZE, j * GRID_SIZE);
                    backp[PIX_WALLTOP_H]->draw(topx, topy, xloc, yloc, endx, endy);
                }
            } else if (gridp->valid()) {
                backp[gridp->data[i + (maxx * j)]]->setxy(i * GRID_SIZE, j * GRID_SIZE);
                backp[gridp->data[i + (maxx * j)]]->draw(topx, topy, xloc, yloc, endx, endy);
            }
        }
    }

    // Moved here to put the radar on top of obs
    draw_obs();

    if (control && !control->dead && (control->user == mynum) && (prefs[PREF_RADAR] == PREF_RADAR_ON)) {
        myradar->start(myscreen->level_data, endx, endy, yloc);
        radarstart = 1;
        myradar->draw(myscreen->level_data, control);
    }

    display_text();

    return true;
}

void ViewScreen::display_text()
{
    Sint32 i;

    for (i = 0; i < MAX_MESSAGES; ++i) {
        // Display text if there's any there...
        if (textcycles[i] > 0) {
            --textcycles[i];
            myscreen->text_normal.write_xy((xview - (textlist[i].length() *  6)) / 2,
                                           30 + (i * 6), textlist[i], YELLOW,
                                           xloc, yloc, endx, endy);
        }
    }

    // Clean up any empty slots
    for (i = 0; i < MAX_MESSAGES; ++i) {
        if ((textcycles[i] < 1)&& textlist[i].length()) {
            // Shift text up, starting at position i
            shift_text(i);
        }
    }
}

void ViewScreen::shift_text(Sint32 row)
{
    Sint32 i;

    for (i = row; i < (MAX_MESSAGES - 1); ++i) {
        textlist[i] = textlist[i + 1];
        textcycles[i] = textcycles[i + 1];
    }

    textlist[MAX_MESSAGES - 1] = nullptr;
    textcycles[MAX_MESSAGES - 1] = 0;
}

bool ViewScreen::refresh()
{
    // The first two values are screwy...I don't kow why
    myscreen->buffer_to_screen(xloc, yloc, xview, yview);

    return true;
}

bool ViewScreen::input(SDL_Event const &event)
{
    // For switching guys
    static Sint16 changedchar[6] = { 0, 0, 0, 0, 0, 0 };
    // For switching special
    static Sint16 changedspec[6] = { 0, 0, 0, 0, 0, 0 };
    // For switching team
    static Sint16 changedteam[6] = { 0, 0, 0, 0, 0, 0 };
    Uint32 totaltime;
    Uint32 totalframes;
    Uint32 framespersec;
    // For general-purpose use
    Walker *newob;
    // So we know if we changed guys
    Walker *oldcontrol = control;

    if (control && (control->user == -1)) {
        control->set_act_type(ACT_CONTROL);
        control->user = mynum;
        control->clear_command();
    }

    // TODO: Factor out this code which is duplicated in continuous_input()
    if (!control || control->dead) {
        control = nullptr;

        // First loop for a player character, not already controller
        for (auto const & w : myscreen->level_data.oblist) {
            if (w
                && !w->dead
                && (w->query_order() == ORDER_LIVING)
                && (w->user == -1) // Means we're not player contolled
                && w->myguy
                && (w->team_num == my_team) /* Makes a difference for PvP */) {
                control = w;

                break;
            }
        }

        if (!control) {
            // Second, look for anyone on our team, NPC or not
            for (auto const & w : myscreen->level_data.oblist) {
                if (w
                    && !w->dead
                    && (w->query_order() == ORDER_LIVING)
                    && (w->user == -1) // Means we're not player controlled
                    && (w->team_num == my_team) /* Makes a difference for PvP */) {
                    control = w;

                    break;
                }
            }

            // Done with second search
        }

        if (!control) {
            // Now try for ANYONE who's left alive...
            // Note: You can end up as a bad guy here if you are using an allied team
            for (auto const & w : myscreen->level_data.oblist) {
                if (w
                    && !w->dead
                    && (w->query_order() == ORDER_LIVING)
                    && w->myguy) {
                    control = w;

                    break;
                }
            }

            // Done with all searches
        }

        // Then there's nobody left!
        if (!control) {
            return myscreen->endgame(1);
        }

        if (control->user == -1) {
            // Show that we're controlled onw
            control->user = mynum;
        }

        control->set_act_type(ACT_CONTROL);
        myscreen->control_hp = control->stats.hitpoints;
    }

    // Do we have extra rounds?
    if (control && control->bonus_rounds) {
        --control->bonus_rounds;

        if ((control->lastx != 0.0f) || (control->lasty != 0.0f)) {
            control->walk();
        }
    }

    if (!isPlayerHoldingKey(mynum, KEY_CHEAT)) {
        if (query_key_event(SDLK_F3, event)) {
            totaltime = (query_timer_control() - myscreen->timerstart) / 72;
            totalframes = myscreen->framecount;
            framespersec = totalframes / totaltime;
            std::stringstream buf;
            buf << framespersec << " FRAMES PER SEC";
            myscreen->viewob[0]->set_display_text(buf.str(), STANDARD_TEXT_TIME);
            buf.clear();
        }

        // Memory report
        if (query_key_event(SDLK_F4, event)) {
            myscreen->report_mem();
        }

        if (didPlayerPressKey(mynum, KEY_PREFS, event)) {
            options_menu();

            return true;
        }
    }

    // TAB (alone) will switch control to the next guy on my team
    if (!didPlayerPressKey(mynum, KEY_SWITCH, event)) {
        changedchar[mynum] = 0;
    } else if (!changedchar[mynum] && isPlayerHoldingKey(mynum, KEY_CHEAT)) {
        // KEY_SHIFTER will go backward
        bool reverse = isPlayerHoldingKey(mynum, KEY_SHIFTER);

        // Unset our control
        changedchar[mynum] = 1;

        if (control->user == mynum) {
            control->restore_act_type();
            control->user = -1;
        }

        control = nullptr;
        auto const &oblist = myscreen->level_data.oblist;

        if (!reverse) {
            // Get where we are in the list
            auto mine = std::find(oblist.begin(), oblist.end(), oldcontrol);

            if (mine == oblist.end()) {
                SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "Failed to find self in oblist!\n");

                return true;
            }

            // Look past our current spot
            auto e = mine;
            ++e;

            while (e != oblist.end()) {
                Walker *w = *e;

                if ((w->query_order() == ORDER_LIVING)
                    && w->is_friendly(oldcontrol)
                    && (w->team_num == my_team)
                    && (w->real_team_num == 255)
                    && (w->user == -1)) {
                    control = w;

                    break;
                }

                ++e;
            }

            if (!control) {
                // Look before our current spot
                for (e = oblist.begin(); e != mine; ++e) {
                    Walker *w = *e;

                    if ((w->query_order() == ORDER_LIVING)
                        && w->is_friendly(oldcontrol)
                        && (w->team_num == my_team)
                        && (w->real_team_num == 255)
                        && (w->user == -1)) {
                        control = w;

                        break;
                    }
                }
            }
        } else {
            // Get where we are in the list
            auto mine = std::find(oblist.rbegin(), oblist.rend(), oldcontrol);

            if (mine == oblist.rend()) {
                SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "Failed to find self in oblist!\n");

                return true;
            }

            // Look past our current spot
            auto e = mine;
            ++e;

            while (e != oblist.rend()) {
                Walker *w = *e;

                if ((w->query_order() == ORDER_LIVING)
                    && w->is_friendly(oldcontrol)
                    && (w->team_num == my_team)
                    && (w->real_team_num == 255)
                    && (w->user == -1)) {
                    control = w;

                    break;
                }

                ++e;
            }

            if (!control) {
                // Look before our current spot
                for (e = oblist.rbegin(); e != mine; ++e) {
                    Walker *w = *e;

                    if ((w->query_order() == ORDER_LIVING)
                        && w->is_friendly(oldcontrol)
                        && (w->team_num == my_team)
                        && (w->real_team_num == 255)
                        && (w->user == -1)) {
                        control = w;

                        break;
                    }
                }
            }
        }

        if (!control) {
            control = oldcontrol;
        }

        myscreen->control_hp = control->stats.hitpoints;

        // End of switch guys
    }

    // Redisplay the scenario text..
    if (query_key_event(SDLK_SLASH, event)
        && !isAnyPlayerKey(SDLK_SLASH)
        && !isPlayerHoldingKey(mynum, KEY_CHEAT) /* Actually "?" */) {
        read_scenario(myscreen);
        myscreen->redrawme = 1;
        clear_keyboard();
    }

    // Change our currently select special
    if (!didPlayerPressKey(mynum, KEY_SPECIAL_SWITCH, event)) {
        changedspec[mynum] = 0;
    }

    if (didPlayerPressKey(mynum, KEY_SPECIAL_SWITCH, event) && !changedspec[mynum]) {
        changedspec[mynum] = 1;

        ++control->current_special;

        if ((control->current_special > (NUM_SPECIALS - 1))
            || (myscreen->special_name[control->query_family()][control->current_special] != "NONE")
            || ((((control->current_special - 1) * 3) + 1) > control->stats.level)) {
            control->current_special = 1;
        }

        // End of switch special
    }

    // Yell for help
    if (didPlayerPressKey(mynum, KEY_YELL, event)
        && !control->yo_delay
        && !isPlayerHoldingKey(mynum, KEY_SHIFTER)
        && !isPlayerHoldingKey(mynum, KEY_CHEAT)) {
        for (auto const & w : myscreen->level_data.oblist) {
            if (w
                && (w->query_order() == ORDER_LIVING)
                && (w->query_act_type() != ACT_CONTROL)
                && (w->team_num == control->team_num)
                && !w->leader) {
                // Remove any curent foe...
                w->leader = control;
                w->foe = nullptr;
                w->stats.force_command(COMMAND_FOLLOW, 100, 0, 0);
            }
        }

        control->yo_delay = 30;
        myscreen->soundp->play_sound(SOUND_YO);
        myscreen->do_notify("Yo!", control);

        // End of yo for friends
    }

    // Summon team defense
    // change guys' behavior
    if (isPlayerHoldingKey(mynum, KEY_SHIFTER)
        && didPlayerPressKey(mynum, KEY_YELL, event)
        && !isPlayerHoldingKey(mynum, KEY_CHEAT)) {
        switch (control->action) {
        case 0:
            // Not set...

            for (auto & w : myscreen->level_data.oblist) {
                if (w && (w->team_num == control->team_num) && w->is_friendly(control)) {
                    // Remove any current foe...
                    w->leader = control;
                    w->foe = nullptr;
                    w->action = ACTION_FOLLOW;
                }
            }

            myscreen->do_notify("SUMMONING DEFENSE!", control);

            break;
        case ACTION_FOLLOW:
            // Turn back to normal mode...
            for (auto & w : myscreen->level_data.oblist) {
                if (w
                    && (w->query_order() == ORDER_LIVING)
                    && (w->query_act_type() != ACT_CONTROL)
                    && (w->team_num == control->team_num)) {
                    // Set to normal operation
                    w->action = 0;
                }
            }

            // For our reference
            control->action = 0;
            myscreen->do_notify("RELEASING MEN!", control);

            break;
        default:
            control->action = 0;

            break;
        } // End of switch action mode

        // End of summon team defense
    }

    // Before here, all keys should check for !KEY_CHEAT

    // Cheat keys...using control
    if (isPlayerHoldingKey(mynum, KEY_CHEAT) && CHEAT_MODE) {
        // Change our team :)
        if (changedteam[mynum] && !didPlayerPressKey(mynum, KEY_SWITCH, event)) {
            changedteam[mynum] = 0;
        }

        if (didPlayerPressKey(mynum, KEY_SWITCH, event) && !changedteam[mynum]) {
            // To debounce keys
            changedteam[mynum] = 1;

            Walker *result = nullptr;
            control->user = -1;
            // Hope this works
            control->set_act_type(ACT_RANDOM);

            Sint16 oldteam = myscreen->save_data.my_team;

            ++myscreen->save_data.my_team;
            myscreen->save_data.my_team %= MAX_TEAM;

            for (auto const & w : myscreen->level_data.oblist) {
                if ((w->team_num == myscreen->save_data.my_team)
                    && (w->query_order() == ORDER_LIVING)) {
                    result = w;

                    // Out of loop; we found someone
                }
            }

            while ((result == nullptr)  && (myscreen->save_data.my_team != oldteam)) {
                ++myscreen->save_data.my_team;
                myscreen->save_data.my_team %= MAX_TEAM;

                for (auto const & w : myscreen->level_data.oblist) {
                    if ((w->team_num == myscreen->save_data.my_team)
                        && (w->query_order() == ORDER_LIVING)) {
                        result = w;

                        // Out of loop; we foudn someone
                    }
                }
            }

            if (result != nullptr) {
                control = result;
            }

            control->user = mynum;
            control->set_act_type(ACT_CONTROL);

            // End of change team
        }

        // Kill living bad guys
        if (query_key_event(SDLK_F12, event)) {
            for (auto & w : myscreen->level_data.oblist) {
                if (w
                    && (w->query_order() == ORDER_LIVING)
                    && !control->is_friendly(w)) {
                    w->stats.hitpoints = -1;
                    control->attack(w);
                    w->death();
                }
            }
        }

        // Up level
        if (query_key_event(SDLK_RIGHTBRACKET, event)) {
            ++control->stats.level;

            // End up level
        }

        // Down level
        if (query_key_event(SDLK_LEFTBRACKET, event)) {
            if (control->stats.level > 1) {
                --control->stats.level;
            }

            // End down level
        }

        // Freeze time
        if (query_key_event(SDLK_F1, event)) {
            myscreen->enemy_freeze += 50;
            set_palette(myscreen->bluepalette);

            // End freeze time
        }

        // Generate magic shield
        if (query_key_event(SDLK_F2, event)) {
            newob = myscreen->level_data.add_ob(ORDER_FX, FAMILY_MAGIC_SHIELD);
            newob->owner = control;
            newob->team_num = control->team_num;
            // Dummy, non-zero value
            newob->ani_type = 1;
            newob->lifetime = 200;

            // End generate magic shield
        }

        // Ability to fly
        if (query_key_event(SDLK_f, event)) {
            if (control->stats.query_bit_flags(BIT_FLYING)) {
                control->stats.set_bit_flags(BIT_FLYING, 0);
            } else {
                control->stats.set_bit_flags(BIT_FLYING, 1);
            }

            // End flying
        }

        // Give controller lots of hitpoints
        if (query_key_event(SDLK_h, event)) {
            control->stats.hitpoints += 100;
            // Why not just reset from the above for sanity's sake?
            myscreen->control_hp += 100;

            // End hitpoints
        }

        // Give invincibility
        if (query_key_event(SDLK_i, event)) {
            if (control->stats.query_bit_flags(BIT_INVINCIBLE)) {
                control->stats.set_bit_flags(BIT_INVINCIBLE, 0);
            } else {
                control->stats.set_bit_flags(BIT_INVINCIBLE, 1);
            }

            // End invincibility
        }

        // Give controller lots of magicpoints
        if (query_key_event(SDLK_m, event)) {
            control->stats.magicpoints += 150;

            // End magic points
        }

        // Give us faster speed...
        if (query_key_event(SDLK_s, event)) {
            control->speed_bonus_left += 20;
            control->speed_bonus = control->normal_stepsize;
        }

        // Transform to new shape
        if (query_key_event(SDLK_t, event)) {
            Uint8 family = (control->query_family() + 1) % NUM_FAMILIES;
            control->transform_to(control->query_order(), family);

            // end transform
        }

        // Invisibility
        if (query_key_event(SDLK_v, event)) {
            if (control->invisibility_left < 3000) {
                control->invisibility_left += 100;
            }
        }

        // End of cheat keys
    }

    // Make sure we're not in use by another player
    if (control->user != mynum) {
        return true;
    }

    // If we changed control characters
    if (control != oldcontrol) {
        control->clear_command();
    }

    // If we're frozen...
    if (control->dead || control->stats.frozen_delay) {
        return true;
    }

    // Movement, etc.
    // Make sure we're not performing some queued action...
    if (control->stats.commands.empty()) {
        control->shifter_down = isPlayerHoldingKey(mynum, KEY_SHIFTER);

        if (didPlayerPressKey(mynum, KEY_SPECIAL, event)) {
            control->special();
        }

        // Standard fire
        if (didPlayerPressKey(mynum, KEY_FIRE, event)) {
            control->init_fire();
        }

        // End of check for queued actions...
    }

    return true;
}

bool ViewScreen::continuous_input()
{
    // So we know if we changed guys
    Walker *oldcontrol = control;

    if (control && (control->user == -1)) {
        control->set_act_type(ACT_CONTROL);
        control->user = mynum;
        control->clear_command();
    }

    if (!control || control->dead) {
        control = nullptr;

        // First look for a player character, not already controllers
        for (auto const w : myscreen->level_data.oblist) {
            if (w
                && !w->dead
                && (w->query_order() == ORDER_LIVING)
                && (w->user == -1) // Means we're not player controller
                && w->myguy
                && (w->team_num == my_team) /* Makes a difference for PvP */) {
                control = w;

                break;
            }
        }

        if (!control) {
            // Second, look for anyone on our team, NPC or note
            for (auto const & w: myscreen->level_data.oblist) {
                if (w
                    && !w->dead
                    && (w->query_order() == ORDER_LIVING)
                    && (w->user == -1) // Means we're not player controller
                    && w->myguy
                    && (w->team_num == my_team) /* Makes a difference for PvP */) {
                    control = w;

                    break;
                }
            }

            // Done with second search
        }

        if (!control) {
            // Now try for ANYONE who's left alive...
            // Note: You can end up as a bad guy here if you are using an allied team
            for (auto const & w : myscreen->level_data.oblist) {
                if (w
                    && !w->dead
                    && (w->query_order() == ORDER_LIVING)
                    && w->myguy) {
                    control = w;

                    break;
                }
            }

            // Done with all searches
        }

        // Then there's nobody left!
        if (!control) {
            return myscreen->endgame(1);
        }

        if (control->user == -1) {
            // Show that we're controlled now
            control->user = mynum;
        }

        control->set_act_type(ACT_CONTROL);
        myscreen->control_hp = control->stats.hitpoints;
    }

    // Do we have extra rounds?
    if (control && control->bonus_rounds) {
        --control->bonus_rounds;

        if (control->lastx || control->lasty) {
            control->walk();
        }
    }

    // Make sure we haven't yelled recently (this is here because it is
    // guaranteed to run exactly once each frame)
    if (control->yo_delay > 0) {
        --control->yo_delay;
    }

    // Before here, all keys should check for !KEY_CHEAT

    // Make sure we're not in use by another player
    if (control->user != mynum) {
        return true;
    }

    if (control->ani_type != ANI_WALK) {
        control->animate();
    }

    // If we changed control characters
    if (control != oldcontrol) {
        control->clear_command();
    }

    // If we're frozen...
    if (control->stats.frozen_delay) {
        --control->stats.frozen_delay;

        return true;
    }

    // Movement, etc.
    // Make sure we're not performing some queued action...
    if (control->stats.commands.empty()) {
        // We will handle this as an action in input() instead
        if (isPlayerHoldingKey(mynum, KEY_SHIFTER)) {
            control->shifter_down = 1;
        } else {
            control->shifter_down = 0;
        }

        Sint32 walkx = 0;
        Sint32 walky = 0;

        if (isPlayerHoldingKey(mynum, KEY_UP)
            || isPlayerHoldingKey(mynum, KEY_UP_LEFT)
            || isPlayerHoldingKey(mynum, KEY_UP_RIGHT)) {
            walky = -1;
        } else if (isPlayerHoldingKey(mynum, KEY_DOWN)
                   || isPlayerHoldingKey(mynum, KEY_DOWN_LEFT)
                   || isPlayerHoldingKey(mynum, KEY_DOWN_RIGHT)) {
            walky = 1;
        }

        if (isPlayerHoldingKey(mynum, KEY_LEFT)
            || isPlayerHoldingKey(mynum, KEY_UP_LEFT)
            || isPlayerHoldingKey(mynum, KEY_DOWN_LEFT)) {
            walkx = -1;
        } else if (isPlayerHoldingKey(mynum, KEY_RIGHT)
                   || isPlayerHoldingKey(mynum, KEY_DOWN_RIGHT)
                   || isPlayerHoldingKey(mynum, KEY_UP_RIGHT)) {
            walkx = 1;
        }

        if ((walkx != 0) || (walky != 0)) {
            control->walkstep(walkx, walky);
        } else if (control->stats.query_bit_flags(BIT_ANIMATE)) {
            // Animate regardless...
            ++control->cycle;

            if (control->ani[control->curdir][control->cycle] == -1) {
                control->cycle = 0;
            }

            control->set_frame(control->ani[control->curdir][control->cycle]);
        }

        // Standard fire
        if (isPlayerHoldingKey(mynum, KEY_FIRE)) {
            control->init_fire();
        }

        // End of check for queued actions...
    }

    return true;
}

void ViewScreen::set_display_text(std::string const &newtext, Sint16 numcycles)
{
    Sint32 i;

    i = 0;

    while (!textlist[i].empty() && (i < MAX_MESSAGES)) {
        ++i;
    }

    // No room, need to scroll messages
    if (i >= MAX_MESSAGES) {
        // Shift up, starting at 0
        shift_text(0);
        i = MAX_MESSAGES - 1;
    }

    // infotext = newtext;
    textlist[i] = newtext;

    if (numcycles > 0) {
        textcycles[i] = numcycles;
    } else {
        textcycles[i] = 0;
    }
}

// Blanks the screen text
void ViewScreen::clear_text()
{
    Sint32 i;

    for (i = 0; i < MAX_MESSAGES; ++i) {
        textlist[i] = nullptr;
    }
}

Sint16 ViewScreen::draw_obs()
{
    LevelData *data = &myscreen->level_data;
    // First draw the special effects
    for (auto & w : data->fxlist) {
        if (w && !w->dead) {
            w->draw(topx, topy, xloc, yloc, endx, endy, control);
        }
    }

    // Now do real objects
    for (auto & w : data->oblist) {
        if (w && !w->dead) {
            w->draw(topx, topy, xloc, yloc, endx, endy, control);
        }
    }

    // Finally draw the weapons
    for (auto & w : data->weaplist) {
        if (w && !w->dead) {
            w->draw(topx, topy, xloc, yloc, endx, endy, control);
        }
    }

    return true;
}

void ViewScreen::resize(Sint16 x, Sint16 y, Sint16 length, Sint16 height)
{
    xloc = x;
    yloc = y;

    xview = length;
    yview = height;

    endx = xloc + length;
    endy = yloc + height;

    if (myradar->bmp) {
        myradar->start(myscreen->level_data, endx, endy, yloc);
    }

    myscreen->redrawme = 1;
}

void ViewScreen::resize(Uint8 whatmode)
{
    switch (myscreen->numviews) {
    case 1:
        // One player mode
        switch (whatmode) {
        case PREF_VIEW_PANELS:
            // Room for score panel...
            resize(44, 12, 232, 176);

            break;
        case PREF_VIEW_1:
            resize(64, 28, 192, 144);

            break;
        case PREF_VIEW_2:
            resize(86, 44, 148, 112);

            break;
        case PREF_VIEW_3:
            resize(106, 60, 108, 80);

            break;
        case PREF_VIEW_FULL:
        default:
            resize(T_LEFT_ONE, T_UP_ONE, T_WIDTH, T_HEIGHT);

            break;
        }

        break;
    case 2:
        // Two player mode

        // Left or right view?
        switch (mynum) {
        case 0:
            switch (whatmode) {
            case PREF_VIEW_PANELS:
                // Room for score panel...
                resize(4, 16, 152, 168);

                break;
            case PREF_VIEW_1:
                resize(4, 32, 152, 136);

                break;
            case PREF_VIEW_2:
                resize(4, 48, 152, 104);

                break;
            case PREF_VIEW_3:
                resize(4, 64, 152, 72);

                break;
            case PREF_VIEW_FULL:
            default:
                resize(T_LEFT_ONE, T_UP_ONE, T_HALF_WIDTH, T_HEIGHT);

                break;
            }

            break;
        case 1:
            switch (whatmode) {
            case PREF_VIEW_PANELS:
                // Room for score panel...
                resize(164, 16, 152, 168);

                break;
            case PREF_VIEW_1:
                resize(164, 32, 152, 136);

                break;
            case PREF_VIEW_2:
                resize(164, 48, 152, 104);

                break;
            case PREF_VIEW_3:
                resize(164, 64, 152, 72);

                break;
            case PREF_VIEW_FULL:
            default:
                resize(T_LEFT_TWO, T_UP_TWO, T_HALF_WIDTH, T_HEIGHT);

                break;
            }

            break;

            // End of mynum switch
        }

        break;
    case 3:
        // Three player mode

        // Left or right view?
        switch (mynum) {
        case 0:
            switch (whatmode) {
            case PREF_VIEW_PANELS:
                // Room for score panel...
                resize(4, 16, 100, 168);

                break;
            case PREF_VIEW_1:
                resize(4, 32, 100, 136);

                break;
            case PREF_VIEW_2:
                resize(4, 48, 100, 104);

                break;
            case PREF_VIEW_3:
                resize(4, 64, 100, 72);

                break;
            case PREF_VIEW_FULL:
            default:
                resize(T_LEFT_ONE, T_UP_ONE, T_HALF_WIDTH, T_HEIGHT);

                break;
            }

            break;
        case 1:
            switch (whatmode) {
            case PREF_VIEW_PANELS:
                // Room for score panel...
                resize(216, 16, 100, 168);

                break;
            case PREF_VIEW_1:
                resize(216, 32, 100, 136);

                break;
            case PREF_VIEW_2:
                resize(216, 48, 100, 104);

                break;
            case PREF_VIEW_3:
                resize(216, 64, 100, 72);

                break;
            case PREF_VIEW_FULL:
            default:
                resize(T_LEFT_TWO, T_UP_TWO, T_HALF_WIDTH, T_HALF_HEIGHT);

                break;
            }

            break;
        case 2:
            // Third player
            switch (whatmode) {
            case PREF_VIEW_PANELS:
                // Room for score panel...
                resize(112, 16, 100, 168);

                break;
            case PREF_VIEW_1:
                resize(112, 32, 100, 136);

                break;
            case PREF_VIEW_2:
                resize(112, 48, 100, 104);

                break;
            case PREF_VIEW_3:
                resize(112, 64, 100, 72);

                break;
            case PREF_VIEW_FULL:
            default:
                resize(T_LEFT_THREE, T_UP_THREE, T_HALF_WIDTH, T_HALF_HEIGHT);

                break;
            }

            break;

            // End of mynum switch
        }

        break;
    case 4:
    default:
        // Four player mode
        // Left or right view?
        switch (mynum) {
        case 0:
            resize(T_LEFT_ONE, T_UP_ONE, T_HALF_WIDTH, T_HALF_HEIGHT);

            break;
        case 1:
            resize(T_LEFT_TWO, T_UP_TWO, T_HALF_WIDTH, T_HALF_HEIGHT);

            break;
        case 2:
            resize(T_LEFT_THREE_FOUR, T_UP_THREE, T_HALF_WIDTH, T_HALF_HEIGHT);

            break;
        case 3:
        default:
            resize(T_LEFT_FOUR, T_UP_FOUR, T_HALF_WIDTH, T_HALF_HEIGHT);

            break;

            // End of mynum switch
        }

        break;

        // End of numviews switch
    }

    // End of resize(whatmode)
}

void ViewScreen::view_team()
{
    view_team(VIEW_TEAM_LEFT, VIEW_TEAM_TOP, VIEW_TEAM_RIGHT, VIEW_TEAM_BOTTOM);
}

void ViewScreen::view_team(Sint16 left, Sint16 top, Sint16 right, Sint16 bottom)
{
    Uint8 teamnum = my_team;
    Uint8 text_down = top + 3;
    Uint8 hpcolor;
    Uint8 mpcolor;
    Uint8 namecolor;
    Uint8 numguys = 0;
    float hp;
    float mp;
    float maxhp;
    float maxmp;
    Text &mytext = myscreen->text_normal;
    Sint32 currentcycle = 0;
    Sint32 cycletime = 30000;

    myscreen->redrawme = 1;
    myscreen->draw_button(left, top, right, bottom, 2);

    mytext.write_xy(left + 5, text_down, "  Name  ", BLACK);
    mytext.write_xy(left + 80, text_down, "Health", BLACK);
    mytext.write_xy(left + 140, text_down, "Power", BLACK);
    mytext.write_xy(left + 190, text_down, "Level", BLACK);

    text_down += 6;

    // Build the list of character
    std::list<Walker *> ls;

    for (auto const & w : myscreen->level_data.oblist) {
        if (w
            && !w->dead
            && (w->query_order() == ORDER_LIVING)
            && (w->team_num == teamnum)
            && (!w->stats.name.empty() || w->myguy)) {
            ls.push_back(w);
        }
    }

    // Note: The old code sorted the list by hitpoints. I would do that again,
    // but I'll probably just be removing this function anyway

    // Go through the list and draw the entries
    for (auto & w : ls) {
        if (w) {
            if (numguys > 30) {
                ++numguys;

                break;
            }

            ++numguys;

            hp = w->stats.hitpoints;
            mp = w->stats.magicpoints;
            maxhp = w->stats.max_hitpoints;
            maxmp = w->stats.max_magicpoints;

            if ((hp * 3) < maxhp) {
                hpcolor = LOW_HP_COLOR;
            } else if (((hp * 3) / 2) < maxhp) {
                hpcolor = MID_HP_COLOR - 3;
            } else if (hp < maxhp) {
                hpcolor = MAX_HP_COLOR + 4;
            } else if (hp == maxhp) {
                hpcolor = HIGH_HP_COLOR + 2;
            } else {
                hpcolor = ORANGE_START;
            }

            if ((mp * 3) < maxmp) {
                mpcolor = LOW_MP_COLOR;
            } else if (((mp * 3) / 2) < maxmp) {
                mpcolor = MID_MP_COLOR;
            } else if (mp < maxmp) {
                mpcolor = MAX_MP_COLOR;
            } else if (mp == maxmp) {
                mpcolor = HIGH_MP_COLOR + 3;
            } else {
                mpcolor = WATER_START;
            }

            if (w == control) {
                namecolor = RED;
            } else {
                namecolor = BLACK;
            }

            if (w->myguy) {
                mytext.write_xy(left + 5, text_down, w->myguy->name, namecolor);
            } else {
                mytext.write_xy(left + 5, text_down, w->stats.name, namecolor);
            }

            std::stringstream buf;
            std::streamsize orig_width = buf.width();

            if (ceilf(hp) > 0) {
                buf << std::setw(4) << ceilf(hp);
                buf.width(orig_width);
            }

            buf << "/";

            if (maxhp > 0) {
                buf << maxhp;
            }

            mytext.write_xy(left + 70, text_down, buf.str(), hpcolor);
            buf.clear();

            if (ceilf(mp) > 0) {
                buf << std::setw(4) << ceilf(mp);
                buf.width(orig_width);
            }

            buf << "/";

            if (maxmp > 0) {
                buf << maxmp;
            }

            mytext.write_xy(left + 130, text_down, buf.str(), mpcolor);
            buf.clear();

            buf << std::setw(2) << w->stats.level;
            buf.width(orig_width);
            mytext.write_xy(left + 195, text_down, buf.str(), BLACK);
            buf.clear();

            text_down += 6;
        }
    }

    myscreen->swap();

    while (!keystates[KEYSTATE_ESCAPE]) {
        myscreen->do_cycle(currentcycle, cycletime);
        ++currentcycle;
        get_input_events(POLL);
    }

    while (keystates[KEYSTATE_ESCAPE]) {
        get_input_events(WAIT);
    }
}

void ViewScreen::options_menu()
{
    Text &optiontext = myscreen->text_normal;
    Sint32 gamespeed;
    std::stringstream buf;
    std::streamsize orig_width = buf.width();
    std::string tempstr;
    Sint8 gamma = prefs[PREF_GAMMA];

    if (!control) {
        SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "No control in ViewScreen::options_menu()\n");

        // Safety check; Shouldn't happen
        return;
    }

    clear_keyboard();

    // Draw the menu button
    myscreen->draw_button(40, 40, 280, 160, 2, 1);
    myscreen->draw_text_bar(40 + 4, 40 + 4, 280 - 4, 40 + 12);

    buf << "Options Menu (" << mynum + 1 << ")";
    std::string title(buf.str());
    buf.clear();
    title.resize(50);
    optiontext.write_xy(160 - (6 * 6), OPLINES(0) + 2, title, RED, 1);

    gamespeed = change_speed(0);
    buf << "Change Game Speed (+/-): " << std::setw(2) << gamespeed;
    buf.width(orig_width);
    buf << "  ";
    optiontext.write_xy(LEFT_OPS, OPLINES(2), buf.str(), BLACK, 1);
    buf.clear();

    buf << "Change View Size ([,]) : ";

    switch (prefs[PREF_VIEW]) {
    case PREF_VIEW_FULL:
        buf << "Full Screen";

        break;
    case PREF_VIEW_PANELS:
        buf << "Large";

        break;
    case PREF_VIEW_1:
        buf << "Medium";

        break;
    case PREF_VIEW_2:
        buf << "Small";

        break;
    case PREF_VIEW_3:
        buf << "Tiny";

        break;
    default:
        buf << "Weird";

        break;
    }

    tempstr = buf.str();
    buf.clear();

    myscreen->draw_box(LEFT_OPS, OPLINES(3), LEFT_OPS + (tempstr.length() * 6), OPLINES(3) + 6, PANEL_COLOR, 1, 1);

    optiontext.write_xy(LEFT_OPS, OPLINES(3), tempstr, BLACK, 1);

    gamma = change_gamma(0);
    buf << "Change Brightness (<,>): " << gamma << " ";
    myscreen->draw_box(45, OPLINES(4), 275, OPLINES(4) + 6, PANEL_COLOR, 1, 1);
    optiontext.write_xy(LEFT_OPS, OPLINES(4), buf.str(), BLACK, 1);
    buf.clear();

    if (prefs[PREF_RADAR]) {
        buf << "Radar Display (R)      : ON ";
    } else {
        buf << "Radar Display (R)      : OFF ";
    }

    myscreen->draw_box(42, OPLINES(5), 275, OPLINES(5) + 6, PANEL_COLOR, 1, 1);
    optiontext.write_xy(LEFT_OPS, OPLINES(5), buf.str(), BLACK, 1);
    buf.clear();

    buf << "Hitpoint Display (H)   : ";

    switch (prefs[PREF_LIFE]) {
    case PREF_LIFE_TEXT:
        buf << "Text Only";

        break;
    case PREF_LIFE_BARS:
        buf << "Bars Only";

        break;
    case PREF_LIFE_BOTH:
        buf << "Bars and Text";

        break;
    case PREF_LIFE_OFF:
        buf << "Off";

        break;
    case PREF_LIFE_SMALL:
    default:
        buf << "On";

        break;
    }

    myscreen->draw_box(45, OPLINES(6), 275, OPLINES(6) + 6, PANEL_COLOR, 1, 1);
    optiontext.write_xy(LEFT_OPS, OPLINES(6), buf.str(), BLACK, 1);
    buf.clear();

    if (prefs[PREF_FOES]) {
        buf << "Foes Display (F)       : ON ";
    } else {
        buf << "Foes Display (F)       : OFF ";
    }

    myscreen->draw_box(45, OPLINES(7), 275, OPLINES(7) + 6, PANEL_COLOR, 1, 1);
    optiontext.write_xy(LEFT_OPS, OPLINES(7), buf.str(), BLACK, 1);
    buf.clear();

    if (prefs[PREF_SCORE]) {
        buf << "Score Display (S)      : ON ";
    } else {
        buf << "Score Display (S)      : OFF ";
    }

    myscreen->draw_box(45, OPLINES(8), 275, OPLINES(8) + 6, PANEL_COLOR, 1, 1);
    optiontext.write_xy(LEFT_OPS, OPLINES(8), buf.str(), BLACK, 1);
    buf.clear();

    optiontext.write_xy(LEFT_OPS, OPLINES(9), "VIEW TEAM INFO (T)", BLACK, 1);

    if (myscreen->cyclemode) {
        buf << "Color Cycling (C)      : ON ";
    } else {
        buf << "Color Cycling (C)      : OFF ";
    }

    myscreen->draw_box(45, OPLINES(10), 275, OPLINES(10) + 6, PANEL_COLOR, 1, 1);
    optiontext.write_xy(LEFT_OPS, OPLINES(10), buf.str(), BLACK, 1);
    buf.clear();

    if (!playerHasJoystick(mynum)) {
        buf << "Joystick Mode (J)      : OFF ";
    } else {
        buf << "Joystick Mode (J)      : ON ";
    }

    myscreen->draw_box(45, OPLINES(11), 275, OPLINES(11) + 6, PANEL_COLOR, 1, 1);
    optiontext.write_xy(LEFT_OPS, OPLINES(11), buf.str(), BLACK, 1);
    buf.clear();

    optiontext.write_xy(LEFT_OPS, OPLINES(12), "EDIT KEY PREFS (K)", BLACK, 1);

    if (prefs[PREF_OVERLAY]) {
        buf << "Text-button Display (B): ON ";
    } else {
        buf << "Text-button Display (B): OFF";
    }

    optiontext.write_xy(LEFT_OPS, OPLINES(13), buf.str(), BLACK, 1);
    buf.clear();

    // Draw the current screen
    myscreen->buffer_to_screen(0, 0, 320, 200);

    // Wait for esc for now
    while (!keystates[KEYSTATE_ESCAPE]) {
        get_input_events(POLL);

        // Faster game speed
        if (keystates[KEYSTATE_KP_PLUS]) {
            gamespeed = change_speed(1);
            buf << "Change Game Speed (+/-): " << std::setw(2) << gamespeed;
            buf.width(orig_width);
            buf << "  ";
            tempstr = buf.str();
            buf.clear();
            myscreen->draw_box(LEFT_OPS, OPLINES(2), LEFT_OPS + (tempstr.length() * 6), OPLINES(2) + 6, PANEL_COLOR, 1, 1);
            optiontext.write_xy(LEFT_OPS, OPLINES(2), tempstr, BLACK, 1);
            myscreen->buffer_to_screen(0, 0, 320, 200);

            while (keystates[KEYSTATE_KP_PLUS]) {
                get_input_events(WAIT);
            }
        }

        // Slower game speed
        if (keystates[KEYSTATE_KP_MINUS]) {
            gamespeed = change_speed(-1);
            buf << "Change Game Speed (+/-): " << std::setw(2) << gamespeed;
            buf.width(orig_width);
            buf << "  ";
            tempstr = buf.str();
            buf.clear();
            myscreen->draw_box(LEFT_OPS, OPLINES(2), LEFT_OPS + (tempstr.length() * 6), OPLINES(2) + 6, PANEL_COLOR, 1, 1);
            optiontext.write_xy(LEFT_OPS, OPLINES(2), tempstr, BLACK, 1);
            myscreen->buffer_to_screen(0, 0, 320, 200);

            while (keystates[KEYSTATE_KP_MINUS]) {
                get_input_events(WAIT);
            }
        }

        // Smaller view size
        if (keystates[KEYSTATE_LEFTBRACKET]) {
            prefs[PREF_VIEW] = prefs[PREF_VIEW] + 1;

            prefs[PREF_VIEW] = std::min(prefs[PREF_VIEW], static_cast<Uint8>(4));

            resize(prefs[PREF_VIEW]);

            buf << "Change View Size ([,]) : ";

            switch (prefs[PREF_VIEW]) {
            case PREF_VIEW_FULL:
                buf << "Full Screen";

                break;
            case PREF_VIEW_PANELS:
                buf << "Large";

                break;
            case PREF_VIEW_1:
                buf << "Medium";

                break;
            case PREF_VIEW_2:
                buf << "Small";

                break;
            case PREF_VIEW_3:
                buf << "Tiny";

                break;
            default:
                buf << "Wierd";

                break;
            }

            buf << "       ";
            myscreen->draw_box(45, OPLINES(3), 275, OPLINES(3) + 6, PANEL_COLOR, 1, 1);
            optiontext.write_xy(LEFT_OPS, OPLINES(2), buf.str(), BLACK, 1);
            myscreen->buffer_to_screen(0, 0, 320, 200);

            while (keystates[KEYSTATE_LEFTBRACKET]) {
                get_input_events(WAIT);
            }
        }

        // Larger view size
        if (keystates[KEYSTATE_RIGHTBRACKET]) {
            prefs[PREF_VIEW] = prefs[PREF_VIEW] - 1;

            prefs[PREF_VIEW] = std::max(prefs[PREF_VIEW], static_cast<Uint8>(0));

            resize(prefs[PREF_VIEW]);

            buf << "Change View Size ([,]) : ";
            switch (prefs[PREF_VIEW]) {
            case PREF_VIEW_FULL:
                buf << "Full Screen";

                break;
            case PREF_VIEW_PANELS:
                buf << "Large";

                break;
            case PREF_VIEW_1:
                buf << "Medium";

                break;
            case PREF_VIEW_2:
                buf << "Small";

                break;
            case PREF_VIEW_3:
                buf << "Tiny";

                break;
            default:
                buf << "Weird";

                break;
            }

            buf << "  ";

            myscreen->draw_box(45, OPLINES(3), 275, OPLINES(3) + 6, PANEL_COLOR, 1, 1);
            optiontext.write_xy(LEFT_OPS, OPLINES(3), buf.str(), BLACK, 1);
            buf.clear();
            myscreen->buffer_to_screen(0, 0, 320, 200);

            while (keystates[KEYSTATE_RIGHTBRACKET]) {
                get_input_events(WAIT);
            }
        }

        // Darken screen
        if (keystates[KEYSTATE_COMMA]) {
            gamma = change_gamma(-2);
            prefs[PREF_GAMMA] = gamma;
            buf << "Change Brightness (<,>): " << gamma << " ";
            myscreen->draw_box(45, OPLINES(4), 275, OPLINES(4) + 6, PANEL_COLOR, 1, 1);
            optiontext.write_xy(LEFT_OPS, OPLINES(4), buf.str(), BLACK, 1);
            buf.clear();
            myscreen->buffer_to_screen(0, 0, 320, 200);

            while (keystates[KEYSTATE_COMMA]) {
                get_input_events(WAIT);
            }
        }

        // Lighten screen
        if (keystates[KEYSTATE_PERIOD]) {
            gamma = change_gamma(2);
            prefs[PREF_GAMMA] = gamma;
            buf << "Change Brightness (<,>): " << gamma << " ";
            myscreen->draw_box(45, OPLINES(4), 275, OPLINES(4) + 6, PANEL_COLOR, 1, 1);
            optiontext.write_xy(LEFT_OPS, OPLINES(4), buf.str(), BLACK, 1);
            buf.clear();
            myscreen->buffer_to_screen(0, 0, 320, 200);

            while (keystates[KEYSTATE_COMMA]) {
                get_input_events(WAIT);
            }
        }

        // Toggle radar display
        if (keystates[KEYSTATE_r]) {
            prefs[PREF_RADAR] = (prefs[PREF_RADAR] + 1) % 2;

            if (prefs[PREF_RADAR]) {
                buf << "Radar Display (R)      : ON ";
            } else {
                buf << "Radar Display (R)      : OFF ";
            }

            myscreen->draw_box(45, OPLINES(5), 275, OPLINES(5) + 6, PANEL_COLOR, 1, 1);
            optiontext.write_xy(LEFT_OPS, OPLINES(5), buf.str(), BLACK, 1);
            buf.clear();
            myscreen->buffer_to_screen(0, 0, 320, 200);

            while (keystates[KEYSTATE_r]) {
                get_input_events(WAIT);
            }
        }

        // Toggle HP display
        if (keystates[KEYSTATE_h]) {
            prefs[PREF_LIFE] = (prefs[PREF_LIFE] + 1) % 5;

            buf << "Hitpoint Display (H)   : ";
            switch (prefs[PREF_LIFE]) {
            case PREF_LIFE_TEXT:
                buf << "Text Only";

                break;
            case PREF_LIFE_BARS:
                buf << "Bars Only";

                break;
            case PREF_LIFE_BOTH:
                buf << "Bars and Text";

                break;
            case PREF_LIFE_OFF:
                buf << "Off";

                break;
            case PREF_LIFE_SMALL:
            default:
                buf << "On";

                break;
            }

            myscreen->draw_box(45, OPLINES(6), 275, OPLINES(6) + 6, PANEL_COLOR, 1, 1);
            optiontext.write_xy(LEFT_OPS, OPLINES(6), buf.str(), BLACK, 1);
            buf.clear();
            myscreen->buffer_to_screen(0, 0, 320, 200);

            while (keystates[KEYSTATE_h]) {
                get_input_events(WAIT);
            }
        }

        // Toggle foes display
        if (keystates[KEYSTATE_f]) {
            prefs[PREF_FOES] = (prefs[PREF_FOES] + 1) % 2;

            if (prefs[PREF_FOES]) {
                buf << "Foes Display (F)       : ON ";
            } else {
                buf << "Foes Display (F)       : OFF ";
            }

            myscreen->draw_box(45, OPLINES(7), 275, OPLINES(7) + 6, PANEL_COLOR, 1, 1);
            optiontext.write_xy(LEFT_OPS, OPLINES(7), buf.str(), BLACK, 1);
            buf.clear();
            myscreen->buffer_to_screen(0, 0, 320, 200);

            while (keystates[KEYSTATE_s]) {
                get_input_events(WAIT);
            }
        }

        // Toggle score display
        if (keystates[KEYSTATE_s]) {
            prefs[PREF_SCORE] = (prefs[PREF_SCORE] + 1) % 2;

            if (prefs[PREF_SCORE]) {
                buf << "Score Display (S)      : ON ";
            } else {
                buf << "Score Display (S)      : OFF ";
            }

            myscreen->draw_box(45, OPLINES(8), 285, OPLINES(8) + 6, PANEL_COLOR, 1, 1);
            optiontext.write_xy(LEFT_OPS, OPLINES(8), buf.str(), BLACK, 1);
            buf.clear();
            myscreen->buffer_to_screen(0, 0, 320, 200);

            while (keystates[KEYSTATE_s]) {
                get_input_events(WAIT);
            }
        }

        // View the teamlist
        if (keystates[KEYSTATE_t]) {
            view_team();
            myscreen->redraw();
            options_menu();

            return;
        }

        if (keystates[KEYSTATE_c]) {
            myscreen->cyclemode = (myscreen->cyclemode + 1) % 2;

            while (keystates[KEYSTATE_c]) {
                get_input_events(WAIT);
            }

            if (myscreen->cyclemode) {
                buf << "Color Cycling (C)      : ON ";
            } else {
                buf << "Color Cycling (C)      : OFF ";
            }

            myscreen->draw_box(45, OPLINES(10), 275, OPLINES(10) + 6, PANEL_COLOR, 1, 1);
            optiontext.write_xy(LEFT_OPS, OPLINES(10), buf.str(), BLACK, 1);
            buf.clear();
            myscreen->buffer_to_screen(0, 0, 320, 200);
        }

        // Toggle joystick display
        if (keystates[KEYSTATE_j]) {
            if (playerHasJoystick(mynum)) {
                disablePlayerJoystick(mynum);
            } else {
                resetJoystick(mynum);
            }

            // Update joystick display message
            if (!playerHasJoystick(mynum)) {
                buf << "Joystick Mode (J)      : OFF ";
            } else {
                buf << "Joystick Mode (J)      : ON ";
            }

            myscreen->draw_box(45, OPLINES(11), 275, OPLINES(11) + 6, PANEL_COLOR, 1, 1);
            optiontext.write_xy(LEFT_OPS, OPLINES(11), buf.str(), BLACK, 1);
            myscreen->buffer_to_screen(0, 0, 320, 200);

            SDL_Delay(500);
            clear_events();
        }

        // Edit the keyboard mappings
        if (keystates[KEYSTATE_k]) {
            if (set_key_prefs()) {
                set_display_text("NEW KEYBOARD STATE SAVED", 30);
                set_display_text("DELETE KEYPREFS.DAT FOR DEFAULTS", 30);
            }

            myscreen->redraw();
            options_menu();

            return;
        }

        // Toggle button display
        if (keystates[KEYSTATE_b]) {
            prefs[PREF_OVERLAY] = (prefs[PREF_OVERLAY] + 1) % 2;

            if (prefs[PREF_OVERLAY]) {
                buf << "Text-button Display (B): ON ";
            } else {
                buf << "Text-button Display (B): OFF ";
            }

            myscreen->draw_box(45, OPLINES(13), 275, OPLINES(13) + 6, PANEL_COLOR, 1, 1);
            optiontext.write_xy(LEFT_OPS, OPLINES(13), buf.str(), BLACK, 1);
            buf.clear();
            myscreen->buffer_to_screen(0, 0, 320, 200);

            while (keystates[KEYSTATE_b]) {
                get_input_events(WAIT);
            }
        }

        // End of wait for ESC press
    }

    while (keystates[KEYSTATE_ESCAPE]) {
        get_input_events(WAIT);
    }

    myscreen->redrawme = 1;
    prefsob->save(mynum, prefs, mykeys);
}

Sint32 ViewScreen::change_speed(Sint32 whichway)
{
    if (whichway > 0) {
        myscreen->timer_wait = std::max(myscreen->timer_wait - 2, 0);
    } else if (whichway < 0) {
        myscreen->timer_wait = std::min(myscreen->timer_wait + 2, 20);
    }

    return ((20 - myscreen->timer_wait) / 2) + 1;
}

Sint32 ViewScreen::change_gamma(Sint32 whichway)
{
    if (whichway > 1) {
        // Lighter
        load_palette(myscreen->newpalette);
        ++gamma;
        adjust_palette(myscreen->newpalette, gamma);
    } else if (whichway < -1) {
        // Darker
        load_palette(myscreen->newpalette);
        --gamma;
        adjust_palette(myscreen->newpalette, gamma);
    } else {
        // Set to default
        gamma = 0;
        load_palette(myscreen->newpalette);
    }

    // So 0 just means report
    return gamma;
}

// set_key_prefs queries the user for key preferences, and places them into
// the proper key-press array.
// It returns success or failure.
bool ViewScreen::set_key_prefs()
{
    Text &keytext = myscreen->text_normal;

    clear_keyboard();

    // Draw the menu button
    // Same as options menu
    myscreen->draw_button(40, 40, 280, 160, 2, 1);
    keytext.write_xy(160 - (6 * 6), OPLINES(0), "Keyboard Menu", RED, 1);
    myscreen->buffer_to_screen(0, 0, 320, 200);

    keytext.write_xy(LEFT_OPS, OPLINES(2), "Press a key for 'UP':", RED, 1);
    myscreen->buffer_to_screen(0, 0, 320, 200);
    assignKeyFromWaitEvent(mynum, KEY_UP);

    keytext.write_xy(LEFT_OPS, OPLINES(3), "Press a key for 'UP-RIGHT':", RED, 1);
    myscreen->buffer_to_screen(0, 0, 320, 200);
    assignKeyFromWaitEvent(mynum, KEY_UP_RIGHT);

    keytext.write_xy(LEFT_OPS, OPLINES(5), "Press a key for 'DOWN-RIGHT':", RED, 1);
    myscreen->buffer_to_screen(0, 0, 320, 200);
    assignKeyFromWaitEvent(mynum, KEY_DOWN_RIGHT);

    keytext.write_xy(LEFT_OPS, OPLINES(6), "Press a key for 'DOWN':", RED, 1);
    myscreen->buffer_to_screen(0, 0, 320, 200);
    assignKeyFromWaitEvent(mynum, KEY_DOWN);

    keytext.write_xy(LEFT_OPS, OPLINES(7), "Press a key for 'DOWN-LEFT':", RED, 1);
    myscreen->buffer_to_screen(0, 0, 320, 200);
    assignKeyFromWaitEvent(mynum, KEY_DOWN_LEFT);

    keytext.write_xy(LEFT_OPS, OPLINES(8), "Press a key for 'LEFT':", RED, 1);
    myscreen->buffer_to_screen(0, 0, 320, 200);
    assignKeyFromWaitEvent(mynum, KEY_LEFT);

    keytext.write_xy(LEFT_OPS, OPLINES(9), "Press a key for 'Up-LEFT':", RED, 1);
    myscreen->buffer_to_screen(0, 0, 320, 200);
    assignKeyFromWaitEvent(mynum, KEY_UP_LEFT);

    // Draw the menu button; back to the top for us!
    // Same as options menu
    myscreen->draw_button(40, 40, 280, 160, 2, 1);
    keytext.write_xy(160 - (6 * 6), OPLINES(0), "Keyboard Menu", RED, 1);
    myscreen->buffer_to_screen(0, 0, 320, 200);

    keytext.write_xy(LEFT_OPS, OPLINES(2), "Press your 'FIRE' key:", RED, 1);
    myscreen->buffer_to_screen(0, 0, 320, 200);
    assignKeyFromWaitEvent(mynum, KEY_FIRE);

    keytext.write_xy(LEFT_OPS, OPLINES(3), "Press your 'SPECIAL' key:", RED, 1);
    myscreen->buffer_to_screen(0, 0, 320, 200);
    assignKeyFromWaitEvent(mynum, KEY_SPECIAL);

    keytext.write_xy(LEFT_OPS, OPLINES(4), "Press your 'SPECIAL SWITCH' key:", RED, 1);
    myscreen->buffer_to_screen(0, 0, 320, 200);
    assignKeyFromWaitEvent(mynum, KEY_SPECIAL_SWITCH);

    keytext.write_xy(LEFT_OPS, OPLINES(5), "Press your 'YELL' key:", RED, 1);
    myscreen->buffer_to_screen(0, 0, 320, 200);
    assignKeyFromWaitEvent(mynum, KEY_YELL);

    keytext.write_xy(LEFT_OPS, OPLINES(6), "Press your 'SWITCHING' key:", RED, 1);
    myscreen->buffer_to_screen(0, 0, 320, 200);
    assignKeyFromWaitEvent(mynum, KEY_SHIFTER);

    // Are cheats enabled?
    if (CHEAT_MODE) {
        keytext.write_xy(LEFT_OPS, OPLINES(9), "Press your 'CHEATS' key:", RED, 1);
        myscreen->buffer_to_screen(0, 0, 320, 200);
        assignKeyFromWaitEvent(mynum, KEY_CHEAT);
    }

    myscreen->redrawme = 1;
    // return save_key_prefs();

    return true;
}

Sint16 ViewScreen::top_x()
{
    return (topx + xview);
}

Sint16 ViewScreen::top_y()
{
    return (topy + yview);
}

// Waits for a key to be pressed and then released...return this key.
SDL_Keycode get_keypress()
{
    // Clear any previous key
    clear_key_press_event();

    while (!query_key_press_event()) {
        get_input_events(WAIT);
    }

    return query_key();
}
