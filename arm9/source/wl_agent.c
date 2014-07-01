#include <nds.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "types.h"
#include "wl_def.h"
#include "gfxv_apo.h"
#include "audiowl6.h"
#include "id_vh.h"
#include "id_sd.h"
#include "wl_agent.h"
#include "id_us.h"
#include "wl_act1.h"
#include "wl_play.h"
#include "wl_state.h"
#include "wl_act2.h"

/* macro defines */

#define MOVESCALE       150l
#define BACKMOVESCALE   100l
#define ANGLESCALE      20
#define PLAYERSIZE      MINDIST    /* player radius */
#define PUSHABLETILE    98
#define ELEVATORTILE    21
#define ALTELEVATORTILE 107
#define EXITTILE        99         /* at end of castle */

/* external variables */

extern S32 viewsize;
extern U8 ingame;
extern gametype gamestate;
extern objtype *player;
extern U16 * mapsegs[MAPPLANES];
extern U32 tics;
extern S32 controlx;
extern S32 controly;
extern fixed sintable[ANGLES+ANGLES/4];
extern U8 tilemap[MAPSIZE][MAPSIZE];
extern U8 buttonheld[NUMBUTTONS];
extern U8 buttonstate[NUMBUTTONS];
extern exit_t playstate;
extern objtype *actorat[MAPSIZE][MAPSIZE];
extern U8 madenoise;
extern objtype *killerobj;
extern S16 centerx;
extern S32 shootdelta;  /* pixels away from centerx a target can be */

/* local variables */

static fixed *costable = sintable+(ANGLES/4);

static struct atkinf attackinfo[4][14] =
{
    { {6,noaction,1},{6,knifeattack,2},{6,noaction,3},       {6,swapstates,4} },
    { {6,noaction,1},{6,gunattack,2},  {6,noaction,3},       {6,swapstates,4} },
    { {6,noaction,1},{6,gunattack,2},  {6,nextattack,3},     {6,swapstates,4} },
    { {6,noaction,1},{6,gunattack,2},  {6,nextchoice,3},     {6,swapstates,4} },
};

/* global variables */

S32 facetimes = 0;
objtype *LastAttacker;
statetype s_player = {0,0,0,(statefunc) T_Player,NULL,NULL};
statetype s_attack = {0,0,0,(statefunc) T_Attack,NULL,NULL};
S32 facecount = 0;
S16 anglefrac = 0;
U16 plux;
U16 pluy;    /* player coordinates scaled to unsigned */
S32 thrustspeed;

/* local prototypes */

static void LatchNumber(S32 x, S32 y, U32 width, S32 number);
static void StatusDrawPic(U32 x, U32 y, U32 picnum);
static void StatusDrawFace(U32 picnum);
static void UpdateFace(void);
static void ControlMovement(objtype *ob);
static void ClipMove(objtype *ob, S32 xmove, S32 ymove);
static U8 TryMove(objtype *ob);
static void Cmd_Fire(void);
static void KnifeAttack(objtype *ob);
static void GunAttack(objtype *ob);
static void Cmd_Use(void);
static void HealSelf(S32 points);
static void GiveAmmo(S32 ammo);
static void GiveWeapon (weapontype weapon);
static void GiveExtraMan(void);
static void GiveKey (S32 key);
static void VictoryTile(void);
static void VictorySpin (void);

/*
================================================================
=
= Function: GivePoints
=
= Description:
=
= Update game score and give extra life if required
=
================================================================
*/
void GivePoints(S32 points)
{
    gamestate.score += points;

    while(gamestate.score >= gamestate.nextextra)
    {
        gamestate.nextextra += EXTRAPOINTS;
        GiveExtraMan();
    }

    DrawScore();
}

/*
================================================================
=
= Function: GiveKey
=
= Description:
=
= Update the amount of keys the player has
=
================================================================
*/
static void GiveKey (S32 key)
{
    gamestate.keys |= (1<<key);
    DrawKeys();
}

/*
================================================================
=
= Function: GiveExtraMan
=
= Description:
=
= Update players live
=
================================================================
*/
static void GiveExtraMan(void)
{
    if (gamestate.lives < 9)
    {
        gamestate.lives++;
    }

    DrawLives();
    SD_PlaySound(BONUS1UPSND);
}

/*
================================================================
=
= Function: GiveWeapon
=
= Description:
=
= if new weapon is better then old weapon update players weapon
=
================================================================
*/
static void GiveWeapon (weapontype weapon)
{
    GiveAmmo(6);

    if(gamestate.bestweapon < weapon)
    {
        gamestate.bestweapon = gamestate.weapon
        = gamestate.chosenweapon = (weapontype) weapon;
    }

    DrawWeapon();
}


/*
================================================================
=
= Function: GiveAmmo
=
= Description:
=
= Update players ammo and change weapon from knife if needed
=
================================================================
*/
static void GiveAmmo(S32 ammo)
{
    if(gamestate.ammo == 0)    /* knife was out */
    {
        if(gamestate.attackframe == 0)
        {
            gamestate.weapon = gamestate.chosenweapon;
            DrawWeapon();
        }
    }

    gamestate.ammo += ammo;

    if (gamestate.ammo > 99)
    {
        gamestate.ammo = 99;
    }

    DrawAmmo ();
}

/*
================================================================
=
= Function: HealSelf
=
= Description:
=
= Update players health
=
================================================================
*/
static void HealSelf(S32 points)
{
    gamestate.health += points;

    if(gamestate.health > 100)
    {
        gamestate.health = 100;
    }

    DrawHealth();
    DrawFace();
}

/*
================================================================
=
= Function: DrawHealth
=
= Description:
=
= Draw Players Health into status bar
=
================================================================
*/
void DrawHealth(void)
{
    if((viewsize == 21) && (ingame == 1))
    {
        return;
    }

    LatchNumber(21,16,3,gamestate.health);
}

/*
================================================================
=
= Function: DrawLevel
=
= Description:
=
= Draw Players level into status bar
=
================================================================
*/
void DrawLevel(void)
{
    if((viewsize == 21) && (ingame == 1))
    {
        return;
    }

    LatchNumber(2,16,2,gamestate.mapon+1);
}

/*
================================================================
=
= Function: DrawLives
=
= Description:
=
= Draw Players lives into status bar
=
================================================================
*/
void DrawLives(void)
{
    if((viewsize == 21) && (ingame == 1))
    {
        return;
    }

    LatchNumber(14,16,1,gamestate.lives);
}

/*
================================================================
=
= Function: DrawScore
=
= Description:
=
= Draw Players score into status bar
=
================================================================
*/
void DrawScore(void)
{
    if((viewsize == 21) && (ingame == 1))
    {
        return;
    }

    LatchNumber(6,16,6,gamestate.score);
}

/*
================================================================
=
= Function: DrawWeapon
=
= Description:
=
= Draw Players weapon symbol into status bar
=
================================================================
*/
void DrawWeapon(void)
{
    if((viewsize == 21) && (ingame == 1))
    {
        return;
    }

    StatusDrawPic(32,8,KNIFEPIC+gamestate.weapon);
}

/*
================================================================
=
= Function: DrawKeys
=
= Description:
=
= Draw Players key symbols into status bar
=
================================================================
*/
void DrawKeys(void)
{
    if((viewsize == 21) && (ingame == 1))
    {
        return;
    }

    if((gamestate.keys & 0x01) == 0x01)
    {
        StatusDrawPic(30,4,GOLDKEYPIC);
    }
    else
    {
        StatusDrawPic(30,4,NOKEYPIC);
    }

    if((gamestate.keys & 0x02) == 0x02)
    {
        StatusDrawPic(30,20,SILVERKEYPIC);
    }
    else
    {
        StatusDrawPic(30,20,NOKEYPIC);
    }
}

/*
================================================================
=
= Function: DrawAmmo
=
= Description:
=
= Draw Players ammo count into status bar
=
================================================================
*/
void DrawAmmo(void)
{
    if((viewsize == 21) && (ingame == 1))
    {
        return;
    }

    LatchNumber(27,16,2,gamestate.ammo);
}

/*
================================================================
=
= Function: DrawFace
=
= Description:
=
= choose which face to draw into status bar
=
================================================================
*/
void DrawFace(void)
{
    if((viewsize == 21) && (ingame ==1))
    {
        return;
    }

    if(SD_SoundPlaying() == GETGATLINGSND)
    {
        StatusDrawFace(GOTGATLINGPIC);
    }
    else if(gamestate.health != 0)
    {
        StatusDrawFace(FACE1APIC+3*((100-gamestate.health)/16)+gamestate.faceframe);
    }
    else
    {
        StatusDrawFace(FACE8APIC);
    }
}

/*
================================================================
=
= Function: SpawnPlayer
=
= Description:
=
= set default player object variables
=
================================================================
*/
void SpawnPlayer(S32 tilex, S32 tiley, S32 dir)
{
    player->obclass = playerobj;
    player->active = ac_yes;
    player->tilex = tilex;
    player->tiley = tiley;
    player->areanumber = (U8) *(mapsegs[0]+(player->tiley<<mapshift)+player->tilex);
    player->x = ((S32)tilex<<TILESHIFT)+TILEGLOBAL/2;
    player->y = ((S32)tiley<<TILESHIFT)+TILEGLOBAL/2;
    player->state = &s_player;
    player->angle = (1-dir)*90;
    if (player->angle < 0)
    {
        player->angle += ANGLES;
    }

    player->flags = FL_NEVERMARK;

    Thrust(0,0);                   /* set some variables */

    InitAreas();
}

/*
================================================================
=
= Function: StatusDrawFace
=
= Description:
=
= Draw Players face into status bar
=
================================================================
*/
static void StatusDrawFace(U32 picnum)
{
    StatusDrawPic(17, 4, picnum);
}

/*
================================================================
=
= Function: LatchNumber
=
= Description:
=
= right justifies number passed and adds leading blanks then
= prints the resultant number into the status bar
=
================================================================
*/
static void LatchNumber(S32 x, S32 y, U32 width, S32 number)
{
    U32 length;
    U32 c;
    char str[20];

    /* convert number to string */
    sprintf(str,"%d",number);

    length = (U32) strlen(str);

    /* draw leading blank spaces */
    while(length < width)
    {
        StatusDrawPic(x,y,N_BLANKPIC);
        x++;
        width--;
    }

    c = (length <= width) ? 0 : (length - width);

    /* draw number */
    while(c < length)
    {
        StatusDrawPic(x,y,((str[c]-'0') + N_0PIC));
        x++;
        c++;
    }
}

/*
================================================================
=
= Function: StatusDrawPic
=
= Description:
=
= Draw picture (picnum) into status bar at x, y
=
================================================================
*/
static void StatusDrawPic(U32 x, U32 y, U32 picnum)
{
    VWB_DrawPic((x * 8),(SCREENHEIGHT - (STATUSLINES - y)),picnum);
}

/*
================================================================
=
= Function: T_Player
=
= Description:
=
= CONTROL PLAYERS ACTIONS
=
================================================================
*/
void T_Player(objtype *ob)
{
    if (gamestate.victoryflag)    /* watching the BJ actor */
    {
        VictorySpin();
        return;
    }

    UpdateFace();

    //CheckWeaponChange();

    if(buttonstate[bt_use] == 1)
    {
        Cmd_Use();
    }

    if((buttonstate[bt_attack] == 1) && (buttonheld[bt_attack] == 0))
    {
        Cmd_Fire();
    }

    ControlMovement(ob);

    if(gamestate.victoryflag)    /* watching the BJ actor */
    {
        return;
    }

    plux = (U16) (player->x >> UNSIGNEDSHIFT);         /* scale to fit in unsigned */
    pluy = (U16) (player->y >> UNSIGNEDSHIFT);
    player->tilex = (S16)(player->x >> TILESHIFT);     /* scale to tile values */
    player->tiley = (S16)(player->y >> TILESHIFT);
}

/*
================================================================
=
= Function: UpdateFace
=
= Description:
=
= Calls draw face if time to change
=
================================================================
*/
static void UpdateFace(void)
{
    facecount += tics;

    if(facecount > US_RndT())
    {
        gamestate.faceframe = (US_RndT() >> 6);

        if(gamestate.faceframe == 3)
        {
            gamestate.faceframe = 1;
        }

        facecount = 0;
        DrawFace();
    }
}

/*
================================================================
=
= Function: GetBonus
=
= Description:
=
= Pickup bonus and remove static object from object list
=
================================================================
*/

void GetBonus(statobj_t *check)
{
    switch(check->itemnumber)
    {
        case    bo_firstaid:
            if (gamestate.health == 100)
            {
                return;
            }

            SD_PlaySound (HEALTH2SND);
            HealSelf(25);
        break;

        case    bo_key1:
        case    bo_key2:
        case    bo_key3:
        case    bo_key4:
            GiveKey(check->itemnumber - bo_key1);
            SD_PlaySound(GETKEYSND);
        break;

        case    bo_cross:
            SD_PlaySound (BONUS1SND);
            GivePoints (100);
            gamestate.treasurecount++;
        break;

        case    bo_chalice:
            SD_PlaySound (BONUS2SND);
            GivePoints (500);
            gamestate.treasurecount++;
        break;

        case    bo_bible:
            SD_PlaySound (BONUS3SND);
            GivePoints (1000);
            gamestate.treasurecount++;
        break;

        case    bo_crown:
            SD_PlaySound (BONUS4SND);
            GivePoints (5000);
            gamestate.treasurecount++;
        break;

        case    bo_clip:
            if (gamestate.ammo == 99)
            {
                return;
            }

            SD_PlaySound (GETAMMOSND);
            GiveAmmo (8);
        break;

        case    bo_clip2:
            if (gamestate.ammo == 99)
            {
                return;
            }

            SD_PlaySound (GETAMMOSND);
            GiveAmmo (4);
        break;

        case    bo_machinegun:
            SD_PlaySound (GETMACHINESND);
            GiveWeapon(wp_machinegun);
        break;

        case    bo_chaingun:
            SD_PlaySound (GETGATLINGSND);
            facetimes = 38;
            GiveWeapon (wp_chaingun);

            if(viewsize != 21)
            {
                StatusDrawFace (GOTGATLINGPIC);
            }
            facecount = 0;
        break;

        case    bo_fullheal:
            SD_PlaySound (BONUS1UPSND);
            HealSelf (99);
            GiveAmmo (25);
            GiveExtraMan();
            gamestate.treasurecount++;
        break;

        case    bo_food:
            if (gamestate.health == 100)
            {
                return;
            }

            SD_PlaySound (HEALTH1SND);
            HealSelf (10);
        break;

        case    bo_alpo:
            if (gamestate.health == 100)
            {
                return;
            }

            SD_PlaySound (HEALTH1SND);
            HealSelf (4);
        break;

        case    bo_gibs:
            if (gamestate.health >10)
            {
                return;
            }

            SD_PlaySound (SLURPIESND);
            HealSelf(1);
        break;
    }

    StartBonusFlash();
    check->shapenum = -1;    /* remove from list */
}

/*
================================================================
=
= Function: ControlMovement
=
= Description:
=
= Takes controlx,controly, and buttonstate[bt_strafe]
=
= Changes the player's angle and position
=
= There is an angle hack because when going 70 fps, the roundoff becomes
= significant
=
================================================================
*/
static void ControlMovement(objtype *ob)
{
    S32    angle;
    S32    angleunits;

    thrustspeed = 0;

    if(buttonstate[bt_strafeleft] == 1)
    {
        angle = ob->angle + ANGLES/4;
        if(angle >= ANGLES)
        {
            angle -= ANGLES;
        }

        if(buttonstate[bt_run] == 1)
        {
            Thrust(angle, RUNMOVE * MOVESCALE * tics);
        }
        else
        {
            Thrust(angle, BASEMOVE * MOVESCALE * tics);
        }
    }

    if(buttonstate[bt_straferight] == 1)
    {
        angle = ob->angle - ANGLES/4;
        if(angle < 0)
        {
            angle += ANGLES;
        }

        if(buttonstate[bt_run] == 1)
        {
            Thrust(angle, RUNMOVE * MOVESCALE * tics );
        }
        else
        {
            Thrust(angle, BASEMOVE * MOVESCALE * tics);
        }
    }

    /* not strafing */
    anglefrac += controlx;
    angleunits = anglefrac / ANGLESCALE;
    anglefrac -= angleunits * ANGLESCALE;
    ob->angle -= angleunits;

    if (ob->angle >= ANGLES)
    {
        ob->angle -= ANGLES;
    }

    if (ob->angle < 0)
    {
        ob->angle += ANGLES;
    }

    /* forward / backwards move */
    if (controly < 0)
    {
        Thrust (ob->angle,-controly*MOVESCALE);     /* move forwards */
    }
    else if (controly > 0)
    {
        angle = ob->angle + ANGLES/2;

        if (angle >= ANGLES)
        {
            angle -= ANGLES;
        }

        Thrust(angle,controly*BACKMOVESCALE);      /* move backwards */
    }

    if (gamestate.victoryflag)    /* watching the BJ actor */
    {
        return;
    }
}

/*
================================================================
=
= Function: Thrust
=
= Description:
=
=
================================================================
*/
void Thrust(S32 angle, S32 speed)
{
    S32 xmove;
    S32 ymove;
    U32 offset;

    thrustspeed += speed;

    /* moving bounds speed */
    if (speed >= MINDIST*2)
    {
        speed = MINDIST*2-1;
    }

    /* calculate new x and y positions of player*/
    xmove = FixedMul(speed,costable[angle]);
    ymove = -FixedMul(speed,sintable[angle]);

    /* check to see if new positions are valid on map*/
    ClipMove(player,xmove,ymove);

    /* update players tile location */
    player->tilex = (S16)(player->x >> TILESHIFT);
    player->tiley = (S16)(player->y >> TILESHIFT);

    /* update areanumber of player */
    offset = (player->tiley<<mapshift)+player->tilex;
    player->areanumber = *(mapsegs[0] + offset) -AREATILE;

    if(*(mapsegs[1] + offset) == EXITTILE)
    {
        VictoryTile();
    }
}

/*
================================================================
=
= Function: ClipMove
=
= Description:
=
=
================================================================
*/
static void ClipMove(objtype *ob, S32 xmove, S32 ymove)
{
    S32    basex;
    S32    basey;

    /* store current position */
    basex = ob->x;
    basey = ob->y;

    /* add new position to current one */
    ob->x = basex + xmove;
    ob->y = basey + ymove;

    /* check if move is ok */
    if(TryMove(ob) == 1)
    {
        return;
    }

    /* we hit a wall so make some noise */
    if(SD_SoundPlaying() == 0)
    {
        SD_PlaySound(HITWALLSND);
    }

    /* try moving in the x direction only */
    ob->x = basex+xmove;
    ob->y = basey;
    if(TryMove (ob))
    {
        return;
    }

    /* try moving in the y direction only */
    ob->x = basex;
    ob->y = basey+ymove;
    if (TryMove(ob))
    {
        return;
    }

    /* no luck stay in current location */
    ob->x = basex;
    ob->y = basey;
}

/*
================================================================
=
= Function: TryMove
=
= Description:
=
= returns true if move ok
=
================================================================
*/
static U8 TryMove(objtype *ob)
{
    S32    xl;
    S32    yl;
    S32    xh;
    S32    yh;
    S32    x;
    S32    y;
    objtype *check;
    S32    deltax,deltay;

    /* work out players max x y size in tile coord's*/
    xl = (ob->x-PLAYERSIZE) >>TILESHIFT;
    yl = (ob->y-PLAYERSIZE) >>TILESHIFT;

    xh = (ob->x+PLAYERSIZE) >>TILESHIFT;
    yh = (ob->y+PLAYERSIZE) >>TILESHIFT;

    /* check for solid walls */
    for(y = yl ; y <= yh; y++)
    {
        for(x = xl; x <= xh; x++)
        {
            check = actorat[x][y];

            if(check && !ISPOINTER(check))
            {
                return 0;
            }
        }
    }

    /* check for actors */
    if (yl>0)
    {
        yl--;
    }

    if (yh<MAPSIZE-1)
    {
        yh++;
    }

    if (xl>0)
    {
        xl--;
    }

    if (xh<MAPSIZE-1)
    {
        xh++;
    }

    for (y=yl;y<=yh;y++)
    {
        for (x=xl;x<=xh;x++)
        {
            check = actorat[x][y];
            if (ISPOINTER(check) && check != player && (check->flags & FL_SHOOTABLE))
            {
                deltax = ob->x - check->x;
                if (deltax < -MINACTORDIST || deltax > MINACTORDIST)
                {
                    continue;
                }

                deltay = ob->y - check->y;
                if (deltay < -MINACTORDIST || deltay > MINACTORDIST)
                {
                    continue;
                }

                return 0;
            }
        }
    }

    return 1;

}

/*
================================================================
=
= Function: Cmd_Use
=
= Description:
=
= if USE button pressed check to see if anything (like open
= door) needs to happen.
=
================================================================
*/

static void Cmd_Use(void)
{
    S32    checkx;
    S32    checky;
    U8     doornum;
//    S32    dir;
    U8 elevatorok;

    /* find which cardinal direction the player is facing */
    if ((player->angle < ANGLES/8) || (player->angle > 7*ANGLES/8))
    {
        checkx = player->tilex + 1;
        checky = player->tiley;
//        dir = di_east;
        elevatorok = 1;
    }
    else if (player->angle < 3*ANGLES/8)
    {
        checkx = player->tilex;
        checky = player->tiley-1;
//        dir = di_north;
        elevatorok = 0;
    }
    else if (player->angle < 5*ANGLES/8)
    {
        checkx = player->tilex - 1;
        checky = player->tiley;
//        dir = di_west;
        elevatorok = 1;
    }
    else
    {
        checkx = player->tilex;
        checky = player->tiley + 1;
//        dir = di_south;
        elevatorok = 0;
    }

    doornum = tilemap[checkx][checky];

    if (*(mapsegs[1]+(checky<<mapshift)+checkx) == PUSHABLETILE)
    {
        /* pushable wall */
        //PushWall(checkx,checky,dir);
        return;
    }

    if((buttonheld[bt_use] == 0) && (doornum == ELEVATORTILE) && (elevatorok == 1))
    {
        /* use elevator */
        buttonheld[bt_use] = 1;

        tilemap[checkx][checky]++;    /* flip switch */

        if (*(mapsegs[0]+(player->tiley<<mapshift)+player->tilex) == ALTELEVATORTILE)
        {
            playstate = ex_secretlevel;
        }
        else
        {
            playstate = ex_completed;
        }

        SD_PlaySound(LEVELDONESND);
        SD_WaitSoundDone();
    }
    else if((buttonheld[bt_use] == 0) && ((doornum & 0x80) == 0x80))
    {
        buttonheld[bt_use] = 1;
        OperateDoor(doornum & ~0x80);
    }
    else
    {
        SD_PlaySound(DONOTHINGSND);
    }
}

/*
================================================================
=
= Function: Cmd_Fire
=
= Description:
=
= if fire button pressed start attack sequence
=
================================================================
*/
static void Cmd_Fire(void)
{
    buttonheld[bt_attack] = 1;

    player->state = &s_attack;      /* players state is now attack state */

    gamestate.attackframe = 0;

    gamestate.attackcount =
        attackinfo[gamestate.weapon][gamestate.attackframe].tics;

    gamestate.weaponframe =
        attackinfo[gamestate.weapon][gamestate.attackframe].frame;
}

/*
================================================================
=
= Function: T_Attack
=
= Description:
=
= control players attack sequence
=
================================================================
*/
void T_Attack(objtype *ob)
{
    struct  atkinf  *cur;

    UpdateFace();

    if (gamestate.victoryflag)    /* watching the BJ actor */
    {
        VictorySpin ();
        return;
    }

    if ( buttonstate[bt_use] && !buttonheld[bt_use] )
    {
        buttonstate[bt_use] = 0;
    }

    if ( buttonstate[bt_attack] && !buttonheld[bt_attack])
    {
        buttonstate[bt_attack] = 0;
    }

    ControlMovement (ob);

    if (gamestate.victoryflag)      /* watching the BJ actor */
    {
        return;
    }

    plux = (U16) (player->x >> UNSIGNEDSHIFT);         /* scale to fit in unsigned */
    pluy = (U16) (player->y >> UNSIGNEDSHIFT);
    player->tilex = (S16)(player->x >> TILESHIFT);    /* scale to tile values */
    player->tiley = (S16)(player->y >> TILESHIFT);

    /* set inital time to stay in attack routine */
    gamestate.attackcount -= (S16) tics;

    while (gamestate.attackcount <= 0)
    {
        /* get attack state */
        cur = &attackinfo[gamestate.weapon][gamestate.attackframe];

        switch (cur->attack)
        {
            case swapstates:

                ob->state = &s_player;    /* swap back to normal state */

                if(gamestate.ammo == 0)
                {
                    gamestate.weapon = wp_knife; /* no ammo change to knife */
                    DrawWeapon ();
                }
                else
                {
                    if (gamestate.weapon != gamestate.chosenweapon)
                    {
                        gamestate.weapon = gamestate.chosenweapon;
                        DrawWeapon ();
                    }
                }

                gamestate.attackframe = 0;
                gamestate.weaponframe = 0;

            return;

            case nextchoice:

                if(gamestate.ammo == 0)
                {
                    break;  /* no ammo so swap back to normal state */
                }

                if (buttonstate[bt_attack] == 1)
                {
                    gamestate.attackframe -= 2;    /* re-start attack sequence */
                }

            case gunattack:

                if(gamestate.ammo == 0)
                {           /* can only happen with chain gun */
                    gamestate.attackframe++;
                    break;
                }

                GunAttack(ob);

                gamestate.ammo--;
                DrawAmmo();
            break;

            case knifeattack:

                KnifeAttack (ob);
            break;

            case nextattack:

                if(gamestate.ammo && buttonstate[bt_attack])
                {
                    gamestate.attackframe -= 2;
                }

            break;

        }

        gamestate.attackcount += cur->tics;     /* update exit timer */
        gamestate.attackframe++;                /* move to next attack frame / state */
        gamestate.weaponframe = attackinfo[gamestate.weapon][gamestate.attackframe].frame;
    }
}


/*
================================================================
=
= Function: GunAttack
=
= Description:
=
= controls gun attack sequence
=
================================================================
*/
static void GunAttack(objtype *ob)
{
    objtype *check;
    objtype *closest;
    objtype *oldclosest;
    S32 damage;
    S32 dx;
    S32 dy;
    S32 dist;
    S32 viewdist;

    switch(gamestate.weapon)
    {
        case wp_pistol:
            SD_PlaySound(ATKPISTOLSND);
        break;

        case wp_machinegun:
            SD_PlaySound(ATKMACHINEGUNSND);
        break;

        case wp_chaingun:
            SD_PlaySound(ATKGATLINGSND);
        break;

        default:
            /* do nothing */
        break;
    }

    madenoise = 1;

    /* find potential targets */
    viewdist = 0x7fffffffl;
    closest = NULL;

    while(1)
    {
        oldclosest = closest;

        /* find closest target */
        for (check = ob->next ; check != NULL ; check = check->next)
        {
            if ((check->flags & FL_SHOOTABLE) && (check->flags & FL_VISABLE)
                && abs(check->viewx-centerx) < shootdelta)
            {
                if (check->transx < viewdist)
                {
                    viewdist = check->transx;
                    closest = check;
                }
            }
        }

        if(closest == oldclosest)
        {
            return;    /* no more targets, all missed */
        }

        /* trace a line from player to enemey */
        if(CheckLine(closest) == 1)
        {
            break;    /* actor is close and can be seen, we have a target */
        }
    }

    /* hit something */
    dx = abs(closest->tilex - player->tilex);
    dy = abs(closest->tiley - player->tiley);
    dist = dx>dy ? dx:dy;

    if (dist<2)
    {
        damage = US_RndT() / 4;
    }
    else if (dist<4)
    {
        damage = US_RndT() / 6;
    }
    else
    {
        if ( (US_RndT() / 12) < dist)    /* missed */
        {
            return;
        }
        damage = US_RndT() / 6;
    }

    DamageActor(closest,damage);
}

/*
================================================================
=
= Function: KnifeAttack
=
= Description:
=
= Update player hands, and try to do damage when the
= proper frame is reached
=
================================================================
*/

static void KnifeAttack(objtype *ob)
{
    objtype *check;
    objtype *closest;
    S32 dist;

    SD_PlaySound(ATKKNIFESND);

    /* actually fire */
    dist = 0x7fffffff;
    closest = NULL;

    for (check=ob->next; check; check=check->next)
    {
        if ( (check->flags & FL_SHOOTABLE) && (check->flags & FL_VISABLE)
            && abs(check->viewx-centerx) < shootdelta)
        {
            if (check->transx < dist)
            {
                dist = check->transx;
                closest = check;
            }
        }
    }

    if(!closest || dist > 0x18000l)
    {
        /* missed */
        return;
    }

    /* hit something */
    DamageActor (closest,US_RndT() >> 4);
}

/*
================================================================
=
= Function: TakeDamage
=
= Description:
=
= take points of players health and store which object created
= the damage
=
================================================================
*/
void TakeDamage(S32 points,objtype *attacker)
{
    LastAttacker = attacker;

    if(gamestate.victoryflag)
    {
        return;
    }

    if(gamestate.difficulty==gd_baby)
    {
        points>>=2;
    }

    gamestate.health -= points;

    if (gamestate.health <= 0)
    {
        gamestate.health = 0;
        playstate = ex_died;
        killerobj = attacker;
    }

    StartDamageFlash(points);

    DrawHealth();
    DrawFace();
}

/*
================================================================
=
= Function: VictoryTile
=
= Description:
=
= run victory scene then exit to menu
=
================================================================
*/
static void VictoryTile(void)
{
    SpawnBJVictory();

    gamestate.victoryflag = 1;
}

/*
================================================================
=
= Function: VictorySpin
=
= Description:
=
=
=
================================================================
*/
static void VictorySpin (void)
{
    S32    desty;

    if (player->angle > 270)
    {
        player->angle -= (S16)(tics * 3);
        if (player->angle < 270)
        {
            player->angle = 270;
        }
    }
    else if (player->angle < 270)
    {
        player->angle += (S16)(tics * 3);
        if (player->angle > 270)
        {
            player->angle = 270;
        }
    }

    desty = (((S32)player->tiley-5)<<TILESHIFT)-0x3000;

    if (player->y > desty)
    {
        player->y -= tics*4096;
        if (player->y < desty)
        {
            player->y = desty;
        }
    }
}
