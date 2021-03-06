/*
 * Omega copyright (C) by Laurence Raphael Brothers, 1987-1989
 *
 * oinv.c
 *
 * Functions having to do with player item inventory
 */
#include "oinv.h"

#include <stdlib.h>
#include <string.h>

#include "oaux1.h"
#include "oaux2.h"
#include "ofile.h"
#include "oglob.h"
#include "oitem.h"
#include "olev.h"
#include "omon.h"
#include "oscr.h"
#include "outil.h"

/* Drops money, heh heh */
void drop_money()
{
    pob money;

    money = detach_money();

    if(money != NULL) {
        if(Current_Environment == E_CITY) {
            print1("As soon as the money leaves your hand,");
            print2("a horde of scrofulous beggars snatch it up and are gone!");
        }
        else {
            drop_at(Player.x, Player.y, money);
        }
    }
    else {
        setgamestatus(SKIP_MONSTERS);
    }
}

/*
 * Returns some money from player back into "money" item. For giving
 * and dropping money
 */
pob detach_money()
{
    int c;
    pob cash = NULL;

    c = get_money(Player.cash);

    if(c != ABORT) {
        Player.cash -= c;
        cash = (pob)malloc(sizeof(objtype));
        make_cash(cash, difficulty());
        cash->basevalue = c;
    }

    return cash;
}

/* Gets a legal amount of money or ABORT */
int get_money(int limit)
{
    int c;

    print1("How much? ");
    c = parsenum();

    if(c > limit) {
        print3("Forget it, buddy.");

        return ABORT;
    }
    else {
        return c;
    }
}

/*
 * Pick up from some location x,y. Lift entire itemlist off ground,
 * pass it to inventory control, which may drop things back onto the
 * now null ground
 */
void pickup_at(int x, int y)
{
    int quit = FALSE;
    char response = 'n';
    pol ol = Level->site[x][y].things;
    pol temp;

    resetgamestatus(FAST_MOVE);
    Level->site[x][y].things = NULL;

    while(ol != NULL) {
        if(!quit) {
            clearmsg1();
            print1("Pick up: ");
            nprint1(itemid(ol->thing));
            nprint1(" [ynq]: ");
            response = ynq1();

            if(response == 'q') {
                quit = 1;
            }
            else {
                quit = 0;
            }
        }

        if(response == 'y') {
            gain_item(ol->thing);
        }
        else {
            drop_at(Player.x, Player.y, ol->thing);
        }

        temp = ol;
        ol = ol->next;
        temp->thing = NULL;
        temp->next = NULL;

        free((char *)temp);
    }
}

/*
 * Put all of o on objlist at x,y on Level->depth. Not necessarily
 * dropped by character, just dropped...
 */
void drop_at(int x, int y, pob o)
{
    pol tmp;
    pob cpy;

    if(Current_Environment != E_COUNTRYSIDE) {
        if((Level->site[x][y].locchar != ' ')
           && (Level->site[x][y].locchar != '0')) {
            cpy = (pob)malloc(sizeof(objtype));
            tmp = (pol)malloc(sizeof(oltype));
            *cpy = *o;
            cpy->used = FALSE;
            tmp->thing = cpy;
            tmp->next = Level->site[x][y].things;
            Level->site[x][y].things = tmp;
        }
        else if(Level->site[x][y].p_locf == L_VOID_STATION) {
            setgamestatus(PREPARED_VOID);
        }
    }
}

/* Put n of o on objlist at x,y on Level->depth */
void p_drop_at(int x, int y, int n, pob o)
{
    pol tmp;

    if(Current_Environment != E_COUNTRYSIDE) {
        if((Level->site[x][y].locchar != ' ')
           && (Level->site[x][y].locchar != '0')) {
            tmp = (pol)malloc(sizeof(oltype));
            tmp->thing = (pob)malloc(sizeof(objtype));
            *(tmp->thing) = *o;
            tmp->thing->used = FALSE;
            tmp->thing->number = n;
            print2("Dropped ");
            nprint2(itemid(tmp->thing));
            morewait();
            tmp->next = Level->site[x][y].things;
            Level->site[x][y].things = tmp;
        }
        else if(Level->site[x][y].p_locf == L_VOID_STATION) {
            setgamestatus(PREPARED_VOID);
        }
    }
}

/* Returns a string for identified items */
char *itemid(pob obj)
{
    char tstr[80];

    if(obj->objchar == CASH) {
        strcpy(Str4, obj->truename);

        return Str4;
    }
    else {
        if(Objects[obj->id].known > obj->known) {
            obj->known = Objects[obj->id].known;
        }

        setnumstr(obj, tstr);
        strcpy(Str4, tstr);

        if(obj->known == 0) {
            strcat(Str4, obj->objstr);
        }
        else if(obj->known == 1) {
            strcat(Str4, obj->truename);
        }
        else {
            if(obj->blessing < 0) {
                strcat(Str4, "cursed ");
                strcat(Str4, obj->cursestr);
            }
            else if(obj->blessing > 0) {
                strcat(Str4, "blessed ");
                strcat(Str4, obj->truename);
            }
            else {
                strcat(Str4, obj->truename);
            }

            if(obj->number > 1) {
                strcat(Str4, "s");
            }

            switch(obj->objchar) {
            case STICK:
                setchargestr(obj, tstr);
                strcat(Str4, tstr);

                break;
            case MISSILEWEAPON:
            case ARMOR:
            case RING:
            case SHIELD:
            case WEAPON:
                setplustr(obj, tstr);
                strcat(Str4, tstr);

                break;
            default:
                strcat(Str4, "");

                break;
            }
        }

        return Str4;
    }
}

char *cashstr()
{
    if(difficulty() < 3) {
        return "copper pieces";
    }
    else if(difficulty() < 5) {
        return "silver pieces";
    }
    else if(difficulty() < 7) {
        return "gold pieces";
    }
    else if(difficulty() < 8) {
        return "semiprecious gems";
    }
    else if(difficulty() < 9) {
        return "mithril pieces";
    }
    else if(difficulty() < 10) {
        return "precious gems";
    }
    else {
        return "orichalc pieces";
    }
}

/* Returns an object's plus as a string */
void setplustr(pob obj, char *pstr)
{
    pstr[0] = ' ';

    if(obj->plus < 0) {
        pstr[1] = '-';
    }
    else {
        pstr[1] = '+';
    }

    if(abs(obj->plus) < 10) {
        pstr[2] = '0' + abs(obj->plus);
        pstr[3] = '\0';
    }
    else {
        pstr[2] = '0' + abs(obj->plus / 10);
        pstr[3] = '0' + abs(obj->plus % 10);
        pstr[4] = '\0';
    }
}

/* Return an object's number as a string */
void setnumstr(pob obj, char *nstr)
{
    if(obj->number < 2) {
        nstr[0] = '\0';
    }
    else if(obj->number < 10) {
        nstr[0] = '0' + obj->number;
        nstr[1] = 'x';
        nstr[2] = ' ';
        nstr[3] = '\0';
    }
    else if(obj->number < 41) {
        nstr[0] = '0' + (int)(obj->number / 10);
        nstr[1] = '0' + (obj->number % 10);
        nstr[2] = 'x';
        nstr[3] = ' ';
        nstr[4] = '\0';
    }
    else {
        strcpy(nstr, "lots of ");
    }
}

/* Return object with charges */
void setchargestr(pob obj, char *cstr)
{
    cstr[0] = ' ';
    cstr[1] = '[';

    if(obj->charge < 0) {
        cstr[2] = 'd';
        cstr[3] = 'e';
        cstr[4] = 'a';
        cstr[5] = 'd';
        cstr[6] = ']';
        cstr[7] = '\0';
    }
    else if(obj->charge < 10) {
        cstr[2] = '0' + obj->charge;
        cstr[3] = ']';
        cstr[4] = '\0';
    }
    else {
        cstr[2] = '0' + (int)(obj->charge / 10);
        cstr[3] = '0' + (obj->charge % 10);
        cstr[4] = ']';
        cstr[5] = '\0';
    }
}

void give_money(struct monster *m)
{
    pob cash;

    cash = detach_money();

    if(cash == NULL) {
        setgamestatus(SKIP_MONSTERS);
    }
    else {
        givemonster(m, cash);
    }
}

void givemonster(struct monster *m, struct object *o)
{
    /* Special case -- give gem to LawBringer */
    if((m->id == (ML10 + 2)) && (o->id == (ARTIFACTID + 21))) {
        clearmsg();
        print1("The LawBringer accepts the gem reverently.");
        print2("He raises it above his head, where it bursts into lambent flame!");
        morewait();
        print1("You are bathed in a shimmering golden light.");
        print2("You feel embedded in an infinite matrix of ordered energy.");
        morewait();

        if(Imprisonment > 0) {
            Imprisonment = 0;
        }

        if(Player.rank[ORDER] == -1) {
            print2("You have been forgiven. You feel like a Paladin...");
            Player.rank[ORDER] = 1;
        }

        Player.alignment += 200;
        Player.maxpow = Player.pow * 2;
        Player.pow = Player.maxpow;
        gain_experience(2000);
        setgamestatus(GAVE_STARGEM);
    }
    else {
        if(m->uniqueness == COMMON) {
            strcpy(Str3, "The ");
            strcat(Str3, m->monstring);
        }
        else {
            strcpy(Str3, m->monstring);
        }

        if(m_statusp(m, GREEDY) || m_statusp(m, NEEDY)) {
            m_pickup(m, o);
            strcat(Str3, " takes your gift");
            print1(Str3);
            ++Player.alignment;

            if(m_statusp(m, GREEDY) && (true_item_value(o) < (m->level * 100))) {
                nprint1("...but does not appear satisfied.");
            }
            else if(m_statusp(m, NEEDY) && (true_item_value(o) < (Level->depth * Level->depth))) {
                nprint1("...and looks chasteningly at you.");
            }
            else {
                nprint1("...and seems happy with it.");
                m_status_reset(m, HOSTILE);
                m_status_reset(m, GREEDY);
                m_status_reset(m, NEEDY);
            }
        }
        else if(m_statusp(m, HUNGRY)) {
            if(((m->id == HORSE) && (o->id == (FOODID + 15)))
               || ((m->id != HORSE) && ((o->usef == I_FOOD) || (o->usef == I_POISON_FOOD)))) {
                strcat(Str3, " wolfs down your food... ");
                print1(Str3);
                m_status_reset(m, HUNGRY);
                m_status_reset(m, HOSTILE);

                if(o->usef == I_POISON_FOOD) {
                    Player.alignment -= 2;
                    nprint1("...and chokes on the poisoned ration!");
                    morewait();
                    m_status_set(m, HOSTILE);
                    m_damage(m, 100, POISON);
                }
                else {
                    nprint1("...and now seems satiated.");
                    morewait();
                    free((char *)o);
                }
            }
            else {
                strcat(Str3, " spurns your offering and leaves it on the ground.");
                print1(Str3);
                drop_at(m->x, m->y, o);
            }
        }
        else {
            strcat(Str3, " doesn't care for your offering and drops it.");
            print1(Str3);
            drop_at(m->x, m->y, o);
        }
    }
}

/* Will clear all, not just one of an object. */
void conform_lost_object(pob obj)
{
    if(obj != NULL) {
        conform_lost_objects(obj->number, obj);
    }
}

/* Removes n of objects from inventory, frees object if appropriate */
void dispose_lost_objects(int n, pob obj)
{
    int i;
    int conformed = FALSE;
    int subtracted = FALSE;

    if(obj != NULL) {
        for(i = 0; i < MAXITEMS; ++i) {
            if(Player.possessions[i] == obj) {
                if(!subtracted) {
                    obj->number -= n;
                    subtracted = TRUE;
                }

                if(obj->number < 1) {
                    if(!conformed) {
                        conform_unused_object(obj);
                        conformed = TRUE;
                    }

                    Player.possessions[i] = NULL;
                }
            }
        }
    }

    if(obj->number < 1) {
        free((char *)obj);
    }
}

/*
 * Removes n of object from inventory without freeing object. Removes
 * all instance of pointer (might be 2-handed weapon, etc.)
 */
void conform_lost_objects(int n, pob obj)
{
    int i;
    int conformed = FALSE;
    int subtracted = FALSE;

    if(obj != NULL) {
        for(i = 0; i < MAXITEMS; ++i) {
            if(Player.possessions[i] == obj) {
                if(!subtracted) {
                    obj->number -= n;
                    subtracted = TRUE;
                }

                if(obj->number < 1) {
                    if(!conformed) {
                        conform_unused_object(obj);
                        conformed = TRUE;
                    }

                    Player.possessions[i] = NULL;
                }
            }
        }
    }
}

/* Clears unused possession */
void conform_unused_object(pob obj)
{
    if(obj->used) {
        obj->used = FALSE;
        item_use(obj);
    }

    calc_melee();
}

/*
 * Select an item from inventory
 * If itype is NULL, any kind of item is acceptable
 * If itype is CASHVALUE, any kind of item or '$' (cash) is acceptable
 * If itype is FOOD, CORPSE or FOOD is acceptable, but only FOOD is listed in 
 *                  the possibilities.
 * If itype is any other object (eg. SCROLL, POTION, etc.), only that type of
 *                  item is acceptable or is listed
 */
int getitem(char itype)
{
    char invstr[64];
    char index;
    int i;
    int k = 0;
    int ok = FALSE;
    int drewmenu = FALSE;
    int found = FALSE;

    if((itype == '\0') || ((itype == CASHVALUE) && (Player.cash > 0))) {
        found = 1;
    }
    else {
        found = 0;
    }

    for(i = 1; i < MAXITEMS; ++i) {
        if(Player.possessions[i] != NULL) {
            if((itype == '\0')
               || (itype == CASHVALUE)
               || (Player.possessions[i]->objchar == itype)
               || ((itype == FOOD) && (Player.possessions[i]->objchar == CORPSE))) {
                found = TRUE;
                invstr[k] = 'a' + i - 1;
                ++k;
                invstr[k] = '\0';
            }
        }
    }

    if((itype == CASHVALUE) && found) {
        invstr[k] = '$';
        ++k;
        invstr[k] = '\0';
    }

    if(!found) {
        print3("Nothing appropriate.");

        return ABORT;
    }
    else {
        print2("Select an item[");
        nprint2(invstr);
        nprint2(",?] ");

        while(!ok) {
            index = mcigetc();

            if(index == '?') {
                drewmenu = TRUE;
                menuclear();

                for(i = 1; i < MAXITEMS; ++i) {
                    if(Player.possessions[i] != NULL) {
                        if((itype == '\0')
                           || (itype == CASHVALUE)
                           || (Player.possessions[i]->objchar == itype)
                           || ((itype == FOOD) && (Player.possessions[i]->objchar == CORPSE))) {
                            display_inventory_slot(i, FALSE);
                        }
                    }
                }
            }
            else if(index == ESCAPE) {
                ok = TRUE;
            }
            else if(index == CASH) {
                if(itype == CASHVALUE) {
                    ok = TRUE;
                }
                else {
                    print3("You cannot select cash now.");
                    ok = FALSE;
                }
            }
            else if(badobject(index) || !strmem(index, invstr)) {
                print3("Nope! Try again [? for inventory, ESCAPE to quit]:");
            }
            else {
                ok = TRUE;
            }
        }

        if(drewmenu) {
            xredraw();
        }

        if(index == ESCAPE) {
            return ABORT;
        }
        else if(index == CASH) {
            return CASHVALUE;
        }
        else {
            return (index + 1 - 'a');
        }
    }
}

/* 
 * True if the numerical index based on 'a' == 1 turns out to be
 * either out of the range of the possessions array or a null item
 */
int badobject(char slotchar)
{
    int slot = slotchar + 1 - 'a';

    if((slot < 1) || (slot >= MAXITEMS)) {
        return TRUE;
    }
    else if(Player.possessions[slot] == NULL) {
        return 1;
    }
    else {
        return 0;
    }
}

/* This takes the numerical index directly for the same effects as badobject */
int baditem(int slotnum)
{
    if((slotnum < 1) || (slotnum >= MAXITEMS)) {
        return TRUE;
    }
    else if(Player.possessions[slotnum] == NULL) {
        return 1;
    }
    else {
        return 0;
    }
}

/* Formerly add_item_to_pack */
void gain_item(struct object *o)
{
    if(o->objchar == CASH) {
        print2("You gained some cash.");
        Player.cash += o->basevalue;
        free((char *)o->objstr);
        free((char *)o);
        dataprint();
    }
    else if(optionp(PACKADD)) {
        if(!get_to_pack(o)) {
            Player.possessions[O_UP_IN_AIR] = o;
            top_inventory_control();
        }
    }
    else {
        Player.possessions[O_UP_IN_AIR] = o;
        top_inventory_control();
    }
}

/* Adds item to pack list */
void add_to_pack(pob o)
{
    if(Player.packptr >= MAXPACK) {
        print3("Your pack is full. The item drops to the ground.");
        drop_at(Player.x, Player.y, o);
    }
    else {
        Player.pack[Player.packptr] = o;
        ++Player.packptr;
        print3("Putting item in pack.");
    }
}

/* Adds item to pack list, maybe going into inventory mode if pack is full */
int get_to_pack(pob o)
{
    if(Player.packptr >= MAXPACK) {
        print3("Your pack is full.");
        morewait();

        return FALSE;
    }
    else {
        Player.pack[Player.packptr] = o;
        ++Player.packptr;
        print3("Putting item in pack.");

        return TRUE;
    }
}

/*
 * Takes something from pack, puts to slot, or to 'up-in-air', one of
 * which at least must be empty
 */
void take_from_pack(int slot, int display)
{
    char response;
    int i;
    int quit = FALSE;
    int ok = TRUE;
    int displayed = FALSE;
    pob item;

    if(Player.possessions[slot] != NULL) {
        slot = O_UP_IN_AIR;
    }

    if(Player.possessions[slot] != NULL) {
        print3("slot is not empty!");
    }
    else if(Player.packptr == 0) {
        print3("Pack is empty!");
    }
    else {
        ok = TRUE;
        print1("Enter pack slot letter, or ? to show pack, or ESCAPE to quit.");
        response = mgetc();

        if(response == '?') {
            display_pack();
            displayed = TRUE;
            ok = FALSE;
        }
        else if(response == ESCAPE) {
            quit = TRUE;
        }
        else {
            if((response >= 'A') && (response < ('A' + Player.packptr))) {
                ok = 1;
            }
            else {
                ok = 0;
            }

            if(ok) {
                ok = slottable(Player.pack[response - 'A'], slot);
            }
        }

        while(!ok) {
            ok = TRUE;
            print1("Enter pack slot letter, or ? to show pack, or ESCAPE to quit.");
            response = mgetc();

            if(response == '?') {
                display_pack();
                displayed = TRUE;
                ok = FALSE;
            }
            else if(response == ESCAPE) {
                quit = TRUE;
            }
            else {
                if((response >= 'A') && (response < ('A' + Player.packptr))) {
                    ok = 1;
                }
                else {
                    ok = 0;
                }

                if(ok) {
                    ok = slottable(Player.pack[response - 'A'], slot);
                }
            }
        }

        if(!quit) {
            if((response - 'A') > 10) {
                print1("You begin to rummage through your pack.");
                morewait();
                Command_Duration += 10;
            }

            if((response - 'A') > 5) {
                print1("You search your pack for the item.");
                Command_Duration += 5;
                morewait();
            }

            print1("You take the item from your pack.");
            morewait();
            Command_Duration += 2;
            Player.possessions[slot] = Player.pack[response - 'A'];
            item = Player.possessions[slot];

            for(i = response - 'A'; i < (Player.packptr - 1); ++i) {
                Player.pack[i] = Player.pack[i + 1];
            }

            --Player.packptr;
            Player.pack[Player.packptr] = NULL;

            if(item_useable(item, slot)) {
                item->used = TRUE;
                item_use(item);
                morewait();

                if(item->number > 1) {
                    pack_extra_items(item);
                }
            }
        }
    }

    if(displayed) {
        xredraw();

        if(display) {
            display_possessions();
        }
    }
}

/* General interface to inventory */
void item_inventory(int topline)
{
    if(topline) {
        menuclear();
        display_possessions();
        inventory_control();
    }
    else {
        top_inventory_control();
    }
}

/*
 * inventory_control assumes a few setup things have been done like
 * displaying the slots, loading the O_UP_IN_AIR item, etc. Each
 * action uses up a little time. If on inspection actions are taken,
 * no time is used up.
 */
void inventory_control()
{
    int slot = 0;
    int done = FALSE;
    char response;

    clearmsg3();

    checkclear();
    print1("Action [^l,d,e,p,s,t,x,>,<,?,ESCAPE]:");
    display_inventory_slot(slot, FALSE);

    if(slot == O_WEAPON_HAND) {
        display_inventory_slot(O_READY_HAND, FALSE);
    }
    else if(slot == O_READY_HAND) {
        display_inventory_slot(O_WEAPON_HAND, FALSE);
    }

    display_inventory_slot(O_UP_IN_AIR, FALSE);
    move_slot(slot, slot, MAXITEMS);
    response = mcigetc();

    switch(response) {
    case 12:
    case 18:
        /* ^l, ^r */
        menuclear();
        display_possessions();

        break;
    case 'd':
        if(Player.possessions[O_UP_IN_AIR] != NULL) {
            drop_from_slot(O_UP_IN_AIR);
        }
        else if(Player.possessions[slot] != NULL) {
            drop_from_slot(slot);
        }
        else {
            print3("Nothing in selected slot!");
        }

        ++Command_Duration;

        break;
    case 'p':
        if(Player.possessions[slot] != NULL) {
            put_to_pack(slot);
        }

        Command_Duration += 5;

        break;
    case 's':
        display_pack();
        morewait();
        xredraw();
        display_possessions();
        Command_Duration += 5;

        break;
    case 't':
        take_from_pack(slot, TRUE);
        Command_Duration += 5;

        break;
    case 'e':
        switch_to_slot(slot);
        Command_Duration += 2;

        break;
    case 'x':
        switch_to_slot(slot);
        Command_Duration += 2;

        if(Player.possessions[O_UP_IN_AIR] == NULL) {
            done = 1;
        }
        else {
            done = 0;
        }

        break;
    case 'j':
    case '>':
        slot = move_slot(slot, slot + 1, MAXITEMS);

        break;
    case 'k':
    case '<':
        slot = move_slot(slot, slot - 1, MAXITEMS);

        break;
    case '?':
        inv_help();
        display_possessions();

        break;
    case ESCAPE:
        if(Player.possessions[O_UP_IN_AIR] != NULL) {
            drop_at(Player.x, Player.y, Player.possessions[O_UP_IN_AIR]);
            Player.possessions[O_UP_IN_AIR] = NULL;
            print3("Object 'up in air' dropped.");
        }

        done = TRUE;

        break;
    }

    calc_melee();

    while(!done) {
        checkclear();
        print1("Action [^l,d,e,p,s,t,x,>,<,?,ESCAPE]:");
        display_inventory_slot(slot, FALSE);

        if(slot == O_WEAPON_HAND) {
            display_inventory_slot(O_READY_HAND, FALSE);
        }
        else if(slot == O_READY_HAND) {
            display_inventory_slot(O_WEAPON_HAND, FALSE);
        }

        display_inventory_slot(O_UP_IN_AIR, FALSE);
        move_slot(slot, slot, MAXITEMS);
        response = mcigetc();

        switch(response) {
        case 12:
        case 18:
            /* ^l, ^r */
            menuclear();
            display_possessions();

            break;
        case 'd':
            if(Player.possessions[O_UP_IN_AIR] != NULL) {
                drop_from_slot(O_UP_IN_AIR);
            }
            else if(Player.possessions[slot] != NULL) {
                drop_from_slot(slot);
            }
            else {
                print3("Nothing in selected slot!");
            }

            ++Command_Duration;

            break;
        case 'p':
            if(Player.possessions[slot] != NULL) {
                put_to_pack(slot);
            }

            Command_Duration += 5;

            break;
        case 's':
            display_pack();
            morewait();
            xredraw();
            display_possessions();
            Command_Duration += 5;

            break;
        case 't':
            take_from_pack(slot, TRUE);
            Command_Duration += 5;

            break;
        case 'e':
            switch_to_slot(slot);
            Command_Duration += 2;

            break;
        case 'x':
            switch_to_slot(slot);
            Command_Duration += 2;

            if(Player.possessions[O_UP_IN_AIR] == NULL) {
                done = 1;
            }
            else {
                done = 0;
            }

            break;
        case 'j':
        case '>':
            slot = move_slot(slot, slot + 1, MAXITEMS);

            break;
        case 'k':
        case '<':
            slot = move_slot(slot, slot - 1, MAXITEMS);

            break;
        case '?':
            inv_help();
            display_possessions();

            break;
        case ESCAPE:
            if(Player.possessions[O_UP_IN_AIR] != NULL) {
                drop_at(Player.x, Player.y, Player.possessions[O_UP_IN_AIR]);
                Player.possessions[O_UP_IN_AIR] = NULL;
                print3("Object 'up in air' dropped.");
            }

            done = TRUE;

            break;
        }

        calc_melee();
    }

    xredraw();
}


/* Same as inventory_control, but only uses msg window for IO */
void top_inventory_control()
{
    int slot = 0;
    int done = FALSE;
    int usedmenu = FALSE;
    char response;

    clearmsg3();

    clearmsg1();
    print1("Action [d,e,p,s,t,x,~,?,ESCAPE]:");
    print2("'Up in air': ");

    if(Player.possessions[O_UP_IN_AIR] == NULL) {
        nprint2("NOTHING");
    }
    else {
        nprint2(itemid(Player.possessions[O_UP_IN_AIR]));
    }

    response = mcigetc();

    switch(response) {
    case 'd':
        if(Player.possessions[O_UP_IN_AIR] != NULL) {
            drop_from_slot(O_UP_IN_AIR);
        }
        else {
            slot = get_inventory_slot();

            if(Player.possessions[slot] != NULL) {
                drop_from_slot(slot);
            }
            else {
                print3("Nothing in selected slot!");
            }
        }

        ++Command_Duration;

        break;
    case 'p':
        if(Player.possessions[O_UP_IN_AIR] == NULL) {
            slot = get_inventory_slot();
        }
        else {
            slot = O_UP_IN_AIR;
        }

        put_to_pack(slot);
        Command_Duration += 5;

        break;
    case 's':
        display_pack();
        usedmenu = TRUE;
        Command_Duration += 5;

        break;
    case 't':
        slot = get_inventory_slot();
        take_from_pack(slot, FALSE);
        Command_Duration += 5;

        break;
    case 'e':
        slot = get_inventory_slot();
        switch_to_slot(slot);
        Command_Duration += 2;

        break;
    case 'x':
        slot = get_inventory_slot();
        switch_to_slot(slot);
        Command_Duration += 2;

        if(Player.possessions[O_UP_IN_AIR] == NULL) {
            done = 1;
        }
        else {
            done = 0;
        }

        break;
    case '~':
        menuclear();
        display_possessions();
        inventory_control();
        usedmenu = TRUE;
        done = TRUE;

        break;
    case '?':
        inv_help();
        usedmenu = TRUE;

        break;
    case ESCAPE:
        if(Player.possessions[O_UP_IN_AIR] != NULL) {
            drop_at(Player.x, Player.y, Player.possessions[O_UP_IN_AIR]);
            Player.possessions[O_UP_IN_AIR] = NULL;
            print3("Object 'up in air' dropped.");
        }

        done = TRUE;

        break;
    }
    
    calc_melee();

    while(!done) {
        clearmsg1();
        print1("Action [d,e,p,s,t,x,~,?,ESCAPE]:");
        print2("'Up in air':");

        if(Player.possessions[O_UP_IN_AIR] == NULL) {
            nprint2("NOTHING");
        }
        else {
            nprint2(itemid(Player.possessions[O_UP_IN_AIR]));
        }

        response = mcigetc();

        switch(response) {
        case 'd':
            if(Player.possessions[O_UP_IN_AIR] != NULL) {
                drop_from_slot(O_UP_IN_AIR);
            }
            else {
                slot = get_inventory_slot();

                if(Player.possessions[slot] != NULL) {
                    drop_from_slot(slot);
                }
                else {
                    print3("Nothing in selected slot!");
                }
            }

            ++Command_Duration;

            break;
        case 'p':
            if(Player.possessions[O_UP_IN_AIR] == NULL) {
                slot = get_inventory_slot();
            }
            else {
                slot = O_UP_IN_AIR;
            }

            put_to_pack(slot);
            Command_Duration += 5;

            break;
        case 's':
            display_pack();
            usedmenu = TRUE;
            Command_Duration += 5;

            break;
        case 't':
            slot = get_inventory_slot();
            take_from_pack(slot, FALSE);
            Command_Duration += 5;

            break;
        case 'e':
            slot = get_inventory_slot();
            switch_to_slot(slot);
            Command_Duration += 2;

            break;
        case 'x':
            slot = get_inventory_slot();
            switch_to_slot(slot);
            Command_Duration += 2;

            if(Player.possessions[O_UP_IN_AIR] == NULL) {
                done = 1;
            }
            else {
                done = 0;
            }

            break;
        case '~':
            menuclear();
            display_possessions();
            inventory_control();
            usedmenu = TRUE;
            done = TRUE;

            break;
        case '?':
            inv_help();
            usedmenu = TRUE;

            break;
        case ESCAPE:
            if(Player.possessions[O_UP_IN_AIR] != NULL) {
                drop_at(Player.x, Player.y, Player.possessions[O_UP_IN_AIR]);
                Player.possessions[O_UP_IN_AIR] = NULL;
                print3("Object 'up in air' dropped.");
            }

            done = TRUE;

            break;
        }

        calc_melee();
    }

    if(usedmenu) {
        xredraw();
    }
}

int get_inventory_slot()
{
    int ok;
    char response;

    clearmsg1();
    print1("Which inventory slot [a..o,*='up-in-air slot]?");
    response = mcigetc();

    if((response == '*')
       || ((response >= 'a') && (response < ('a' + MAXITEMS - 1)))) {
        ok = 1;
    }
    else {
        ok = 0;
    }

    while(!ok) {
        clearmsg1();
        print1("Which inventory slot [a..o,*='up-in-air' slot]?");
        response = mcigetc();

        if((response == '*')
           || ((response >= 'a') && (response < ('a' + MAXITEMS - 1)))) {
            ok = 1;
        }
        else {
            ok = 0;
        }
    }

    if(response == '*') {
        return 0;
    }
    else {
        return (int)(response + 1 - 'a');
    }
}

/* Returns some number between 0 and o->number */
int get_item_number(pob o)
{
    int n = 0;

    print1("How many? -- max");
    mnumprint(o->number);
    nprint1(" :");
    n = parsenum();

    if(n > o->number) {
        print3("Too many!");
    }
    else if(n < 1) {
        n = 0;
    }

    while(n > o->number) {
        print1("How many? -- max");
        mnumprint(o->number);
        nprint1(" :");
        n = parsenum();

        if(n > o->number) {
            print3("Too many!");
        }
        else if(n < 1) {
            n = 0;
        }
    }

    if(n < 1) {
        n = 0;
    }

    return n;
}

void drop_from_slot(int slot)
{
    int n;
    int waitflag;

    if(Player.possessions[slot] != NULL) {
        if(cursed(Player.possessions[slot]) == (TRUE + TRUE)) {
            print3("It sticks to your fingers!");
        }
        else {
            n = Player.possessions[slot]->number;

            if(n > 1) {
                n = get_item_number(Player.possessions[slot]);
            }

            if(n > 0) {
                p_drop_at(Player.x, Player.y, n, Player.possessions[slot]);

                if(Player.possessions[slot]->used
                   && (Player.possessions[slot]->number == n)) {
                    waitflag = 1;
                }
                else {
                    waitflag = 0;
                }

                conform_lost_objects(n, Player.possessions[slot]);

                if(waitflag) {
                    morewait();
                }
            }
            else {
                print3("Didn't drop anything.");
            }
        }
    }
    else {
        print3("Didn't drop anything.");
    }
}

void put_to_pack(int slot)
{
    int waitflag;
    int num = 1;
    pob temp;
    pob oslot = Player.possessions[slot];

    if(oslot == NULL) {
        print3("Slot is empty!");
    }
    else if(cursed(oslot) == (TRUE + TRUE)) {
        print3("Item is cursed!");
    }
    else {
        if(oslot->number > 1) {
            num = get_item_number(oslot);
        }

        if(num > 0) {
            temp = split_item(num, oslot);

            if(oslot->used && (oslot->number == num)) {
                waitflag = 1;
            }
            else {
                waitflag = 0;
            }

            conform_lost_objects(num, oslot);

            if(waitflag) {
                morewait();
            }

            add_to_pack(temp);
        }
    }
}

/*
 * Splits num off of item to make newitem which is returned something
 * else (conform_lost_objects) has to reduce the actual number value
 * of item and Player.itemweight
 */
pob split_item(int num, pob item)
{
    pob newitem = NULL;

    if(item != NULL) {
        newitem = (pob)malloc(sizeof(objtype));
        *newitem = *item;

        if(num <= item->number) {
            newitem->number = num;
        }

        /* Else num > item->number, so return NULL on this error condition */
    }

    return newitem;
}

/*
 * Trades contents of "up in air" slot withe selected slot. One or
 * both may be null. If both slots are 'objequal' merges two groups
 * into one in the selected slot. If one slot is null and the number
 * of the other is greater than one, requests how many to move.
 */
void switch_to_slot(int slot)
{
    pob oslot = Player.possessions[slot];
    pob oair = Player.possessions[O_UP_IN_AIR];
    pob otemp = NULL;
    int slotnull;
    int airnull;
    int num = 1;
    int trade = FALSE;
    int put = FALSE;
    int take = FALSE;
    int merge = FALSE;
    int a2h = FALSE;

    /* i.e., is cursed and in use */
    if(slot == O_UP_IN_AIR) {
        print3("This action makes no sense!");
    }
    else if(cursed(oslot) == (TRUE + TRUE)) {
        print3("The object in that slot is cursed -- you can't get rid of it!");
    }
    else {
        if(oslot == NULL) {
            slotnull = 1;
        }
        else {
            slotnull = 0;
        }

        if(oair == NULL) {
            airnull = 1;
        }
        else {
            airnull = 0;
        }

        if(!airnull) {
            if(twohandedp(oair->id)
               && ((slot == O_READY_HAND) || (slot == O_WEAPON_HAND))) {
                a2h = 1;
            }
            else {
                a2h = 0;
            }
        }

        /* Figure out which action to take */
        /* Merge if both are same kind of object */
        merge = objequal(oslot, oair);

        if(!merge && !slotnull && !airnull) {
            take = 1;
        }
        else {
            take = 0;
        }

        if(!merge && slotnull && !airnull && slottable(oair, slot)) {
            put = 1;
        }
        else {
            put = 0;
        }

        if(!merge && !slotnull && !airnull && slottable(oair, slot)) {
            trade = 1;
        }
        else {
            trade = 0;
        }

        if(merge) {
            merge_item(slot);
        }
        else if(put) {
            /* Deal with a 2-handed weapon */
            if(a2h) {
                if(Player.possessions[O_READY_HAND] == NULL) {
                    Player.possessions[O_READY_HAND] = oair;
                }

                if(Player.possessions[O_WEAPON_HAND] == NULL) {
                    Player.possessions[O_WEAPON_HAND] = oair;
                }
            }
            else {
                Player.possessions[slot] = oair;
            }

            Player.possessions[O_UP_IN_AIR] = NULL;

            if(item_useable(oair, slot)) {
                oair->used = TRUE;
                item_use(oair);
                morewait();

                if(oair->number > 1) {
                    pack_extra_items(oair);
                }
            }

            Player.possessions[O_UP_IN_AIR] = NULL;
        }
        else if(take) {
            if(oslot->number > 1) {
                num = get_item_number(oslot);
            }

            if(num > 0) {
                otemp = split_item(num, oslot);
                dispose_lost_objects(num, oslot);
                Player.possessions[O_UP_IN_AIR] = otemp;
                otemp->used = FALSE;
            }
        }
        else if(trade) {
            /* First remove item from slot */
            num = oslot->number;
            conform_lost_objects(oslot->number, oslot);
            oslot->number = num;
            Player.possessions[O_UP_IN_AIR] = oslot;
            Player.possessions[slot] = oair;

            if(a2h) {
                if(Player.possessions[O_READY_HAND] == NULL) {
                    Player.possessions[O_READY_HAND] = oair;
                }

                if(Player.possessions[O_WEAPON_HAND] == NULL) {
                    Player.possessions[O_WEAPON_HAND] = oair;
                }
            }

            if(item_useable(oair, slot)) {
                oair->used = TRUE;
                item_use(oair);
                morewait();

                if(oair->number > 1) {
                    pack_extra_items(oair);
                }
            }
        }
    }
}

/* Merges the up-in-air items into the select items */
void merge_item(int slot)
{
    Player.possessions[slot]->number += Player.possessions[O_UP_IN_AIR]->number;
    Player.possessions[O_UP_IN_AIR] = NULL;
}

/* 
 * Are two objects equal except for their number field? Returns false
 * if either object is null
 */
int objequal(struct object *o, struct object *p)
{
    if((o == NULL) || (p == NULL)) {
        return FALSE;
    }
    else if((o->id == p->id)
            && (o->plus == p->plus)
            && (o->charge == 0)
            && (p->charge == 0)
            && (o->dmg == p->dmg)
            && (o->hit == p->hit)
            && (o->aux == p->aux)
            && (o->known == p->known)
            && (o->blessing == p->blessing)
            && (o->usef == p->usef)) {
        return 1;
    }
    else {
        return 0;
    }
}

/* Criteria for being able to put some item in some slot */
int slottable(pob o, int slot)
{
    int ok = TRUE;

    if(o == NULL) {
        ok = FALSE;
    }
    else if(slot == O_ARMOR) {
        if(o->objchar != ARMOR) {
            print3("Only armor can go in the armor slot!");
            ok = FALSE;
        }
    }
    else if(slot == O_SHIELD) {
        if(o->objchar != SHIELD) {
            print3("Only a shield can go in the shield slot!");
            ok = FALSE;
        }
    }
    else if(slot == O_BOOTS) {
        if(o->objchar != BOOTS) {
            print3("Only boots can go in the boots slot!");
            ok = FALSE;
        }
    }
    else if(slot == O_CLOAK) {
        if(o->objchar != CLOAK) {
            print3("Only a cloak can go in the cloak slot!");
            ok = FALSE;
        }
    }
    else if(slot >= O_RING1) {
        if(o->objchar != RING) {
            print3("Only a ring can go in a ring slot!");
            ok = FALSE;
        }
    }

    return ok;
}

/*
 * Whether or not an item o can be used in a slot. Assumes o can in
 * fact be placed in the slot.
 */
int item_useable(pob o, int slot)
{
    /*
     * Don't have to check the object in the first if since only armor
     * can go in armor slot, cloak in cloak slot etc.
     */
    if((slot == O_ARMOR)
       || (slot == O_CLOAK)
       || (slot == O_SHIELD)
       || (slot == O_BOOTS)
       || (slot >= O_RING1)) {
        return TRUE;
    }
    else if((o->objchar == WEAPON) || (o->objchar == MISSILEWEAPON)) {
        /* 
         * Weapon is useable if it is put in weapon and or if it is
         * two-handed and put in either hand when the other also holds the
         * weapon
         */
        if(twohandedp(o->id)
           && ((slot == O_READY_HAND) || (slot == O_WEAPON_HAND))) {
            if(Player.possessions[O_READY_HAND] == Player.possessions[O_WEAPON_HAND]) {
                print1("You heft the weapon and find you must use both hands.");
                morewait();

                return TRUE;
            }
            else {
                print1("This weapon is two-handed, so at the moment, ");
                print2("you are just lugging it around...");
                morewait();

                return FALSE;
            }
        }
        else {
            if(slot == O_WEAPON_HAND) {
                return 1;
            }
            else {
                return 0;
            }
        }
    }
    else {
        return FALSE;
    }
}

/*
 * Return FALSE if not cursed, TRUE if cursed but not used, TRUE +
 * TRUE if cursed and used
 */
int cursed(pob obj)
{
    if(obj == NULL) {
        return FALSE;
    }
    else if(obj->blessing < 0) {
        if(obj->used == TRUE) {
            return (1 + TRUE);
        }
        else {
            return TRUE;
        }
    }
    else {
        return FALSE;
    }
}

/*
 * Returns true if item with id and charge is found in pack or in
 * inventory slot. Charge is used to differentiate coprses instead of
 * aux, which is their food value.
 */
int find_item(pob *o, int id, int chargeval)
{
    int i;
    int found = FALSE;
    *o = NULL;

    for(i = 1; (i < MAXITEMS) && !found; ++i) {
        if(Player.possessions[i] != NULL) {
            if((Player.possessions[i]->id == id)
               && ((chargeval == -1)
                   || (Player.possessions[i]->charge == chargeval))) {
                *o = Player.possessions[i];
                found = TRUE;
            }
        }
    }

    if(!found) {
        for(i = 0; (i < Player.packptr) && !found; ++i) {
            if(Player.pack[i] != NULL) {
                if((Player.pack[i]->id == id)
                   && ((chargeval == -1)
                       || (Player.pack[i]->charge == chargeval))) {
                    *o = Player.pack[i];
                    found = TRUE;
                }
            }
        }
    }

    return found;
}

/*
 * Returns true if item with id and charge is found in pack or in
 * inventory slot. Destroys item. Charge is used to differentiate
 * corpses instead of aux, which is their food value.
 */
int find_and_remove_item(int id, int chargeval)
{
    int i;
    int found = FALSE;
    pob o = NULL;

    for(i = 1; (i < MAXITEMS) && !found; ++i) {
        if(Player.possessions[i] != NULL) {
            if((Player.possessions[i]->id == id)
               && ((chargeval == -1)
                   || (Player.possessions[i]->charge == chargeval))) {
                o = Player.possessions[i];
                conform_lost_object(o);
                found = TRUE;
            }
        }
    }

    if(!found) {
        for(i = 0; (i < Player.packptr) && !found; ++i) {
            if(Player.pack[i] != NULL) {
                if((Player.pack[i]->id == id)
                   && ((chargeval == -1)
                       || (Player.pack[i]->charge == chargeval))) {
                    free((char *)Player.pack[i]);
                    Player.pack[i] = NULL;
                    found = TRUE;
                }
            }
        }
    }

    fixpack();

    return found;
}

void lose_all_items()
{
    int i;

    print1("You notice that you are completely devoid of all possessions.");
    morewait();

    for(i = 0; i < MAXITEMS; ++i) {
        if(Player.possessions[i] != NULL) {
            dispose_lost_objects(Player.possessions[i]->number, Player.possessions[i]);
            Player.possessions[i] = NULL;
        }
    }

    for(i = 0; i < MAXPACK; ++i) {
        if(Player.pack[i] != NULL) {
            free((char *)Player.pack[i]);
        }

        Player.pack[i] = NULL;
    }

    Player.packptr = 0;
    calc_melee();
    morewait();
}

/* Prevents people form wielding 3 short swords, etc. */
void pack_extra_items(pob item)
{
    pob extra = (pob)malloc(sizeof(objtype));

    if(Player.packptr < MAXPACK) {
        print3("Putting extra items back in pack.");
        morewait();
        *extra = *item;
        extra->number = item->number - 1;
        item->number = 1;
        Player.pack[Player.packptr] = extra;
        ++Player.packptr;
    }
    else if(Player.possessions[O_UP_IN_AIR] == NULL) {
        print3("Extra copies of item are 'up in air'");
        Player.possessions[O_UP_IN_AIR] = extra;
    }
    else {
        print3("No room for extra copies of item -- dropping them.");
        drop_at(Player.x, Player.y, extra);
    }

    calc_melee();
}

/* Makes sure Player.pack is OK, (used after sale from pack) */
void fixpack()
{
    pob tpack[MAXPACK];
    int i;
    int tctr = 0;

    for(i = 0; i < MAXPACK; ++i) {
        tpack[i] = NULL;
    }

    for(i = 0; i < MAXPACK; ++i) {
        if(Player.pack[i] != NULL) {
            tpack[tctr] = Player.pack[i];
            ++tctr;
        }
    }

    for(i = 0; i < MAXPACK; ++i) {
        Player.pack[i] = tpack[i];
    }

    Player.packptr = tctr;
}
