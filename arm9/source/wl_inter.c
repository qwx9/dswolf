#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "types.h"
#include "audiowl6.h"
#include "wl_inter.h"
#include "wl_def.h"
#include "id_ca.h"
#include "id_vl.h"
#include "id_vh.h"
#include "id_in.h"
#include "id_sd.h"
#include "id_us.h"
#include "gfxv_apo.h"
#include "wl_menu.h"
#include "wl_game.h"
#include "id_time.h"
#include "wl_agent.h"
#include "wl_text.h"

/* macro defines */

#define MaxScores   7
#define STR_BONUS   "bonus"
#define STR_TIME    "time"
#define STR_PAR     " par"
#define STR_RAT2KILL            "kill ratio    %"
#define STR_RAT2SECRET          "secret ratio    %"
#define STR_RAT2TREASURE        "treasure ratio    %"
#define VBLWAIT 30
#define PAR_AMOUNT      500
#define PERCENT100AMT   10000
#define RATIOXX         37
#define VW_WaitVBL(a)   Delay_ms((a)*8)
#define RATIOX  6
#define RATIOY  14
#define TIMEX   14
#define TIMEY   8
#define STR_YOUWIN      "you win!"
#define STR_TOTALTIME   "total time"
#define STR_RATKILL     "kill    %"
#define STR_RATSECRET   "secret    %"
#define STR_RATTREASURE "treasure    %"

/* external variables */

extern U16 PrintX;
extern U16 PrintY;
extern gametype gamestate;
extern char *DS_USERNAME;
extern S32  mapon;
extern U16 WindowX;
extern U16 WindowY;
extern U16 WindowW;
extern U16 WindowH;

/* local variables */

static HighScore Scores[MaxScores] =
{
    {"id software-'92",10000,1},
    {"Adrian Carmack",10000,1},
    {"John Carmack",10000,1},
    {"Kevin Cloud",10000,1},
    {"Tom Hall",10000,1},
    {"John Romero",10000,1},
    {"Jay Wilbur",10000,1},
};

static const times_type parTimes[] =
{
        /* Episode One Par Times */
        {1.5 * 4200, "01:30"},
        {2 * 4200, "02:00"},
        {2 * 4200, "02:00"},
        {3.5 * 4200, "03:30"},
        {3 * 4200, "03:00"},
        {3 * 4200, "03:00"},
        {2.5 * 4200, "02:30"},
        {2.5 * 4200, "02:30"},
        {0 * 4200, "??:??"},           /* Boss level */
        {0 * 4200, "??:??"},           /* Secret level */

        /* Episode Two Par Times */
        {1.5 * 4200, "01:30"},
        {3.5 * 4200, "03:30"},
        {3 * 4200, "03:00"},
        {2 * 4200, "02:00"},
        {4 * 4200, "04:00"},
        {6 * 4200, "06:00"},
        {1 * 4200, "01:00"},
        {3 * 4200, "03:00"},
        {0 * 4200, "??:??"},
        {0 * 4200, "??:??"},

        /* Episode Three Par Times */
        {1.5 * 4200, "01:30"},
        {1.5 * 4200, "01:30"},
        {2.5 * 4200, "02:30"},
        {2.5 * 4200, "02:30"},
        {3.5 * 4200, "03:30"},
        {2.5 * 4200, "02:30"},
        {2 * 4200, "02:00"},
        {6 * 4200, "06:00"},
        {0 * 4200, "??:??"},
        {0 * 4200, "??:??"},

        /* Episode Four Par Times */
        {2 * 4200, "02:00"},
        {2 * 4200, "02:00"},
        {1.5 * 4200, "01:30"},
        {1 * 4200, "01:00"},
        {4.5 * 4200, "04:30"},
        {3.5 * 4200, "03:30"},
        {2 * 4200, "02:00"},
        {4.5 * 4200, "04:30"},
        {0 * 4200, "??:??"},
        {0 * 4200, "??:??"},

        /* Episode Five Par Times */
        {2.5 * 4200, "02:30"},
        {1.5 * 4200, "01:30"},
        {2.5 * 4200, "02:30"},
        {2.5 * 4200, "02:30"},
        {4 * 4200, "04:00"},
        {3 * 4200, "03:00"},
        {4.5 * 4200, "04:30"},
        {3.5 * 4200, "03:30"},
        {0 * 4200, "??:??"},
        {0 * 4200, "??:??"},

        /* Episode Six Par Times */
        {6.5 * 4200, "06:30"},
        {4 * 4200, "04:00"},
        {4.5 * 4200, "04:30"},
        {6 * 4200, "06:00"},
        {5 * 4200, "05:00"},
        {5.5 * 4200, "05:30"},
        {5.5 * 4200, "05:30"},
        {8.5 * 4200, "08:30"},
        {0 * 4200, "??:??"},
        {0 * 4200, "??:??"}
};

static const S32 alpha[] =
 {
        L_NUM0PIC, L_NUM1PIC, L_NUM2PIC, L_NUM3PIC, L_NUM4PIC, L_NUM5PIC,
        L_NUM6PIC, L_NUM7PIC, L_NUM8PIC, L_NUM9PIC, L_COLONPIC, 0, 0, 0, 0, 0, 0, L_APIC, L_BPIC,
        L_CPIC, L_DPIC, L_EPIC, L_FPIC, L_GPIC, L_HPIC, L_IPIC, L_JPIC, L_KPIC,
        L_LPIC, L_MPIC, L_NPIC, L_OPIC, L_PPIC, L_QPIC, L_RPIC, L_SPIC, L_TPIC,
        L_UPIC, L_VPIC, L_WPIC, L_XPIC, L_YPIC, L_ZPIC
};

static S32 lastBreathTime = 0;

/* global variables */

LRstruct LevelRatios[LRpack];

/* local prototypes */

static void Write(S32 x, S32 y, const char *string);
static void BJ_Breathe (void);
static void ClearSplitVWB(void);
static U8 PreloadUpdate(U32 current, U32 total);

/*
================================================================
=
= Function: PG13
=
= Description:
=
= Prints PG13 banner to screen at startup
=
================================================================
*/
void PG13(void)
{
    VW_FadeOut();
    VL_Bar(0, 0, 320, 200, 0x82);     /* background */

    CA_CacheGrChunk(PG13PIC);
    VWB_DrawPic(216, 110, PG13PIC);
    UpdateScreen(0);

    CA_UnCacheGrChunk(PG13PIC);

    VW_FadeIn();

    IN_UserInput(TickBase * 7);

    VW_FadeOut();
}

/*
================================================================
=
= Function: DrawHighScores
=
= Description:
=
= Draws high scores to screen
=
================================================================
*/
void DrawHighScores(void)
{
    char buffer[16];
    char *str;

    U16 i;
    U16 w;
    U16 h;
    HighScore *s;

    CA_CacheGrChunk(HIGHSCORESPIC);
    CA_CacheGrChunk(STARTFONT);
    CA_CacheGrChunk(C_LEVELPIC);
    CA_CacheGrChunk(C_SCOREPIC);
    CA_CacheGrChunk(C_NAMEPIC);

    ClearMScreen();
    DrawStripes(10);

    VWB_DrawPic(48, 0, HIGHSCORESPIC);
    CA_UnCacheGrChunk(HIGHSCORESPIC);

    VWB_DrawPic(4 * 8, 68, C_NAMEPIC);
    VWB_DrawPic(20 * 8, 68, C_LEVELPIC);
    VWB_DrawPic(28 * 8, 68, C_SCOREPIC);

    SetFontNum(0);

    SetFontColor(15, 0x29);

    s = Scores;

    for (i = 0; i < MaxScores; i++, s++)
    {
        PrintY = 76 + (16 * i);

        /* display name */
        PrintX = 4 * 8;
        US_Print(s->name);

        /* display level */
        sprintf(buffer,"%d",s->completed);

        for (str = buffer; *str != '\0'; str++)
        {
            *str = *str + (129 - '0');  /* Used fixed-width numbers (129...) */
        }

        VW_MeasurePropString(buffer, &w, &h);
        PrintX = (22 * 8) - w;

        US_Print(buffer);

        /* score */
        sprintf(buffer,"%d",s->score);
        for (str = buffer; *str != '\0'; str++)
        {
            *str = *str + (129 - '0');  /* Used fixed-width numbers (129...) */
        }

        VW_MeasurePropString(buffer, &w, &h);
        PrintX = (34 * 8) - 8 - w;

        US_Print(buffer);
    }
}


/*
================================================================
=
= Function: CheckHighScore
=
= Description:
=
= update high score's if needed
=
================================================================
*/
void CheckHighScore(S32 score, U16 other)
{
    U16 i, j;
    S32 n;
    HighScore myscore;

    strcpy (myscore.name, "");
    myscore.score = score;
    myscore.episode = gamestate.episode;
    myscore.completed = other;

    /* check to see if new score is higher then stored scores */
    for (i = 0, n = -1; i < MaxScores; i++)
    {
        if ((myscore.score > Scores[i].score)
            || ((myscore.score == Scores[i].score) && (myscore.completed > Scores[i].completed)))
        {
            /* we have a higher score */
            for (j = MaxScores; --j > i;)
            {
                /* shift scores down by one to make room for new high score */
                Scores[j] = Scores[j - 1];
            }
            /* store new high score across */
            Scores[i] = myscore;
            n = i;
            break;
        }
    }

    /* got a high score */
    if(n != -1)
    {
        /* copy user name into high score board */
        for(i = 0; i < MaxHighName; i++)
        {
            Scores[n].name[i] = DS_USERNAME[i];
        }
    }

    StartCPMusic(ROSTER_MUS);

    DrawHighScores();

    UpdateScreen(0);

    VW_FadeIn();

    IN_UserInput(500);

}

/*
================================================================
=
= Function: LevelCompleted
=
= Description:
=
= Entered with the screen faded out
= Still in split screen mode with the status bar
=
= Exit with the screen faded out
=
================================================================
*/

void LevelCompleted(void)
{
    S32 x, i, min, sec, ratio, kr, sr, tr;
    char tempstr[20];
    S32 bonus;
    S32 timeleft = 0;

    CacheLump (LEVELEND_LUMP_START, LEVELEND_LUMP_END);
    ClearSplitVWB();           /* set up for double buffering in split screen */
    VL_Bar(0, 0, 320, SCREENHEIGHT - STATUSLINES + 1, VIEWCOLOR);

    StartCPMusic (ENDLEVEL_MUS);

    /* do the intermission */
    VWB_DrawPic (0, 16, L_GUYPIC);

    if(mapon < 8)
    {
        Write (14, 2, "floor\ncompleted");

        Write (14, 7, STR_BONUS "     0");
        Write (16, 10, STR_TIME);
        Write (16, 12, STR_PAR);

        Write (9, 14, STR_RAT2KILL);
        Write (5, 16, STR_RAT2SECRET);
        Write (1, 18, STR_RAT2TREASURE);

        /* convert number to string */
        sprintf(tempstr,"%d",gamestate.mapon + 1);
        Write (26, 2, tempstr);

        Write (26, 12, parTimes[gamestate.episode * 10 + mapon].timestr);

        /* PRINT TIME */
        sec = gamestate.TimeCount / 70;

        if (sec > 99 * 60)      /* 99 minutes max */
        {
            sec = 99 * 60;
        }

        if(gamestate.TimeCount < parTimes[gamestate.episode * 10 + mapon].time)
        {
            timeleft = (S32) (parTimes[gamestate.episode * 10 + mapon].time / 70 - sec);
        }

        min = sec / 60;
        sec %= 60;

        i = 26 * 8;

        VWB_DrawPic (i, 10 * 8, L_NUM0PIC + (min / 10));
        i += 2 * 8;
        VWB_DrawPic (i, 10 * 8, L_NUM0PIC + (min % 10));
        i += 2 * 8;
        Write (i / 8, 10, ":");
        i += 1 * 8;
        VWB_DrawPic (i, 10 * 8, L_NUM0PIC + (sec / 10));
        i += 2 * 8;
        VWB_DrawPic (i, 10 * 8, L_NUM0PIC + (sec % 10));

        UpdateScreen(0);
        VW_FadeIn ();

        /* FIGURE RATIOS OUT BEFOREHAND */
        kr = sr = tr = 0;

        if (gamestate.killtotal)
        {
            kr = (gamestate.killcount * 100) / gamestate.killtotal;
        }

        if (gamestate.secrettotal)
        {
            sr = (gamestate.secretcount * 100) / gamestate.secrettotal;
        }

        if (gamestate.treasuretotal)
        {
            tr = (gamestate.treasurecount * 100) / gamestate.treasuretotal;
        }

        /* PRINT TIME BONUS */
        bonus = timeleft * PAR_AMOUNT;

        if(bonus)
        {
            for (i = 0; i <= timeleft; i++)
            {
                /* convert number to string */
                sprintf(tempstr,"%d",i * PAR_AMOUNT);

                x = 36 - (S32) strlen(tempstr) * 2;
                Write (x, 7, tempstr);

                if (!(i % (PAR_AMOUNT / 10)))
                {
                    SD_PlaySound (ENDBONUS1SND);
                }

                if(!(i % (PAR_AMOUNT / 50)))
                {
                    UpdateScreen(0);
                }

                while(SD_SoundPlaying())
                {
                    BJ_Breathe();
                }

                if(IN_CheckAck())
                {
                    goto done;
                }
            }

            UpdateScreen(0);

            SD_PlaySound(ENDBONUS2SND);

            while(SD_SoundPlaying ())
            {
                BJ_Breathe ();
            }
        }

        /* KILL RATIO */

        ratio = kr;

        for (i = 0; i <= ratio; i++)
        {
            /* convert number to string */
            sprintf(tempstr,"%d",i);
            x = RATIOXX - (S32) strlen(tempstr) * 2;
            Write (x, 14, tempstr);

            if(!(i % 10))
            {
                SD_PlaySound (ENDBONUS1SND);
            }

            if(!(i & 1))
            {
                UpdateScreen(0);
            }

            while(SD_SoundPlaying ())
            {
                BJ_Breathe ();
            }

            if (IN_CheckAck ())
            {
                goto done;
            }
        }

        if (ratio >= 100)
        {
            VW_WaitVBL(VBLWAIT);
            SD_StopSound ();
            bonus += PERCENT100AMT;
            /* convert number to string */
            sprintf(tempstr,"%d",bonus);
            x = (RATIOXX - 1) - (S32) strlen(tempstr) * 2;
            Write (x, 7, tempstr);
            UpdateScreen(0);
            SD_PlaySound (PERCENT100SND);
        }
        else if (!ratio)
        {
            VW_WaitVBL (VBLWAIT);
            SD_StopSound ();
            SD_PlaySound (NOBONUSSND);
        }
        else
        {
            SD_PlaySound (ENDBONUS2SND);
        }

        UpdateScreen(0);

        while (SD_SoundPlaying())
        {
            BJ_Breathe ();
        }

        /* SECRET RATIO */
        ratio = sr;
        for (i = 0; i <= ratio; i++)
        {
            /* convert number to string */
            sprintf(tempstr,"%d",i);
            x = RATIOXX - (S32) strlen(tempstr) * 2;
            Write (x, 16, tempstr);

            if (!(i % 10))
            {
                SD_PlaySound (ENDBONUS1SND);
            }

            if(!(i & 1))
            {
                UpdateScreen(0);
            }

            while (SD_SoundPlaying ())
            {
                BJ_Breathe ();
            }

            if (IN_CheckAck())
            {
                goto done;
            }
        }

        if(ratio >= 100)
        {
            VW_WaitVBL (VBLWAIT);
            SD_StopSound ();
            bonus += PERCENT100AMT;
            /* convert number to string */
            sprintf(tempstr,"%d",bonus);
            x = (RATIOXX - 1) - (S32) strlen(tempstr) * 2;
            Write (x, 7, tempstr);
            UpdateScreen(0);
            SD_PlaySound (PERCENT100SND);
        }
        else if (!ratio)
        {
            VW_WaitVBL (VBLWAIT);
            SD_StopSound ();
            SD_PlaySound (NOBONUSSND);
        }
        else
        {
            SD_PlaySound (ENDBONUS2SND);
        }

        UpdateScreen(0);

        while (SD_SoundPlaying ())
        {
            BJ_Breathe();
        }

        /* TREASURE RATIO */
        ratio = tr;

        for (i = 0; i <= ratio; i++)
        {
            /* convert number to string */
            sprintf(tempstr,"%d",i);
            x = RATIOXX - (S32) strlen(tempstr) * 2;
            Write (x, 18, tempstr);

            if (!(i % 10))
            {
                SD_PlaySound (ENDBONUS1SND);
            }

            if(!(i & 1))
            {
                UpdateScreen(0);
            }

            while (SD_SoundPlaying ())
            {
                BJ_Breathe ();
            }

            if (IN_CheckAck ())
            {
                goto done;
            }
        }

        if (ratio >= 100)
        {
            VW_WaitVBL (VBLWAIT);
            SD_StopSound ();
            bonus += PERCENT100AMT;
            /* convert number to string */
            sprintf(tempstr,"%d",bonus);
            x = (RATIOXX - 1) - (S32) strlen(tempstr) * 2;
            Write (x, 7, tempstr);
            UpdateScreen(0);
            SD_PlaySound (PERCENT100SND);
        }
        else if (!ratio)
        {
            VW_WaitVBL (VBLWAIT);
            SD_StopSound ();
            SD_PlaySound (NOBONUSSND);
        }
        else
        {
            SD_PlaySound (ENDBONUS2SND);
        }

        UpdateScreen(0);

        while (SD_SoundPlaying ())
        {
            BJ_Breathe ();
        }


        /* JUMP STRAIGHT HERE IF KEY PRESSED */
done:
        /* convert number to string */
        sprintf(tempstr,"%d",kr);
        x = RATIOXX - (S32) strlen(tempstr) * 2;
        Write (x, 14, tempstr);

        sprintf(tempstr,"%d",sr);
        x = RATIOXX - (S32) strlen(tempstr) * 2;
        Write (x, 16, tempstr);

        sprintf(tempstr,"%d",tr);
        x = RATIOXX - (S32) strlen(tempstr) * 2;
        Write (x, 18, tempstr);

        bonus = (S32) timeleft *PAR_AMOUNT +
            (PERCENT100AMT * (kr >= 100)) +
            (PERCENT100AMT * (sr >= 100)) + (PERCENT100AMT * (tr >= 100));

        GivePoints (bonus);
        /* convert number to string */
        sprintf(tempstr,"%d",bonus);
        x = 36 - (S32) strlen(tempstr) * 2;
        Write (x, 7, tempstr);

        /* SAVE RATIO INFORMATION FOR ENDGAME */
        LevelRatios[mapon].kill = kr;
        LevelRatios[mapon].secret = sr;
        LevelRatios[mapon].treasure = tr;
        LevelRatios[mapon].time = min * 60 + sec;
    }
    else
    {
        Write (14, 4, "secret floor\n completed!");

        Write (10, 16, "15000 bonus!");

        UpdateScreen(0);
        VW_FadeIn ();

        GivePoints (15000);
    }

    DrawScore();
    UpdateScreen(0);

    lastBreathTime = GetTimeCount();

    while (!IN_CheckAck())
    {
        BJ_Breathe();
        Delay_ms(10);
    }

    VW_FadeOut ();
    DrawPlayBorder();

    UnCacheLump (LEVELEND_LUMP_START, LEVELEND_LUMP_END);
}

/*
================================================================
=
= Function: ClearSplitVWB
=
= Description:
=
= reset print screen coords
=
================================================================
*/
static void ClearSplitVWB(void)
{
    WindowX = 0;
    WindowY = 0;
    WindowW = 320;
    WindowH = 160;
}

/*
================================================================
=
= Function: BJ_Breathe
=
= Description:
=
= Code that handles BJ breathe animation
=
================================================================
*/
static void BJ_Breathe (void)
{
    static U32 which = 0;
    static S32 max = 10;
    S32 pics[2] = { L_GUYPIC, L_GUY2PIC };

    Delay_ms(10);

    if((S32)GetTimeCount() - lastBreathTime > max)
    {
        which ^= 1;
        VWB_DrawPic(0, 16, pics[which]);
        UpdateScreen(0);
        lastBreathTime = GetTimeCount();
        max = 35;
    }
}

/*
================================================================
=
= Function: Write
=
= Description:
=
= write string to screen using defined pics in alpha array
=
================================================================
*/
static void Write(S32 x, S32 y, const char *string)
{
    S32 i, ox, nx, ny, len = (S32) strlen(string);
    char ch;

    ox = nx = x * 8;
    ny = y * 8;

    for (i = 0; i < len; i++)
    {
        if (string[i] == '\n')
        {
            nx = ox;
            ny += 16;
        }
        else
        {
            ch = string[i];
            if (ch >= 'a')
            {
                ch -= ('a' - 'A');
            }

            ch -= '0';

            switch (string[i])
            {
                case '!':
                    VWB_DrawPic (nx, ny, L_EXPOINTPIC);
                    nx += 8;
                    continue;

                case '\'':
                    VWB_DrawPic (nx, ny, L_APOSTROPHEPIC);
                    nx += 8;
                    continue;

                case ' ':
                    break;

                case 0x3a:     /* ':' */
                    VWB_DrawPic(nx, ny, L_COLONPIC);
                    nx += 8;
                    continue;

                case '%':
                    VWB_DrawPic(nx, ny, L_PERCENTPIC);
                    break;

                default:
                    VWB_DrawPic(nx, ny, alpha[(U8)ch]);
                    break;
            }

            nx += 16;
        }
    }
}

/*
================================================================
=
= Function: PreloadGraphics
=
= Description:
=
= Load get psyched message
=
================================================================
*/
void PreloadGraphics(void)
{
    DrawLevel();

    ClearSplitVWB();

    /* paint background in colour VIEWCOLOR */
    VL_Bar(0, 0, SCREENWIDTH, SCREENHEIGHT - (STATUSLINES - 1), VIEWCOLOR);

    /* display GETPSYCHEDPIC picture in the centre of screen */
    VWB_DrawPic((SCREENWIDTH - 224)/2,(SCREENHEIGHT - (STATUSLINES+48))/2, GETPSYCHEDPIC);

    /* setup screen cursor so that its ready to display progress bar under picture */
    WindowX = (SCREENWIDTH - 224)/2;
    WindowY = (SCREENHEIGHT - (STATUSLINES+48))/2;
    WindowW = 28 * 8;
    WindowH = 48;

    UpdateScreen(0);
    VW_FadeIn();

    /* display progress bar */
    PreloadUpdate(10, 10);

    IN_UserInput (70);
    VW_FadeOut();

    DrawPlayBorder();
    UpdateScreen(0);
}

/*
================================================================
=
= Function: PreloadUpdate
=
= Description:
=
= display get psyched progress bar
=
================================================================
*/
static U8 PreloadUpdate(U32 current, U32 total)
{
    U32 w = WindowW - 10;

    VL_Bar(WindowX + 5, WindowY + WindowH - 3, w, 2, 0);

    w = ((S32) w * current) / total;

    if(w != 0)
    {
        VL_Bar(WindowX + 5, WindowY + WindowH - 3, w, 2, 0x37);
        VL_Bar(WindowX + 5, WindowY + WindowH - 3, w - 1,  1, 0x32);
    }

    UpdateScreen(0);

    return(0);
}

/*
================================================================
=
= Function: Victory
=
= Description:
=
=
=
================================================================
*/
void Victory (void)
{
    S32 sec;
    S32 i, min, kr, sr, tr, x;
    char tempstr[8];

    StartCPMusic (URAHERO_MUS);
    ClearSplitVWB ();
    CacheLump (LEVELEND_LUMP_START, LEVELEND_LUMP_END);
    CA_CacheGrChunk (STARTFONT);

    CA_CacheGrChunk (C_TIMECODEPIC);

    VL_Bar(0, 0, 320, (SCREENHEIGHT - STATUSLINES + 1), VIEWCOLOR);

    Write (18, 2, STR_YOUWIN);

    Write (TIMEX, TIMEY - 2, STR_TOTALTIME);

    Write (12, RATIOY - 2, "averages");

    Write (RATIOX + 8, RATIOY, STR_RATKILL);
    Write (RATIOX + 4, RATIOY + 2, STR_RATSECRET);
    Write (RATIOX, RATIOY + 4, STR_RATTREASURE);

    VWB_DrawPic (8, 4, L_BJWINSPIC);

    for (kr = sr = tr = sec = i = 0; i < LRpack; i++)
    {
        sec += LevelRatios[i].time;
        kr += LevelRatios[i].kill;
        sr += LevelRatios[i].secret;
        tr += LevelRatios[i].treasure;
    }

    kr /= LRpack;
    sr /= LRpack;
    tr /= LRpack;

    min = sec / 60;
    sec %= 60;

    if (min > 99)
    {
        min = sec = 99;
    }

    i = TIMEX * 8 + 1;
    VWB_DrawPic (i, TIMEY * 8, L_NUM0PIC + (min / 10));
    i += 2 * 8;
    VWB_DrawPic (i, TIMEY * 8, L_NUM0PIC + (min % 10));
    i += 2 * 8;
    Write (i / 8, TIMEY, ":");
    i += 1 * 8;
    VWB_DrawPic (i, TIMEY * 8, L_NUM0PIC + (sec / 10));
    i += 2 * 8;
    VWB_DrawPic (i, TIMEY * 8, L_NUM0PIC + (sec % 10));
    UpdateScreen(0);

    sprintf(tempstr,"%d",kr);
    x = RATIOX + 24 - (S32) strlen(tempstr) * 2;
    Write (x, RATIOY, tempstr);

    sprintf(tempstr,"%d",sr);
    x = RATIOX + 24 - (S32) strlen(tempstr) * 2;
    Write (x, RATIOY + 2, tempstr);

    sprintf(tempstr,"%d",tr);
    x = RATIOX + 24 - (S32) strlen(tempstr) * 2;
    Write (x, RATIOY + 4, tempstr);

#ifndef SHAREWARE
    /* TOTAL TIME VERIFICATION CODE */
    if (gamestate.difficulty >= gd_medium)
    {
        VWB_DrawPic (30 * 8, TIMEY * 8, C_TIMECODEPIC);
        SetFontNum(0);
        SetFontColor(READHCOLOR,BKGDCOLOR);
        PrintX = 30 * 8 - 3;
        PrintY = TIMEY * 8 + 8;
        PrintX += 4;
        tempstr[0] = (((min / 10) ^ (min % 10)) ^ 0xa) + 'A';
        tempstr[1] = (S32) ((((sec / 10) ^ (sec % 10)) ^ 0xa) + 'A');
        tempstr[2] = (tempstr[0] ^ tempstr[1]) + 'A';
        tempstr[3] = 0;
        US_Print (tempstr);
    }
#endif

    SetFontNum(1);

    UpdateScreen(0);
    VW_FadeIn();

    IN_Ack ();

    VW_FadeOut ();

    CA_UnCacheGrChunk(C_TIMECODEPIC);

    UnCacheLump (LEVELEND_LUMP_START, LEVELEND_LUMP_END);

    EndText();
}
