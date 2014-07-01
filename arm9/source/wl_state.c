#include <nds.h>
#include <stdio.h>
#include <stdlib.h>
#include "types.h"
#include "audiowl6.h"
#include "id_us.h"
#include "id_sd.h"
#include "wl_def.h"
#include "wl_play.h"
#include "wl_act1.h"
#include "wl_act2.h"
#include "wl_state.h"
#include "wl_agent.h"

/* macro defines */

#define MINSIGHT     0x18000l

#define CHECKDIAG(x,y)                              \
{                                                   \
    temp = (uintptr_t)actorat[x][y];                \
    if (temp)                                       \
    {                                               \
        if (temp<256)                               \
        {                                           \
            return 0;                               \
        }                                           \
        if (((objtype *)temp)->flags&FL_SHOOTABLE)  \
        {                                           \
            return 0;                               \
        }                                           \
    }                                               \
}

#define DOORCHECK                                   \
            doornum = (S32) temp & 127;             \
            OpenDoor(doornum);                      \
            ob->distance = -doornum - 1;            \
            return 1;

#define CHECKSIDE(x,y)                                  \
{                                                       \
    temp=(uintptr_t)actorat[x][y];                      \
    if (temp)                                           \
    {                                                   \
        if (temp<128)                                   \
        {                                               \
            return 0;                                   \
        }                                               \
        if (temp<256)                                   \
        {                                               \
            DOORCHECK                                   \
        }                                               \
        else if (((objtype *)temp)->flags&FL_SHOOTABLE) \
        {                                               \
            return 0;                                   \
        }                                               \
    }                                                   \
}

/* external variables */

extern objtype *newobj;
extern objtype *actorat[MAPSIZE][MAPSIZE];
extern U16 * mapsegs[MAPPLANES];
extern U32 tics;
extern U8 areabyplayer[NUMAREAS];
extern U8 madenoise;
extern objtype *player;
extern U16 plux;
extern U16 pluy;    /* player coordinates scaled to unsigned */
extern U8 tilemap[MAPSIZE][MAPSIZE];
extern U16 doorposition[MAXDOORS];    /* leading edge of door 0=closed, 0xffff = fully open */
extern gametype gamestate;

/* global variables */

/* None */

/* local prototypes */

static U8 CheckSight(objtype *ob);
static void FirstSighting(objtype *ob);
static void KillActor(objtype *ob);

/* local variables */

static const dirtype opposite[9] = {west,southwest,south,southeast,east,northeast,north,northwest,nodir};

static const dirtype diagonal[9][9] =
{
    /* east */  {nodir,nodir,northeast,nodir,nodir,nodir,southeast,nodir,nodir},
                {nodir,nodir,nodir,nodir,nodir,nodir,nodir,nodir,nodir},
    /* north */ {northeast,nodir,nodir,nodir,northwest,nodir,nodir,nodir,nodir},
                {nodir,nodir,nodir,nodir,nodir,nodir,nodir,nodir,nodir},
    /* west */  {nodir,nodir,northwest,nodir,nodir,nodir,southwest,nodir,nodir},
                {nodir,nodir,nodir,nodir,nodir,nodir,nodir,nodir,nodir},
    /* south */ {southeast,nodir,nodir,nodir,southwest,nodir,nodir,nodir,nodir},
                {nodir,nodir,nodir,nodir,nodir,nodir,nodir,nodir,nodir},
                {nodir,nodir,nodir,nodir,nodir,nodir,nodir,nodir,nodir}
};

/*
================================================================
=
= Function: SpawnNewObj
=
= Description:
=
= Spaws a new actor at the given TILE coordinates, with the given state, and
= the given size in GLOBAL units.
=
= newobj = a pointer to an initialized new actor
=
================================================================
*/
void SpawnNewObj(U32 tilex, U32 tiley, statetype *state)
{
    GetNewActor ();
    newobj->state = state;

    newobj->ticcount = 0;

    newobj->tilex = (S16) tilex;
    newobj->tiley = (S16) tiley;
    newobj->x = ((S32)tilex<<TILESHIFT)+TILEGLOBAL/2;
    newobj->y = ((S32)tiley<<TILESHIFT)+TILEGLOBAL/2;
    newobj->dir = nodir;

    actorat[tilex][tiley] = newobj;

    newobj->areanumber =
        *(mapsegs[0] + (newobj->tiley<<mapshift)+newobj->tilex) - AREATILE;
}

/*
================================================================
=
= Function: SightPlayer
=
= Description:
=
= Called by actors that ARE NOT chasing the player. If the player
= is detected (by sight, noise, or proximity), the actor is put
= into it's combat frame and true is returned.
=
= Incorporates a random reaction delay
=
================================================================
*/
U8 SightPlayer(objtype *ob)
{
    if(ob->flags & FL_ATTACKMODE)
    {
        iprintf("An actor in ATTACKMODE called SightPlayer!");
        while(1){ /* hang system */ };
    }

    if(ob->temp2 != 0)
    {
        /* count down reaction time */
        ob->temp2 -= (S16) tics;
        if (ob->temp2 > 0)
        {
            return 0;
        }
        ob->temp2 = 0;    /* time to react */
    }
    else
    {
        /* is actor in the same area / location as player ? */
        if(areabyplayer[ob->areanumber] == 0)
        {
            return 0;    /* no then return */
        }

        /* if this flag is set actor will try and ambush player        */
        /* as soon as he is the same area and the actor can see player */
        if(ob->flags & FL_AMBUSH)
        {
            /* can actor see player ? */
            if(!CheckSight(ob))
            {
                return 0; /* no then return */
            }

            ob->flags &= ~FL_AMBUSH;
        }
        else
        {
            /* if player didnt make a noise and actor cant see player return */
            if(!madenoise && !CheckSight(ob))
            {
                return 0;
            }
        }

        /* player is in current location but not    */
        /* visable set new reaction time and return */
        switch (ob->obclass)
        {
            case guardobj:
                ob->temp2 = 1+US_RndT()/4;
            break;

            case officerobj:
                ob->temp2 = 2;
            break;

            case mutantobj:
                ob->temp2 = 1+US_RndT()/6;
            break;

            case ssobj:
                ob->temp2 = 1+US_RndT()/6;
            break;

            case dogobj:
                ob->temp2 = 1+US_RndT()/8;
            break;

            case bossobj:
            case schabbobj:
            case fakeobj:
            case mechahitlerobj:
            case realhitlerobj:
            case gretelobj:
            case giftobj:
            case fatobj:
            case spectreobj:
            case angelobj:
            case transobj:
            case uberobj:
            case willobj:
            case deathobj:
                ob->temp2 = 1;
            break;

            default:
                /* do nothing */
            break;
        }

        return 0;
    }

    /* put actor into combat state */
    FirstSighting(ob);

    return 1;
}

/*
================================================================
=
= Function: CheckSight
=
= Description:
=
= Checks a straight line between player and current object If
= the sight is ok, check alertness and angle to see if they
= notice
=
= returns true if the player has been spoted
=
================================================================
*/
static U8 CheckSight(objtype *ob)
{
    S32 deltax,deltay;

    /* don't bother tracing a line if the area isn't connected to the player's */
    if(areabyplayer[ob->areanumber] == 0)
    {
        return 0;
    }

    /* if the player is real close, sight is automatic */
    deltax = player->x - ob->x;
    deltay = player->y - ob->y;

    if (deltax > -MINSIGHT && deltax < MINSIGHT
                                 && deltay > -MINSIGHT && deltay < MINSIGHT)
    {
        return 1;
    }

    /* see if they are looking in the right direction */
    switch (ob->dir)
    {
        case north:
            if(deltay > 0)
            {
                return 0;
            }
        break;

        case east:
            if(deltax < 0)
            {
                return 0;
            }
        break;

        case south:
            if (deltay < 0)
            {
                return 0;
            }
        break;

        case west:
            if(deltax > 0)
            {
                return 0;
            }
        break;

        /* check diagonal moving guards */
        case northwest:
            if(deltay > -deltax)
            {
                return 0;
            }
        break;

        case northeast:
            if(deltay > deltax)
            {
                return 0;
            }
        break;

        case southwest:
            if(deltax > deltay)
            {
                return 0;
            }
        break;

        case southeast:
            if(-deltax > deltay)
            {
                return 0;
            }
        break;

        default:
            /* do nothing */
        break;
    }

    /* trace a line to check for blocking tiles (corners) */
    return CheckLine(ob);
}

/*
================================================================
=
= Function: CheckLine
=
= Description:
=
= Returns true if a straight line between the player and
= ob is unobstructed
=
================================================================
*/
U8 CheckLine(objtype *ob)
{
    S32 xstep,ystep;
    S32 xdist,ydist;
    S32 ltemp;
    S32 x1,y1,x2,y2;
    S32 xt1,yt1,xt2,yt2;
    S32 partial;
    S32 deltafrac;
    S32 delta;
    S32 x,y;
    S32 yfrac,xfrac;
    U32 value;
    U32 intercept;

    /* convert actor coords to tile coords */
    x1 = ob->x >> UNSIGNEDSHIFT;    /* 1/256 tile precision */
    y1 = ob->y >> UNSIGNEDSHIFT;
    xt1 = x1 >> 8;
    yt1 = y1 >> 8;

    x2 = plux;
    y2 = pluy;
    xt2 = player->tilex;
    yt2 = player->tiley;

    /* x tile distance between actor and player */
    xdist = abs(xt2-xt1);

    /* check to see if player and actor are not in a straight */
    /* vertical line with each other. if not we need to inc   */
    /* x by one tile and adjust y by slope of line of sight   */
    if (xdist > 0)
    {
        /* is player to the right of actor */
         if (xt2 > xt1)
        {
            /* yes xstep is positive (increasing), also round */
            /* actor partial position to the right            */
            partial = 256-(x1&0xff);
            xstep = 1;
        }
        else
        {
            /* no xstep is negative (decreasing), also round */
            /* actor partial position to the left            */
            partial = x1&0xff;
            xstep = -1;
        }

        /* work out slope of line (ystep), first get delta x and y */
        deltafrac = abs(x2 - x1);
        delta = y2 - y1;
        /* second divide y / x to get slope (ystep) */
        ltemp = ((S32)delta<<8)/deltafrac;
        /* limit slope Of line to 7fff */
        if (ltemp > 0x7fffl)
        {
            ystep = 0x7fff;
        }
        else if (ltemp < -0x7fffl)
        {
            ystep = -0x7fff;
        }
        else
        {
            ystep = ltemp;
        }

        /* set starting y point of line of sight */
        yfrac = y1 + (((S32)ystep*partial) >> 8);
        /* set starting x point of line of sight */
        x = xt1+xstep;
        /* set end x point of line of sight */
        xt2 += xstep;

         do
        {
            y = yfrac >> 8;
            yfrac += ystep;

            value = (U32)tilemap[x][y];
            x += xstep;

            /* check to see if tile location is empty */
            if(value == 0)
            {
                continue; /* yes so check next tile location */
            }

            /* check to see if tile holds a wall value */
            if (value<128 || value>256)
            {
                return 0; /* it does line of sight broken */
            }

            /* see if the door is open enough */
            value &= ~0x80;
            intercept = yfrac-ystep/2;

            if (intercept > doorposition[value])
            {
                return 0; /* line of sight broken by closed door */
            }

        }while(x != xt2);

    }

    /* y tile distance between actor and player */
    ydist = abs(yt2-yt1);

    /* check to see if player and actor are not in a straight */
    /* horizontal line with each other. if not we need to inc */
    /* y by one tile and adjust x by slope of line of sight   */
    if(ydist > 0)
    {
        /* is player below actor */
        if (yt2 > yt1)
        {
            /* yes ystep is positive (increasing), also round */
            /* actor partial position downwards               */
            partial = 256-(y1&0xff);
            ystep = 1;
        }
        else
        {
            /* no ystep is negative (decreasing), also round */
            /* actor partial position up                     */
            partial = y1&0xff;
            ystep = -1;
        }

        /* work out slope of line (xstep), first get delta x and y */
        deltafrac = abs(y2-y1);
        delta = x2-x1;
        /* second divide y / x to get slope (Xstep) */
        ltemp = ((S32)delta<<8)/deltafrac;
        /* limit slope Of line to 7fff */
        if (ltemp > 0x7fffl)
        {
            xstep = 0x7fff;
        }
        else if (ltemp < -0x7fffl)
        {
            xstep = -0x7fff;
        }
        else
        {
            xstep = ltemp;
        }

        /* set starting x point of line of sight */
        xfrac = x1 + (((S32)xstep*partial) >>8);
        /* set starting y point of line of sight */
        y = yt1 + ystep;
        /* set end y point of line of sight */
        yt2 += ystep;

        do
        {
            x = xfrac>>8;
            xfrac += xstep;

            value = (U32)tilemap[x][y];
            y += ystep;

            /* check to see if tile location is empty */
            if(value == 0)
            {
                continue; /* yes so check next tile location */
            }

            if(value<128 || value>256)
            {
                return 0;
            }

            /* see if the door is open enough */
            value &= ~0x80;
            intercept = xfrac-xstep/2;

            if (intercept>doorposition[value])
            {
                return 0; /* line of sight broken by closed door */
            }

        }while(y != yt2);
    }

    /* clear line of sight to player so return true */
    return 1;
}

/*
================================================================
=
= Function: FirstSighting
=
= Description:
=
= Puts an actor into attack mode and possibly reverses the
= direction if the player is behind it.
=
================================================================
*/
static void FirstSighting(objtype *ob)
{
    /* react to the player */
    switch (ob->obclass)
    {
        case guardobj:
            PlaySoundLocActor(HALTSND,ob);
            NewState(ob,&s_grdchase1);
            ob->speed *= 3;                 /* go faster when chasing player */
        break;

        case officerobj:
            PlaySoundLocActor(SPIONSND,ob);
            NewState (ob,&s_ofcchase1);
            ob->speed *= 5;                 /* go faster when chasing player */
        break;

        case mutantobj:
            NewState (ob,&s_mutchase1);
            ob->speed *= 3;                 /* go faster when chasing player */
        break;

        case ssobj:
            PlaySoundLocActor(SCHUTZADSND,ob);
            NewState (ob,&s_sschase1);
            ob->speed *= 4;                 /* go faster when chasing player */
        break;

        case dogobj:
            PlaySoundLocActor(DOGBARKSND,ob);
            NewState (ob,&s_dogchase1);
            ob->speed *= 2;                 /* go faster when chasing player */
        break;

        case bossobj:
            SD_PlaySound(GUTENTAGSND);
            NewState (ob,&s_bosschase1);
            ob->speed = SPDPATROL*3;        /* go faster when chasing player */
        break;

        case gretelobj:
            SD_PlaySound(KEINSND);
            NewState (ob,&s_gretelchase1);
            ob->speed *= 3;                 /* go faster when chasing player */
        break;

        case giftobj:
            SD_PlaySound(EINESND);
            NewState (ob,&s_giftchase1);
            ob->speed *= 3;                 /* go faster when chasing player */
        break;

        case fatobj:
            SD_PlaySound(ERLAUBENSND);
            NewState (ob,&s_fatchase1);
            ob->speed *= 3;                 /* go faster when chasing player */
        break;

        case schabbobj:
            SD_PlaySound(SCHABBSHASND);
            NewState (ob,&s_schabbchase1);
            ob->speed *= 3;                 /* go faster when chasing player */
        break;

        case fakeobj:
            SD_PlaySound(TOT_HUNDSND);
            NewState (ob,&s_fakechase1);
            ob->speed *= 3;                 /* go faster when chasing player */
        break;

        case mechahitlerobj:
            SD_PlaySound(DIESND);
            NewState (ob,&s_mechachase1);
            ob->speed *= 3;                 /* go faster when chasing player */
        break;

        case realhitlerobj:
            SD_PlaySound(DIESND);
            NewState (ob,&s_hitlerchase1);
            ob->speed *= 5;                 /* go faster when chasing player */
        break;

        case ghostobj:
            NewState (ob,&s_blinkychase1);
            ob->speed *= 2;                 /* go faster when chasing player */
        break;

        default:
            /* do nothing */
        break;
    }

    if (ob->distance < 0)
    {
        ob->distance = 0;    /* ignore the door opening command */
    }

    ob->flags |= FL_ATTACKMODE|FL_FIRSTATTACK;
}

/*
================================================================
=
= Function: NewState
=
= Description:
=
= Changes ob to a new state, setting ticcount to the max
= for that state
=
================================================================
*/
void NewState(objtype *ob, statetype *state)
{
    ob->state = state;
    ob->ticcount = state->tictime;
}

/*
================================================================
=
= Function: SelectDodgeDir
=
= Description:
=
=
= SelectDodgeDir
=
= Attempts to choose and initiate a movement for ob that sends it towards
= the player while dodging
=
= If there is no possible move (ob is totally surrounded)
=
= ob->dir           = nodir
=
= Otherwise
=
= ob->dir           = new direction to follow
= ob->distance      = TILEGLOBAL or -doornumber
= ob->tilex         = new destination
= ob->tiley
= ob->areanumber    = the floor tile number (0-(NUMAREAS-1)) of destination
=
================================================================
*/
void SelectDodgeDir(objtype *ob)
{
    S32        deltax,deltay,i;
    U32        absdx,absdy;
    dirtype    dirtry[5];
    dirtype    turnaround,tdir;

    if(ob->flags & FL_FIRSTATTACK)
    {
        /* turning around is only ok the very first time after */
        /* noticing the player                                 */
        turnaround = nodir;
        ob->flags &= ~FL_FIRSTATTACK;
    }
    else
    {
        turnaround = opposite[ob->dir];
    }

    deltax = player->tilex - ob->tilex;
    deltay = player->tiley - ob->tiley;

    /* arange 5 direction choices in order of preference               */
    /* the four cardinal directions plus the diagonal straight towards */
    /* the player                                                      */

    if(deltax>0)
    {
        dirtry[1]= east;
        dirtry[3]= west;
    }
    else
    {
        dirtry[1]= west;
        dirtry[3]= east;
    }

    if(deltay>0)
    {
        dirtry[2]= south;
        dirtry[4]= north;
    }
    else
    {
        dirtry[2]= north;
        dirtry[4]= south;
    }

    /* randomize a bit for dodging */
    absdx = abs(deltax);
    absdy = abs(deltay);

    if (absdx > absdy)
    {
        tdir = dirtry[1];
        dirtry[1] = dirtry[2];
        dirtry[2] = tdir;
        tdir = dirtry[3];
        dirtry[3] = dirtry[4];
        dirtry[4] = tdir;
    }

    if (US_RndT() < 128)
    {
        tdir = dirtry[1];
        dirtry[1] = dirtry[2];
        dirtry[2] = tdir;
        tdir = dirtry[3];
        dirtry[3] = dirtry[4];
        dirtry[4] = tdir;
    }

    dirtry[0] = diagonal[dirtry[1]][dirtry[2]];

    /* try the directions util one works */
    for(i=0;i<5;i++)
    {
        if( dirtry[i] == nodir || dirtry[i] == turnaround)
        {
            continue;
        }

        ob->dir = dirtry[i];

        if(TryWalk(ob))
        {
            return;
        }
    }

    /* turn around only as a last resort */
    if(turnaround != nodir)
    {
        ob->dir = turnaround;

        if(TryWalk(ob))
        {
            return;
        }
    }

    ob->dir = nodir;
}

/*
================================================================
=
= Function: SelectChaseDir
=
= Description:
=
= As SelectDodgeDir, but doesn't try to dodge
=
= If there is no possible move (ob is totally surrounded)
=
= ob->dir           = nodir
=
= Otherwise
=
= ob->dir           = new direction to follow
= ob->distance      = TILEGLOBAL or -doornumber
= ob->tilex         = new destination
= ob->tiley
= ob->areanumber    = the floor tile number (0-(NUMAREAS-1)) of destination
=
================================================================
*/
void SelectChaseDir(objtype *ob)
{
    S32 deltax,deltay;
    dirtype d[3];
    dirtype tdir, olddir, turnaround;

    olddir = ob->dir;
    turnaround = opposite[olddir];

    deltax=player->tilex - ob->tilex;
    deltay=player->tiley - ob->tiley;

    d[1]=nodir;
    d[2]=nodir;

    if(deltax>0)
    {
        d[1] = east;
    }
    else if(deltax < 0)
    {
        d[1] = west;
    }

    if(deltay > 0)
    {
        d[2] = south;
    }
    else if(deltay < 0)
    {
        d[2] = north;
    }

    if(abs(deltay) > abs(deltax))
    {
        tdir = d[1];
        d[1] = d[2];
        d[2] = tdir;
    }

    if(d[1] == turnaround)
    {
        d[1] = nodir;
    }

    if(d[2] == turnaround)
    {
        d[2] = nodir;
    }

    if(d[1] != nodir)
    {
        ob->dir=d[1];

        if (TryWalk(ob))
        {
            return;     /*either moved forward or attacked*/
        }
    }

    if (d[2] != nodir)
    {
        ob->dir=d[2];

        if (TryWalk(ob))
        {
            return;
        }
    }

    /* there is no direct path to the player, so pick another direction */
    if(olddir != nodir)
    {
        ob->dir=olddir;

        if(TryWalk(ob))
        {
            return;
        }
    }

    if (US_RndT()>128)      /*randomly determine direction of search*/
    {
        for (tdir=north; tdir<=west; tdir=(dirtype)(tdir+1))
        {
            if (tdir!=turnaround)
            {
                ob->dir=tdir;
                if ( TryWalk(ob) )
                {
                    return;
                }
            }
        }
    }
    else
    {
        for (tdir=west; tdir>=north; tdir=(dirtype)(tdir-1))
        {
            if (tdir!=turnaround)
            {
                ob->dir=tdir;
                if ( TryWalk(ob) )
                {
                    return;
                }
            }
        }
    }

    if (turnaround !=  nodir)
    {
        ob->dir=turnaround;

        if (ob->dir != nodir)
        {
            if ( TryWalk(ob) )
            {
                return;
            }
        }
    }

    ob->dir = nodir;    /* can't move */

}

/*
================================================================
=
= Function: MoveObj
=
= Description:
=
= Moves ob be move global units in ob->dir direction
= Actors are not allowed to move inside the player
= Does NOT check to see if the move is tile map valid
=
= ob->x                 = adjusted for new position
= ob->y
=
================================================================
*/
void MoveObj(objtype *ob, S32 move)
{
    S32 deltax,deltay;

    switch (ob->dir)
    {
        case north:
            ob->y -= move;
        break;

        case northeast:
            ob->x += move;
            ob->y -= move;
        break;

        case east:
            ob->x += move;
        break;

        case southeast:
            ob->x += move;
            ob->y += move;
        break;

        case south:
            ob->y += move;
        break;

        case southwest:
            ob->x -= move;
            ob->y += move;
        break;

        case west:
            ob->x -= move;
        break;

        case northwest:
            ob->x -= move;
            ob->y -= move;
        break;

        case nodir:
            return;
        break;

        default:
            iprintf("MoveObj: bad dir!");
            while(1){ /* hang system */}
        break;
    }

    /* check to make sure it's not on top of player */
    if (areabyplayer[ob->areanumber])
    {
        deltax = ob->x - player->x;

        if (deltax < -MINACTORDIST || deltax > MINACTORDIST)
        {
            /* move ok */
            ob->distance -=move;
            return;
        }

        deltay = ob->y - player->y;

        if (deltay < -MINACTORDIST || deltay > MINACTORDIST)
        {
            /* move ok */
            ob->distance -=move;
            return;
        }

        if (ob->hidden)    /* move closer until he meets CheckLine */
        {
            /* move ok */
            ob->distance -=move;
            return;
        }


        if (ob->obclass == ghostobj || ob->obclass == spectreobj)
        {
            TakeDamage (tics*2,ob);
        }

        /* back up */
        switch (ob->dir)
        {
            case north:
                ob->y += move;
            break;

            case northeast:
                ob->x -= move;
                ob->y += move;
            break;

            case east:
                ob->x -= move;
            break;

            case southeast:
                ob->x -= move;
                ob->y -= move;
            break;

            case south:
                ob->y -= move;
            break;

            case southwest:
                ob->x += move;
                ob->y -= move;
            break;

            case west:
                ob->x += move;
            break;

            case northwest:
                ob->x += move;
                ob->y += move;
            break;

            case nodir:
                return;
            break;
        }
    }
}

/*
================================================================
=
= Function: TryWalk
=
= Description:
=
= Attempts to move ob in its current (ob->dir) direction.
=
= If blocked by either a wall or an actor returns FALSE
=
= If move is either clear or blocked only by a door, returns TRUE and sets
=
= ob->tilex         = new destination
= ob->tiley
= ob->areanumber    = the floor tile number (0-(NUMAREAS-1)) of destination
= ob->distance      = TILEGLOBAl, or -doornumber if a door is blocking the way
=
= If a door is in the way, an OpenDoor call is made to start it opening.
= The actor code should wait until
=       doorobjlist[-ob->distance].action = dr_open, meaning the door has been
=       fully opened
=
=
================================================================
*/
U8 TryWalk (objtype *ob)
{
    S32 doornum = -1;
    uintptr_t temp;

    if(ob->obclass == inertobj)
    {
        switch (ob->dir)
        {
            case north:
                ob->tiley--;
            break;

            case northeast:
                ob->tilex++;
                ob->tiley--;
            break;

            case east:
                ob->tilex++;
            break;

            case southeast:
                ob->tilex++;
                ob->tiley++;
            break;

            case south:
                ob->tiley++;
            break;

            case southwest:
                ob->tilex--;
                ob->tiley++;
            break;

            case west:
                ob->tilex--;
            break;

            case northwest:
                ob->tilex--;
                ob->tiley--;
            break;

            default:
                /* do nothing */
            break;

        }
    }
    else
    {
        switch (ob->dir)
        {
            case north:
                if (ob->obclass == dogobj || ob->obclass == fakeobj
                    || ob->obclass == ghostobj || ob->obclass == spectreobj)
                {
                    CHECKDIAG(ob->tilex,ob->tiley-1);
                }
                else
                {
                    CHECKSIDE(ob->tilex,ob->tiley-1);
                }
                ob->tiley--;
            break;

            case northeast:
                CHECKDIAG(ob->tilex+1,ob->tiley-1);
                CHECKDIAG(ob->tilex+1,ob->tiley);
                CHECKDIAG(ob->tilex,ob->tiley-1);
                ob->tilex++;
                ob->tiley--;
            break;

            case east:
                if (ob->obclass == dogobj || ob->obclass == fakeobj
                    || ob->obclass == ghostobj || ob->obclass == spectreobj)
                {
                    CHECKDIAG(ob->tilex+1,ob->tiley);
                }
                else
                {
                    CHECKSIDE(ob->tilex+1,ob->tiley);
                }
                ob->tilex++;
            break;

            case southeast:
                CHECKDIAG(ob->tilex+1,ob->tiley+1);
                CHECKDIAG(ob->tilex+1,ob->tiley);
                CHECKDIAG(ob->tilex,ob->tiley+1);
                ob->tilex++;
                ob->tiley++;
            break;

            case south:
                if (ob->obclass == dogobj || ob->obclass == fakeobj
                    || ob->obclass == ghostobj || ob->obclass == spectreobj)
                {
                    CHECKDIAG(ob->tilex,ob->tiley+1);
                }
                else
                {
                    CHECKSIDE(ob->tilex,ob->tiley+1);
                }
                ob->tiley++;
            break;

            case southwest:
                CHECKDIAG(ob->tilex-1,ob->tiley+1);
                CHECKDIAG(ob->tilex-1,ob->tiley);
                CHECKDIAG(ob->tilex,ob->tiley+1);
                ob->tilex--;
                ob->tiley++;
            break;

            case west:
                if (ob->obclass == dogobj || ob->obclass == fakeobj
                    || ob->obclass == ghostobj || ob->obclass == spectreobj)
                {
                    CHECKDIAG(ob->tilex-1,ob->tiley);
                }
                else
                {
                    CHECKSIDE(ob->tilex-1,ob->tiley);
                }
                ob->tilex--;
            break;

            case northwest:
                CHECKDIAG(ob->tilex-1,ob->tiley-1);
                CHECKDIAG(ob->tilex-1,ob->tiley);
                CHECKDIAG(ob->tilex,ob->tiley-1);
                ob->tilex--;
                ob->tiley--;
            break;

            case nodir:
                return 0;
            break;

            default:
                iprintf("Walk: Bad dir");
                while(1){/*  hang system */}
            break;
        }
    }

    ob->areanumber =
        *(mapsegs[0] + (ob->tiley<<mapshift)+ob->tilex) - AREATILE;

    ob->distance = TILEGLOBAL;
    return 1;
}

/*
================================================================
=
= Function: DamageActor
=
= Description:
=
= Called when the player succesfully hits an enemy.
=
= Does damage points to enemy ob, either putting it into a stun frame or
= killing it.
=
================================================================
*/
void DamageActor (objtype *ob, U32 damage)
{
    madenoise = true;

    /* do double damage if shooting a non attack mode actor */
    if( !(ob->flags & FL_ATTACKMODE) )
    {
        damage <<= 1;
    }

    ob->hitpoints -= (short)damage;

    if (ob->hitpoints<=0)
    {
        KillActor(ob);
    }
    else
    {
        if(!(ob->flags & FL_ATTACKMODE) )
        {
            FirstSighting (ob);    /* put into combat mode */
        }

        switch (ob->obclass)    /* dogs only have one hit point */
        {
            case guardobj:
                if (ob->hitpoints&1)
                {
                    NewState(ob,&s_grdpain);
                }
                else
                {
                    NewState(ob,&s_grdpain1);
                }
            break;

            case officerobj:
                if (ob->hitpoints&1)
                {
                    NewState (ob,&s_ofcpain);
                }
                else
                {
                    NewState (ob,&s_ofcpain1);
                }
            break;

            case mutantobj:
                if (ob->hitpoints&1)
                {
                    NewState (ob,&s_mutpain);
                }
                else
                {
                    NewState (ob,&s_mutpain1);
                }
            break;

            case ssobj:
                if (ob->hitpoints&1)
                {
                    NewState (ob,&s_sspain);
                }
                else
                {
                    NewState (ob,&s_sspain1);
                }
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
= Function: KillActor
=
= Description:
=
= start the actors death sequence.
=
================================================================
*/
static void KillActor(objtype *ob)
{
    S32    tilex;
    S32    tiley;

    tilex = ob->tilex = (U16)(ob->x >> TILESHIFT);    /* drop item on center */
    tiley = ob->tiley = (U16)(ob->y >> TILESHIFT);

    switch (ob->obclass)
    {
        case guardobj:
            GivePoints(100);
            NewState(ob,&s_grddie1);
            PlaceItemType(bo_clip2,tilex,tiley);
        break;

        case officerobj:
            GivePoints(400);
            NewState(ob,&s_ofcdie1);
            PlaceItemType(bo_clip2,tilex,tiley);
        break;

        case mutantobj:
            GivePoints (700);
            NewState (ob,&s_mutdie1);
            PlaceItemType (bo_clip2,tilex,tiley);
        break;

        case ssobj:
            GivePoints (500);
            NewState (ob,&s_ssdie1);
            if (gamestate.bestweapon < wp_machinegun)
            {
                PlaceItemType (bo_machinegun,tilex,tiley);
            }
            else
            {
                PlaceItemType (bo_clip2,tilex,tiley);
            }
        break;

        case dogobj:
            GivePoints (200);
            NewState (ob,&s_dogdie1);
        break;

        case bossobj:
            GivePoints (5000);
            NewState (ob,&s_bossdie1);
            PlaceItemType (bo_key1,tilex,tiley);
        break;

        case gretelobj:
            GivePoints (5000);
            NewState (ob,&s_greteldie1);
            PlaceItemType (bo_key1,tilex,tiley);
        break;

        case giftobj:
            GivePoints (5000);
            gamestate.killx = player->x;
            gamestate.killy = player->y;
            NewState (ob,&s_giftdie1);
        break;

        case fatobj:
            GivePoints (5000);
            gamestate.killx = player->x;
            gamestate.killy = player->y;
            NewState (ob,&s_fatdie1);
        break;

        case schabbobj:
            GivePoints (5000);
            gamestate.killx = player->x;
            gamestate.killy = player->y;
            NewState (ob,&s_schabbdie1);
        break;

        case fakeobj:
            GivePoints (2000);
            NewState (ob,&s_fakedie1);
        break;

        case mechahitlerobj:
            GivePoints (5000);
            NewState (ob,&s_mechadie1);
        break;

        case realhitlerobj:
            GivePoints (5000);
            gamestate.killx = player->x;
            gamestate.killy = player->y;
            NewState (ob,&s_hitlerdie1);
        break;

        default:
            /* do nothing */
        break;
    }

    gamestate.killcount++;
    ob->flags &= ~FL_SHOOTABLE;
    actorat[ob->tilex][ob->tiley] = NULL;
    ob->flags |= FL_NONMARK;

}
