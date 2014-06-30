#include <nds.h>
#include <stdio.h>
#include <stdlib.h>
#include "types.h"
#include "wl_def.h"
#include "audiowl6.h"
#include "id_sd.h"
#include "wl_act1.h"

/* macro defines */

#define OPENTICS        300
#define PlaySoundLocTile(s,tx,ty)       PlaySoundLocGlobal(s,(((int32_t)(tx) << TILESHIFT) + (1L << (TILESHIFT - 1))),(((int32_t)ty << TILESHIFT) + (1L << (TILESHIFT - 1))))


/* external variables */

extern U8 tilemap[MAPSIZE][MAPSIZE];
extern objtype *actorat[MAPSIZE][MAPSIZE];
extern U16 * mapsegs[MAPPLANES];
extern U8 loadedgame;
extern gametype gamestate;
extern objtype *player;
extern U32 tics;
extern U8 spotvis[MAPSIZE][MAPSIZE];

/* local variables */

static statinfo_type statinfo[] =
{
    {SPR_STAT_0},                           /* puddle          spr1v */
    {SPR_STAT_1,block},                     /* Green Barrel    "     */
    {SPR_STAT_2,block},                     /* Table/chairs    "     */ 
    {SPR_STAT_3,block,FL_FULLBRIGHT},       /* Floor lamp      "     */
    {SPR_STAT_4,none,FL_FULLBRIGHT},        /* Chandelier      "     */
    {SPR_STAT_5,block},                     /* Hanged man      "     */
    {SPR_STAT_6,bo_alpo},                   /* Bad food        "     */
    {SPR_STAT_7,block},                     /* Red pillar      "     */
    /* NEW PAGE */
    {SPR_STAT_8,block},                     /* Tree            spr2v */
    {SPR_STAT_9},                           /* Skeleton flat   "     */
    {SPR_STAT_10,block},                    /* Sink            " (SOD:gibs) */
    {SPR_STAT_11,block},                    /* Potted plant    "     */
    {SPR_STAT_12,block},                    /* Urn             "     */
    {SPR_STAT_13,block},                    /* Bare table      "     */
    {SPR_STAT_14,none,FL_FULLBRIGHT},       /* Ceiling light   "     */
    {SPR_STAT_15},                          /* Kitchen stuff   "     */
    /* NEW PAGE */
    {SPR_STAT_16,block},                    /* suit of armor   spr3v */
    {SPR_STAT_17,block},                    /* Hanging cage    "     */
    {SPR_STAT_18,block},                    /* SkeletoninCage  "     */
    {SPR_STAT_19},                          /* Skeleton relax  "     */
    {SPR_STAT_20,bo_key1},                  /* Key 1           "     */
    {SPR_STAT_21,bo_key2},                  /* Key 2           "     */
    {SPR_STAT_22,block},                    /* stuff             (SOD:gibs) */
    {SPR_STAT_23},                          /* stuff                 */
    /* NEW PAGE */
    {SPR_STAT_24,bo_food},                  /* Good food       spr4v  */
    {SPR_STAT_25,bo_firstaid},              /* First aid       "      */
    {SPR_STAT_26,bo_clip},                  /* Clip            "      */
    {SPR_STAT_27,bo_machinegun},            /* Machine gun     "      */
    {SPR_STAT_28,bo_chaingun},              /* Gatling gun     "      */
    {SPR_STAT_29,bo_cross},                 /* Cross           "      */
    {SPR_STAT_30,bo_chalice},               /* Chalice         "      */
    {SPR_STAT_31,bo_bible},                 /* Bible           "      */
    /* NEW PAGE */
    {SPR_STAT_32,bo_crown},                 /* crown           spr5v  */
    {SPR_STAT_33,bo_fullheal,FL_FULLBRIGHT},/* one up          "      */
    {SPR_STAT_34,bo_gibs},                  /* gibs            "      */
    {SPR_STAT_35,block},                    /* barrel          "      */
    {SPR_STAT_36,block},                    /* well            "      */
    {SPR_STAT_37,block},                    /* Empty well      "      */
    {SPR_STAT_38,bo_gibs},                  /* Gibs 2          "      */
    {SPR_STAT_39,block},                    /* flag            "      */
    /* NEW PAGE */
    {SPR_STAT_40,block},                    /* Call Apogee     spr7v  */
    /* NEW PAGE */
    {SPR_STAT_41},                          /* junk            "      */
    {SPR_STAT_42},                          /* junk            "      */
    {SPR_STAT_43},                          /* junk            "      */
    {SPR_STAT_44},                          /* pots            "      */
    {SPR_STAT_45,block},                    /* stove           " (SOD:gibs) */
    {SPR_STAT_46,block},                    /* spears          " (SOD:gibs) */
    {SPR_STAT_47},                          /* vines           "      */
    /* NEW PAGE */
    {SPR_STAT_26,bo_clip2},                 /* Clip            "      */
    {-1}                                    /* terminator             */
};

/* global variables */

U8           areabyplayer[NUMAREAS];
U8           areaconnect[NUMAREAS][NUMAREAS];
doorobj_t    doorobjlist[MAXDOORS];
doorobj_t    *lastdoorobj;
S16          doornum;
statobj_t    statobjlist[MAXSTATS];
statobj_t    *laststatobj;
U16          doorposition[MAXDOORS];    /* leading edge of door 0=closed, 0xffff = fully open */

/* local prototypes */

static void CloseDoor(U8 door);
static void DoorOpening(U8 door);
static void DoorClosing(U8 door);
static void ConnectAreas(void);
static void RecursiveConnect(U8 areanumber);

/*
=============================================================================

                                  DOORS

doorobjlist[] holds most of the information for the doors

doorposition[] holds the amount the door is open, ranging from 0 to 0xffff
        this is directly accessed by AsmRefresh during rendering

The number of doors is limited to 64 because a spot in tilemap holds the
        door number in the low 6 bits, with the high bit meaning a door center
        and bit 6 meaning a door side tile

Open doors conect two areas, so sounds will travel between them and sight
        will be checked when the player is in a connected area.

Areaconnect is incremented/decremented by each door. If >0 they connect

Every time a door opens or closes the areabyplayer matrix gets recalculated.
        An area is true if it connects with the player's current spot.

=============================================================================
*/

/* 
================================================================
=
= Function: ConnectAreas
=
= Description:
=
= Scans outward from playerarea, marking all connected areas
=
================================================================
*/
static void RecursiveConnect(U8 areanumber)
{
    S32 i;

    for (i=0;i<NUMAREAS;i++)
    {
        if((areaconnect[areanumber][i]) && (areabyplayer[i] == 0))
        {
            areabyplayer[i] = 1;
            RecursiveConnect(i);
        }
    }
}

/* 
================================================================
=
= Function: ConnectAreas
=
= Description:
=
= if door is open / closed this function is called to reconnect
= areas.
=
================================================================
*/
static void ConnectAreas(void)
{
    memset (areabyplayer,0,sizeof(areabyplayer));
    areabyplayer[player->areanumber] = 1;
    RecursiveConnect(player->areanumber);
}

/* 
================================================================
=
= Function: InitAreas
=
= Description:
=
= Init player area
=
================================================================
*/
void InitAreas(void)
{
    memset(areabyplayer,0,sizeof(areabyplayer));
    
    if(player->areanumber < NUMAREAS)
    {
        areabyplayer[player->areanumber] = 1;
    }
}

/* 
================================================================
=
= Function: InitDoorList
=
= Description:
=
= Init door variables
=
================================================================
*/
void InitDoorList(void)
{
    memset (areabyplayer,0,sizeof(areabyplayer));
    memset (areaconnect,0,sizeof(areaconnect));

    lastdoorobj = &doorobjlist[0];
    doornum = 0;
}

/* 
================================================================
=
= Function: InitStaticList
=
= Description:
=
= Init StaticList
=
================================================================
*/
void InitStaticList(void)
{
    laststatobj = &statobjlist[0];
}

/* 
================================================================
=
= Function: SpawnDoor
=
= Description:
=
= Create a door at x, y
=
================================================================
*/
void SpawnDoor(S32 tilex, S32 tiley, U8 vertical, S32 lock)
{
    U16 *map;

    if(doornum == MAXDOORS)
    {
        iprintf("64+ doors on level!");
        while(1){ /* hang system */ };
    }

    doorposition[doornum] = 0;              /* doors start out fully closed */
    lastdoorobj->tilex = tilex;
    lastdoorobj->tiley = tiley;
    lastdoorobj->vertical = vertical;
    lastdoorobj->lock = lock;
    lastdoorobj->action = dr_closed;

    actorat[tilex][tiley] = (objtype *)(uintptr_t)(doornum | 0x80);   /* consider it a solid wall */
    
    /* make the door tile a special tile, and mark the adjacent tiles */
    /* for door sides                                                 */
    tilemap[tilex][tiley] = doornum | 0x80;
    map = mapsegs[0] + (tiley<<mapshift) + tilex;
    if(vertical)
    {
        *map = *(map-1);                        /* set area number */
        tilemap[tilex][tiley-1] |= 0x40;
        tilemap[tilex][tiley+1] |= 0x40;
    }
    else
    {
        *map = *(map-MAPWIDTH);                 /* set area number */
        tilemap[tilex-1][tiley] |= 0x40;
        tilemap[tilex+1][tiley] |= 0x40;
    }

    doornum++;
    lastdoorobj++;
}

/* 
================================================================
=
= Function: SpawnStatic
=
= Description:
=
= Create a static object at x, y
=
================================================================
*/
void SpawnStatic(S32 tilex, S32 tiley, S32 type)
{
    laststatobj->shapenum = statinfo[type].picnum;
    laststatobj->tilex = tilex;
    laststatobj->tiley = tiley;
    laststatobj->visspot = &spotvis[tilex][tiley];

    switch (statinfo[type].type)
    {
        case block:
            actorat[tilex][tiley] = (objtype *) 64; /* consider it a blocking tile */
        case none:
            laststatobj->flags = 0;
        break;

        case    bo_cross:
        case    bo_chalice:
        case    bo_bible:
        case    bo_crown:
        case    bo_fullheal:
            if (loadedgame == 0)
            {
                gamestate.treasuretotal++;
            }
            
        case    bo_firstaid:
        case    bo_key1:
        case    bo_key2:
        case    bo_key3:
        case    bo_key4:
        case    bo_clip:
        case    bo_25clip:
        case    bo_machinegun:
        case    bo_chaingun:
        case    bo_food:
        case    bo_alpo:
        case    bo_gibs:
        case    bo_spear:
            laststatobj->flags = FL_BONUS;
            laststatobj->itemnumber = statinfo[type].type;
        break;
        
        default:
            /* Do nothing */
        break;
    }

    laststatobj->flags |= statinfo[type].specialFlags;

    laststatobj++;

    if (laststatobj == &statobjlist[MAXSTATS])
    {
        iprintf("Too many static objects!\n");
        while(1){ /* hang system */ };
    }
}

/* 
================================================================
=
= Function: OperateDoor
=
= Description:
=
= The player wants to change the door's direction
=
================================================================
*/
void OperateDoor(U8 door)
{
    S32 lock;

    lock = doorobjlist[door].lock;
    
    /* do we have the right key to unlock door ? */
    if((lock >= dr_lock1) && (lock <= dr_lock4))
    {
        if ( ! (gamestate.keys & (1 << (lock-dr_lock1) ) ) )
        {
            SD_PlaySound(NOWAYSND);    /* No: it locked */
            return;
        }
    }

    switch(doorobjlist[door].action)
    {
        case dr_closed:
        case dr_closing:
            OpenDoor(door);
        break;
        
        case dr_open:
        case dr_opening:
            CloseDoor(door);
        break;
        
        default:
            /* do nothing */
        break;
    }
}

/* 
================================================================
=
= Function: OpenDoor
=
= Description:
=
= change door action to opening
=
================================================================
*/
void OpenDoor (U8 door)
{
    if (doorobjlist[door].action == dr_open)
    {
        doorobjlist[door].ticcount = 0;         /* reset open time */
    }
    else
    {
        doorobjlist[door].action = dr_opening;  /* start it opening */
    }
}

/* 
================================================================
=
= Function: CloseDoor
=
= Description:
=
= change door action to closing
=
================================================================
*/
static void CloseDoor(U8 door)
{
    S32    tilex;
    S32    tiley;
    S32    area;
    objtype *check;

    /* don't close on anything solid */
    tilex = doorobjlist[door].tilex;
    tiley = doorobjlist[door].tiley;

    if(actorat[tilex][tiley])
    {
        return;
    }

    if((player->tilex == tilex) && (player->tiley == tiley))
    {
        return;
    }

    if (doorobjlist[door].vertical)
    {
        if ( player->tiley == tiley )
        {
            if ( ((player->x+MINDIST) >>TILESHIFT) == tilex )
            {
                return;
            }
            
            if ( ((player->x-MINDIST) >>TILESHIFT) == tilex )
            {
                return;
            }
        }
        
        check = actorat[tilex-1][tiley];
        
        if (ISPOINTER(check) && ((check->x+MINDIST) >> TILESHIFT) == tilex )
        {
            return;
        }
        
        check = actorat[tilex+1][tiley];
        
        if (ISPOINTER(check) && ((check->x-MINDIST) >> TILESHIFT) == tilex )
        {
            return;
        }
    }
    else if (!doorobjlist[door].vertical)
    {
        if (player->tilex == tilex)
        {
            if ( ((player->y+MINDIST) >>TILESHIFT) == tiley )
            {
                return;
            }
            
            if ( ((player->y-MINDIST) >>TILESHIFT) == tiley )
            {
                return;
            }
        }
        
        check = actorat[tilex][tiley-1];
        
        if (ISPOINTER(check) && ((check->y+MINDIST) >> TILESHIFT) == tiley )
        {
            return;
        }
        
        check = actorat[tilex][tiley+1];
        
        if (ISPOINTER(check) && ((check->y-MINDIST) >> TILESHIFT) == tiley )
        {
            return;
        }
    }

    /* play door sound if in a connected area */
    area = *(mapsegs[0] + (doorobjlist[door].tiley<<mapshift)
        + doorobjlist[door].tilex) - AREATILE;
        
    if(areabyplayer[area])
    {
        PlaySoundLocTile(CLOSEDOORSND,doorobjlist[door].tilex,doorobjlist[door].tiley);
    }

    doorobjlist[door].action = dr_closing;

    /* make the door space solid */
    actorat[tilex][tiley] = (objtype *)(uintptr_t)(door | 0x80);
}

/* 
================================================================
=
= Function: DoorOpen
=
= Description:
=
= Close the door after three seconds
=
================================================================
*/
static void DoorOpen (U8 door)
{
    if((doorobjlist[door].ticcount += (S16) tics) >= OPENTICS)
    {
        CloseDoor(door);
    }
}

/* 
================================================================
=
= Function: DoorOpening
=
= Description:
=
= Open door and connect areas
=
================================================================
*/
static void DoorOpening(U8 door)
{
    U32 area1;
    U32 area2;
    U16 *map;
    U32 position;

    position = doorposition[door];
    
    if(position == 0)
    {
        /* door is just starting to open, so connect the areas */
        map = mapsegs[0] + (doorobjlist[door].tiley<<mapshift)
            +doorobjlist[door].tilex;

        if(doorobjlist[door].vertical)
        {
            area1 = *(map+1);
            area2 = *(map-1);
        }
        else
        {
            area1 = *(map-MAPWIDTH);
            area2 = *(map+MAPWIDTH);
        }
        area1 -= AREATILE;
        area2 -= AREATILE;

        /* are areas both sides of the door floor tiles */
        if (area1 < NUMAREAS && area2 < NUMAREAS)
        {
            areaconnect[area1][area2]++;
            areaconnect[area2][area1]++;

            if (player->areanumber < NUMAREAS)
            {
                ConnectAreas();
            }

            if (areabyplayer[area1])
            {
                PlaySoundLocTile(OPENDOORSND,doorobjlist[door].tilex,doorobjlist[door].tiley);
            }
        }
    }

    /* slide the door by an adaptive amount */
    position += tics<<10;
    if (position >= 0xffff)
    {
        /* door is all the way open */
        position = 0xffff;
        doorobjlist[door].ticcount = 0;
        doorobjlist[door].action = dr_open;
        actorat[doorobjlist[door].tilex][doorobjlist[door].tiley] = 0;
    }

    doorposition[door] = (U16) position;
}

/* 
================================================================
=
= Function: DoorClosing
=
= Description:
=
= Close doors and disconnect areas
=
================================================================
*/
static void DoorClosing(U8 door)
{
    U32 area1;
    U32 area2;
    U16 *map;
    S32 position;
    S32 tilex;
    S32 tiley;

    tilex = doorobjlist[door].tilex;
    tiley = doorobjlist[door].tiley;

    if ( ((S32)(uintptr_t)actorat[tilex][tiley] != (door | 0x80))
        || ((player->tilex == tilex) && (player->tiley == tiley)) )
    {                       
        /* something got inside the door */
        OpenDoor(door);
        return;
    };

    position = doorposition[door];

    /* slide the door by an adaptive amount */
    
    position -= tics << 10;
    
    if(position <= 0)
    {
        /* door is closed all the way, so disconnect the areas */
        position = 0;

        doorobjlist[door].action = dr_closed;

        map = mapsegs[0] + (doorobjlist[door].tiley<<mapshift) + doorobjlist[door].tilex;

        if (doorobjlist[door].vertical)
        {
            area1 = *(map+1);
            area2 = *(map-1);
        }
        else
        {
            area1 = *(map-MAPWIDTH);
            area2 = *(map+MAPWIDTH);
        }
        
        area1 -= AREATILE;
        area2 -= AREATILE;

         /* are areas both sides of the door floor tiles */
        if (area1 < NUMAREAS && area2 < NUMAREAS)
        {
            areaconnect[area1][area2]--;
            areaconnect[area2][area1]--;

            if (player->areanumber < NUMAREAS)
            {
                ConnectAreas();
            }
        }
    }

    doorposition[door] = (U16) position;
}

/* 
================================================================
=
= Function: MoveDoors
=
= Description:
=
= Called from PlayLoop updates door action (ie open / close)
=
================================================================
*/
void MoveDoors(void)
{
    U8 door;

    if(gamestate.victoryflag)    /* don't move door during victory sequence */
    {
        return;
    }

    for (door = 0; door < doornum; door++)
    {
        switch(doorobjlist[door].action)
        {
            case dr_open:
                DoorOpen(door);
            break;

            case dr_opening:
                DoorOpening(door);
            break;

            case dr_closing:
                DoorClosing(door);
            break;
            
            default:
                /* do nothing */
            break;
        }
    }
}

/* 
================================================================
=
= Function: PlaceItemType
=
= Description:
=
= Called during game play to drop actors' items.  It finds the proper
= item number based on the item type (bo_???).  If there are no free item
= spots, nothing is done.
=
================================================================
*/
void PlaceItemType (S32 itemtype, S32 tilex, S32 tiley)
{
    S32 type;
    statobj_t *spot;

    /* find the item number */
    for(type = 0; ; type++)
    {
        if (statinfo[type].picnum == -1)    /* end of list */
        {
            iprintf("PlaceItemType: couldn't find type!");
            while(1){ /* hang system */ };
        }
        
        if (statinfo[type].type == itemtype)
        {
            break;
        }
    }

    /* find a spot in statobjlist to put it in */
    for(spot = &statobjlist[0]; ; spot++)
    {
        if (spot==laststatobj)
        {
            if (spot == &statobjlist[MAXSTATS])
            {
                return;    /* no free spots */
            }
            laststatobj++;    /* space at end */
            break;
        }

        if (spot->shapenum == -1)     /* -1 is a free spot */
        {
            break;
        }
    }

    /* place it */
    spot->shapenum = statinfo[type].picnum;
    spot->tilex = tilex;
    spot->tiley = tiley;
    spot->visspot = &spotvis[tilex][tiley];
    spot->flags = FL_BONUS | statinfo[type].specialFlags;
    spot->itemnumber = statinfo[type].type;
}