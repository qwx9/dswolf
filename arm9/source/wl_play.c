#include <nds.h>
#include <stdio.h>
#include <stdlib.h>
#include "types.h"
#include "gfxv_apo.h"
#include "audiowl6.h"
#include "wl_def.h"
#include "wl_draw.h"
#include "wl_play.h"
#include "wl_menu.h"
#include "wl_game.h"
#include "id_in.h"
#include "id_vl.h"
#include "id_vh.h"
#include "id_sd.h"
#include "id_time.h"
#include "id_ca.h"
#include "wl_act1.h"

/* macro defines */

#define WHITETICS       6

/* external variables */

extern U8 screenfaded;
extern U8 startgame;
extern U8 loadedgame;
extern S32 facecount;
extern S16 anglefrac;
extern gametype gamestate;
extern U32 lasttimecount;
extern U8 palshifted;
extern U8 areabyplayer[NUMAREAS];

/* local variables */

static S32 damagecount;
static S32 bonuscount;
static musicnames lastmusicchunk = (musicnames) 0;

/* LIST OF SONGS FOR EACH VERSION */
static S32 songs[] = {

    /* Episode One */
    GETTHEM_MUS,
    SEARCHN_MUS,
    POW_MUS,
    SUSPENSE_MUS,
    GETTHEM_MUS,
    SEARCHN_MUS,
    POW_MUS,
    SUSPENSE_MUS,

    WARMARCH_MUS,   /* Boss level */
    CORNER_MUS,     /* Secret level */

    /* Episode Two */
    NAZI_OMI_MUS,
    PREGNANT_MUS,
    GOINGAFT_MUS,
    HEADACHE_MUS,
    NAZI_OMI_MUS,
    PREGNANT_MUS,
    HEADACHE_MUS,
    GOINGAFT_MUS,

    WARMARCH_MUS,   /* Boss level */
    DUNGEON_MUS,    /* Secret level */

    /* Episode Three */
    INTROCW3_MUS,
    NAZI_RAP_MUS,
    TWELFTH_MUS,
    ZEROHOUR_MUS,
    INTROCW3_MUS,
    NAZI_RAP_MUS,
    TWELFTH_MUS,
    ZEROHOUR_MUS,

    ULTIMATE_MUS,   /* Boss level */
    PACMAN_MUS,     /* Secret level */

    /* Episode Four */
    GETTHEM_MUS,
    SEARCHN_MUS,
    POW_MUS,
    SUSPENSE_MUS,
    GETTHEM_MUS,
    SEARCHN_MUS,
    POW_MUS,
    SUSPENSE_MUS,

    WARMARCH_MUS,   /* Boss level */
    CORNER_MUS,     /* Secret level */

    /* Episode Five */
    NAZI_OMI_MUS,
    PREGNANT_MUS,
    GOINGAFT_MUS,
    HEADACHE_MUS,
    NAZI_OMI_MUS,
    PREGNANT_MUS,
    HEADACHE_MUS,
    GOINGAFT_MUS,

    WARMARCH_MUS,   /* Boss level */
    DUNGEON_MUS,    /* Secret level */

    /* Episode Six */
    INTROCW3_MUS,
    NAZI_RAP_MUS,
    TWELFTH_MUS,
    ZEROHOUR_MUS,
    INTROCW3_MUS,
    NAZI_RAP_MUS,
    TWELFTH_MUS,
    ZEROHOUR_MUS,

    ULTIMATE_MUS,   /* Boss level */
    FUNKYOU_MUS     /* Secret level */
};

/* global variables */

S32 viewsize = 20;
S32 buttonmouse[4] = {bt_attack, bt_strafe, bt_use, bt_nobutton};
S32 buttonjoy[4] = {bt_attack, bt_strafe, bt_use, bt_run};
U8 tilemap[MAPSIZE][MAPSIZE]; /* wall values only */
objtype *actorat[MAPSIZE][MAPSIZE];
U8 spotvis[MAPSIZE][MAPSIZE];
U32 tics;
U8 buttonheld[NUMBUTTONS];
U8 buttonstate[NUMBUTTONS];
S32 controlx;
S32 controly;                 /* range from -100 to 100 per tic */
exit_t playstate;
objtype objlist[MAXACTORS];
objtype *objfreelist;
objtype *obj;
objtype *killerobj;
objtype *player;
objtype *lastobj;
objtype *newobj;
S32 objcount;
U8 madenoise;                 /* true when shooting or screaming */

/* local prototypes */

static void PollControls(void);
static void PollJoystickButtons(void);
static void DoActor(objtype * ob);
static void CheckKeys(void);
static void ClearPaletteShifts(void);
static void UpdatePaletteShifts (void);
static void RemoveObj(objtype * gone);

/*
================================================================
=
= Function: PlayLoop
=
= Description:
=
=
=
================================================================
*/
void PlayLoop(void)
{

    playstate = ex_stillplaying;
    lasttimecount = GetTimeCount();
    facecount = 0;
    anglefrac = 0;
    memset(buttonstate, 0, sizeof (buttonstate));
    ClearPaletteShifts();

    do
    {
        PollControls();

        madenoise = 0;

        MoveDoors();
        /* FIXME: kluge */
        MovePWalls();

        for(obj = player; obj != NULL; obj = obj->next)
        {
            DoActor(obj);
        }

        UpdatePaletteShifts();

        ThreeDRefresh();

        gamestate.TimeCount += tics;

        if(screenfaded == 1)
        {
            VW_FadeIn();
        }

        CheckKeys();

    }while((playstate == ex_stillplaying) && (startgame == 0));

    if(playstate != ex_died)
    {
        FinishPaletteShifts();
    }
}

/*
================================================================
=
= Function: CheckKeys
=
= Description:
=
= check to see if pause or esc was pressed
=
================================================================
*/
static void CheckKeys(void)
{
    S32 lastoffs;

    if(buttonstate[bt_pause] == 1)
    {
        lastoffs = StopMusic();
        VWB_DrawPic(((20 - 4) * 8), 80 - 2 * 8, PAUSEDPIC);
        UpdateScreen(0);
        IN_Ack();
        ContinueMusic(lastoffs);
        lasttimecount = GetTimeCount();
        return;
    }

    if(buttonstate[bt_esc] == 1)
    {
        lastoffs = StopMusic();
        VW_FadeOut();

        US_ControlPanel(0);

        SetFontColor(0, 15);

        if(screenfaded != 1)
        {
            VW_FadeOut();
        }

        if(viewsize != 21)
        {
            DrawPlayScreen();
        }

        if((startgame == 0) && (loadedgame == 0))
        {
            ContinueMusic(lastoffs);
        }

        if(loadedgame == 1)
        {
            playstate = ex_abort;
        }

        lasttimecount = GetTimeCount();

        return;
    }
}

/*
================================================================
=
= Function: PollControls
=
= Description:
=
= Gets user or demo input, call once each frame
=
= controlx              set between -100 and 100 per tic
= controly
= buttonheld[]  the state of the buttons LAST frame
= buttonstate[] the state of the buttons THIS frame
=
================================================================
*/
static void PollControls(void)
{
    S32 max;
    S32 min;

    CalcTics();

    controlx = 0;
    controly = 0;

    memcpy(buttonheld,buttonstate,sizeof(buttonstate));
    memset(buttonstate,0,sizeof(buttonstate));

    PollJoystickButtons();

    /* bound movement to a maximum */
    max = 100 * tics;
    min = -max;

    if(controlx > max)
    {
        controlx = max;
    }
    else if(controlx < min)
    {
        controlx = min;
    }

    if(controly > max)
    {
        controly = max;
    }
    else if(controly < min)
    {
        controly = min;
    }
}

/*
================================================================
=
= Function: PollJoystickButtons
=
= Description:
=
= Poll nds inputs and then update game buttonstate with results
=
================================================================
*/
static void PollJoystickButtons(void)
{
    ControlInfo ci;
    S32 delta;

    /* clear button struct */
    ci.button0 = 0;
    ci.button1 = 0;
    ci.button2 = 0;
    ci.button3 = 0;
    ci.dir = dir_None;
    ci.straferight = 0;
    ci.strafeleft = 0;
    ci.pause = 0;
    ci.esc = 0;

    /* read nds buttons */
    IN_ReadControl(&ci);

    if(ci.button0 == 1)
    {
        buttonstate[buttonjoy[0]] = 1;
    }

    if(ci.button1 == 1)
    {
        buttonstate[buttonjoy[1]] = 1;
    }

    if(ci.button2 == 1)
    {
        buttonstate[buttonjoy[2]] = 1;
    }

    if(ci.button3 == 1)
    {
        buttonstate[buttonjoy[3]] = 1;
    }

    delta = (buttonstate[bt_run] == 1) ? (RUNMOVE * tics) : (BASEMOVE * tics);

    if(ci.dir == dir_North)
    {
        controly -= delta;
    }

    if(ci.dir == dir_South)
    {
        controly += delta;
    }

    if(ci.dir == dir_West)
    {
        controlx -= delta;
    }

    if(ci.dir == dir_East)
    {
        controlx += delta;
    }

    if(ci.straferight == 1)
    {
        buttonstate[bt_straferight] = 1;
    }

    if(ci.strafeleft == 1)
    {
        buttonstate[bt_strafeleft] = 1;
    }

    if(ci.pause == 1)
    {
        buttonstate[bt_pause] = 1;
    }

    if(ci.esc == 1)
    {
        buttonstate[bt_esc] = 1;
    }
}

/*
================================================================
=
= Function: DoActo
=
= Description:
=
=
=
================================================================
*/
static void DoActor(objtype * ob)
{
    void (*think) (objtype *);

    /* if object is not active and is not in same */
    /* area as player return                      */
    if (!ob->active && !areabyplayer[ob->areanumber])
    {
        return;
    }

    if(!(ob->flags & (FL_NONMARK | FL_NEVERMARK)))
    {
        actorat[ob->tilex][ob->tiley] = NULL;
    }

    /* non transitional object */
    if(ob->ticcount == 0)
    {
        think = (void (*)(objtype *)) ob->state->think;
        if(think != NULL)
        {
            think(ob);
            if(ob->state == NULL)
            {
                RemoveObj(ob);
                return;
            }
        }

        if (ob->flags & FL_NEVERMARK)
        {
            return;
        }

        if ((ob->flags & FL_NONMARK) && actorat[ob->tilex][ob->tiley])
        {
            return;
        }

        actorat[ob->tilex][ob->tiley] = ob;
        return;
    }

    /* transitional object */
    ob->ticcount -= (S16) tics;
    while(ob->ticcount <= 0)
    {
        think = (void (*)(objtype *)) ob->state->action;    /* end of state action */
        if(think != NULL)
        {
            think (ob);
            if(ob->state == NULL)
            {
                RemoveObj (ob);
                return;
            }
        }

        ob->state = ob->state->next;

        if (!ob->state)
        {
            RemoveObj (ob);
            return;
        }

        if (!ob->state->tictime)
        {
            ob->ticcount = 0;
            break;
        }

        ob->ticcount += ob->state->tictime;
    }

    /* think */
    think = (void (*)(objtype *)) ob->state->think;
    if(think != NULL)
    {
        think (ob);
        if(ob->state == NULL)
        {
            RemoveObj(ob);
            return;
        }
    }

    if (ob->flags & FL_NEVERMARK)
    {
        return;
    }

    if ((ob->flags & FL_NONMARK) && actorat[ob->tilex][ob->tiley])
    {
        return;
    }

    actorat[ob->tilex][ob->tiley] = ob;
}

/*
================================================================
=
= Function: StopMusic
=
= Description:
=
= stop playing current music
=
================================================================
*/
S32 StopMusic(void)
{
    S32 lastoffs = SD_MusicOff();

    return lastoffs;
}

/*
================================================================
=
= Function: StartMusic
=
= Description:
=
= start playing music based on level / episode selected
=
================================================================
*/
void StartMusic(void)
{
    SD_MusicOff();
    lastmusicchunk = (musicnames) songs[gamestate.mapon + gamestate.episode * 10];
    SD_StartMusic(STARTMUSIC + lastmusicchunk);
}

/*
================================================================
=
= Function:  ContinueMusic
=
= Description:
=
= start playing music from the point it was paused
=
================================================================
*/
void ContinueMusic(S32 offs)
{
    SD_MusicOff();
    lastmusicchunk = (musicnames) songs[gamestate.mapon + gamestate.episode * 10];
    SD_ContinueMusic(STARTMUSIC + lastmusicchunk, offs);
}

/*
#############################################################################

                        The objlist data structure

#############################################################################

objlist containt structures for every actor currently playing.  The structure
is accessed as a linked list starting at *player, ending when ob->next ==
NULL.  GetNewObj inserts a new object at the end of the list, meaning that
if an actor spawn another actor, the new one WILL get to think and react the
same frame.  RemoveObj unlinks the given object and returns it to the free
list, but does not damage the objects ->next pointer, so if the current object
removes itself, a linked list following loop can still safely get to the
next element.

<backwardly linked free list>

#############################################################################
*/

/*
================================================================
=
= Function:  InitActorList
=
= Description:
=
= Call to clear out the actor object lists returning them
= all to the free list. Allocates a special spot for the player.
=
================================================================
*/
void InitActorList(void)
{
    S32 i;

    /* init the actor lists */

    for (i = 0; i < MAXACTORS; i++)
    {
        objlist[i].prev = &objlist[i + 1];
        objlist[i].next = NULL;
    }

    objlist[MAXACTORS - 1].prev = NULL;

    objfreelist = &objlist[0];
    lastobj = NULL;

    objcount = 0;

    /* give the player the first free spots */

    GetNewActor();
    player = newobj;

}

/*
================================================================
=
= Function: GetNewActor
=
= Description:
=
= Sets the global variable new to point to a free spot in objlist.
= The free spot is inserted at the end of the liked list
=
= When the object list is full, the caller can either have it bomb
= out ot return a dummy object pointer that will never get used
=
================================================================
*/
void GetNewActor(void)
{
    /* do we have any free objtype spaces left in the list ?*/
    if(objfreelist == NULL)
    {
        iprintf("GetNewActor: No free spots in objlist!");
        while(1) { /* hang system */ };
    }

    /* point the newobj at the next free space on the list */
    newobj = objfreelist;

    /* move the free space list pointer along on */
    objfreelist = newobj->prev;

    /* clear the object before it is used */
    memset(newobj, 0, sizeof (*newobj));

    /* do we already have a object on the active link list ?*/
    if(lastobj != NULL)
    {
        lastobj->next = newobj;    /* yes, link last object to new object */
    }

    /* link new object to last object */
    newobj->prev = lastobj;     /* new->next is allready NULL from memset */

    newobj->active = ac_no;

    /* update lastobj with current ojects */
    lastobj = newobj;

    /* keep a count of active objects */
    objcount++;
}

/*
================================================================
=
= Function: StartBonusFlash
=
= Description:
=
= Flash screen white after picking up bonus
=
================================================================
*/
void StartBonusFlash(void)
{
    bonuscount = NUMWHITESHIFTS * WHITETICS;    /* white shift palette */
}

/*
================================================================
=
= Function: ClearPaletteShifts
=
= Description:
=
= clear palette shift variables
=
================================================================
*/
static void ClearPaletteShifts(void)
{
    bonuscount = 0;
    damagecount = 0;
    palshifted = 0;
}


/*
================================================================
=
= Function: StartDamageFlash
=
= Description:
=
= if damagecount is non zero when UpdatePaletteShifts is called
= the screen will flash to indicate player has been damaged
=
================================================================
*/
void StartDamageFlash (S32 damage)
{
    damagecount += damage;
}

/*
================================================================
=
= Function: UpdatePaletteShifts
=
= Description:
=
= Update screen palette if required
=
================================================================
*/
static void UpdatePaletteShifts(void)
{
    S32 red;
    S32 white;

    if(bonuscount != 0)
    {
        white = bonuscount / WHITETICS + 1;

        if (white > NUMWHITESHIFTS)
        {
            white = NUMWHITESHIFTS;
        }

        bonuscount -= tics;

        if (bonuscount < 0)
        {
            bonuscount = 0;
        }
    }
    else
    {
        white = 0;
    }


    if(damagecount != 0)
    {
        red = damagecount / 10 + 1;

        if (red > NUMREDSHIFTS)
        {
            red = NUMREDSHIFTS;
        }

        damagecount -= tics;

        if (damagecount < 0)
        {
            damagecount = 0;
        }
    }
    else
    {
        red = 0;
    }

    if(red != 0)
    {
        SetPalette(red_pal, red, 1);
        palshifted = 1;
    }
    else if(white != 0)
    {
        SetPalette(white_pal, white, 1);
        palshifted = 1;
    }
    else if(palshifted == 1)
    {
        FinishPaletteShifts();    /* back to normal */
    }

}

/*
================================================================
=
= Function: RemoveObj
=
= Description:
=
= Add the given object back into the free list, and unlink
= it from it's neighbors
=
================================================================
*/
static void RemoveObj(objtype * gone)
{
    if(gone == player)
    {
        iprintf("RemoveObj: Tried to remove the player!");
        while(1){/* hang system */};
    }

    gone->state = NULL;

    /* fix the next object's back link */
    if (gone == lastobj)
    {
        lastobj = (objtype *) gone->prev;
    }
    else
    {
        gone->next->prev = gone->prev;
    }

    /* fix the previous object's forward link */
    gone->prev->next = gone->next;

    /* add it back in to the free list */
    gone->prev = objfreelist;
    objfreelist = gone;

    objcount--;
}
