#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include "types.h"
#include "wl_menu.h"
#include "audiowl6.h"
#include "gfxv_apo.h"
#include "wl_def.h"
#include "id_us.h"
#include "id_time.h"
#include "id_sd.h"
#include "id_in.h"
#include "id_ca.h"
#include "id_vh.h"
#include "id_vl.h"
#include "wl_inter.h"
#include "main.h"
#include "wl_text.h"

/* macro defines */

#define MAINCOLOR   0x6c
#define EMSCOLOR    0x6c
#define XMSCOLOR    0x6c
#define FILLCOLOR   14
#define STRIPE      0x2c
#define BORD2COLOR  0x23
#define DEACTIVE    0x2b
#define BORDCOLOR   0x29
#define HIGHLIGHT   0x13
#define READCOLOR   0x4a
#define MENUSONG    WONDERIN_MUS
#define MENU_X      76
#define MENU_Y      55
#define MENU_W      178
#define MENU_H      13*10+6

#define STR_NG      "New Game"
#define STR_SD      "Sound"
#define STR_CL      "Control"
#define STR_LG      "Load Game"
#define STR_SG      "Save Game"
#define STR_CV      "Change View"
#define STR_EG      "End Game"
#define STR_BD      "Back to Demo"
#define STR_QT      "Quit"
#define STR_DEMO    "Demo"
#define STR_GAME    "Game"

#define STARTITEM       readthis

#define SM_X        48
#define SM_W        250

#define SM_Y1       20
#define SM_H1       4*13-7
#define SM_Y2       SM_Y1+5*13
#define SM_H2       4*13-7
#define SM_Y3       SM_Y2+5*13
#define SM_H3       3*13-7

#define STR_NONE    "None"
#define STR_PC      "PC Speaker"
#define STR_ALSB    "AdLib/Sound Blaster"
#define STR_DISNEY  "Disney Sound Source"
#define STR_SB      "Sound Blaster"

#define YESBUTTONNAME "Y"
#define NOBUTTONNAME  "N"

#define ENDSTR1 "Dost thou wish to\nleave with such hasty\nabandon?"
#define ENDSTR2 "Chickening out...\nalready?"
#define ENDSTR3 "Press " NOBUTTONNAME " for more carnage.\nPress " YESBUTTONNAME " to be a weenie."
#define ENDSTR4 "So, you think you can\nquit this easily, huh?"
#define ENDSTR5 "Press " NOBUTTONNAME " to save the world.\nPress " YESBUTTONNAME " to abandon it in\nits hour of need."
#define ENDSTR6 "Press " NOBUTTONNAME " if you are brave.\nPress " YESBUTTONNAME " to cower in shame."
#define ENDSTR7 "Heroes, press " NOBUTTONNAME ".\nWimps, press " YESBUTTONNAME "."
#define ENDSTR8 "You are at an intersection.\nA sign says, 'Press " YESBUTTONNAME " to quit.'\n>"
#define ENDSTR9 "For guns and glory, press " NOBUTTONNAME ".\nFor work and worry, press " YESBUTTONNAME "."

#define STR_SIZE1   "Use arrows to size"
#define STR_SIZE2   "ENTER to accept"
#define STR_SIZE3   "ESC to cancel"
#define STR_THINK   "Thinking"

#define STR_MOUSEEN "Mouse Enabled"
#define STR_JOYEN   "Joystick Enabled"
#define STR_SENS    "Mouse Sensitivity"
#define STR_CUSTOM  "Customize controls"
#define CTL_W   284
#define CTL_H   60

#define CST_Y           48
#define CST_START       60
#define CST_SPC         60
#define STR_CRUN        "Run"
#define STR_COPEN       "Open"
#define STR_CFIRE       "Fire"
#define STR_CSTRAFE     "Strafe"

#define NE_X    10
#define NE_Y    23
#define NE_W    320-NE_X*2
#define NE_H    200-NE_Y*2

#define NM_X    50
#define NM_Y    100
#define NM_W    225
#define NM_H    (13*4+15)

#define STR_DADDY   "Can I play, Daddy?"
#define STR_HURTME  "Don't hurt me."
#define STR_BRINGEM "Bring 'em on!"
#define STR_DEATH   "I am Death incarnate!"

#define ENDGAMESTR  "Are you sure you want\n"\
                    "to end the game you\n"\
                    "are playing? (A or B):"

#define CURGAME     "You are currently in\n"\
                    "a game. Continuing will\n"\
                    "erase old game. Ok?"

#define LSM_X   86
#define LSM_Y   55
#define LSM_W   174
#define LSM_H   10*13+10

#define STR_EMPTY    "empty"

#define LSA_X   96
#define LSA_Y   80
#define LSA_W   130
#define LSA_H   42

#define STR_LOADING    "Loading"
#define STR_SAVING    "Saving"

#define GAMESVD	    "There's already a game\n"\
                    "saved at this position.\n"\
                    "      Overwrite?"

/* external variables */

extern U16 WindowX;
extern U16 WindowY;
extern U16 WindowW;
extern U16 WindowH;
extern U16 PrintX;
extern U16 PrintY;
extern U8 * grsegs[NUMCHUNKS];
extern char extension[5];
extern char graphext[5];
extern char audioext[5];
extern S32  numEpisodesMissing;
extern S32  viewsize;
extern S32 buttonmouse[4];
extern S32 buttonjoy[4];
extern char configname[13];
extern char demoname[13];
extern U8 loadedgame;
extern U8 startgame;
extern U8 ingame;
extern gametype gamestate;
extern exit_t playstate;
extern objtype *killerobj;

/* global variables */

CP_itemtype MainMenu[] =
{
    {1, STR_NG, CP_NewGame},
    {1, STR_SD, CP_Sound},
    {1, STR_CL, CP_Control},
    {1, STR_LG, CP_LoadGame},
    {0, STR_SG, CP_SaveGame},
    {1, STR_CV, CP_ChangeView},
    {2, "Read This!", CP_ReadThis},
    {1, STR_VS, CP_ViewScores},
    {1, STR_BD, NULL},
    {1, STR_QT, NULL}
};

S32 lastgamemusicoffset = 0;
U8  ExitMenu = 0;

/* local variables */

enum
{
    FIRE,
    STRAFE,
    RUN,
    OPEN
};

enum
{
    CTL_MOUSEENABLE,
    CTL_MOUSESENS,
    CTL_JOYENABLE,
    CTL_CUSTOMIZE
};

enum
{
    MOUSE,
    JOYSTICK,
    KEYBOARDBTNS,
    KEYBOARDMOVE
};  /* FOR INPUT TYPES */

static S32 color_hlite[] =
{
    DEACTIVE,
    HIGHLIGHT,
    READHCOLOR,
    0x67
};

static S32 color_norml[] =
{
    DEACTIVE,
    TEXTCOLOR,
    READCOLOR,
    0x6b
};

static CP_itemtype SndMenu[] =
{
    {1, STR_NONE, 0},
    {0, STR_PC, 0},
    {1, STR_ALSB, 0},
    {0, "", 0},
    {0, "", 0},
    {1, STR_NONE, 0},
    {0, STR_DISNEY, 0},
    {1, STR_SB, 0},
    {0, "", 0},
    {0, "", 0},
    {1, STR_NONE, 0},
    {1, STR_ALSB, 0}
};

static CP_itemtype CtlMenu[] =
{
    {0, STR_MOUSEEN, 0},
    {0, STR_SENS, 0},
    {0, STR_JOYEN, 0},
    {1, STR_CUSTOM, CustomControls}
};

static CP_itemtype LSMenu[] =
{
    {1, "", 0},
    {1, "", 0},
    {1, "", 0},
    {1, "", 0},
    {1, "", 0},
    {1, "", 0},
    {1, "", 0},
    {1, "", 0},
    {1, "", 0},
    {1, "", 0}
};

static CP_itemtype CusMenu[] =
{
    {0, "", 0},
    {0, "", 0},
    {0, "", 0},
    {1, "", 0},
    {0, "", 0},
    {0, "", 0},
    {0, "", 0},
    {0, "", 0},
    {0, "", 0}
};

static CP_itemtype NewEmenu[] =
{
    {1, "Episode 1\n" "Escape from Wolfenstein", 0},
    {0, "", 0},
    {3, "Episode 2\n" "Operation: Eisenfaust", 0},
    {0, "", 0},
    {3, "Episode 3\n" "Die, Fuhrer, Die!", 0},
    {0, "", 0},
    {3, "Episode 4\n" "A Dark Secret", 0},
    {0, "", 0},
    {3, "Episode 5\n" "Trail of the Madman", 0},
    {0, "", 0},
    {3, "Episode 6\n" "Confrontation", 0}
};

static CP_itemtype NewMenu[] =
{
    {1, STR_DADDY, 0},
    {1, STR_HURTME, 0},
    {1, STR_BRINGEM, 0},
    {1, STR_DEATH, 0}
};

static char endStrings[9][80] =
{
    ENDSTR1,
    ENDSTR2,
    ENDSTR3,
    ENDSTR4,
    ENDSTR5,
    ENDSTR6,
    ENDSTR7,
    ENDSTR8,
    ENDSTR9
};

/* CP_iteminfo struct format: short x, y, amount, curpos, indent; */
static CP_iteminfo MainItems = { MENU_X, MENU_Y, (sizeof(MainMenu) / sizeof(*(MainMenu))), STARTITEM, 24 };
static CP_iteminfo SndItems  = { SM_X, SM_Y1, (sizeof(SndMenu) / sizeof(*(SndMenu))), 0, 52 };
static CP_iteminfo CtlItems  = { CTL_X, CTL_Y, (sizeof(CtlMenu) / sizeof(*(CtlMenu))), -1, 56 };
static CP_iteminfo LSItems   = { LSM_X, LSM_Y, (sizeof(LSMenu) / sizeof(*(LSMenu))), 0, 24 };
static CP_iteminfo CusItems  = { 8, CST_Y + 13 * 2, (sizeof(CusMenu) / sizeof(*(CusMenu))), -1, 0};
static CP_iteminfo NewEitems = { NE_X, NE_Y, (sizeof(NewEmenu) / sizeof(*(NewEmenu))), 0, 88 };
static CP_iteminfo NewItems  = { NM_X, NM_Y, (sizeof(NewMenu) / sizeof(*(NewMenu))), 2, 24 };

static char mbarray[4][3] = { " A", " B", " X", " Y" };
static S32 order[4] = { RUN, OPEN, FIRE, STRAFE };

static S32 fontnumber;
static U8  fontcolor;
static U8  backcolor;
static S32 lastmusic = -1;
static S32 EpisodeSelect[6] = { 1 };
static char SaveName[13] = "savegam?.";
static S32 SaveGamesAvail[10] = {0,0,0,0,0,0,0,0,0,0};
static char SaveGameNames[10][32] =
{
    {""},
    {""},
    {""},
    {""},
    {""},
    {""},
    {""},
    {""},
    {""},
    {""},
};

/* local prototypes */

static void DrawOutline(S32 x, S32 y, S32 w, S32 h, S32 color1, S32 color2);
static void SetTextColor(CP_itemtype * items, U8 hlight);
static void ReadAnyControl(ControlInfo * ci);
static void EraseGun(CP_iteminfo * item_i, CP_itemtype * items, S32 x, S32 y, S32 which);
static void DrawHalfStep(S32 x, S32 y);
static void DrawGun(CP_iteminfo * item_i, CP_itemtype * items, S32 x, S16 *y, S32 which, S32 basey,void (*pt2function) (S32 w));
static void SetupControlPanel(void);
static void DrawMainMenu(void);
static void DrawSoundMenu(void);
static void DrawMenuGun (CP_iteminfo * iteminfo);
static S32 CP_Quit(S32 temp);
static S32 Confirm(const char *string);
static void DrawChangeView(S32 view);
static void DrawCtlScreen(void);
static void DrawCustomScreen(void);
static void DrawCustMouse(S32 hilight);
static void PrintCustMouse(S32 i);
static void DrawCustJoy(S32 hilight);
static void PrintCustJoy(S32 i);
static void DefineJoyBtns(void);
static void EnterCtrlData (S32 index, CustomCtrls * cust, void (*DrawRtn) (S32), void (*PrintRtn) (S32), S32 type);
static void DrawNewEpisode(void);
static void CleanupControlPanel(void);
static void DrawNewGame(void);
static void DrawNewGameDiff(S32 w);
static S32 CP_EndGame(void);
static void EnableEndGameMenuItem(void);
static void DrawLoadSaveScreen(S32 loadsave);
static void PrintLSEntry (S32 w, S32 color);
static void TrackWhichGame (S32 w);
static void DrawLSAction(S32 which);

/*
================================================================
=
= Function: StartCPMusic
=
= Description:
=
= Starts playing requested music chunk
=
================================================================
*/
S16 StartCPMusic(S32 song)
{

    S16 lastoffs = 0;

    /* if this is not the first song being played uncache it */
    if(lastmusic != -1)
    {
        lastoffs = SD_MusicOff();
        CA_UncacheAudioChunk(STARTMUSIC + lastmusic);
    }

    lastmusic = song;

    SD_StartMusic(STARTMUSIC + song);
    return lastoffs;
}

/*
================================================================
=
= Function: FreeMusic
=
= Description:
=
= uncache last song played
=
================================================================
*/
void FreeMusic(void)
{
    CA_UncacheAudioChunk(STARTMUSIC + lastmusic);
}

/*
================================================================
=
= Function: IntroScreen
=
= Description:
=
= HANDLE INTRO SCREEN (SYSTEM CONFIG)
=
================================================================
*/
void IntroScreen(void)
{
    U8 i;

    /* draw main memory */
    for (i = 0; i < 10; i++)
    {
        VL_Bar(49, 163 - 8 * i, 6, 5, MAINCOLOR - i);
    }

    /* draw ems memory */
    for (i = 0; i < 10; i++)
    {
        VL_Bar(89, 163 - 8 * i, 6, 5, EMSCOLOR - i);
    }

    /* draw xms memory */
    for (i = 0; i < 10; i++)
    {
        VL_Bar(129, 163 - 8 * i, 6, 5, XMSCOLOR - i);
    }

    /* fill joystick slot */
    VL_Bar(164, 105, 12, 2, FILLCOLOR);


    /* fill SoundBlaster slot */
    VL_Bar(164, 151, 12, 2, FILLCOLOR);
}

/*
================================================================
=
= Function: CacheLump
=
= Description:
=
= Cache a LUMP of graphics
=
================================================================
*/
void CacheLump(S32 lumpstart, S32 lumpend)
{
    S32 i;

    for (i = lumpstart; i <= lumpend; i++)
    {
        CA_CacheGrChunk(i);
    }
}

/*
================================================================
=
= Function: UnCacheLump
=
= Description:
=
= UnCache a LUMP of graphics
=
================================================================
*/
void UnCacheLump(S32 lumpstart, S32 lumpend)
{
    S32 i;

    for (i = lumpstart; i <= lumpend; i++)
    {
        if(grsegs[i] != NULL)
        {
            CA_UnCacheGrChunk(i);
        }
    }
}

/*
================================================================
=
= Function: DrawStripes
=
= Description:
=
= DRAW SCREEN TITLE STRIPES
=
================================================================
*/
void DrawStripes(S32 y)
{
    VL_Bar(0, y, 320, 24, 0);
    VWB_Hlin(0, 319, y + 22, STRIPE);
}

/*
================================================================
=
= Function: DrawWindow
=
= Description:
=
= Draw a window for a menu
=
================================================================
*/
void DrawWindow(S32 x, S32 y, S32 w, S32 h, S32 wcolor)
{
    VL_Bar(x, y, w, h, wcolor);
    DrawOutline(x, y, w, h, BORD2COLOR, DEACTIVE);
}

/*
================================================================
=
= Function: DrawOutline
=
= Description:
=
= Draw a small frame (outline) around a given window
=
================================================================
*/
static void DrawOutline (S32 x, S32 y, S32 w, S32 h, S32 color1, S32 color2)
{
    VWB_Hlin(x, x + w, y, color2);
    VWB_Vlin(y, y + h, x, color2);
    VWB_Hlin(x, x + w, y + h, color1);
    VWB_Vlin(y, y + h, x + w, color1);
}

/*
================================================================
=
= Function: ClearMScreen
=
= Description:
=
= Clears screen buffer to a requested colour
=
================================================================
*/
void ClearMScreen(void)
{
    VL_Bar(0, 0, 320, 200, BORDCOLOR);
}

/*
================================================================
=
= Function: DrawMenu
=
= Description:
=
= Draws menu pointed to by items using information pointed to
= by item_i
=
================================================================
*/
void DrawMenu(CP_iteminfo * item_i, CP_itemtype * items)
{
    S32 i;
    U8 highlight_text;
    S16 selected_text = item_i->curpos;

    WindowX = item_i->x + item_i->indent;
    PrintX = item_i->x + item_i->indent;
    WindowY = item_i->y;
    PrintY = item_i->y;
    WindowW = 320;
    WindowH = 200;

    for(i = 0; i < item_i->amount; i++)
    {
        highlight_text = (U8)(selected_text == i) ? 1 : 0;
        SetTextColor(items + i, highlight_text);

        PrintY = item_i->y + i * 13;

        if ((items + i)->active)
        {
            US_Print((items + i)->string);
        }
        else
        {
            SetFontColor(DEACTIVE, BKGDCOLOR);
            US_Print((items + i)->string);
            SetFontColor(TEXTCOLOR, BKGDCOLOR);
        }

        US_Print("\n");
    }
}

/*
================================================================
=
= Function: SetTextColor
=
= Description:
=
= SET TEXT COLOR (HIGHLIGHT OR NO)
=
================================================================
*/
static void SetTextColor(CP_itemtype * items, U8 hlight)
{
    if(hlight == 1)
    {
        SetFontColor(color_hlite[items->active], BKGDCOLOR);
    }
    else
    {
        SetFontColor(color_norml[items->active], BKGDCOLOR);
    }
}

/*
================================================================
=
= Function: HandleMenu
=
= Description:
=
= Handle moving gun around a menu
=
================================================================
*/

S32 HandleMenu(CP_iteminfo * item_i, CP_itemtype * items, void (*pt2function) (S32 w))
{
    static S16 redrawitem = 1;
    static S16 lastitem = -1;
    S16 x, y, basey, exit, which, shape;
    U32 lastBlinkTime;
    S32 timer;
    ControlInfo ci;

    which = item_i->curpos;
    /* make sure picture start on a location that is a multiple of 8*/
    x = (S16)((U16)item_i->x & 0xFFF8);
    basey = item_i->y - 2;
    y = basey + which * 13;

    /* Draw default gun picture to screen */
    VWB_DrawPic(x, y, C_CURSOR1PIC);

    SetTextColor(items + which, 1);
    if(redrawitem != 0)
    {
        PrintX = item_i->x + item_i->indent;
        PrintY = item_i->y + which * 13;
        US_Print((items + which)->string);
    }


    /* CALL CUSTOM ROUTINE IF IT IS NEEDED */
    if(pt2function != NULL)
    {
        pt2function(which);
    }

    UpdateScreen(0);

    /* set the default variable values before entering the do loop  */
    shape = C_CURSOR1PIC;
    timer = 8;
    exit = 0;
    lastBlinkTime = GetTimeCount();

    do
    {
        /* CHANGE GUN SHAPE IF NEEDED */
        if ((GetTimeCount() - lastBlinkTime) > timer)
        {
            /* reset timer */
            lastBlinkTime = GetTimeCount();

            /* change gun pic */
            if (shape == C_CURSOR1PIC)
            {
                shape = C_CURSOR2PIC;
                timer = 8;
            }
            else
            {
                shape = C_CURSOR1PIC;
                timer = 70;
            }

            VWB_DrawPic(x, y, shape);

            if(pt2function != NULL)
            {
                pt2function(which);
            }

            UpdateScreen(0);
        }
        else
        {
            Delay_ms(5);
        }

        /* clear last button read */
        ci.button0 = 0;
        ci.button1 = 0;
        ci.button2 = 0;
        ci.button3 = 0;
        ci.dir = dir_None;
        ci.straferight = 0;
        ci.strafeleft = 0;
        ci.pause = 0;
        ci.esc = 0;

        /* GET USER INPUT */
        ReadAnyControl(&ci);

        switch (ci.dir)
        {
            /* MOVE UP */
            case dir_North:

                EraseGun(item_i, items, x, y, which);

                /* ANIMATE HALF-STEP */
                if ((which != 0) && ((items + which - 1)->active != 0))
                {
                    y -= 6;
                    DrawHalfStep(x, y);
                }

                /* MOVE TO NEXT AVAILABLE SPOT */
                do
                {
                    if (which == 0)
                    {
                        which = item_i->amount - 1;
                    }
                    else
                    {
                        which--;
                    }
                }
                while((items + which)->active == 0);

                DrawGun(item_i, items, x, &y, which, basey, pt2function);

                /* WAIT FOR BUTTON-UP OR DELAY NEXT MOVE */
                TicDelay(20);
            break;

            /* MOVE DOWN */
            case dir_South:

                EraseGun(item_i, items, x, y, which);

                /* ANIMATE HALF-STEP */
                if ((which != item_i->amount - 1) && ((items + which + 1)->active != 0))
                {
                    y += 6;
                    DrawHalfStep(x, y);
                }

                do
                {
                    if (which == item_i->amount - 1)
                    {
                        which = 0;
                    }
                    else
                    {
                        which++;
                    }
                }
                while((items + which)->active == 0);

                DrawGun(item_i, items, x, &y, which, basey, pt2function);

                /* WAIT FOR BUTTON-UP OR DELAY NEXT MOVE */
                TicDelay (20);
            break;

            default:
                /* Do nothing */
            break;
        }


        if(ci.button0 == 1)
        {
            exit = 1;
        }

        if(ci.button1 == 1)
        {
            exit = 2;
        }


    }while(exit == 0);

    /* if user changed menu selection position erase gun */
    /* and set font colour back to background colour */
    if (lastitem != which)
    {
        VL_Bar(x - 1, y, 25, 16, BKGDCOLOR);
        PrintX = item_i->x + item_i->indent;
        PrintY = item_i->y + which * 13;
        US_Print((items + which)->string);
        redrawitem = 1;
    }
    else
    {
        redrawitem = 0;
    }

    if(pt2function != NULL)
    {
        pt2function(which);
    }

    UpdateScreen(0);

    item_i->curpos = which;

    lastitem = which;

    switch (exit)
    {
        case 1:
            /* CALL THE ROUTINE  if required */
            if ((items + which)->pt2function != NULL)
            {
                SD_PlaySound(SHOOTSND);
                MenuFadeOut();
                (items + which)->pt2function(0);
            }
            return which;
        break;

        case 2:
            SD_PlaySound(ESCPRESSEDSND);
            return -1;
        break;

        default:
            return 0;
        break;
    }

}

/*
================================================================
=
= Function: ReadAnyControl
=
= Description:
=
= READ NDS DPAD AND BUTTON INPUTS
=
================================================================
*/
static void ReadAnyControl(ControlInfo * ci)
{
    /* read nds controls */
    IN_ReadControl(ci);
}

/*
================================================================
=
= Function: SetFontNUm
=
= Description:
=
= set Font number to use during printing of string
=
================================================================
*/
void SetFontNum(U8 FontNum)
{
    if(FontNum <= 1)
    {
        fontnumber = FontNum;
    }
}

/*
================================================================
=
= Function: GetFontNum
=
= Description:
=
= return current font number used for printing strings
=
================================================================
*/
U8 GetFontNum(void)
{
    return fontnumber;
}

/*
================================================================
=
= Function: SetFontColor
=
= Description:
=
= set Font colour to use during printing of string
=
================================================================
*/
void SetFontColor(U8 f, U8 b)
{
   fontcolor = f;
   backcolor = b;
}

/*
================================================================
=
= Function: GetFontColor
=
= Description:
=
= return current font colour used for printing strings
=
================================================================
*/
U8 GetFontColor(void)
{
   return fontcolor;
}

/*
================================================================
=
= Function: GetBackColour
=
= Description:
=
= return current background colour
=
================================================================
*/
U8 GetBackColour(void)
{
    return backcolor;
}

/*
================================================================
=
= Function: EraseGun
=
= Description:
=
= ERASE GUN & DE-HIGHLIGHT STRING
=
================================================================
*/
static void EraseGun(CP_iteminfo * item_i, CP_itemtype * items, S32 x, S32 y, S32 which)
{
    VL_Bar(x - 1, y, 25, 16, BKGDCOLOR);
    SetTextColor(items + which, 0);

    PrintX = item_i->x + item_i->indent;
    PrintY = item_i->y + which * 13;
    US_Print((items + which)->string);
    UpdateScreen(0);
}

/*
================================================================
=
= Function: DrawHalfStep
=
= Description:
=
= DRAW HALF STEP OF GUN TO NEXT POSITION
=
================================================================
*/
static void DrawHalfStep(S32 x, S32 y)
{
    VWB_DrawPic(x, y, C_CURSOR1PIC);
    UpdateScreen(0);
    SD_PlaySound(MOVEGUN1SND);
    Delay_ms(8 * 100 / 7);
}

/*
================================================================
=
= Function: DrawGun
=
= Description:
=
= DRAW GUN AT NEW POSITION
=
================================================================
*/
static void DrawGun(CP_iteminfo * item_i, CP_itemtype * items, S32 x, S16 *y, S32 which, S32 basey,void (*pt2function) (S32 w))
{
    VL_Bar(x - 1, *y, 25, 16, BKGDCOLOR);
    *y = basey + which * 13;
    VWB_DrawPic(x, *y, C_CURSOR1PIC);
    SetTextColor(items + which, 1);

    PrintX = item_i->x + item_i->indent;
    PrintY = item_i->y + which * 13;
    US_Print ((items + which)->string);

    /* CALL CUSTOM ROUTINE IF IT IS NEEDED */
    if (pt2function != NULL)
    {
        pt2function(which);
    }

    UpdateScreen(0);
    SD_PlaySound(MOVEGUN2SND);
}

/*
================================================================
=
= Function: TicDelay
=
= Description:
=
= DELAY FOR AN AMOUNT OF TICS OR UNTIL CONTROLS ARE INACTIVE
=
================================================================
*/
void TicDelay(S32 count)
{
    ControlInfo ci;

    S32 startTime = GetTimeCount();

    do
    {
        /* clear last button read */
        ci.button0 = 0;
        ci.button1 = 0;
        ci.button2 = 0;
        ci.button3 = 0;
        ci.dir = dir_None;
        ci.straferight = 0;
        ci.strafeleft = 0;
        ci.pause = 0;
        ci.esc = 0;

        Delay_ms(10);
        ReadAnyControl(&ci);
    }
    while (((GetTimeCount() - startTime) < count) && (ci.dir != dir_None));
}

/*
================================================================
=
= Function: CheckForEpisodes
=
= Description:
=
= checks for shareware or full version of game
=
================================================================
*/
void CheckForEpisodes(void)
{
    struct stat statbuf;

    #ifdef SHAREWARE
    if(stat("/dswolf/vswap.wl1", &statbuf) >= 0)
    {
        strcpy (extension, "wl1");
        numEpisodesMissing = 5;
    }
    #else
    if(stat("/dswolf/vswap.wl6", &statbuf) >= 0)
    {
        strcpy (extension, "wl6");
        NewEmenu[2].active = 1;
        NewEmenu[4].active = 1;
        NewEmenu[6].active = 1;
        NewEmenu[8].active = 1;
        NewEmenu[10].active = 1;
        EpisodeSelect[1] = 1;
        EpisodeSelect[2] = 1;
        EpisodeSelect[3] = 1;
        EpisodeSelect[4] = 1;
        EpisodeSelect[5] = 1;
        numEpisodesMissing = 0;
    }
    #endif

    if(numEpisodesMissing == -1)
    {
        printf("cant open valid vswap file \n");
        /* TODO: reset or poweroff instead (wait for keypress) */
        while(1){}; /* hang system */
    }

    strcpy(graphext, extension);
    strcpy(audioext, extension);

    strcat(configname, extension);
    strcat(SaveName, extension);
    strcat(demoname, extension);
}

/*
================================================================
=
= Function: US_ControlPanel
=
= Description:
=
= Wolfenstein Control Panel!  Ta Da!
=
================================================================
*/
void US_ControlPanel(ScanCode scancode)
{
    S32 which;

    /* store in game music position */
    if(ingame == 1)
    {
        lastgamemusicoffset = StartCPMusic (MENUSONG);
    }
    else
    {
        StartCPMusic(MENUSONG);
    }

    SetupControlPanel();

    DrawMainMenu();
    MenuFadeIn();

    ExitMenu = 0;

    /* MAIN MENU LOOP */
    do
    {

        which = HandleMenu(&MainItems, &MainMenu[0], NULL);

        switch (which)
        {

            case viewscores:
                if (MainMenu[viewscores].pt2function == NULL)
                {
                    if(CP_EndGame() == 1)
                    {
                        ExitMenu = 1;
                    }
                }
                else
                {
                    DrawMainMenu();
                    MenuFadeIn();
                }
            break;

            case backtodemo:
                ExitMenu = 1;
                if(ingame == 0)
                {
                    StartCPMusic(INTROSONG);
                }
                VL_FadeOut(0, 255, 0, 0, 0, 10);
            break;

            case -1:
            case quit:
                CP_Quit(0);
            break;

            default:
                if(ExitMenu == 0)
                {
                    DrawMainMenu();
                    MenuFadeIn();
                }
            break;
        }

    /* "EXIT OPTIONS" OR "NEW GAME" EXITS */
    }while(ExitMenu == 0);

    /* DEALLOCATE EVERYTHING */
    CleanupControlPanel();

    /* CHANGE MAINMENU ITEM */
    if((startgame == 1) || (loadedgame == 1))
    {
        EnableEndGameMenuItem();
    }

    /* RETURN/START GAME EXECUTION */
}

/*
================================================================
=
= Function: EnableEndGameMenuItem
=
= Description:
=
= Change viewscore to read end game within main menu
=
================================================================
*/
static void EnableEndGameMenuItem(void)
{
    MainMenu[viewscores].pt2function = NULL;

    strcpy (MainMenu[viewscores].string, STR_EG);
}

/*
================================================================
=
= Function: CP_EndGame
=
= Description:
=
= END THE CURRENT GAME
=
================================================================
*/
static S32 CP_EndGame(void)
{
    S32 res;

    res = Confirm(ENDGAMESTR);

    DrawMainMenu();

    if(res == 0)
    {
        return 0;
    }

    gamestate.lives = 0;
    playstate = ex_died;
    killerobj = NULL;

    MainMenu[savegame].active = 0;
    MainMenu[viewscores].pt2function = CP_ViewScores;
    strcpy (MainMenu[viewscores].string, STR_VS);

    return 1;
}

/*
================================================================
=
= Function: CleanupControlPanel
=
= Description:
=
= Clean up all the Control Panel stuff
=
================================================================
*/
static void CleanupControlPanel(void)
{
    UnCacheLump(CONTROLS_LUMP_START, CONTROLS_LUMP_END);
    SetFontNum(0);
}

/*
================================================================
=
= Function: SetupControlPanel
=
= Description:
=
= Setup Control Panel stuff - graphics, etc.
=
================================================================
*/
static void SetupControlPanel(void)
{
    /* CACHE GRAPHICS & SOUNDS */
    CA_CacheGrChunk(STARTFONT + 1);

    CacheLump(CONTROLS_LUMP_START, CONTROLS_LUMP_END);

    SetFontColor(TEXTCOLOR, BKGDCOLOR);
    SetFontNum(1);

    WindowH = 200;

    if(ingame == 0)
    {
        CA_LoadAllSounds();
    }
    else
    {
        MainMenu[savegame].active = 1;
    }
}

/*
================================================================
=
= Function: DrawMainMenu
=
= Description:
=
= DRAW MAIN MENU SCREEN
=
================================================================
*/
static void DrawMainMenu(void)
{
    ClearMScreen();

    VWB_DrawPic(112, 184, C_MOUSELBACKPIC);
    DrawStripes(10);
    VWB_DrawPic(84, 0, C_OPTIONSPIC);

    DrawWindow(MENU_X - 8, MENU_Y - 3, MENU_W, MENU_H, BKGDCOLOR);

    /* CHANGE "GAME" AND "DEMO" */
    if(ingame == 1)
    {
        strcpy(&MainMenu[backtodemo].string[8], STR_GAME);
        MainMenu[backtodemo].active = 2;
    }
    else
    {
        strcpy(&MainMenu[backtodemo].string[8], STR_DEMO);
        MainMenu[backtodemo].active = 1;
    }

    DrawMenu(&MainItems, &MainMenu[0]);

    UpdateScreen(0);
}

/*
================================================================
=
= Function: CP_ViewScores
=
= Description:
=
= VIEW THE HIGH SCORES
=
================================================================
*/
S32 CP_ViewScores(S32 temp)
{
    SetFontNum(0);

    StartCPMusic(ROSTER_MUS);

    DrawHighScores();
    UpdateScreen(0);
    MenuFadeIn();

    SetFontNum(1);

    IN_Ack();

    StartCPMusic(MENUSONG);
    MenuFadeOut();

    return 0;
}

/*
================================================================
=
= Function: CP_Sound
=
= Description:
=
= HANDLE SOUND MENU
=
================================================================
*/
S32 CP_Sound(S32 temp)
{
    S32 which;

    DrawSoundMenu();
    MenuFadeIn();

    do
    {
        which = HandleMenu (&SndItems, &SndMenu[0], NULL);

        /* HANDLE MENU CHOICES */
        switch (which)
        {

            /* SOUND EFFECTS */
            case 0:
                if(SD_GetSoundMode() != sdm_Off)
                {
                    SD_WaitSoundDone();
                    SD_SetSoundMode(sdm_Off);
                    DrawSoundMenu();
                }
            break;

            case 2:
                if(SD_GetSoundMode() != sdm_AdLib)
                {
                    SD_WaitSoundDone();
                    SD_SetSoundMode(sdm_AdLib);
                    CA_LoadAllSounds();
                    DrawSoundMenu();
                    SD_PlaySound(SHOOTSND);
                }
            break;

            /* DIGITIZED SOUND */
            case 5:
                if(SD_GetDigiDevice() != sds_Off)
                {
                    SD_SetDigiDevice(sds_Off);
                    DrawSoundMenu();
                }
            break;

            case 7:
                if(SD_GetDigiDevice() != sds_SoundBlaster)
                {
                    SD_SetDigiDevice(sds_SoundBlaster);
                    DrawSoundMenu();
                    SD_PlaySound(SHOOTSND);
                }
            break;

            /* MUSIC */
            case 10:
                if (SD_GetMusicMode() != smm_Off)
                {
                    SD_SetMusicMode(smm_Off);
                    DrawSoundMenu();
                    SD_PlaySound(SHOOTSND);
                }
            break;

            case 11:
                if (SD_GetMusicMode() != smm_AdLib)
                {
                    SD_SetMusicMode(smm_AdLib);
                    DrawSoundMenu();
                    SD_PlaySound(SHOOTSND);
                    SD_WaitSoundDone();
                    StartCPMusic(MENUSONG);
                }
            break;

            default:
                /* do nothing */
            break;

        }

    }while(which >= 0);

    MenuFadeOut();

    return 0;
}

/*
================================================================
=
= Function: DrawSoundMenu
=
= Description:
=
= DRAW THE SOUND MENU
=
================================================================
*/
static void DrawSoundMenu(void)
{
    S32 i;
    S32 on;

    /* DRAW SOUND MENU */
    ClearMScreen();
    VWB_DrawPic(112, 184, C_MOUSELBACKPIC);

    DrawWindow(SM_X - 8, SM_Y1 - 3, SM_W, SM_H1, BKGDCOLOR);
    DrawWindow(SM_X - 8, SM_Y2 - 3, SM_W, SM_H2, BKGDCOLOR);
    DrawWindow(SM_X - 8, SM_Y3 - 3, SM_W, SM_H3, BKGDCOLOR);

    DrawMenu(&SndItems, &SndMenu[0]);

    VWB_DrawPic (100, SM_Y1 - 20, C_FXTITLEPIC);
    VWB_DrawPic (100, SM_Y2 - 20, C_DIGITITLEPIC);
    VWB_DrawPic (100, SM_Y3 - 20, C_MUSICTITLEPIC);

    for (i = 0; i < SndItems.amount; i++)
    {
        if(SndMenu[i].string[0] != '\0')
        {
            /* DRAW SELECTED/NOT SELECTED GRAPHIC BUTTONS */
            on = 0;
            switch(i)
            {
                /* SOUND EFFECTS */
                case 0:
                    if(SD_GetSoundMode() == sdm_Off)
                    {
                        on = 1;
                    }
                break;

                case 2:
                    if(SD_GetSoundMode() == sdm_AdLib)
                    {
                        on = 1;
                    }
                break;

                /* DIGITIZED SOUND */
                case 5:
                    if(SD_GetDigiDevice() == sds_Off)
                    {
                        on = 1;
                    }
                break;

                case 7:
                    if(SD_GetDigiDevice() == sds_SoundBlaster)
                    {
                        on = 1;
                    }
                break;

                /* MUSIC */
                case 10:
                    if(SD_GetMusicMode() == smm_Off)
                    {
                        on = 1;
                    }
                break;

                case 11:
                    if(SD_GetMusicMode() == smm_AdLib)
                    {
                        on = 1;
                    }
                break;

                default:
                    /* do nothing */
                break;
            }

            if(on == 1)
            {
                VWB_DrawPic(SM_X + 24, SM_Y1 + i * 13 + 2, C_SELECTEDPIC);
            }
            else
            {
                VWB_DrawPic(SM_X + 24, SM_Y1 + i * 13 + 2, C_NOTSELECTEDPIC);
            }
        }
    }

    DrawMenuGun(&SndItems);
    UpdateScreen(0);
}

/*
================================================================
=
= Function: DrawMenuGun
=
= Description:
=
= DRAW GUN CURSOR AT CORRECT POSITION IN MENU
=
================================================================
*/
static void DrawMenuGun (CP_iteminfo * iteminfo)
{
    S32 x;
    S32 y;

    x = iteminfo->x;
    y = iteminfo->y + iteminfo->curpos * 13 - 2;
    VWB_DrawPic(x, y, C_CURSOR1PIC);
}

/*
================================================================
=
= Function: CP_Quit
=
= Description:
=
= QUIT THIS INFERNAL GAME!
=
================================================================
*/
static S32 CP_Quit(S32 temp)
{
    if(Confirm(endStrings[(US_RndT() & 0x07) + (US_RndT() & 0x01)]))
    {
        UpdateScreen(0);
        SD_MusicOff();
        SD_StopSound();
        MenuFadeOut();
        while(1){}; /* hang system */
    }

    DrawMainMenu();
    return 0;
}

/*
================================================================
=
= Function: Confirm
=
= Description:
=
= DRAW DIALOG AND CONFIRM YES OR NO TO QUESTION
=
================================================================
*/
static S32 Confirm(const char *string)
{
    S32 xit = 0;
    S32 x;
    S32 y;
    U8 tick = 0;
    U32 lastBlinkTime;
    S32 whichsnd[2] = { ESCPRESSEDSND, SHOOTSND };
    ControlInfo ci;

    Message(string);

    /* BLINK CURSOR */
    x = PrintX;
    y = PrintY;
    lastBlinkTime = GetTimeCount();

    do
    {
         /* clear last button read */
        ci.button0 = 0;
        ci.button1 = 0;
        ci.button2 = 0;
        ci.button3 = 0;
        ci.dir = dir_None;
        ci.straferight = 0;
        ci.strafeleft = 0;
        ci.pause = 0;
        ci.esc = 0;

        ReadAnyControl(&ci);

        if((GetTimeCount() - lastBlinkTime) >= 10)
        {
            switch(tick)
            {
                case 0:
                    VL_Bar(x, y, 8, 13, TEXTCOLOR);
                break;

                case 1:
                    PrintX = x;
                    PrintY = y;
                    US_Print("_");
                break;

                default:
                    /* do nothing */
                break;
            }
            UpdateScreen(0);
            tick ^= 1;
            lastBlinkTime = GetTimeCount();
        }
        else
        {
            Delay_ms(5);
        }

    }while((ci.button0 == 0) && (ci.button1 == 0));

    if(ci.button0 == 1)
    {
        xit = 1;
    }

    SD_PlaySound((soundnames) whichsnd[xit]);

    return xit;
}

/*
================================================================
=
= Function: Message
=
= Description:
=
= PRINT A MESSAGE IN A WINDOW
=
================================================================
*/
void Message(const char *string)
{
    S32 h = 0;
    S32 w = 0;
    S32 mw = 0;
    S32 i;
    S32 len = (S32) strlen(string);
    fontstruct *font;

    CA_CacheGrChunk(STARTFONT + 1);
    fontnumber = 1;
    font = (fontstruct *) grsegs[STARTFONT + fontnumber];
    h = font->height;

    for(i = 0; i < len; i++)
    {
        if (string[i] == '\n')
        {
            if (w > mw)
            {
                mw = w;
            }
            w = 0;
            h += font->height;
        }
        else
        {
            w += font->width[(S8)string[i]];
        }
    }

    if(w + 10 > mw)
    {
        mw = w + 10;
    }

    PrintY = (WindowH / 2) - h / 2;
    PrintX = WindowX = 160 - mw / 2;

    DrawWindow(WindowX - 5, PrintY - 5, mw + 10, h + 10, TEXTCOLOR);
    DrawOutline(WindowX - 5, PrintY - 5, mw + 10, h + 10, 0, HIGHLIGHT);
    SetFontColor(0, TEXTCOLOR);
    US_Print(string);
    UpdateScreen(0);
}

/*
================================================================
=
= Function: CP_ChangeView
=
= Description:
=
= CHANGE SCREEN VIEWING SIZE
=
================================================================
*/
S32 CP_ChangeView(S32 temp)
{
    S32 exit = 0;
    S32 oldview;
    S32 newview;
    ControlInfo ci;

    WindowX = WindowY = 0;
    WindowW = 320;
    WindowH = 200;
    newview = oldview = viewsize;
    DrawChangeView(oldview);
    MenuFadeIn();

    do
    {
        Delay_ms(10);
        /* clear last button read */
        ci.button0 = 0;
        ci.button1 = 0;
        ci.button2 = 0;
        ci.button3 = 0;
        ci.dir = dir_None;
        ci.straferight = 0;
        ci.strafeleft = 0;
        ci.pause = 0;
        ci.esc = 0;

        ReadAnyControl (&ci);

        switch (ci.dir)
        {
            case dir_South:
            case dir_West:
                newview--;
                if (newview < 4)
                {
                    newview = 4;
                }

                if(newview >= 19)
                {
                    DrawChangeView(newview);
                }
                else
                {
                    ShowViewSize(newview);
                }

                UpdateScreen(0);
                SD_PlaySound (HITWALLSND);
                TicDelay(10);
            break;

            case dir_North:
            case dir_East:
                newview++;
                if (newview >= 21)
                {
                    newview = 21;
                    DrawChangeView(newview);
                }
                else
                {
                    ShowViewSize (newview);
                }
                UpdateScreen(0);
                SD_PlaySound (HITWALLSND);
                TicDelay(10);
            break;

            default:
                /* do nothing */
            break;
        }

        if(ci.button0 == 1)
        {
            exit = 1;
        }
        else if(ci.button1 == 1)
        {
            SD_PlaySound(ESCPRESSEDSND);
            MenuFadeOut ();
            return 0;
        }
    }
    while(!exit);

    if(oldview != newview)
    {
        SD_PlaySound(SHOOTSND);
        Message(STR_THINK "...");
        NewViewSize(newview);
    }

    SD_PlaySound(SHOOTSND);
    MenuFadeOut();
    return 0;
}

/*
================================================================
=
= Function: DrawChangeView
=
= Description:
=
= DRAW THE CHANGEVIEW SCREEN
=
================================================================
*/
static void DrawChangeView(S32 view)
{
    if(view != 21)
    {
        VL_Bar(0, SCREENHEIGHT - 40, 320, 40, VIEWCOLOR);
    }

    ShowViewSize(view);

    PrintY = SCREENHEIGHT - 39;
    WindowX = 0;
    WindowY = 320;
    SetFontColor(HIGHLIGHT, BKGDCOLOR);

    US_CPrint(STR_SIZE1 "\n");
    US_CPrint(STR_SIZE2 "\n");
    US_CPrint(STR_SIZE3);
    UpdateScreen(0);
}

/*
================================================================
=
= Function: CP_ReadThis
=
= Description:
=
= READ THIS!
=
================================================================
*/
S32 CP_ReadThis (S32 temp)
{
    StartCPMusic(CORNER_MUS);
    HelpScreens();
    StartCPMusic(MENUSONG);
    return 1;
}

/*
================================================================
=
= Function: CP_Control
=
= Description:
=
= DEFINE CONTROLS
=
================================================================
*/
S32 CP_Control(S32 temp)
{
    S32 which;

    DrawCtlScreen();
    MenuFadeIn();

    do
    {
        which = HandleMenu (&CtlItems, CtlMenu, NULL);
        switch (which)
        {
            case CTL_CUSTOMIZE:
                DrawCtlScreen();
                MenuFadeIn();
            break;

            default:
                /* do nothing */
            break;
        }
    }
    while (which >= 0);

    MenuFadeOut ();

    return 0;
}

/*
================================================================
=
= Function: DrawCtlScreen
=
= Description:
=
= DRAW CONTROL MENU SCREEN
=
================================================================
*/
static void DrawCtlScreen(void)
{
    S32 i;
    S32 x;
    S32 y;

    ClearMScreen();
    DrawStripes(10);
    VWB_DrawPic(80, 0, C_CONTROLPIC);
    VWB_DrawPic(112, 184, C_MOUSELBACKPIC);
    DrawWindow(CTL_X - 8, CTL_Y - 5, CTL_W, CTL_H, BKGDCOLOR);

    WindowX = 0;
    WindowW = 320;
    SetFontColor(TEXTCOLOR, BKGDCOLOR);

    DrawMenu (&CtlItems, CtlMenu);

    x = CTL_X + CtlItems.indent - 24;
    y = CTL_Y + 3;
    /* No mouse*/
    VWB_DrawPic (x, y, C_NOTSELECTEDPIC);

    /* joystick enabled */
    y = CTL_Y + 29;
    VWB_DrawPic(x, y, C_SELECTEDPIC);

    /* PICK FIRST AVAILABLE SPOT */
    if((CtlItems.curpos < 0) || (!CtlMenu[CtlItems.curpos].active))
    {
        for (i = 0; i < CtlItems.amount; i++)
        {
            if (CtlMenu[i].active)
            {
                CtlItems.curpos = i;
                break;
            }
        }
    }

    DrawMenuGun(&CtlItems);
    UpdateScreen(0);
}

/*
================================================================
=
= Function: CustomControls
=
= Description:
=
= HANDLE CUSTOMIZE CONTROLS MENu OPTIONS
=
================================================================
*/
S32 CustomControls (S32 temp)
{
    S32 which;

    DrawCustomScreen();

    do
    {
        which = HandleMenu(&CusItems, &CusMenu[0], FixupCustom);

        switch (which)
        {
            case 3:
                DefineJoyBtns();
                DrawCustJoy(0);
            break;

            default:
                /* do nothing */
            break;
        }

    }
    while(which >= 0);

    MenuFadeOut ();

    return 0;
}

/*
================================================================
=
= Function: DrawCustomScreen
=
= Description:
=
= DRAW CUSTOMIZE SCREEN
=
================================================================
*/
static void DrawCustomScreen(void)
{
    S32 i;

    ClearMScreen();
    WindowX = 0;
    WindowW = 320;
    VWB_DrawPic(112, 184, C_MOUSELBACKPIC);
    DrawStripes(10);
    VWB_DrawPic(80, 0, C_CUSTOMIZEPIC);

    /* MOUSE */
    SetFontColor(READCOLOR, BKGDCOLOR);
    WindowX = 0;
    WindowW = 320;

    PrintY = CST_Y;
    US_CPrint ("Mouse\n");

    SetFontColor(TEXTCOLOR, BKGDCOLOR);

    PrintX = CST_START;
    US_Print (STR_CRUN);
    PrintX = CST_START + CST_SPC * 1;
    US_Print (STR_COPEN);
    PrintX = CST_START + CST_SPC * 2;
    US_Print (STR_CFIRE);
    PrintX = CST_START + CST_SPC * 3;
    US_Print(STR_CSTRAFE "\n");

    DrawWindow(5, PrintY - 1, 310, 13, BKGDCOLOR);
    DrawCustMouse(0);
    US_Print ("\n");

    /* JOYSTICK/PAD */
    SetFontColor(READCOLOR, BKGDCOLOR);
    US_CPrint ("Joystick/Gravis GamePad\n");

    SetFontColor(TEXTCOLOR, BKGDCOLOR);

    PrintX = CST_START;
    US_Print (STR_CRUN);
    PrintX = CST_START + CST_SPC * 1;
    US_Print (STR_COPEN);
    PrintX = CST_START + CST_SPC * 2;
    US_Print (STR_CFIRE);
    PrintX = CST_START + CST_SPC * 3;
    US_Print (STR_CSTRAFE "\n");

    DrawWindow(5, PrintY - 1, 310, 13, BKGDCOLOR);
    DrawCustJoy(0);
    US_Print ("\n");

    /* PICK STARTING POINT IN MENU */
    if (CusItems.curpos < 0)
    {
        for (i = 0; i < CusItems.amount; i++)
        {
            if(CusMenu[i].active == 1)
            {
                CusItems.curpos = i;
                break;
            }
        }
    }

    UpdateScreen(0);
    MenuFadeIn();
}

/*
================================================================
=
= Function: DrawCustJoy
=
= Description:
=
= HANDLE DRAWING OF CUSTOMIZED JOYPAD BUTTONS
=
================================================================
*/
static void DrawCustJoy(S32 hilight)
{
    S32 i;
    S32 color;

    color = TEXTCOLOR;

    if(hilight == 1)
    {
        color = HIGHLIGHT;
    }

    SetFontColor(color, BKGDCOLOR);

    PrintY = CST_Y + 13 * 5;
    for (i = 0; i < 4; i++)
    {
        PrintCustJoy(i);
    }
}

/*
================================================================
=
= Function: PrintCustJoy
=
= Description:
=
= DRAW CUSTOMIZE JOYPAD BUTTONS ON SCREEN
=
================================================================
*/
static void PrintCustJoy(S32 i)
{
    S32 j;

    for(j = 0; j < 4; j++)
    {
        if (order[i] == buttonjoy[j])
        {
            PrintX = CST_START + CST_SPC * i;
            US_Print(mbarray[j]);
            break;
        }
    }
}

/*
================================================================
=
= Function: DrawCustMouse
=
= Description:
=
= HANDLE DRAWING OF CUSTOMIZED MOUSE BUTTON
=
================================================================
*/
static void DrawCustMouse (S32 hilight)
{
    S32 i;

    SetFontColor(DEACTIVE, BKGDCOLOR);
    CusMenu[0].active = 0;

    PrintY = CST_Y + 13 * 2;

    for (i = 0; i < 4; i++)
    {
        PrintCustMouse(i);
    }
}

/*
================================================================
=
= Function: PrintCustMouse
=
= Description:
=
= DRAW CUSTOMIZE MOUSE BUTTONS ON SCREEN
=
================================================================
*/
static void PrintCustMouse(S32 i)
{
    S32 j;

    for (j = 0; j < 4; j++)
    {
        if (order[i] == buttonmouse[j])
        {
            PrintX = CST_START + CST_SPC * i;
            US_Print(mbarray[j]);
            break;
        }
    }
}

/*
================================================================
=
= Function: FixupCustom
=
= Description:
=
= FIXUP GUN CURSOR OVERDRAW (GUN background is draw as a square
= which dont look right in custom menu so this routine corrects
= this)
=
================================================================
*/
void FixupCustom(S32 w)
{
    static S32 lastwhich = -1;
    S32 y = CST_Y + 26 + w * 13;

    VWB_Hlin (7, 32, y - 1, DEACTIVE);
    VWB_Hlin (7, 32, y + 12, BORD2COLOR);

    VWB_Hlin (7, 32, y - 2, BORDCOLOR);
    VWB_Hlin (7, 32, y + 13, BORDCOLOR);

    switch(w)
    {
        case 0:
            DrawCustMouse (1);
        break;

        case 3:
            DrawCustJoy (1);
        break;

        default:
            /* no nothing */
        break;
    }

    if(lastwhich >= 0)
    {
        y = CST_Y + 26 + lastwhich * 13;
        VWB_Hlin (7, 32, y - 1, DEACTIVE);
        VWB_Hlin (7, 32, y + 12, BORD2COLOR);

        VWB_Hlin (7, 32, y - 2, BORDCOLOR);
        VWB_Hlin (7, 32, y + 13, BORDCOLOR);

        if(lastwhich != w)
        {
            switch (lastwhich)
            {
                case 0:
                    DrawCustMouse (0);
                break;

                case 3:
                    DrawCustJoy (0);
                break;

                default:
                    /* do nothing */
                break;

            }
        }
    }

    lastwhich = w;
}

/*
================================================================
=
= Function: DefineJoyBtns
=
= Description:
=
= DEFINE THE JOYSTICK BUTTONS
=
================================================================
*/
static void DefineJoyBtns(void)
{
    CustomCtrls joyallowed = {{1, 1, 1, 1}};
    EnterCtrlData (5, &joyallowed, DrawCustJoy, PrintCustJoy, JOYSTICK);
}

/*
================================================================
=
= Function: EnterCtrlData
=
= Description:
=
= ENTER CONTROL DATA FOR ANY TYPE OF CONTROL
=
================================================================
*/
static void EnterCtrlData (S32 index, CustomCtrls * cust, void (*DrawRtn) (S32), void (*PrintRtn) (S32), S32 type)
{
    S32 j;
    S32 exit;
    U8 tick;
    S32 redraw;
    S32 which = 0;
    S32 x = 0;
    S32 picked;
    S32 lastFlashTime;
    ControlInfo ci;
    S32 result;
    S32 z;

    SD_PlaySound(SHOOTSND);
    PrintY = CST_Y + 13 * index;
    exit = 0;
    redraw = 1;

    /* FIND FIRST SPOT IN ALLOWED ARRAY */
    for (j = 0; j < 4; j++)
    {
        if (cust->allowed[j] == 1)
        {
            which = j;
            break;
        }
    }

    do
    {
        if(redraw == 1)
        {
            x = CST_START + CST_SPC * which;
            DrawWindow (5, PrintY - 1, 310, 13, BKGDCOLOR);

            DrawRtn(1);
            DrawWindow(x - 2, PrintY, CST_SPC, 11, TEXTCOLOR);
            DrawOutline(x - 2, PrintY, CST_SPC, 11, 0, HIGHLIGHT);
            SetFontColor(0, TEXTCOLOR);
            PrintRtn(which);
            PrintX = x;
            SetFontColor(TEXTCOLOR, BKGDCOLOR);
            UpdateScreen(0);
            redraw = 0;
        }

        Delay_ms(10);
        /* clear last button read */
        ci.button0 = 0;
        ci.button1 = 0;
        ci.button2 = 0;
        ci.button3 = 0;
        ci.dir = dir_None;
        ci.straferight = 0;
        ci.strafeleft = 0;
        ci.pause = 0;
        ci.esc = 0;
        ReadAnyControl(&ci);

        /* CHANGE BUTTON VALUE? */
        if((ci.button0 == 1) || (ci.button1 == 1) || (ci.button2 == 1) || (ci.button3 == 1))
        {
            lastFlashTime = GetTimeCount();
            tick = 0;
            picked = 0;
            SetFontColor(0, TEXTCOLOR);

            while(1)
            {
                result = 0;

                /* clear last button read */
                ci.button0 = 0;
                ci.button1 = 0;
                ci.button2 = 0;
                ci.button3 = 0;
                ci.dir = dir_None;
                ReadAnyControl(&ci);

                /* FLASH CURSOR */
                if((GetTimeCount() - lastFlashTime) > 10)
                {
                    switch (tick)
                    {
                        case 0:
                            VL_Bar(x, PrintY + 1, CST_SPC - 2, 10, TEXTCOLOR);
                        break;

                        case 1:
                            PrintX = x;
                            US_Print ("?");
                            SD_PlaySound(HITWALLSND);
                        break;

                        default:
                            /* do nothing */
                        break;
                    }

                    tick ^= 1;
                    lastFlashTime = GetTimeCount();
                    UpdateScreen(0);
                }
                else
                {
                    Delay_ms(5);
                }

                /* WHICH TYPE OF INPUT DO WE PROCESS? */
                switch(type)
                {
                    case JOYSTICK:
                        if(ci.button0 == 1)
                        {
                            result = 1;
                        }
                        else if(ci.button1 == 1)
                        {
                            result = 2;
                        }
                        else if(ci.button2 == 1)
                        {
                            result = 3;
                        }
                        else if(ci.button3 == 1)
                        {
                            result = 4;
                        }

                        if(result != 0)
                        {
                            for(z = 0; z < 4; z++)
                            {
                                if(order[which] == buttonjoy[z])
                                {
                                    buttonjoy[z] = bt_nobutton;
                                    break;
                                }
                            }

                            buttonjoy[result - 1] = order[which];
                            picked = 1;
                            SD_PlaySound(SHOOTDOORSND);
                        }
                    break;

                    default:
                        /* do nothing */
                    break;
                }

                /* EXIT INPUT? */
                if(picked == 1)
                {
                    break;
                }

            }

            SetFontColor(TEXTCOLOR, BKGDCOLOR);
            redraw = 1;
            continue;
        }

        if(ci.button1 == 1)
        {
            exit = 1;
        }

        /* MOVE TO ANOTHER SPOT? */
        switch(ci.dir)
        {
            case dir_West:
                do
                {
                    which--;
                    if (which < 0)
                    {
                        which = 3;
                    }
                }
                while(cust->allowed[which] == 0);

                redraw = 1;
                SD_PlaySound(MOVEGUN1SND);

                while(ci.dir != dir_None)
                {
                     /* clear last button read */
                    ci.button0 = 0;
                    ci.button1 = 0;
                    ci.button2 = 0;
                    ci.button3 = 0;
                    ci.dir = dir_None;
                    ReadAnyControl(&ci);
                    Delay_ms(5);
                }
            break;

            case dir_East:
                do
                {
                    which++;
                    if (which > 3)
                    {
                        which = 0;
                    }
                }
                while (cust->allowed[which] == 0);

                redraw = 1;
                SD_PlaySound(MOVEGUN1SND);

                while(ci.dir != dir_None)
                {
                     /* clear last button read */
                    ci.button0 = 0;
                    ci.button1 = 0;
                    ci.button2 = 0;
                    ci.button3 = 0;
                    ci.dir = dir_None;
                    ReadAnyControl(&ci);
                    Delay_ms(5);
                }

            break;

            case dir_North:
            case dir_South:
                exit = 1;
            break;

            default:
                /* do nothing */
            break;
        }

    }
    while(exit == 0);

    SD_PlaySound(ESCPRESSEDSND);
    DrawWindow (5, PrintY - 1, 310, 13, BKGDCOLOR);
}

/*
================================================================
=
= Function: CP_NewGame
=
= Description:
=
= START A NEW GAME
=
================================================================
*/
S32 CP_NewGame(S32 temp)
{
    S32 which;
    S32 episode;

    firstpart:
    DrawNewEpisode();

    do
    {
        which = HandleMenu(&NewEitems, &NewEmenu[0], NULL);
        switch (which)
        {
            case -1:
                MenuFadeOut();
                return 0;
            break;

            default:
                if(EpisodeSelect[which / 2] == 0)
                {
                    SD_PlaySound (NOWAYSND);
                    Message("Please select \"Read This!\"\n"
                             "from the Options menu to\n"
                             "find out how to order this\n" "episode from Apogee.");
                    IN_Ack ();
                    DrawNewEpisode();
                    which = 0;
                }
                else
                {
                    episode = which / 2;
                    which = 1;
                }
            break;
        }
    }
    while(which == 0);

    SD_PlaySound(SHOOTSND);

    /* ALREADY IN A GAME? */
    if(ingame == 1)
    {
        if(Confirm(CURGAME) == 0)
        {
            MenuFadeOut();
            return 0;
        }
    }

    MenuFadeOut();

    DrawNewGame();
    which = HandleMenu(&NewItems, &NewMenu[0], DrawNewGameDiff);

    if (which < 0)
    {
        MenuFadeOut();
        goto firstpart;
    }

    SD_PlaySound(SHOOTSND);
    NewGame(which, episode);
    ExitMenu = 1;
    MenuFadeOut();

    /* CHANGE "READ THIS!" TO NORMAL COLOR */
    MainMenu[readthis].active = 1;

    return 0;
}

/*
================================================================
=
= Function: DrawNewGame
=
= Description:
=
= DRAW NEW GAME MENU
=
================================================================
*/
static void DrawNewGame(void)
{
    ClearMScreen();
    VWB_DrawPic(112, 184, C_MOUSELBACKPIC);

    SetFontColor(READHCOLOR, BKGDCOLOR);
    PrintX = NM_X + 20;
    PrintY = NM_Y - 32;

    US_Print ("How tough are you?");

    DrawWindow (NM_X - 5, NM_Y - 10, NM_W, NM_H, BKGDCOLOR);

    DrawMenu (&NewItems, &NewMenu[0]);
    DrawNewGameDiff(NewItems.curpos);
    UpdateScreen(0);
    MenuFadeIn();
}

/*
================================================================
=
= Function: DrawNewGameDiff
=
= Description:
=
= DRAW NEW GAME GRAPHIC
=
================================================================
*/
static void DrawNewGameDiff(S32 w)
{
    VWB_DrawPic(NM_X + 185, NM_Y + 7, w + C_BABYMODEPIC);
}

/*
================================================================
=
= Function: DrawNewEpisode
=
= Description:
=
= DRAW NEW EPISODE MENU
=
================================================================
*/
static void DrawNewEpisode(void)
{
    U8 i;

    ClearMScreen ();
    VWB_DrawPic(112, 184, C_MOUSELBACKPIC);

    DrawWindow(NE_X - 4, NE_Y - 4, NE_W + 8, NE_H + 8, BKGDCOLOR);
    SetFontColor(READHCOLOR, BKGDCOLOR);
    PrintY = 2;
    WindowX = 0;

    US_CPrint("Which episode to play?");

    SetFontColor(TEXTCOLOR, BKGDCOLOR);
    DrawMenu(&NewEitems, &NewEmenu[0]);

    for (i = 0; i < 6; i++)
    {
        VWB_DrawPic(NE_X + 32, NE_Y + i * 26, C_EPISODE1PIC + i);
    }

    UpdateScreen(0);
    MenuFadeIn();
}

/*
================================================================
=
= Function: CP_LoadGame
=
= Description:
=
= Load saved game
=
================================================================
*/
S32 CP_LoadGame(S32 quick)
{
    FILE *file;
    S32 which;
    S32 exit = 0;
    char name[13];
    /* FIXME: buffer length */
    char loadpath[30] = "/dswolf/";

    strcpy(name, SaveName);

    DrawLoadSaveScreen(0);

    do
    {
        which = HandleMenu(&LSItems, &LSMenu[0], TrackWhichGame);

        if((which >= 0) && (SaveGamesAvail[which] == 1))
        {
            SD_PlaySound(SHOOTSND);
            name[7] = which + '0';

            strcat(loadpath,name);

            file = fopen(loadpath, "rb");
            fseek(file, 32, SEEK_SET);

            DrawLSAction(0);
            loadedgame = 1;

            LoadTheGame(file, LSA_X + 8, LSA_Y + 5);
            fclose(file);

            ExitMenu = 1;
            SD_PlaySound(SHOOTSND);

            /* CHANGE "READ THIS!" TO NORMAL COLOR */
            MainMenu[readthis].active = 1;

            exit = 1;
            break;
        }

    }while(which >= 0);

    MenuFadeOut();

    return exit;
}

/*
================================================================
=
= Function: CP_SaveGame
=
= Description:
=
= SAVE CURRENT GAME
=
================================================================
*/
S32 CP_SaveGame (S32 quick)
{
    S32 which;
    S32 exit = 0;
    FILE *file;
    char name[13];
    char input[32] = "";
    /* FIXME: buffer length */
    char savepath[30] = "/dswolf/";

    strcpy (name, SaveName);

    DrawLoadSaveScreen(1);

    do
    {
        which = HandleMenu(&LSItems, &LSMenu[0], TrackWhichGame);
        if(which >= 0)
        {
            /* OVERWRITE EXISTING SAVEGAME? */
            if(SaveGamesAvail[which] == 1)
            {
                if(Confirm (GAMESVD) == 0)
                {
                    DrawLoadSaveScreen(1);
                    continue;
                }
                else
                {
                    DrawLoadSaveScreen(1);
                    PrintLSEntry(which, HIGHLIGHT);
                    UpdateScreen(0);
                }
            }
            else
            {
                SD_PlaySound(SHOOTSND);
            }

            strcpy (input, &SaveGameNames[which][0]);
            name[7] = which + '0';

            SetFontNum(0);

            if(SaveGamesAvail[which] == 0)
            {
                VL_Bar(LSM_X + LSItems.indent + 1, LSM_Y + which * 13 + 1,
                         LSM_W - LSItems.indent - 16, 9, BKGDCOLOR);
            }

            UpdateScreen(0);

            if (US_LineInput(LSM_X + LSItems.indent + 2, LSM_Y + which * 13 + 1, input, 31,
                 LSM_W - LSItems.indent - 30))
            {
                SaveGamesAvail[which] = 1;
                strcpy (&SaveGameNames[which][0], input);

                strcat(savepath, name);

                file = fopen (savepath, "wb");
                fwrite (input, 32, 1, file);
                fseek (file, 32, SEEK_SET);

                DrawLSAction(1);
                SaveTheGame(file, LSA_X + 8, LSA_Y + 5);

                fclose (file);

                SD_PlaySound(SHOOTSND);
                exit = 1;
            }
            else
            {
                VL_Bar(LSM_X + LSItems.indent + 1, LSM_Y + which * 13 + 1,
                         LSM_W - LSItems.indent - 16, 10, BKGDCOLOR);
                PrintLSEntry (which, HIGHLIGHT);
                UpdateScreen(0);
                SD_PlaySound (ESCPRESSEDSND);
                continue;
            }

            SetFontNum(1);
            break;
        }

    }while(which >= 0);

    MenuFadeOut ();

    return exit;
}

/*
================================================================
=
= Function: TrackWhichGame
=
= Description:
=
= HIGHLIGHT CURRENT SELECTED ENTRY
=
================================================================
*/
static void TrackWhichGame (S32 w)
{
    static S32 lastgameon = 0;

    PrintLSEntry (lastgameon, TEXTCOLOR);
    PrintLSEntry (w, HIGHLIGHT);

    lastgameon = w;
}

/*
================================================================
=
= Function: DrawLSAction
=
= Description:
=
= DRAW LOAD/SAVE IN PROGRESS
=
================================================================
*/
static void DrawLSAction(S32 which)
{

    DrawWindow (LSA_X, LSA_Y, LSA_W, LSA_H, TEXTCOLOR);
    DrawOutline (LSA_X, LSA_Y, LSA_W, LSA_H, 0, HIGHLIGHT);
    VWB_DrawPic (LSA_X + 8, LSA_Y + 5, C_DISKLOADING1PIC);

    SetFontNum(1);
    SetFontColor(0, TEXTCOLOR);
    PrintX = LSA_X + 46;
    PrintY = LSA_Y + 13;

    if(which == 0)
    {
        US_Print (STR_LOADING "...");
    }
    else
    {
        US_Print (STR_SAVING "...");
    }

    UpdateScreen(0);
}

/*
================================================================
=
= Function: DrawLoadSaveScreen
=
= Description:
=
= DRAW THE LOAD/SAVE SCREEN
=
================================================================
*/
static void DrawLoadSaveScreen(S32 loadsave)
{

    S32 i;

    ClearMScreen ();
    SetFontNum(1);
    VWB_DrawPic (112, 184, C_MOUSELBACKPIC);
    DrawWindow (LSM_X - 10, LSM_Y - 5, LSM_W, LSM_H, BKGDCOLOR);
    DrawStripes (10);

    if(loadsave == 0)
    {
        VWB_DrawPic (60, 0, C_LOADGAMEPIC);
    }
    else
    {
        VWB_DrawPic (60, 0, C_SAVEGAMEPIC);
    }

    for (i = 0; i < 10; i++)
    {
        PrintLSEntry(i, TEXTCOLOR);
    }

    DrawMenu(&LSItems, &LSMenu[0]);
    UpdateScreen(0);
    MenuFadeIn();
}

/*
================================================================
=
= Function: PrintLSEntry
=
= Description:
=
= PRINT LOAD/SAVE GAME ENTRY With BOX OUTLINE
=
================================================================
*/
static void PrintLSEntry (S32 w, S32 color)
{
    SetFontColor(color, BKGDCOLOR);
    DrawOutline (LSM_X + LSItems.indent, LSM_Y + w * 13, LSM_W - LSItems.indent - 15, 10, color, color);
    PrintX = LSM_X + LSItems.indent + 2;
    PrintY = LSM_Y + w * 13 + 1;
    SetFontNum(0);

    if(SaveGamesAvail[w] == 1)
    {
        US_Print(SaveGameNames[w]);
    }
    else
    {
        US_Print("      - " STR_EMPTY " -");
    }

    SetFontNum(1);
}

/*
================================================================
=
= Function: SetupSaveGames
=
= Description:
=
= SEE WHICH SAVE GAME FILES ARE AVAILABLE & READ STRING IN
=
================================================================
*/
void SetupSaveGames(void)
{
    char name[13];
    char savepath[100];
    FILE *file = NULL;
    S32 i;

    strcpy(name, SaveName);

    for(i = 0; i < 10; i++)
    {
        name[7] = '0' + i;

        strcpy(savepath,"/dswolf/");
        strcat(savepath, name);

        file = fopen(savepath,"rb");

        if(file != NULL)
        {
            char temp[32];

            SaveGamesAvail[i] = 1;
            fread(temp,1, 32,file);
            fclose(file);
            strcpy(&SaveGameNames[i][0], temp);
        }

    }
}
