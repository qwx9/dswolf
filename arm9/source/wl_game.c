#include <nds.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "types.h"
#include "wl_def.h"
#include "gfxv_apo.h"
#include "audiowl6.h"
#include "wl_game.h"
#include "id_vl.h"
#include "id_vh.h"
#include "wl_menu.h"
#include "wl_agent.h"
#include "id_ca.h"
#include "wl_play.h"
#include "wl_inter.h"
#include "wl_draw.h"
#include "id_sd.h"
#include "id_in.h"
#include "wl_act1.h"
#include "wl_act2.h"


/* macro defines */

#define DEATHROTATE     2

/* external variables */

extern S32 viewheight;
extern S32 viewwidth;
extern S32 viewsize;
extern U8 loadedgame;
extern U8 startgame;
extern S32 facetimes;
extern objtype *LastAttacker;
extern objtype *killerobj;
extern S32  mapon;
extern U8 tilemap[MAPSIZE][MAPSIZE];
extern objtype *actorat[MAPSIZE][MAPSIZE];
extern U16 * mapsegs[MAPPLANES];
extern exit_t playstate;
extern CP_itemtype MainMenu[];
extern U8 screenfaded;
extern objtype *player;
extern U32 tics;
extern S32 viewwidth;
extern S32 viewheight;
extern S32 viewscreenx;
extern S32 viewscreeny;
extern S32 lastgamemusicoffset;

/* local variables */

S32 ElevatorBackTo[]={1,1,7,3,5,3};    /* ELEVATOR BACK MAPS - REMEMBER (-1)!! */

/* global variables */

char demoname[13] = "DEMO?.";
gametype gamestate;
U8 ingame = 0;

/* local prototypes */

static void ScanInfoPlane(void);
static void Died(void);

/*
================================================================
=
= Function: DrawPlayBorder
=
= Description:
=
= Draw Players Borders
=
================================================================
*/

void DrawPlayBorder(void)
{
    /* colour in status bar area only */
    VL_Bar(0, (SCREENHEIGHT - STATUSLINES), 8, STATUSLINES, VIEWCOLOR);
    VL_Bar((SCREENWIDTH - 8), (SCREENHEIGHT- STATUSLINES), 8, STATUSLINES, VIEWCOLOR);

    /* fill in the rest of the screen to a background colour  */
    VL_Bar(0,0,SCREENWIDTH,(SCREENHEIGHT - STATUSLINES),VIEWCOLOR);

    const S32 xl = (SCREENWIDTH / 2) - (viewwidth / 2);
    const S32 yl = (SCREENHEIGHT - STATUSLINES - viewheight) / 2;
    /* fill in the black player view window */
    VL_Bar(xl,yl,viewwidth,viewheight,0);

    if(xl != 0)
    {
        /* Paint game view border lines */
        VL_Bar(xl-1, yl-1, viewwidth+1, 1, 0);                       /* upper border */
        VL_Bar(xl, yl+viewheight, viewwidth+1, 1, VIEWCOLOR-2);      /* lower border */
        VL_Bar(xl-1, yl-1, 1, viewheight+1, 0);                      /* left border */
        VL_Bar(xl+viewwidth, yl-1, 1, viewheight+2, VIEWCOLOR-2);    /* right border */
        VL_Bar(xl-1, yl+viewheight, 1, 1, VIEWCOLOR-3);              /* lower left highlight */
    }
    else
    {
        /* Just paint a lower border line */
        VL_Bar(0, (yl + viewheight), viewwidth, 1, VIEWCOLOR-2);           /* lower border */
    }
}

/*
================================================================
=
= Function: GameLoop
=
= Description:
=
= Main game loop
=
================================================================
*/
void GameLoop(void)
{
    U8 died;

    ClearMemory();
    SetFontColor(0,15);
    DrawPlayScreen();
    died = 0;

    do
    {
        if((startgame == 1) || (loadedgame == 1))
        {
            SetFontColor(0,15);
            died = 0;
        }

        if(loadedgame == 0)
        {
            gamestate.score = gamestate.oldscore;
        }

        if((died == 0) || (viewsize != 21))
        {
            DrawScore();
        }

        startgame = 0;

        if(loadedgame == 0)
        {
            SetupGameLevel();
        }

        ingame = 1;
        loadedgame = 0;

        StartMusic();

        if(died == 0)
        {
            PreloadGraphics();    /* Load get psyched message */
        }
        else
        {
            died = 0;
            //fizzlein = 1;
        }

        DrawLevel();

        PlayLoop();

        StopMusic();

        ingame = 0;

        switch (playstate)
        {
            case ex_completed:
            case ex_secretlevel:

                if(viewsize == 21)
                {
                    DrawPlayScreen();
                }

                gamestate.keys = 0;
                DrawKeys();
                VW_FadeOut();

                ClearMemory();

                LevelCompleted();    /* do the intermission */

                if(viewsize == 21)
                {
                    DrawPlayScreen();
                }

                gamestate.oldscore = gamestate.score;

                /* COMING BACK FROM SECRET LEVEL */
                if (gamestate.mapon == 9)
                {
                    gamestate.mapon = ElevatorBackTo[gamestate.episode];    /* back from secret */
                }
                else
                {
                    /* GOING TO SECRET LEVEL */
                    if (playstate == ex_secretlevel)
                    {
                        gamestate.mapon = 9;
                    }
                    else
                    {
                        /* GOING TO NEXT LEVEL */
                        gamestate.mapon++;
                    }
                }
            break;

            case ex_died:
                Died();
                died = 1;           /* don't "get psyched!" */

                if(gamestate.lives > -1)
                {
                    break;          /* more lives left */
                }

                VW_FadeOut();

                ClearMemory ();

                CheckHighScore(gamestate.score,gamestate.mapon + 1);

                strcpy(MainMenu[viewscores].string,STR_VS);

                MainMenu[viewscores].pt2function = CP_ViewScores;

                MainMenu[savegame].active = 0;

                return;

            case ex_victorious:

                if(viewsize == 21)
                {
                    DrawPlayScreen();
                }

                VW_FadeOut();

                ClearMemory();

                Victory();

                ClearMemory();

                CheckHighScore (gamestate.score,gamestate.mapon+1);

                strcpy(MainMenu[viewscores].string,STR_VS);

                MainMenu[viewscores].pt2function = CP_ViewScores;

                MainMenu[savegame].active = 0;

                return;

            default:
                if(viewsize == 21)
                {
                    DrawPlayScreen();
                }
                ClearMemory ();
            break;
        }

    }while(1);
}

/*
================================================================
=
= Function: SetupGameLevel
=
= Description:
=
= cache level data and setup level variables
=
================================================================
*/
void SetupGameLevel(void)
{
    S32  x,y;
    U16 *map;
    U16 tile;

    if(loadedgame == 0)
    {
        gamestate.TimeCount     = 0;
        gamestate.secrettotal   = 0;
        gamestate.killtotal     = 0;
        gamestate.treasuretotal = 0;
        gamestate.secretcount   = 0;
        gamestate.killcount     = 0;
        gamestate.treasurecount = 0;
        //pwallstate              = 0;
        //pwallpos                = 0;
        facetimes               = 0;

        LastAttacker = NULL;
        killerobj = NULL;
    }

    /* load the level */
    CA_CacheMap(gamestate.mapon + 10 * gamestate.episode);
    mapon-=gamestate.episode*10;

    /* copy the wall data to a data segment array */
    memset(tilemap,0,sizeof(tilemap));
    memset(actorat,0,sizeof(actorat));
    map = mapsegs[0];

    for(y=0;y<MAPHEIGHT;y++)
    {
        for(x=0;x<MAPWIDTH;x++)
        {
            tile = *map++;
            if (tile<AREATILE)
            {
                /* solid wall */
                tilemap[x][y] = (U8) tile;
                actorat[x][y] = (objtype *)(uintptr_t) tile;
            }
            else
            {
                /* area floor */
                tilemap[x][y] = 0;
                actorat[x][y] = 0;
            }
        }
    }

    /* start spawning things with a clean slate */
    InitActorList();
    InitDoorList();
    InitStaticList();

    /* spawn doors */
    map = mapsegs[0];
    for(y=0;y<MAPHEIGHT;y++)
    {
        for(x=0;x<MAPWIDTH;x++)
        {
            tile = *map++;
            if (tile >= 90 && tile <= 101)
            {
                /* door */
                switch (tile)
                {
                    case 90:
                    case 92:
                    case 94:
                    case 96:
                    case 98:
                    case 100:
                        SpawnDoor(x,y,1,(tile-90)/2);
                        break;
                    case 91:
                    case 93:
                    case 95:
                    case 97:
                    case 99:
                    case 101:
                        SpawnDoor(x,y,0,(tile-91)/2);
                        break;
                }
            }
        }
    }

    /* spawn actors */
    ScanInfoPlane();

    /* take out the ambush markers */
    map = mapsegs[0];
    for (y=0;y<MAPHEIGHT;y++)
    {
        for (x=0;x<MAPWIDTH;x++)
        {
            tile = *map++;
            if (tile == AMBUSHTILE)
            {
                tilemap[x][y] = 0;

                if ( (unsigned)(uintptr_t)actorat[x][y] == AMBUSHTILE)
                {
                    actorat[x][y] = NULL;
                }

                if (*map >= AREATILE)
                {
                    tile = *map;
                }

                if (*(map-1-MAPWIDTH) >= AREATILE)
                {
                    tile = *(map-1-MAPWIDTH);
                }

                if (*(map-1+MAPWIDTH) >= AREATILE)
                {
                    tile = *(map-1+MAPWIDTH);
                }

                if ( *(map-2) >= AREATILE)
                {
                    tile = *(map-2);
                }

                *(map-1) = tile;
            }
        }
    }

    /* have the caching manager load and purge stuff to make */
    /* sure all marks are in memory */
    CA_LoadAllSounds();

}

/*
================================================================
=
= Function: ScanInfoPlane
=
= Description:
=
= Spawn all actors and mark down special places
=
================================================================
*/
static void ScanInfoPlane(void)
{
    U32     x,y;
    S32     tile;
    U16    *start;

    start = mapsegs[1];

    for (y=0;y<MAPHEIGHT;y++)
    {
        for (x=0;x<MAPWIDTH;x++)
        {
            tile = *start++;

            if(tile == 0)
            {
                continue;
            }

            switch(tile)
            {
                case 19:
                case 20:
                case 21:
                case 22:
                    SpawnPlayer(x,y,NORTH+tile-19);
                break;

                case 23:
                case 24:
                case 25:
                case 26:
                case 27:
                case 28:
                case 29:
                case 30:

                case 31:
                case 32:
                case 33:
                case 34:
                case 35:
                case 36:
                case 37:
                case 38:

                case 39:
                case 40:
                case 41:
                case 42:
                case 43:
                case 44:
                case 45:
                case 46:

                case 47:
                case 48:
                case 49:
                case 50:
                case 51:
                case 52:
                case 53:
                case 54:

                case 55:
                case 56:
                case 57:
                case 58:
                case 59:
                case 60:
                case 61:
                case 62:

                case 63:
                case 64:
                case 65:
                case 66:
                case 67:
                case 68:
                case 69:
                case 70:
                case 71:
                case 72:
                    SpawnStatic(x,y,tile-23);
                break;

                /* P wall */
                case 98:
                    if (loadedgame == 0)
                    {
                        gamestate.secrettotal++;
                    }
                break;

                /* guard */
                case 180:
                case 181:
                case 182:
                case 183:
                    if(gamestate.difficulty < gd_hard)
                    {
                        break;
                    }
                    tile -= 36;
                case 144:
                case 145:
                case 146:
                case 147:
                    if (gamestate.difficulty<gd_medium)
                    {
                        break;
                    }
                    tile -= 36;
                case 108:
                case 109:
                case 110:
                case 111:
                    SpawnStand(en_guard,x,y,tile-108);
                break;


                case 184:
                case 185:
                case 186:
                case 187:
                    if (gamestate.difficulty<gd_hard)
                    {
                        break;
                    }
                    tile -= 36;
                case 148:
                case 149:
                case 150:
                case 151:
                    if (gamestate.difficulty<gd_medium)
                    {
                        break;
                    }
                    tile -= 36;
                case 112:
                case 113:
                case 114:
                case 115:
                    SpawnPatrol(en_guard,x,y,tile-112);
                break;

                case 124:
                    SpawnDeadGuard (x,y);
                break;

                /* officer */
                case 188:
                case 189:
                case 190:
                case 191:
                    if (gamestate.difficulty<gd_hard)
                    {
                        break;
                    }
                    tile -= 36;
                case 152:
                case 153:
                case 154:
                case 155:
                    if (gamestate.difficulty<gd_medium)
                    {
                        break;
                    }
                    tile -= 36;
                case 116:
                case 117:
                case 118:
                case 119:
                    SpawnStand(en_officer,x,y,tile-116);
                break;

                case 192:
                case 193:
                case 194:
                case 195:
                    if (gamestate.difficulty<gd_hard)
                    {
                        break;
                    }
                    tile -= 36;
                case 156:
                case 157:
                case 158:
                case 159:
                    if (gamestate.difficulty<gd_medium)
                    {
                        break;
                    }
                    tile -= 36;
                case 120:
                case 121:
                case 122:
                case 123:
                    SpawnPatrol(en_officer,x,y,tile-120);
                break;

                /* ss */
                case 198:
                case 199:
                case 200:
                case 201:
                    if (gamestate.difficulty<gd_hard)
                    {
                        break;
                    }
                    tile -= 36;
                case 162:
                case 163:
                case 164:
                case 165:
                    if (gamestate.difficulty<gd_medium)
                    {
                        break;
                    }
                    tile -= 36;
                case 126:
                case 127:
                case 128:
                case 129:
                    SpawnStand(en_ss,x,y,tile-126);
                break;

                case 202:
                case 203:
                case 204:
                case 205:
                    if (gamestate.difficulty<gd_hard)
                    {
                        break;
                    }
                    tile -= 36;
                case 166:
                case 167:
                case 168:
                case 169:
                    if (gamestate.difficulty<gd_medium)
                    {
                        break;
                    }
                    tile -= 36;
                case 130:
                case 131:
                case 132:
                case 133:
                    SpawnPatrol(en_ss,x,y,tile-130);
                break;

                /* dogs */
                case 206:
                case 207:
                case 208:
                case 209:
                    if (gamestate.difficulty<gd_hard)
                    {
                        break;
                    }
                    tile -= 36;
                case 170:
                case 171:
                case 172:
                case 173:
                    if (gamestate.difficulty<gd_medium)
                    {
                        break;
                    }
                    tile -= 36;
                case 134:
                case 135:
                case 136:
                case 137:
                    SpawnStand(en_dog,x,y,tile-134);
                break;

                case 210:
                case 211:
                case 212:
                case 213:
                    if (gamestate.difficulty<gd_hard)
                    {
                        break;
                    }
                    tile -= 36;
                case 174:
                case 175:
                case 176:
                case 177:
                    if (gamestate.difficulty<gd_medium)
                    {
                        break;
                    }
                    tile -= 36;
                case 138:
                case 139:
                case 140:
                case 141:
                    SpawnPatrol(en_dog,x,y,tile-138);
                break;

                /* boss */
                case 214:
                    SpawnBoss(x,y);
                break;

                case 197:
                    SpawnGretel(x,y);
                break;

                case 215:
                    SpawnGift(x,y);
                break;

                case 179:
                    SpawnFat(x,y);
                break;

                case 196:
                    SpawnSchabbs(x,y);
                break;

                case 160:
                    SpawnFakeHitler(x,y);
                break;

                case 178:
                    SpawnHitler(x,y);
                break;

                /* mutants */
                case 252:
                case 253:
                case 254:
                case 255:
                    if (gamestate.difficulty<gd_hard)
                    {
                        break;
                    }
                    tile -= 18;
                case 234:
                case 235:
                case 236:
                case 237:
                    if (gamestate.difficulty<gd_medium)
                    {
                        break;
                    }
                    tile -= 18;
                case 216:
                case 217:
                case 218:
                case 219:
                    SpawnStand(en_mutant,x,y,tile-216);
                break;

                case 256:
                case 257:
                case 258:
                case 259:
                    if (gamestate.difficulty<gd_hard)
                    {
                        break;
                    }
                    tile -= 18;
                case 238:
                case 239:
                case 240:
                case 241:
                    if (gamestate.difficulty<gd_medium)
                    {
                        break;
                    }
                    tile -= 18;
                case 220:
                case 221:
                case 222:
                case 223:
                    SpawnPatrol(en_mutant,x,y,tile-220);
                break;

                /* ghosts */
                case 224:
                    SpawnGhosts (en_blinky,x,y);
                break;

                case 225:
                    SpawnGhosts (en_clyde,x,y);
                break;

                case 226:
                    SpawnGhosts (en_pinky,x,y);
                break;

                case 227:
                    SpawnGhosts (en_inky,x,y);
                break;

                default:
                    /* do nothing */
                break;
            }
        }
    }
}

/*
================================================================
=
= Function: DrawPlayScreen
=
= Description:
=
= Draw players screen (ie status bar with score etc)
=
================================================================
*/
void DrawPlayScreen(void)
{
    VWB_DrawPic(0,(SCREENHEIGHT-STATUSLINES),STATUSBARPIC);
    DrawPlayBorder();

    DrawFace();
    DrawHealth();
    DrawLives();
    DrawLevel();
    DrawAmmo();
    DrawKeys();
    DrawWeapon();
    DrawScore();
}

/*
================================================================
=
= Function: Died
=
= Description:
=
= handle action when player dies
=
================================================================
*/
static void Died(void)
{
    S32 iangle = 0;
    S32 curangle;
    S32 clockwise;
    S32 counter;
    S32 change;
    s32 dx;
    s32 dy;
    float    fangle;

    if(screenfaded == 1)
    {
        ThreeDRefresh();
        VW_FadeIn();
    }

    gamestate.weapon = wp_none; /* take away weapon */
    SD_PlaySound(PLAYERDEATHSND);

    /* swing around to face attacker */
    if(killerobj != NULL)
    {
        dx = killerobj->x - player->x;
        dy = player->y - killerobj->y;

        fangle = (float) atan2((float) dy, (float) dx);    /* returns -pi to pi */

        if (fangle<0)
        {
            fangle = (float) (M_PI*2+fangle);
        }

        iangle = (s32) (fangle/(M_PI*2)*ANGLES);
    }
    else
    {
        /* add 180 to players angle */
        iangle = player->angle + ANGLES / 2;

        /* if over 360 minus 360 */
        if(iangle >= ANGLES)
        {
            iangle -= ANGLES;
        }
    }

    /* work out counter and clockwise distance from current angle to new angle */
    if (player->angle > iangle)
    {
        counter = player->angle - iangle;
        clockwise = ANGLES-player->angle + iangle;
    }
    else
    {
        clockwise = iangle - player->angle;
        counter = player->angle + ANGLES-iangle;
    }

    curangle = player->angle;

    /* is turning clockwise closer to new position */
    if(clockwise<counter)
    {
        /* yes rotate clockwise */
        if(curangle > iangle)
        {
            curangle -= ANGLES;
        }

        do
        {
            change = tics*DEATHROTATE;

            if(curangle + change > iangle)
            {
                change = iangle-curangle;
            }

            curangle += change;
            player->angle += change;

            if (player->angle >= ANGLES)
            {
                player->angle -= ANGLES;
            }

            ThreeDRefresh();
            CalcTics();

        } while(curangle != iangle);
    }
    else
    {
        /* rotate counterclockwise */
        if (curangle<iangle)
        {
            curangle += ANGLES;
        }

        do
        {
            change = -(S32)tics*DEATHROTATE;

            if (curangle + change < iangle)
            {
                change = iangle-curangle;
            }

            curangle += change;
            player->angle += change;

            if (player->angle < 0)
            {
                player->angle += ANGLES;
            }

            ThreeDRefresh();
            CalcTics();

        } while(curangle != iangle);
    }

    FinishPaletteShifts();

    /* fade to red */
    FizzleFade(viewscreenx,viewscreeny,viewwidth,viewheight,70);

    IN_UserInput(100);

    SD_WaitSoundDone();

    gamestate.lives--;

    if (gamestate.lives > -1)
    {
        gamestate.health = 100;
        gamestate.weapon = gamestate.bestweapon
            = gamestate.chosenweapon = wp_pistol;
        gamestate.ammo = STARTAMMO;
        gamestate.keys = 0;
        //pwallstate = pwallpos = 0;
        gamestate.attackframe = gamestate.attackcount =
            gamestate.weaponframe = 0;

        if(viewsize != 21)
        {
            DrawKeys ();
            DrawWeapon ();
            DrawAmmo ();
            DrawHealth ();
            DrawFace ();
            DrawLives ();
        }
    }
}
