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
#include "glad.hpp"

#include <cstring>
#include <ctime>
#include <filesystem>
#include <sstream>
#include <string>
#include <vector>

#include "base.hpp"
#include "colors.hpp"
#include "game.hpp"
#include "gparser.hpp"
#include "guy.hpp"
#include "help.hpp"
#include "input.hpp"
#include "intro.hpp"
#include "io.hpp"
#include "joy_data.hpp"
#include "mouse_state.hpp"
#include "options.hpp"
#include "pal32.hpp"
#include "picker.hpp"
#include "results_screen.hpp"
#include "util.hpp"
#include "version.hpp"
#include "view.hpp"
#include "walker.hpp"

#define L_D(x) x * 8

VideoScreen *screen;

Sint16 remaining_team(VideoScreen *myscreen, Uint8 myteam);
Sint16 score_panel(VideoScreen *myscreen);
Sint16 score_panel(VideoScreen *myscreen, Sint16 do_it);
Sint16 new_score_panel(VideoScreen *myscreen, Sint16 do_it);
void draw_value_bar(Sint16 left, Sint16 top, Walker *control, Sint16 mode, VideoScreen *myscreen);
void new_draw_value_bar(Sint16 left, Sint16 top, Walker *control, Sint16 mode, VideoScreen *myscreen);
void draw_percentage_bar(Sint16 left, Sint16 top, Uint8 somecolor, Sint16 somelength, VideoScreen *myscreen);
void draw_radar_gems(VideoScreen *myscreen);
void draw_gem(Sint16 x, Sint16 y, Sint16 color, VideoScreen *myscreen);
bool float_eq(float a, float b);

Options *theprefs;
ConfigStore cfg;

int main(int argc, char *argv[])
{
    Sint32 arg_count = argc;
    std::vector<std::string> args;
    for (Sint32 i = arg_count; i <= argc; ++i) {
        args.push_back(argv[i]);
    }

    io_init(std::filesystem::path(argv[0]));

    cfg.load_settings();
    cfg.save_settings();
    cfg.commandline(arg_count, args);

    theprefs = new Options();

    SDL_Init(SDL_INIT_VIDEO);

    myscreen = new VideoScreen(1);

    // buffers: Setting the seed
    srand(time(nullptr));

    init_input();
    init_joysticks();
    intro_main();
    picker_main();

    io_exit();

    return 0;
}

void glad_main(Sint32 playermode)
{
    Sint16 currentcycle = 0;
    Sint16 cycletime = 3;

    // Zardus: PORT: fade out
    clear_keyboard();
    myscreen->fadeblack(0);
    myscreen->clearbuffer();
    // Load the default saved game...
    load_saved_game(myscreen);

    // This will update the ;control' so the screen centers on our guy
    myscreen->continuous_input();

    // Fade in
    myscreen->redraw();
    myscreen->fadeblack(1);

    // Keyboard loop

    // This is the main program loop
    myscreen->redraw();
    myscreen->refresh();
    read_scenario(myscreen);
    myscreen->redrawme = 1;
    myscreen->framecount = 0;
    myscreen->timerstart = query_timer_control();

    bool done = false;

    while (!done) {
        // Reset the time count to zero...
        reset_timer();

        if (myscreen->redrawme) {
            myscreen->draw_panels(myscreen->numviews);
            score_panel(myscreen, 1);
            myscreen->refresh();
            myscreen->redrawme = 0;
        }

        if (myscreen->end) {
            break;
        }

        myscreen->act();
        ++myscreen->framecount;

        if (myscreen->end) {
            break;
        }

        myscreen->redraw();

        if (debug_draw_obmap) {
            myscreen->level_data.myobmap.draw(); // Debug drawing for object collision map
        }

        score_panel(myscreen);
        myscreen->refresh();

        SDL_Event event;

        while (SDL_PollEvent(&event)) {
            handle_events(event);
            handle_joystick_events(event);
            handle_mouse_events(event);

            if (event.type == SDL_KEYDOWN) {

                if (event.key.keysym.sym == SDLK_F11) {
                    debug_draw_paths = !debug_draw_paths;
                } else if (event.key.keysym.sym == SDLK_F12) {
                    debug_draw_obmap = !debug_draw_obmap;
                } else if (event.key.keysym.sym == SDLK_ESCAPE) {
                    bool result = yes_or_no_prompt("Abort Mission", "Quit this mission?", false);
                    myscreen->redrawme = 1;

                    // Player wants to quit
                    if (result) {
                        done = true;
                        results_screen(2, -1); // Should not show an extra popup
                    } else {
                        set_palette(myscreen->ourpalette); // Restore normal palette
                        adjust_palette(myscreen->ourpalette, myscreen->viewob[0]->gamma);
                    }

                    break;
                }
            }

            myscreen->input(event);
        }

        if (myscreen->end || done) {
            break;
        }

        myscreen->continuous_input();

        if (myscreen->end) {
            break;
        }

        // Now cycle palette...
        if (myscreen->cyclemode) {
            myscreen->do_cycle(currentcycle, cycletime);
            ++currentcycle;
        }

        // Zardus: PORT: this is the new FPS capacity
        time_delay(myscreen->timer_wait - query_timer());
    }

    clear_keyboard();

    myscreen->level_data.delete_objects();

    return; // Return to picker
}

// Remaining foes returns # of livings left not on control's team
Sint16 remaining_foes(VideoScreen *myscreen, Walker *myguy)
{
    Sint16 myfoes = 0;

    std::list<Walker *> const &foelist = myscreen->level_data.oblist;

    for (auto itr = foelist.begin(); itr != foelist.end(); itr++) {
        Walker *w = *itr;

        if (w && !w->dead && (w->query_order() == ORDER_LIVING) && !myguy->is_friendly(w)) {
            ++myfoes;
        }
    }

    return myfoes;
}

// Remaining team returns # of livings left on team myteam
Sint16 remaining_team(VideoScreen *myscreen, Uint8 myteam)
{
    Sint16 myfoes = 0;

    std::list<Walker *> const &foelist = myscreen->level_data.oblist;

    for (auto itr=foelist.begin(); itr != foelist.end(); ++itr) {
        Walker *w = *itr;

        if (w && !w->dead && (w->query_order() == ORDER_LIVING) && (myteam == w->team_num)) {
            ++myfoes;
        }
    }

    return myfoes;
}

Sint16 score_panel(VideoScreen *myscreen)
{
    return score_panel(myscreen, 0);
}

Sint16 score_panel(VideoScreen *myscreen, Sint16 do_it)
{
    return new_score_panel(myscreen, 1);
}

void draw_radar_gems(VideoScreen *myscreen)
{
    Sint16 upper_left_x = 246;
    Sint16 upper_left_y = 140;
    Sint16 upper_right_x = upper_left_x + 65;
    Sint16 upper_right_y = upper_left_y;
    Sint16 lower_left_x = upper_left_x;
    Sint16 lower_left_y = upper_left_y + 49;
    Sint16 lower_right_x = upper_right_x;
    Sint16 lower_right_y = lower_left_y;

    Sint16 team_light;

    static Sint16 old_team_num = -1;

    if (old_team_num == myscreen->viewob[0]->control->team_num) {
        return;
    }

    old_team_num = myscreen->viewob[0]->control->team_num;
    team_light = myscreen->viewob[0]->control->query_team_color();

    draw_gem(upper_left_x, upper_left_y, team_light, myscreen);
    draw_gem(upper_right_x, upper_right_y, team_light, myscreen);
    draw_gem(lower_left_x, lower_left_y, team_light, myscreen);
    draw_gem(lower_right_x, lower_right_y, team_light, myscreen);
}

void draw_gem(Sint16 x, Sint16 y, Sint16 color, VideoScreen *myscreen)
{
    Sint16 light = color;
    Sint16 med = light + 2;
    Sint16 darker = med + 2;
    Sint16 darkest = darker + 2;

    myscreen->point(x, y, light);
    myscreen->point(x - 1, y + 1, light);
    myscreen->point(x, y + 1, med);
    myscreen->point(x + 1, y + 1, darker);
    myscreen->point(x - 2, y + 2, light);
    myscreen->hor_line(x - 1, y + 2, 3, med);
    myscreen->point(x + 2, y + 2, darkest);
    myscreen->point(x - 1, y + 3, darker);
    myscreen->point(x, y + 3, med);
    myscreen->point(x + 1, y + 3, darkest);
    myscreen->point(x, y + 4, darkest);
}

void draw_value_bar(Sint16 left, Sint16 top, Walker *control, Sint16 mode, VideoScreen *myscreen)
{
    float points;
    Sint16 totallength = 60;
    Sint16 bar_length = 0;
    Sint16 bar_remainder = totallength - bar_length;
    Sint16 i;
    Sint16 j;
    Uint8 whatcolor;

    // Hitpoint bar
    if (mode == 0) {
        points = control->stats.hitpoints;

        if (float_eq(points, control->stats.max_hitpoints)) {
            whatcolor = MAX_HP_COLOR;
        } else if ((points * 3) < control->stats.max_hitpoints) {
            whatcolor = LOW_HP_COLOR;
        } else if (((points * 3) / 2) < control->stats.max_hitpoints) {
            whatcolor = MID_HP_COLOR;
        } else if (points < control->stats.max_hitpoints) {
            whatcolor = HIGH_HP_COLOR;
        } else {
            whatcolor = ORANGE_START;
        }

        if (points > control->stats.max_hitpoints) {
            bar_length = 60;
        } else {
            bar_length = ceilf((points * 60) / control->stats.max_hitpoints);
        }

        bar_remainder = 60 - bar_length;

        myscreen->draw_box(left, top, left + 61, top + 6, BOX_COLOR, 0);

        if (points > control->stats.max_hitpoints) {
            for (i = 0; i < (bar_length / 2); ++i) {
                for (j = 0; j < 3; ++j) {
                    myscreen->ver_line((left + (bar_length / 2)) - i, (top + 3) - j, 1, whatcolor + ((i + j) % 16));
                    myscreen->ver_line((left + (bar_length / 2)) - i, (top + 3) + j, 1, whatcolor + ((i + j) % 16));
                    myscreen->ver_line((left + (bar_length / 2)) + i, (top + 3) - j, 1, whatcolor + ((i + j) % 16));
                    myscreen->ver_line((left + (bar_length / 2)) + i, (top + 3) + j, 1, whatcolor + ((i + j) % 16));
                }
            }
        } else {
            myscreen->fastbox(left + 1, top + 1, bar_length, 5, whatcolor);
        }

        myscreen->fastbox((left + bar_length) + 1, top + 1, bar_remainder, 5, BAR_BACK_COLOR);

        // This part rounds the corners (via 4 masks)
        for (i = 0; i < 4; ++i) {
            // Upper left
            myscreen->ver_line(left + i, top, 3 - i, 0);

            if ((2 - i) > 0) {
                myscreen->ver_line(left + i, top, 2 - i, 27);
            }

            // Upper right
            myscreen->ver_line((left + 61) - i, top, 3 - i, 0);

            if ((2 - i) > 0) {
                myscreen->ver_line((left + 61) - i, top, 2 - 1, 27);
            }

            // Lower left
            myscreen->ver_line(left + i, (top + 4) + i, 3 - i, 0);

            if ((2 - i) > 0) {
                myscreen->ver_line(left + i, (top + 5) + i, 2 - i, 27);
            }

            // Lower right
            myscreen->ver_line((left + 61) - i, (top + 4) + i, 3 - i, 0);

            if ((2 - i) > 0) {
                myscreen->ver_line((left + 61) - i, (top + 5) + i, 2 - i, 27);
            }
        }
        // End of doing hp stuff...
    } else if (mode == 1) {
        // sp stuff...
        points = control->stats.magicpoints;

        if (float_eq(points, control->stats.max_magicpoints)) {
            whatcolor = MAX_MP_COLOR;
        } else if ((points * 3) < control->stats.max_magicpoints) {
            whatcolor = LOW_MP_COLOR;
        } else if (((points * 3) / 2) < control->stats.max_magicpoints) {
            whatcolor = MID_MP_COLOR;
        } else if (points < control->stats.max_magicpoints) {
            whatcolor = HIGH_MP_COLOR;
        } else {
            whatcolor = WATER_START;
        }

        if (points > control->stats.max_magicpoints) {
            bar_length = 60;
        } else {
            bar_length = ceilf((points * 60) / control->stats.max_magicpoints);
        }

        bar_remainder = 60 - bar_length;

        myscreen->draw_box(left, top, left + 61, top + 6, BOX_COLOR, 0);

        if (points > control->stats.max_magicpoints) {
            for (i = 0; i < (bar_length / 2); ++i) {
                for (j = 0; j < 3; ++j) {
                    myscreen->ver_line((left + (bar_length / 2)) - i, (top + 3) - j, 1, whatcolor + ((i + j) % 16));
                    myscreen->ver_line((left + (bar_length / 2)) - i, (top + 3) + j, 1, whatcolor + ((i + j) % 16));
                    myscreen->ver_line((left + (bar_length / 2)) + i, (top + 3) - j, 1, whatcolor + ((i + j) % 16));
                    myscreen->ver_line((left + (bar_length / 2)) + i, (top + 3) + j, 1, whatcolor + ((i + j) % 16));
                }
            }
        } else {
            myscreen->fastbox(left + 1, top + 1, bar_length, 5, whatcolor);
        }

        myscreen->fastbox((left + bar_length) + 1, top + 1, bar_remainder, 5, BAR_BACK_COLOR);

        // This part rounds the corners (via 4 masks)
        for (i = 0; i < 4; ++i) {
            // Upper left
            myscreen->ver_line(left + i, top, 3 - i, 0);

            if ((2 - i) > 0) {
                myscreen->ver_line(left + i, top, 2 - i, 27);
            }

            // Upper right
            myscreen->ver_line((left + 61) - i, top, 3 - i, 0);

            if ((2 - i) > 0) {
                myscreen->ver_line((left + 61) - i, top, 2 - i, 27);
            }

            // Lower left
            myscreen->ver_line(left + i, (top + 4) + i, 3 - i, 0);

            if ((2 - i) > 0) {
                myscreen->ver_line(left + i, (top + 5) + i, 2 - i, 27);
            }

            // Lower right
            myscreen->ver_line((left + 61) - i, (top + 4) + i, 3 - i, 0);

            if ((2 - i) > 0) {
                myscreen->ver_line((left + 61) - i, (top + 5) + i, 2 - i, 27);
            }
        }
    } // End of sp stuff
} // End of drawing routine...

void new_draw_value_bar(Sint16 left, Sint16 top, Walker *control, Sint16 mode, VideoScreen *myscreen)
{
    float points;
    short bar_length = 0;
    Uint8 whatcolor;

    // Hitpoint bar
    if (mode == 0) {
        points = control->stats.hitpoints;

        if (float_eq(points, control->stats.max_hitpoints)) {
            whatcolor = MAX_HP_COLOR;
        } else if ((points * 3) < control->stats.max_hitpoints) {
            whatcolor = LOW_HP_COLOR;
        } else if (((points * 3) / 2) < control->stats.max_hitpoints) {
            whatcolor = MID_HP_COLOR;
        } else if (points < control->stats.max_hitpoints) {
            whatcolor = HIGH_HP_COLOR;
        } else {
            whatcolor = ORANGE_START;
        }

        if (points > control->stats.max_hitpoints) {
            bar_length = 60;
        } else {
            bar_length = ceilf((points * 60) / control->stats.max_hitpoints);
        }

        draw_percentage_bar(left, top, BAR_BACK_COLOR, 60, myscreen);
        draw_percentage_bar(left, top, whatcolor, bar_length, myscreen);

        // End of doing hp stuf...
    } else if (mode == 1) { // sp stuff...
        points = control->stats.magicpoints;

        if (float_eq(points, control->stats.max_magicpoints)) {
            whatcolor = MAX_MP_COLOR;
        }

        if (float_eq(points, control->stats.max_magicpoints)) {
            whatcolor = MAX_MP_COLOR;
        } else if ((points * 3) < control->stats.max_magicpoints) {
            whatcolor = LOW_MP_COLOR;
        } else if (((points * 3) / 2) < control->stats.max_magicpoints) {
            whatcolor = MID_MP_COLOR;
        } else if (points < control->stats.max_magicpoints) {
            whatcolor = HIGH_MP_COLOR;
        } else {
            whatcolor = WATER_START;
        }

        if (points > control->stats.max_magicpoints) {
            bar_length = 60;
        } else {
            bar_length = ceilf((points * 60) / control->stats.max_magicpoints);
        }

        draw_percentage_bar(left, top, BAR_BACK_COLOR, 60, myscreen);
        draw_percentage_bar(left, top, whatcolor, bar_length, myscreen);
    } // End of sp stuff
} // End of drawing routine...

Sint16 new_score_panel(VideoScreen *myscreen, Sint16 do_it)
{
    std::string message;
    std::string tempname;
    std::stringstream buf;
    Sint16 tempfoes = 0;
    Sint16 players;
    Sint16 tempallies = 0;
    Text &mytext = myscreen->text_normal;

#if 0
    static Uint32 family[5] = {-1, -1, -1, -1, -1};
    static Uint32 act[5] = {-1, -1, -1, -1, -1};
#endif

    Walker *control;
    Sint16 lm; // Left margin
    Sint16 tm; // Top margin
    Sint16 rm; // Right margin
    Sint16 bm; // hottom margin
    Uint8 draw_button; // Do we draw a button backgroun?
    Uint8 text_color;
    Uint32 myscore;

    static std::string namelist[NUM_FAMILIES] = {
        std::string("SOLDIER"),
        std::string("ELF"),
        std::string("ARCHER"),
        std::string("MAGE"),
        std::string("SKELETON"),
        std::string("CLERIC"),
        std::string("ELEMENTAL"),
        std::string("FAERIE"),
        std::string("SLIME"),
        std::string("SLIME"),
        std::string("SLIME"),
        std::string("THIEF"),
        std::string("GHOST"),
        std::string("DRUID"),
        std::string("ORC"),
        std::string("ORC CAPTAIN"),
        std::string("BARBARIAN"),
        std::string("ARCHMAGE"),
        std::string("GOLEM"),
        std::string("GIANT SKEL"),
        std::string("TOWER")
    };

    static Uint32 scorecountup[4] = {
        myscreen->save_data.m_score[0],
        myscreen->save_data.m_score[1],
        myscreen->save_data.m_score[2],
        myscreen->save_data.m_score[3]
    };

    for (players = 0; players < myscreen->numviews; ++players) {
        control = myscreen->viewob[players]->control;
        lm = myscreen->viewob[players]->xloc;
        tm = myscreen->viewob[players]->yloc;
        rm = myscreen->viewob[players]->endx;
        bm = myscreen->viewob[players]->endy;

        if (control && !control->dead && (control->user == players)) {
            // Get the button drawing info...
            draw_button = myscreen->viewob[players]->prefs[PREF_OVERLAY];

            if (draw_button) {
                text_color = DARK_BLUE;
            } else {
                text_color = YELLOW;
            }

            // Get current number of foes
            tempfoes = remaining_foes(myscreen, control);

            // Get current number of team members
            tempallies = remaining_team(myscreen, control->team_num);

            // Display name or type, upper left
            if (control->myguy) {
                tempname = control->myguy->name;
            } else if (!control->stats.name.empty()) {
                tempname = control->stats.name;
            } else {
                tempname = namelist[control->query_family()];
            }

            tempname.resize(20);

            // buffers: This strcpy actually copies the name to be displayed
            message = tempname;

            if (draw_button) {
                myscreen->draw_button(lm + 1, tm + 2, lm + 63, tm + 9, 1, 1);
            }

            mytext.write_xy(lm + 3, tm + 4, message, text_color, 1);

            // HP/MP bars; dependent on user settings
            switch (myscreen->viewob[players]->prefs[PREF_LIFE]) {
            case PREF_LIFE_TEXT: // Display numeric values only
                if (draw_button) {
                    myscreen->draw_button(lm + 1, tm + 10, lm + 63, tm + 26, 1, 1);
                }

                buf << "HP: ";
                if (ceilf(control->stats.hitpoints) != 0) {
                    buf << ceilf(control->stats.hitpoints);
                }

                message = buf.str();
                buf.clear();
                message.resize(50);

                // To buffer
                mytext.write_xy(lm + 5, tm + 12, message, text_color, 1);

                buf << "MP: ";
                if (ceilf(control->stats.magicpoints) != 0) {
                    buf << ceilf(control->stats.magicpoints);
                }

                message = buf.str();
                buf.clear();
                message.resize(50);
                mytext.write_xy(lm + 5, tm + 20, message, text_color, 1);

                break; // end of 'text' case
            case PREF_LIFE_BARS: // Display graphical bars only
                // if (draw_button) {
                //     myscreen->draw_button(lm + 1, tm + 9, lm + 63, tm + 25, 1, 1);
                // }

                new_draw_value_bar(lm + 2, tm + 10, control, 0, myscreen);
                new_draw_value_bar(lm + 2, tm + 18, control, 1, myscreen);

                break; // End of 'bars' case
            case PREF_LIFE_OFF: // Do nothing

                break;
            case PREF_LIFE_BOTH: // Default case
            default:
                // HP STATUS BAR
                // HP_COLORs are defined in graph.hpp

                new_draw_value_bar(lm + 2, tm + 10, control, 0, myscreen);

                buf << "HP: ";
                if (ceilf(control->stats.hitpoints) != 0) {
                    buf << ceilf(control->stats.hitpoints);
                }

                message = buf.str();
                buf.clear();
                message.resize(50);

                // To buffer
                mytext.write_xy(lm + 5, tm + 11, message, BLACK, 1);

                // SP BAR
                // SP_COLORs are defined in graph.hpp
                new_draw_value_bar(lm + 2, tm + 18, control, 1, myscreen);

                buf << "MP: ";
                if (ceilf(control->stats.magicpoints) != 0) {
                    buf << ceilf(control->stats.magicpoints);
                }

                message = buf.str();
                buf.clear();
                message.resize(50);

                mytext.write_xy(lm + 5, tm + 19, message, BLACK, 1);

                break; // End of 'both' case
            }

            if (myscreen->viewob[players]->prefs[PREF_SCORE] == PREF_SCORE_ON) {
                // Score, bottom left corner
                Sint32 special_offset = -24;

                // Draw box, if needed
                if (draw_button) {
                    myscreen->draw_button(lm + 1, bm - 26, lm + 98, bm - 2, 1, 1);
                }

                // Get our score...
                if (control) {
                    myscore = myscreen->save_data.m_score[control->team_num];
                } else {
                    myscore = 0;
                }

                if (scorecountup[control->team_num] > myscore) {
                    scorecountup[control->team_num] = myscore;
                }

                if (scorecountup[control->team_num] < myscore) {
                    ++scorecountup[control->team_num];
                    Sint32 value = (myscore - scorecountup[control->team_num]) / 12;
                    scorecountup[control->team_num] += getRandomSint32(value);
                }

                if (scorecountup[control->team_num] > myscore) {
                    scorecountup[control->team_num] = myscore;
                }

                myscreen->save_data.m_score[control->team_num] = myscore;

                // Above should count up the score towards the current amount

                Sint32 special_y = bm + special_offset;

                // Don't show score and XP (clutter) when in a small viewport
                if ((myscreen->numviews > 2) && ((myscreen->numviews != 3) || (players != 0))) {
                    special_y = bm - 8;
                } else {
                    buf << "SC: " << scorecountup[control->team_num];
                    message = buf.str();
                    buf.clear();
                    message.resize(50);
                    mytext.write_xy(lm + 2, bm - 8, message, text_color, 1);

                    // Level or exp, 2nd bottom left
                    if (control->myguy) {
                        buf << "XP: " << control->myguy->exp;
                    } else {
                        buf << "LEVEL: " << control->stats.level;
                    }

                    message = buf.str();
                    buf.clear();
                    message.resize(50);
                    mytext.write_xy(lm + 2, bm - 16, message, text_color, 1);
                }

                // Currently select special
                buf << "SPC: ";
                if (control->shifter_down
                    && (myscreen->alternate_name[control->query_family()][control->current_special] == "NONE")) {
                    buf << myscreen->alternate_name[control->query_family()][control->current_special];
                } else {
                    buf << myscreen->special_name[control->query_family()][control->current_special];
                }

                message = buf.str();
                buf.clear();
                message.resize(50);

                if (control->stats.magicpoints >= control->stats.special_cost[control->current_special]) {
                    mytext.write_xy(lm + 2, special_y, message, text_color, 1);
                } else {
                    mytext.write_xy(lm + 2, special_y, message, RED, 1);
                }

            } // End of score/exp display

            // Number of allies, upper right
            if (myscreen->viewob[players]->prefs[PREF_FOES] == PREF_FOES_ON) {
                if (draw_button) {
                    myscreen->draw_button(rm - 57, tm + 1, rm - 2, tm + 16, 1, 1);
                }

                buf << "TEAM: " << tempallies;
                message = buf.str();
                buf.clear();
                message.resize(50);

                mytext.write_xy(rm - 55, tm + 2, message, text_color, 1);

                // Number of foes, 2nd upper right
                buf << "FOES: " << tempfoes;
                message = buf.str();
                buf.clear();
                message.resize(50);

                mytext.write_xy(rm - 55, tm + 2, message, text_color, 1);
            }
        }
    } // End of one player mode

    return 1;
}

void draw_percentage_bar(Sint16 left, Sint16 top, Uint8 somecolor, Sint16 somelength, VideoScreen *myscreen)
{
    Sint16 i;
    Sint16 j;
    // Uint8 tempcolor;

    // Draw the black border...
    myscreen->fastbox(left + 2, top, somelength - 4, 1, 0, 1);
    myscreen->fastbox(left + 1, top + 1, 1, 1, 0, 1);
    myscreen->fastbox(left + 58, top + 1, 1, 1, 0, 1);
    myscreen->fastbox(left, top + 2, 1, 3, 0, 1);
    myscreen->fastbox(left + 59, top + 2, 1, 3, 0, 1);
    myscreen->fastbox(left + 1, top + 5, 1, 1, 0, 1);
    myscreen->fastbox(left + 58, top + 5, 1, 1, 0, 1);
    myscreen->fastbox(left + 2, top + 6, somelength - 4, 1, 0, 1);

    // Draw the box...
    myscreen->fastbox(left + 2, top + 1, somelength - 4, 1, somecolor, 1);
    myscreen->fastbox(left + 1, top + 2, somelength - 2, 3, somecolor, 1);
    myscreen->fastbox(left + 2, top + 5, somelength - 4, 1, somecolor, 1);

    // Rotating colors...do special...
    if ((somecolor == ORANGE_START) || (somecolor == WATER_START)) {
        for (i = 0; i < ((somelength - 4) / 2); ++i) {
            for (j = 0; j < 3; ++j) {
                myscreen->fastbox(((left + (somelength / 2)) - i) - 1, (top + 3) - j, 1, 1, somecolor + ((i + j) % 16), 1);
                myscreen->fastbox(((left + (somelength / 2)) - i) - 1, (top + 3) + j, 1, 1, somecolor + ((i + j) % 16), 1);
                myscreen->fastbox((left + (somelength / 2)) + i, (top + 3) - j, 1, 1, somecolor + ((i + j) % 16), 1);
                myscreen->fastbox((left + (somelength / 2)) + i, (top + 2) - j, 1, 1, somecolor + ((i + j) % 16), 1);
            }
        }
    } // End of special color check...
}
