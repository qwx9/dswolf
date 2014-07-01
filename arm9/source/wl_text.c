/*
=============================================================================

TEXT FORMATTING COMMANDS
------------------------
^C<hex digit>           Change text color
^E[enter]               End of layout (all pages)
^G<y>,<x>,<pic>[enter]  Draw a graphic and push margins
^P[enter]               start new page, must be the first chars in a layout
^L<x>,<y>[ENTER]        Locate to a specific spot, x in pixels, y in lines
^T<x>,<y>,<pic>,<t>     Timed draw graphic command

=============================================================================
*/

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include "types.h"
#include "wl_def.h"
#include "audiowl6.h"
#include "wl_text.h"
#include "wl_menu.h"
#include "gfxv_apo.h"
#include "id_ca.h"
#include "id_vl.h"
#include "id_vh.h"
#include "id_time.h"
#include "id_in.h"
#include "id_sd.h"

/* macro defines */

#define BACKCOLOR       0x11

#define WORDLIMIT       80
#define FONTHEIGHT      10
#define TOPMARGIN       16
#define BOTTOMMARGIN    32
#define LEFTMARGIN      16
#define RIGHTMARGIN     16
#define PICMARGIN       8
#define TEXTROWS        ((200-TOPMARGIN-BOTTOMMARGIN)/FONTHEIGHT)
#define SPACEWIDTH      7
#define SCREENPIXWIDTH  320
#define SCREENMID       (SCREENPIXWIDTH/2)

/* external variables */

extern U8 * grsegs[NUMCHUNKS];
extern S16 px;
extern S16 py;
extern pictabletype *pictable;
extern gametype gamestate;

/* local variables */

static S32 helpextern = T_HELPART;
static char* text;
static S32 pagenum;
static S32 numpages;
static S32 picx;
static S32 picy;
static S32 picnum;
static S32 picdelay;
static U32 leftmargin[TEXTROWS];
static U32 rightmargin[TEXTROWS];
static U32 rowon;
static U8 layoutdone;
static char str[20];

/* global variables */

/* None */

/* local prototypes */

static void ShowArticle (char *article);
static void CacheLayoutGraphics(void);
static void ParsePicCommand(void);
static S32 ParseNumber(void);
static void RipToEOL(void);
static void ParseTimedCommand(void);
static void PageLayout(U8 shownumber);
static void HandleCtrls(void);
static void NewLine(void);
static void HandleWord(void);
static void HandleCommand(void);
static void TimedPicCommand(void);
static void BackPage(void);

/*
================================================================
=
= Function: HelpScreens
=
= Description:
=
= display help screen
=
================================================================
*/
void HelpScreens(void)
{
    S32  artnum;
    char *text;

    artnum = helpextern;
    CA_CacheGrChunk(artnum);
    text = (char *)grsegs[artnum];

    ShowArticle(text);

    CA_UnCacheGrChunk(artnum);

    VW_FadeOut();

    FreeMusic();
}

/*
================================================================
=
= Function: ShowArticle
=
= Description:
=
= displays a help page on screen
=
================================================================
*/
static void ShowArticle(char *article)
{
    U8 oldfontnumber;
    U8 newpage;
    U8 firstpage;
    ControlInfo ci;

    text = article;
    oldfontnumber = GetFontNum();
    SetFontNum(0);

    CA_CacheGrChunk(STARTFONT);
    VL_Bar(0,0,320,200,BACKCOLOR);

    CacheLayoutGraphics();

    newpage = 1;
    firstpage = 1;

    do
    {
        if(newpage == 1)
        {
            newpage = 0;
            PageLayout(1);

            UpdateScreen(0);

            if(firstpage == 1)
            {
                MenuFadeIn();
                firstpage = 0;
            }
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
        IN_ReadControl(&ci);

        Direction dir = ci.dir;

        switch(dir)
        {
            case dir_North:
            case dir_South:
                /* DO NOTHING DIRECTION ALREADY SET */
            break;

            default:
                if(ci.button0 == 1)
                {
                    dir = dir_South;
                }
            break;
        }

        switch(dir)
        {
            case dir_North:
            case dir_West:
                if(pagenum > 1)
                {
                    BackPage();
                    BackPage();
                    newpage = 1;
                }
                TicDelay(20);
            break;

            case dir_South:
            case dir_East:
                if(pagenum < numpages)
                {
                    newpage = 1;
                }
                TicDelay(20);
            break;

            default:
                /* do nothing */
            break;
        }

    }while(ci.button1 == 0);

    SetFontNum(oldfontnumber);
}

/*
================================================================
=
= Function: CacheLayoutGraphics
=
= Description:
=
= Scans an entire layout file (until a ^E) marking all graphics used, and
= counting pages, then caches the graphics in
=
================================================================
*/
static void CacheLayoutGraphics(void)
{
    char    *bombpoint = text + 30000;
    char    *textstart = text;
    char    ch;
    numpages = 0;
    pagenum = 0;

    do
    {
        /* check for text format command */
        if(*text == '^')
        {
            ch = (char)toupper((S32)*++text);

            if(ch == 'P')          /* start of a page */
            {
                numpages++;
            }

            if(ch == 'E')          /* end of file, so load graphics and return */
            {
                CA_CacheGrChunk(H_TOPWINDOWPIC);
                CA_CacheGrChunk(H_LEFTWINDOWPIC);
                CA_CacheGrChunk(H_RIGHTWINDOWPIC);
                CA_CacheGrChunk(H_BOTTOMINFOPIC);
                text = textstart;
                return;
            }

            if(ch == 'G')          /* draw graphic command, so mark graphics */
            {
                ParsePicCommand();
                CA_CacheGrChunk(picnum);
            }

            if(ch == 'T')        /* timed draw graphic command, so mark graphics */
            {
                ParseTimedCommand();
                CA_CacheGrChunk(picnum);
            }
        }
        else
        {
            text++;
        }

    }while(text<bombpoint);

    printf("CacheLayoutGraphics: No ^E to terminate file! \n");
    while(1){};     /* hang system */

}

/*
================================================================
=
= Function: ParsePicCommand
=
= Description:
=
= Call with text pointing just after a ^G
= Upon exit text points to the start of next line
=
================================================================
*/
static void ParsePicCommand(void)
{
    picy = ParseNumber();
    picx = ParseNumber();
    picnum = ParseNumber();
    RipToEOL();
}

/*
================================================================
=
= Function: ParseTimedCommand
=
= Description:
=
= Call with text pointing just after a ^T
= Upon exit text points to the start of next line
=
================================================================
*/

static void ParseTimedCommand(void)
{
    picy=ParseNumber();
    picx=ParseNumber();
    picnum=ParseNumber();
    picdelay=ParseNumber();
    RipToEOL();
}

/*
================================================================
=
= Function: ParseNumber
=
= Description:
=
= search for and return a number value after ^G / ^T commands
=
================================================================
*/
static S32 ParseNumber(void)
{
    char  ch;
    char  num[80];
    char *numptr;


    /* scan until a number is found */
    ch = *text;
    while (ch < '0' || ch >'9')
    {
        ch = *++text;
    }

    /* copy the number out */
    numptr = num;
    do
    {
        *numptr++ = ch;
        ch = *++text;
    } while (ch >= '0' && ch <= '9');

    *numptr = 0;

    return atoi(num);
}

/*
================================================================
=
= Function: RipToEOL
=
= Description:
=
= scan to end of line
=
================================================================
*/

static void RipToEOL(void)
{
    while (*text++ != '\n'){ /* do nothing */};
}

/*
================================================================
=
= Function: PageLayout
=
= Description:
=
= Clears the screen, draws the pics on the page, and word wraps
= the text. Returns a pointer to the terminating command
=
================================================================
*/

static void PageLayout(U8 shownumber)
{
    S32  i;
    U8   oldfontcolor;
    U8   oldbackcolor;
    char ch;

    oldfontcolor = GetFontColor();
    oldbackcolor = GetBackColour();
    SetFontColor(0, oldbackcolor);

    /* clear the screen */
    VL_Bar(0,0,320,200,BACKCOLOR);
    VWB_DrawPic(0,0,H_TOPWINDOWPIC);
    VWB_DrawPic(0,8,H_LEFTWINDOWPIC);
    VWB_DrawPic(312,8,H_RIGHTWINDOWPIC);
    VWB_DrawPic(8,176,H_BOTTOMINFOPIC);

    for(i = 0; i < TEXTROWS; i++)
    {
        leftmargin[i] = LEFTMARGIN;
        rightmargin[i] = SCREENPIXWIDTH-RIGHTMARGIN;
    }

    px = LEFTMARGIN;
    py = TOPMARGIN;
    rowon = 0;
    layoutdone = 0;

    /* make sure we are starting layout text (^P first command) */
    while(*text <= 32)
    {
        text++;
    }

    if((*text != '^') || ((char)toupper((S32)*++text) != 'P'))
    {
        printf("PageLayout: Text not headed with ^P \n");
        while(1){}; /* hang system */
    }

    while (*text++ != '\n'){ /* do nothing */};

    /* process text stream */
    do
    {
        ch = *text;

        if (ch == '^')
        {
            HandleCommand();
        }
        else
        {
            /* test for tab  character*/
            if(ch == 9)
            {
                px = px + 8;
                px = ((U16)px & 0Xf8);
                text++;
            }
            else if(ch <= 32)
            {
                HandleCtrls();
            }
            else
            {
                HandleWord();
            }
        }
    }while(layoutdone == 0);

    pagenum++;

    if(shownumber == 1)
    {
        sprintf(str, "pg %d of %d", pagenum, numpages);
        px = 213;
        py = 183;
        SetFontColor(0x4f, oldbackcolor);   /* 12^BACKCOLOR */
        VWB_DrawPropString(str);
    }

    SetFontColor(oldfontcolor, oldbackcolor);
}

/*
================================================================
=
= Function: HandleCtrls
=
= Description:
=
= check for new line control character
=
================================================================
*/

static void HandleCtrls(void)
{
    char ch;

    ch = *text++;   /* get the character and advance */

    if(ch == '\n')
    {
        NewLine();
        return;
    }
}

/*
================================================================
=
= Function: NewLine
=
= Description:
=
= move cursor to new row
=
================================================================
*/
static void NewLine(void)
{
    char ch;

    if (++rowon == TEXTROWS)
    {
        /* overflowed the page, so skip until next page break */
        layoutdone = 1;
        do
        {
            if (*text == '^')
            {
                ch = (char)toupper((S32)*(text+1));
                if (ch == 'E' || ch == 'P')
                {
                    layoutdone = 1;
                    return;
                }
            }
            text++;
        } while (1);
    }

    px = leftmargin[rowon];
    py+= FONTHEIGHT;
}

/*
================================================================
=
= Function: HandleWord
=
= Description:
=
= write word into screen buffer
=
================================================================
*/
static void HandleWord(void)
{
    char wword[WORDLIMIT];
    S32  wordindex;
    U16  wwidth;
    U16  wheight;
    U16  newpos;

    /* copy the next word into [word] */
    wword[0] = *text++;
    wordindex = 1;

    /* while character is not a space fill up word buffer */
    while(*text>32)
    {
        wword[wordindex] = *text++;
        if(++wordindex == WORDLIMIT)
        {
            printf("PageLayout: Word limit exceeded \n");
            while(1){}; /* hang system */
        }
    }
    wword[wordindex] = '\0';    /* stick a null at end for correct C string formatting */

    /* see if it fits on this line */
    VW_MeasurePropString(wword,&wwidth,&wheight);

    while((px + wwidth) > (S32)rightmargin[rowon])
    {
        NewLine ();
        if (layoutdone)
        {
            return;         /* overflowed page */
        }
    }

    /* print it */
    newpos = px+wwidth;
    VWB_DrawPropString(wword);
    px = newpos;

    /* suck up any extra spaces */
    while(*text == ' ')
    {
        px += SPACEWIDTH;
        text++;
    }
}

/*
================================================================
=
= Function: HandleCommand
=
= Description:
=
= handles the commands embedded within the help file
=
================================================================
*/
static void HandleCommand(void)
{
    S32     i;
    S32     margin;
    S32     top;
    S32     bottom;
    S32     picwidth;
    S32     picheight;
    S32     picmid;
    S32     Newfontcolor = 0;
    U8      oldbackcolor = 0;

    oldbackcolor = GetBackColour();

    /* test for supported commands */
    switch((char)toupper((S32)*++text))
    {
        case 'B':
            picy = ParseNumber();
            picx = ParseNumber();
            picwidth = ParseNumber();
            picheight = ParseNumber();
            VL_Bar(picx,picy,picwidth,picheight,BACKCOLOR);
            RipToEOL();
        break;

        case ';':           /* comment */
            RipToEOL();
        break;

        case 'P':           /* ^P is start of next page, ^E is end of file */
        case 'E':
            layoutdone = 1;
            text--;         /* back up to the '^' */
        break;

        case 'C':           /* ^c<hex digit> changes text color */
            i = toupper((S32)*++text);

            if((i >= '0') && (i <= '9'))
            {
                Newfontcolor = i - '0';
            }
            else if ((i >= 'A') && (i <= 'F'))
            {
                Newfontcolor = i - 'A' + 10;
            }

            Newfontcolor *= 16;
            i = toupper((S32)*++text);

            if ((i >= '0') && (i <= '9'))
            {
                Newfontcolor += i-'0';
            }
            else if ((i >= 'A') && (i <= 'F'))
            {
                Newfontcolor += i-'A' + 10;
            }

            SetFontColor(Newfontcolor, oldbackcolor);
            text++;
        break;

        case '>':
            px = 160;
            text++;
        break;

        case 'L':
            py = ParseNumber();
            rowon = (py-TOPMARGIN)/FONTHEIGHT;
            py = TOPMARGIN+rowon*FONTHEIGHT;
            px = ParseNumber();
            while (*text++ != '\n'){};      /* scan to end of line */
        break;

        case 'T':       /*  ^Tyyy,xxx,ppp,ttt waits ttt tics, then draws pic */
            TimedPicCommand();
        break;

         case 'G':      /* ^Gyyy,xxx,ppp draws graphic */
            ParsePicCommand();
            VWB_DrawPic(((U32)picx & ~7),picy,picnum);
            picwidth = pictable[picnum-STARTPICS].width;
            picheight = pictable[picnum-STARTPICS].height;

            /* adjust margins */
            picmid = picx + picwidth / 2;
            if(picmid > SCREENMID)
            {
                margin = picx-PICMARGIN;     /* new right margin */
            }
            else
            {
                margin = picx+picwidth+PICMARGIN;   /* new left margin */
            }

            top = (picy-TOPMARGIN)/FONTHEIGHT;
            if(top < 0)
            {
                top = 0;
            }

            bottom = (picy+picheight-TOPMARGIN)/FONTHEIGHT;

            if(bottom>=TEXTROWS)
            {
                bottom = TEXTROWS - 1;
            }

            for (i=top;i<=bottom;i++)
            {
                if (picmid > SCREENMID)
                {
                    rightmargin[i] = margin;
                }
                else
                {
                    leftmargin[i] = margin;
                }
            }

            /* adjust this line if needed */
            if (px < (int) leftmargin[rowon])
            {
                px = leftmargin[rowon];
            }

        break;

        default:
            /* do nothing */
        break;
    }
}

/*
================================================================
=
= Function: TimedPicCommand
=
= Description:
=
= wait for a set amount of time and the display pic
=
================================================================
*/
static void TimedPicCommand(void)
{
    ParseTimedCommand ();

    /* update the screen, and wait for time delay */
    UpdateScreen(0);


    /* wait for time */
    Delay_ms(picdelay);

    /* draw pic */
    VWB_DrawPic(((U32)picx & ~7),picy,picnum);
}

/*
================================================================
=
= Function: BackPage
=
= Description:
=
= Scans for a previous ^P
=
================================================================
*/
static void BackPage(void)
{
    pagenum--;

    do
    {
        text--;
        if ((*text == '^') && ((char)toupper((S32)*(text+1)) == 'P'))
        {
            return;
        }
    } while (1);
}

/*
================================================================
=
= Function: EndText
=
= Description:
=
= END ARTICLES
=
================================================================
*/
void EndText(void)
{
    S32     artnum;
    char    *text;

    ClearMemory();

    artnum = T_ENDART1 + gamestate.episode;
    CA_CacheGrChunk (artnum);
    text = (char *)grsegs[artnum];

    ShowArticle(text);

    CA_UnCacheGrChunk(artnum);

    VW_FadeOut();
    SetFontColor(0,15);

    FreeMusic();
}
