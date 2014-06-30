#include <fat.h>
#include <nds.h>
#include <stdio.h>
#include <stdlib.h>
#include "types.h"
#include "audiowl6.h"
#include "gfxv_apo.h"
#include "wl_def.h"
#include "id_time.h"
#include "id_us.h"
#include "id_sd.h"
#include "wl_menu.h"
#include "id_in.h"
#include "id_Ca.h"
#include "id_vh.h"
#include "id_vl.h"
#include "wl_inter.h"
#include "wl_game.h"
#include "main.h"
#include "id_pm.h"
#include "wl_agent.h"
#include "wl_play.h"

/* macro defines */

#define VIEWGLOBAL      0x10000     /* globals visable flush to wall */
#define STR_SAVECHT1    "Your Save Game file is,"
#define STR_SAVECHT2    "shall we say, \"corrupted\"."
#define STR_SAVECHT3    "But I'll let you go on and"
#define STR_SAVECHT4    "play anyway...."

/* external variables */

extern U16 WindowX;
extern U16 WindowY;
extern U16 WindowW;
extern U16 PrintY;
extern S32 numEpisodesMissing;
extern S32 viewsize;
extern gametype gamestate;
extern U16 horizwall[MAXWALLTILES];
extern U16 vertwall[MAXWALLTILES];
extern S16 pixelangle320[320];
extern S16 pixelangle304[304];
extern S16 pixelangle288[288];
extern S16 pixelangle272[272];
extern S16 pixelangle256[256];
extern S16 pixelangle240[240];
extern S16 pixelangle224[224];
extern S16 pixelangle208[208];
extern S16 pixelangle192[192];
extern S16 pixelangle176[176];
extern S16 pixelangle160[160];
extern S16 pixelangle144[144];
extern S16 pixelangle128[128];
extern S16 pixelangle112[112];
extern S16 pixelangle96[96];
extern S16 pixelangle80[80];
extern S16 pixelangle64[64];
extern S16 DigiMap[LASTSOUND];
extern S32 lastgamemusicoffset;
extern doorobj_t doorobjlist[MAXDOORS];
extern U16 doorposition[MAXDOORS];
extern U8 spotvis[MAPSIZE][MAPSIZE];
extern statobj_t *laststatobj;
extern statobj_t statobjlist[MAXSTATS];
extern statetype s_player;
extern statetype s_grdstand;
extern objtype *player;
extern U8 areabyplayer[NUMAREAS];
extern U8 areaconnect[NUMAREAS][NUMAREAS];
extern objtype objlist[MAXACTORS];
extern objtype *actorat[MAPSIZE][MAPSIZE];
extern U8 tilemap[MAPSIZE][MAPSIZE];
extern LRstruct LevelRatios[LRpack];
extern objtype *newobj;

/* local variables */

static CP_iteminfo MusicItems = {CTL_X,CTL_Y,6,0,32};
static CP_itemtype MusicMenu[] =
{
    {1,"Get Them!",NULL},
    {1,"Searching",NULL},
    {1,"P.O.W.",NULL},
    {1,"Suspense",NULL},
    {1,"War March",NULL},
    {1,"Around The Corner!",NULL},

    {1,"Nazi Anthem",NULL},
    {1,"Lurking...",NULL},
    {1,"Going After Hitler",NULL},
    {1,"Pounding Headache",NULL},
    {1,"Into the Dungeons",NULL},
    {1,"Ultimate Conquest",NULL},

    {1,"Kill the S.O.B.",NULL},
    {1,"The Nazi Rap",NULL},
    {1,"Twelfth Hour",NULL},
    {1,"Zero Hour",NULL},
    {1,"Ultimate Conquest",NULL},
    {1,"Wolfpack",NULL}
};

/* InitDigiMap channel mapping */
static S16 wolfdigimap[] =
{
    /* These first sounds are in the upload version */
    HALTSND,                0,
    DOGBARKSND,             1,
    CLOSEDOORSND,           2,
    OPENDOORSND,            3,
    ATKMACHINEGUNSND,       4,
    ATKPISTOLSND,           5,
    ATKGATLINGSND,          6,
    SCHUTZADSND,            7,
    GUTENTAGSND,            8,
    MUTTISND,               9,
    BOSSFIRESND,            10,
    SSFIRESND,              11,
    DEATHSCREAM1SND,        12,
    DEATHSCREAM2SND,        13,
    DEATHSCREAM3SND,        13,
    TAKEDAMAGESND,          14,
    PUSHWALLSND,            15,

    LEBENSND,               20,
    NAZIFIRESND,            21,
    SLURPIESND,             22,

    YEAHSND,                32,
    
#ifndef SHAREWARE
    /* These are in all other episodes */
    DOGDEATHSND,            16,
    AHHHGSND,               17,
    DIESND,                 18,
    EVASND,                 19,

    TOT_HUNDSND,            23,
    MEINGOTTSND,            24,
    SCHABBSHASND,           25,
    HITLERHASND,            26,
    SPIONSND,               27,
    NEINSOVASSND,           28,
    DOGATTACKSND,           29,
    LEVELDONESND,           30,
    MECHSTEPSND,            31,

    SCHEISTSND,             33,
    DEATHSCREAM4SND,        34,         /* AIIEEE                   */
    DEATHSCREAM5SND,        35,         /* DEE-DEE                  */
    DONNERSND,              36,         /* EPISODE 4 BOSS DIE       */
    EINESND,                37,         /* EPISODE 4 BOSS SIGHTING  */
    ERLAUBENSND,            38,         /* EPISODE 6 BOSS SIGHTING  */
    DEATHSCREAM6SND,        39,         /* FART                     */
    DEATHSCREAM7SND,        40,         /* GASP                     */
    DEATHSCREAM8SND,        41,         /* GUH-BOY!                 */
    DEATHSCREAM9SND,        42,         /* AH GEEZ!                 */
    KEINSND,                43,         /* EPISODE 5 BOSS SIGHTING  */
    MEINSND,                44,         /* EPISODE 6 BOSS DIE       */
    ROSESND,                45,         /* EPISODE 5 BOSS DIE       */
#endif
    LASTSOUND
};

/* global variables */

S32 viewwidth;
S32 viewheight;
char configname[13] = "config.";    /* set default config file name to config */
char demoname[13];
U8 loadedgame = 0;
U8 startgame = 0;
U32 screenofs;
S32 heightnumerator;
S32 scale;
S32 focallength;
S16 *pixelangle = NULL;
char *DS_USERNAME = NULL;
S32 viewscreenx;
S32 viewscreeny;
S16 centerx;
S32 shootdelta;  /* pixels away from centerx a target can be */

/* local prototypes */

static void InitGame(void);
static void ShutdownId(void);
static void DoJukebox(void);
static void FinishSignon(void);
static void DemoLoop(void);
static U8 SetViewSize(U16 width, U16 height);
static void SetupWalls(void);
static void CalcProjection(S32 focal, U16 width);
static void DSgetUserName(void);
static void DiskFlopAnim(S32 x,S32 y);
static S32 DoChecksum(const U8 *source, U32 size,S32 checksum);

/* 
================================================================
=
= Function: main
=
= Description:
=
= Program entry / exit point
=
================================================================ 
*/
S32 main(void) 
{
    
    powerOn(POWER_ALL);
    soundEnable();
    
    if(fatInitDefault() == 0)
    {
        printf("Unable to initialize media device!\n");
    }
    else
    {
        printf("fatInitDefault(): initialized.\n");
    }
    
    /* essential, retrieves username via Fifo buffer. */
    DSgetUserName();	
    
    InitTimer();
    
    VL_SetVGAPlaneMode();
    
    CheckForEpisodes();
    
    InitGame();
    
    DemoLoop();
    
    ShutdownId();

    return 0;
}

/* 
================================================================
=
= Function: InitDigiMap
=
= Description:
=
= load digital sounds
=
================================================================ 
*/
static void InitDigiMap(void)
{
    S16 *map;

    for (map = wolfdigimap; *map != LASTSOUND; map += 2)
    {
        DigiMap[map[0]] = map[1];
        SD_PrepareSound(map[1]);
    }
}

/* 
================================================================
=
= Function: InitGame
=
= Description:
=
= Load a few things right away
=
================================================================ 
*/
static void InitGame(void)
{
    U8 didjukebox = 0;
    ControlInfo ci = {0, 0, 0, 0, dir_None, 0, 0, 0, 0};
    
    SignonScreen();
    
    UpdateScreen(1);
    
    PM_Startup();
    SD_Startup();
    CA_Startup();
    US_InitRndT(1);
    VL_Startup();
    
    InitDigiMap(); /* build sound tables */
    
    SetupSaveGames();
    
    /* GET USER INPUT */
    IN_ReadControl(&ci);
    
    if(ci.dir == dir_North)
    {
        iprintf("\n\n\tWolf JukeBox\n");
        iprintf("\n\n\tported to NDS by happy bunny\n");
        iprintf("\taka Steven Taffs\n");
        iprintf("\n\n\tbased on code by ID\n");
        iprintf("\tand Moritz \"Ripper\" Kroll\n");
        DoJukebox();
        didjukebox = 1;
    }
    else
    {
        iprintf("\n\n\tWolfenstein 3D\n");
        iprintf("\n\n\tported to NDS by happy bunny\n");
        iprintf("\taka Steven Taffs\n");
        iprintf("\n\n\tbased on code by ID\n");
        iprintf("\tand Moritz \"Ripper\" Kroll\n");
        IntroScreen();
    }
    
    /* load in and lock down some basic chunks */
    CA_CacheGrChunk(STATUSBARPIC);
    CA_CacheGrChunk(STARTFONT);
    SetFontNum(0);
    
    /* cache head up display pic's */
    CacheLump(LATCHPICS_LUMP_START,LATCHPICS_LUMP_END);
    
    /* map tile values to scaled wall pics*/
    SetupWalls();
    
    NewViewSize(viewsize);
    
    InitRedShifts();
    
    if(didjukebox == 0)
    {
        FinishSignon();
    }
}

/* 
================================================================
=
= Function: ShutdownId
=
= Description:
=
= Returns malloced memory to heap and closes file handles
=
================================================================ 
*/
static void ShutdownId(void)
{
    SD_Shutdown();
    PM_Shutdown();
    VL_Shutdown();
    CA_Shutdown();
}

/* 
================================================================
=
= Function: DoJukebox
=
= Description:
=
= Runs wolfenstain music juke box menu
=
================================================================ 
*/
static void DoJukebox(void)
{
    S32 which;
    S32 lastsong = -1;
    U32 start;
    U32 songs[] =
    {
        GETTHEM_MUS,
        SEARCHN_MUS,
        POW_MUS,
        SUSPENSE_MUS,
        WARMARCH_MUS,
        CORNER_MUS,

        NAZI_OMI_MUS,
        PREGNANT_MUS,
        GOINGAFT_MUS,
        HEADACHE_MUS,
        DUNGEON_MUS,
        ULTIMATE_MUS,

        INTROCW3_MUS,
        NAZI_RAP_MUS,
        TWELFTH_MUS,
        ZEROHOUR_MUS,
        ULTIMATE_MUS,
        PACMAN_MUS
    };
    
    MenuFadeOut();
    
    /* check for shareware version */
    if(numEpisodesMissing == 5)
    {
        start = 0;      /* shareware only supports first menu only  */
    }
    else
    {
        start = ((GetTimeMS()/1000)%3)*6; /* randomly pick a menu */
    }
    
    CA_CacheGrChunk(STARTFONT+1);
    
    CacheLump(CONTROLS_LUMP_START,CONTROLS_LUMP_END);
    
    CA_LoadAllSounds();
    
    SetFontNum(1);
    ClearMScreen();
    VWB_DrawPic(112,184,C_MOUSELBACKPIC);
    DrawStripes(10);
    SetFontColor(TEXTCOLOR,BKGDCOLOR);
    
    DrawWindow(CTL_X-2,CTL_Y-6,280,13*7,BKGDCOLOR);

    DrawMenu(&MusicItems,&MusicMenu[start]);
    
    SetFontColor(READHCOLOR,BKGDCOLOR);
    PrintY = 15;
    WindowX = 0;
    WindowY = 320;
    US_CPrint("Robert's Jukebox");

    SetFontColor(TEXTCOLOR,BKGDCOLOR);
    UpdateScreen(0);

    MenuFadeIn();
    
    do
    {
        which = HandleMenu(&MusicItems,&MusicMenu[start],NULL);
        
        if (which >= 0)
        {
            if (lastsong >= 0)
            {
                MusicMenu[start+lastsong].active = 1;
            }

            StartCPMusic(songs[start + which]);
            MusicMenu[start+which].active = 2;
            DrawMenu(&MusicItems,&MusicMenu[start]);
            UpdateScreen(0);
            lastsong = which;
        }
        
    }while(which >= 0);
    
    printf("\n\n\texiting wolf jukebox !\n");
    
    UnCacheLump(CONTROLS_LUMP_START,CONTROLS_LUMP_END);
}

/* 
================================================================
=
= Function: FinishSignon
=
= Description:
=
= waits for user to read signon screen and press a button
= to continue
=
================================================================ 
*/

static void FinishSignon(void)
{

    VL_Bar(0,189,300,11,VL_GetPixel(0,0));
    WindowX = 0;
    WindowW = 320;
    PrintY = 190;

    SetFontColor(14,4);
    
    US_CPrint("Press a key");
    
    UpdateScreen(0);
    
    IN_Ack();
    
    VL_Bar(0,189,300,11,VL_GetPixel(0,0));
    
    PrintY = 190;
    SetFontColor(10,4);
    
    US_CPrint("Working...");
    
    UpdateScreen(0);
    
    SetFontColor(0,15);
    
    UpdateScreen(0);
    
    Delay_ms(3*70);
}

/* 
================================================================
=
= Function: DemoLoop
=
= Description:
=
= starts a demo loop and waits for user to press a key
=
================================================================ 
*/
static void DemoLoop(void)
{

    StartCPMusic(INTROSONG);

    PG13();
    
    while(1)
    {
        while(1)
        {
    
            /* title page */
            CA_CacheScreen(TITLEPIC);
            UpdateScreen(0);
            VW_FadeIn();
    
            if(IN_UserInput(TickBase*15) == 1)
            {
                break;
            }
    
            VW_FadeOut();

            /* credits page */

            CA_CacheScreen(CREDITSPIC);
            UpdateScreen(0);
            VW_FadeIn();
            if (IN_UserInput(TickBase*10) == 1)
            {
                break;
            }
    
            VW_FadeOut();
        
            /* high scores */
            DrawHighScores();
            UpdateScreen(0);
            VW_FadeIn();

            if (IN_UserInput(TickBase*10) == 1)
            {
                break;
            }
        
            VW_FadeOut();
        }
    
        VW_FadeOut();
    
        US_ControlPanel(0);
        
        if((startgame == 1) || (loadedgame == 1))
        {
            GameLoop();
            VW_FadeOut();
            StartCPMusic(INTROSONG);
        }
    }
}

/* 
================================================================
=
= Function: DSgetUserName
=
= Description:
=
= retrieve profile username
=
================================================================ 
*/
static void DSgetUserName(void)
{
	S32 i;
	S32 nameLen = PersonalData->nameLen;
	DS_USERNAME = malloc(nameLen + 1);
    memset(DS_USERNAME,0,(nameLen + 1));
 
	/* safety fail */
	if(nameLen <= 0)
	{
		DS_USERNAME = "Player1";
		return;
	}

	for(i=0; i < nameLen; i++)
	{
		/* pretend to get ascii-bits from utf-16 name */
		DS_USERNAME[i] = (char)PersonalData->name[i] & 255;
	}
    
	/* zero terminate the string */
	DS_USERNAME[i] = 0;
}

/* 
================================================================
=
= Function: ShowViewSize
=
= Description:
=
= show new view window size 
=
================================================================ 
*/
void ShowViewSize(S32 width)
{
    S32 oldwidth;
    S32 oldheight;

    oldwidth = viewwidth;
    oldheight = viewheight;

    if(width == 21)
    {
        viewwidth = SCREENWIDTH;
        viewheight = SCREENHEIGHT;
        VL_Bar(0, 0, SCREENWIDTH, SCREENHEIGHT, 0);
    }
    else if(width == 20)
    {
        viewwidth = SCREENWIDTH;
        viewheight = SCREENHEIGHT - STATUSLINES;
        DrawPlayBorder();
    }
    else
    {
        viewwidth = width*16;
        viewheight = (width*16)/2;
        DrawPlayBorder();
    }

    viewwidth = oldwidth;
    viewheight = oldheight;
}

/* 
================================================================
=
= Function: NewViewSize
=
= Description:
=
= works out new view size and then calls setviewsize
=
================================================================ 
*/
void NewViewSize (S32 width)
{
    viewsize = width;
    
    if(viewsize == 21)
    {
        SetViewSize(SCREENWIDTH, SCREENHEIGHT);
    }
    else if(viewsize == 20)
    {
        SetViewSize(SCREENWIDTH, (SCREENHEIGHT - STATUSLINES));
    }
    else
    {
        SetViewSize(width*16, ((width*16)/2));
    }
}

/* 
================================================================
=
= Function: SetViewSize
=
= Description:
=
= set the new view screen size
=
================================================================ 
*/
static U8 SetViewSize(U16 width, U16 height)
{
    viewwidth = (S32)(width & 0xfff0);           /* must be divisable by 16 */
    viewheight = (S32)(height& 0xfffE);          /* must be even */
    
    centerx = viewwidth / 2-1;
    shootdelta = viewwidth / 10;
    
    if(viewheight == SCREENHEIGHT)
    {
        viewscreenx = 0;
        viewscreeny = 0;
        screenofs = 0;
    }
    else
    {
        viewscreenx = (SCREENWIDTH - viewwidth) / 2;
        viewscreeny = (SCREENHEIGHT - STATUSLINES - viewheight) / 2;
        screenofs = viewscreeny*SCREENWIDTH + viewscreenx;
    }

    /* calculate trace angles and projection constants */
    CalcProjection(FOCALLENGTH,width);

    return 1;
}

/* 
================================================================
=
= Function: NewGame
=
= Description:
=
= Set up new game to start from the beginning
=
================================================================ 
*/
void NewGame(S32 difficulty,S32 episode)
{
    memset (&gamestate,0,sizeof(gamestate));
    gamestate.difficulty = difficulty;
    gamestate.weapon = wp_pistol;
    gamestate.bestweapon = wp_pistol;
    gamestate.chosenweapon = wp_pistol;
    gamestate.health = 100;
    gamestate.ammo = STARTAMMO;
    gamestate.lives = 3;
    gamestate.nextextra = EXTRAPOINTS;
    gamestate.episode=episode;

    startgame = 1;
}

/* 
================================================================
=
= Function: SetupWalls
=
= Description:
=
= Map tile values to scaled pics
=
================================================================ 
*/
static void SetupWalls(void)
{
    S32 i;

    horizwall[0] = 0;
    vertwall[0] = 0;

    for(i = 1; i < MAXWALLTILES; i++)
    {
        horizwall[i] = (i-1)*2;
        vertwall[i] = (i-1)*2+1;
    }
}

/* 
================================================================
=
= Function: CalcProjection
=
= Description:
=
= re-calculate projection plane using the new view width
=
================================================================ 
*/
static void CalcProjection(S32 focal, U16 width)
{
    S32 halfview;
    S32 facedist;

    halfview = viewwidth / 2; 
    focallength = focal;
    facedist = focal + MINDIST;   /* 0x5700l + 0x5800l */

    /* calculate scale value for vertical height calculations */
    /* and sprite x calculations                              */
    scale = (halfview * facedist / (VIEWGLOBAL/2));

    /* divide heightnumerator by a posts distance to get the posts */
    /* height for the heightbuffer.  The pixel height is height>>2 */
    heightnumerator = (TILEGLOBAL * scale) >> 6;
    
    switch(width)
    {
        case SCREENWIDTH:
            pixelangle = &pixelangle320[0];
        break;
        
        case  304:
            pixelangle = &pixelangle304[0];
        break;
        
        case  288:
            pixelangle = &pixelangle288[0];
        break;
        
        case  272:
            pixelangle = &pixelangle272[0];
        break;
        
        case  256:
            pixelangle = &pixelangle256[0];
        break;
        
        case  240:
            pixelangle = &pixelangle240[0];
        break;
        
        case  224:
            pixelangle = &pixelangle224[0];
        break;
        
        case  208:
            pixelangle = &pixelangle208[0];
        break;
        
        case  192:
            pixelangle = &pixelangle192[0];
        break;
        
        case  176:
            pixelangle = &pixelangle176[0];
        break;
        
        case  160:
            pixelangle = &pixelangle160[0];
        break;
        
        case  144:
            pixelangle = &pixelangle144[0];
        break;
        
        case  128:
            pixelangle = &pixelangle128[0];
        break;
        
        case  112:
            pixelangle = &pixelangle112[0];
        break;
        
        case  96:
            pixelangle = &pixelangle96[0];
        break;
        
        case  80:
            pixelangle = &pixelangle80[0];
        break;
        
        case  64:
            pixelangle = &pixelangle64[0];
        break;
    };
}

/* 
================================================================
=
= Function: SaveTheGame
=
= Description:
=
= save the current game state to file pointed to be *file
=
================================================================ 
*/
U8 SaveTheGame(FILE *file,S32 x,S32 y)
{
    S32 checksum = 0;
    S32 i;
    S32 j;
    U16 actnum;
    objtype *ob;
    objtype nullobj;
    statobj_t nullstat;

    /* save game state */
    DiskFlopAnim(x,y);
    fwrite(&gamestate,sizeof(gamestate),1,file);
    checksum = DoChecksum((U8 *)&gamestate,sizeof(gamestate),checksum);
    
    /* save kill ratio's etc */
    DiskFlopAnim(x,y);
    fwrite(&LevelRatios[0],sizeof(LRstruct)*LRpack,1,file);
    checksum = DoChecksum((U8 *)&LevelRatios[0],sizeof(LRstruct)*LRpack,checksum);
    
    /* save current tile map layout */
    DiskFlopAnim(x,y);
    fwrite(tilemap,sizeof(tilemap),1,file);
    checksum = DoChecksum((U8 *)tilemap,sizeof(tilemap),checksum);
    
    /* store actor type */
    DiskFlopAnim(x,y);
    for(i = 0; i < MAPSIZE; i++)
    {
        for(j = 0; j < MAPSIZE; j++)
        {
            objtype *objptr = actorat[i][j];
            
            if(ISPOINTER(objptr))
            {
                actnum = 0x8000 | (U16)(objptr-objlist);
            }
            else
            {
                actnum = (U16)(uintptr_t)objptr;
            }
            
            fwrite(&actnum,sizeof(actnum),1,file);
            checksum = DoChecksum((U8 *)&actnum,sizeof(actnum),checksum);
        }
    }
    
    /* store connected areas */
    fwrite(areaconnect,sizeof(areaconnect),1,file);
    fwrite(areabyplayer,sizeof(areabyplayer),1,file);
    
    /* player object needs special treatment as it's in WL_AGENT.CPP and not in */
    /* WL_ACT2.CPP which could cause problems for the relative addressing       */

    /* copy player object */
    ob = player;
    DiskFlopAnim(x,y);
    memcpy(&nullobj,ob,sizeof(nullobj));
    nullobj.state=(statetype *) ((uintptr_t)nullobj.state-(uintptr_t)&s_player);
    fwrite(&nullobj,sizeof(nullobj),1,file);
    ob = ob->next;
    
    /* copy all the enemy objects */
    DiskFlopAnim(x,y);
    for (; ob != NULL ; ob=ob->next)
    {
        memcpy(&nullobj,ob,sizeof(nullobj));
        nullobj.state=(statetype *) ((uintptr_t)nullobj.state-(uintptr_t)&s_grdstand);
        fwrite(&nullobj,sizeof(nullobj),1,file);
    }
    
    nullobj.active = ac_badobject;    /* end of file marker */
    DiskFlopAnim(x,y);
    fwrite(&nullobj,sizeof(nullobj),1,file);
    
    /* store the number of static objects */
    DiskFlopAnim(x,y);
    U16 laststatobjnum=(U16) (laststatobj-statobjlist);
    fwrite(&laststatobjnum,sizeof(laststatobjnum),1,file);
    checksum = DoChecksum((U8 *)&laststatobjnum,sizeof(laststatobjnum),checksum);
    
    /* store static objects */
    DiskFlopAnim(x,y);
    for(i = 0;i < MAXSTATS;i++)
    {
        memcpy(&nullstat,statobjlist+i,sizeof(nullstat));
        nullstat.visspot=(U8 *) ((uintptr_t) nullstat.visspot-(uintptr_t)spotvis);
        fwrite(&nullstat,sizeof(nullstat),1,file);
        checksum = DoChecksum((U8 *)&nullstat,sizeof(nullstat),checksum);
    }
    
    /* store door related info */
    DiskFlopAnim(x,y);
    fwrite (doorposition,sizeof(doorposition),1,file);
    checksum = DoChecksum((U8 *)doorposition,sizeof(doorposition),checksum);
    DiskFlopAnim(x,y);
    fwrite (doorobjlist,sizeof(doorobjlist),1,file);
    checksum = DoChecksum((U8 *)doorobjlist,sizeof(doorobjlist),checksum);
    
    /* WRITE OUT CHECKSUM */
    fwrite (&checksum,sizeof(checksum),1,file);

    /* store in game music */
    fwrite (&lastgamemusicoffset,sizeof(lastgamemusicoffset),1,file);

    return(1);
}

/* 
================================================================
=
= Function: LoadTheGame
=
= Description:
=
= Load a game state to file pointed to be *file
=
================================================================ 
*/
U8 LoadTheGame(FILE *file,S32 x,S32 y)
{
    S32 checksum = 0;
    S32 actnum = 0;
    S32 i;
    S32 j;
    S32 oldchecksum;
    objtype nullobj;
    statobj_t nullstat;
    
    /* load game state */
    DiskFlopAnim(x,y);
    fread (&gamestate,sizeof(gamestate),1,file);
    checksum = DoChecksum((U8 *)&gamestate,sizeof(gamestate),checksum);
    
    /* load kill ratio's etc */
    DiskFlopAnim(x,y);
    fread(&LevelRatios[0],sizeof(LRstruct)*LRpack,1,file);
    checksum = DoChecksum((U8 *)&LevelRatios[0],sizeof(LRstruct)*LRpack,checksum);
    
    DiskFlopAnim(x,y);
    SetupGameLevel();
    
    /* LOAD tile map layout */
    DiskFlopAnim(x,y);
    fread (tilemap,sizeof(tilemap),1,file);
    checksum = DoChecksum((U8 *)tilemap,sizeof(tilemap),checksum);
    
    /* load actor type */
    for(i=0;i<MAPSIZE;i++)
    {
        for(j = 0 ; j < MAPSIZE;j++)
        {
            fread (&actnum,sizeof(U16),1,file);
            checksum = DoChecksum((U8 *) &actnum,sizeof(U16),checksum);
            
            if(actnum&0x8000)
            {
                actorat[i][j]=objlist+(actnum&0x7fff);
            }
            else
            {
                actorat[i][j]=(objtype *)(uintptr_t) actnum;
            }
        }
    }
    
    /* load connected areas */
    fread(areaconnect,sizeof(areaconnect),1,file);
    fread(areabyplayer,sizeof(areabyplayer),1,file);
    
    /* load player object */
    InitActorList ();
    DiskFlopAnim(x,y);
    fread (player,sizeof(*player),1,file);
    player->state=(statetype *) ((uintptr_t)player->state+(uintptr_t)&s_player);
    
    /* load enemy objects */
    while(1)
    {
        DiskFlopAnim(x,y);
        fread (&nullobj,sizeof(nullobj),1,file);
        
        if(nullobj.active == ac_badobject)
        {
            break;
        }
        GetNewActor();
        nullobj.state=(statetype *) ((uintptr_t)nullobj.state+(uintptr_t)&s_grdstand);
        /* don't copy over the links */
        memcpy(newobj,&nullobj,sizeof(nullobj)-8);
    }
    
    /* load number of static objects */
    DiskFlopAnim(x,y);
    U16 laststatobjnum;
    fread (&laststatobjnum,sizeof(laststatobjnum),1,file);
    laststatobj=statobjlist+laststatobjnum;
    checksum = DoChecksum((U8 *)&laststatobjnum,sizeof(laststatobjnum),checksum);
    
    /* load static objects */
    DiskFlopAnim(x,y);
    for(i = 0; i < MAXSTATS;i++)
    {
        fread(&nullstat,sizeof(nullstat),1,file);
        checksum = DoChecksum((U8 *)&nullstat,sizeof(nullstat),checksum);
        nullstat.visspot=(U8 *) ((uintptr_t)nullstat.visspot+(uintptr_t)spotvis);
        memcpy(statobjlist+i,&nullstat,sizeof(nullstat));
    }
    
    /* load door related info */
    DiskFlopAnim(x,y);
    fread (doorposition,sizeof(doorposition),1,file);
    checksum = DoChecksum((U8 *)doorposition,sizeof(doorposition),checksum);
    DiskFlopAnim(x,y);
    fread (doorobjlist,sizeof(doorobjlist),1,file);
    checksum = DoChecksum((U8 *)doorobjlist,sizeof(doorobjlist),checksum);
    
    Thrust(0,0);    /* set player->areanumber to the floortile you're standing on */
    
    /* load old checksum */
    fread (&oldchecksum,sizeof(oldchecksum),1,file);
    
    /* load in game music */
    fread (&lastgamemusicoffset,sizeof(lastgamemusicoffset),1,file);
    
    if(lastgamemusicoffset<0)
    {
        lastgamemusicoffset = 0;
    }
    
    if(oldchecksum != checksum)
    {
        Message(STR_SAVECHT1"\n"
                STR_SAVECHT2"\n"
                STR_SAVECHT3"\n"
                STR_SAVECHT4);

        IN_Ack();

        gamestate.oldscore = gamestate.score = 0;
        gamestate.lives = 1;
        gamestate.weapon =
        gamestate.chosenweapon =
        gamestate.bestweapon = wp_pistol;
        gamestate.ammo = 8;
    }

    return 1;
}

/* 
================================================================
=
= Function: DiskFlopAnim
=
= Description:
=
= move disk save image
=
================================================================ 
*/
static void DiskFlopAnim(S32 x,S32 y)
{
    static S32 which = 0;
    
    if((x == 0) && (y == 0))
    {
        return;
    }    
    
    VWB_DrawPic(x,y,C_DISKLOADING1PIC+which);
    UpdateScreen(0);
    which ^= 1;
    
}

/* 
================================================================
=
= Function: DiskFlopAnim
=
= Description:
=
= move disk save image
=
================================================================ 
*/
static S32 DoChecksum(const U8 *source, U32 size,S32 checksum)
{
    U32 i;

    for (i = 0; i < size - 1 ; i++)
    {
        checksum += source[i] ^ source[i+1];
    }

    return checksum;
}
