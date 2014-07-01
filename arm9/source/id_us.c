#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "types.h"
#include "wl_def.h"
#include "id_us.h"
#include "id_vh.h"
#include "id_vl.h"
#include "id_time.h"
#include "id_in.h"

/* macro defines */

#define    MAXSTRING	128

/* external variables */

extern S16 px;
extern S16 py;
extern char *DS_USERNAME;
extern gametype gamestate;

/* local variables */

static U32 rndindex;

static U8 rndtable[] =
{
      0,   8, 109, 220, 222, 241, 149, 107,  75, 248, 254, 140,  16,  66,
    74,  21, 211,  47,  80, 242, 154,  27, 205, 128, 161,  89,  77,  36,
    95, 110,  85,  48, 212, 140, 211, 249,  22,  79, 200,  50,  28, 188,
    52, 140, 202, 120,  68, 145,  62,  70, 184, 190,  91, 197, 152, 224,
    149, 104,  25, 178, 252, 182, 202, 182, 141, 197,   4,  81, 181, 242,
    145,  42,  39, 227, 156, 198, 225, 193, 219,  93, 122, 175, 249,   0,
    175, 143,  70, 239,  46, 246, 163,  53, 163, 109, 168, 135,   2, 235,
    25,  92,  20, 145, 138,  77,  69, 166,  78, 176, 173, 212, 166, 113,
    94, 161,  41,  50, 239,  49, 111, 164,  70,  60,   2,  37, 171,  75,
    136, 156,  11,  56,  42, 146, 138, 229,  73, 146,  77,  61,  98, 196,
    135, 106,  63, 197, 195,  86,  96, 203, 113, 101, 170, 247, 181, 113,
    80, 250, 108,   7, 255, 237, 129, 226,  79, 107, 112, 166, 103, 241,
    24, 223, 239, 120, 198,  58,  60,  82, 128,   3, 184,  66, 143, 224,
    145, 224,  81, 206, 163,  45,  63,  90, 168, 114,  59,  33, 159,  95,
    28, 139, 123,  98, 125, 196,  15,  70, 194, 253,  54,  14, 109, 226,
    71,  17, 161,  93, 186,  87, 244, 138,  20,  52, 123, 251,  26,  36,
    17,  46,  52, 231, 232,  76,  31, 221,  84,  37, 216, 165, 212, 106,
    197, 242,  98,  43,  39, 175, 254, 145, 190,  84, 118, 222, 187, 136,
    120, 163, 236, 249
};

/* global variables */

U16 WindowX;
U16 WindowY;
U16 WindowW;
U16 WindowH;
U16 PrintX;
U16 PrintY;

/* local prototypes */

static void US_CPrintLine(const char *s);

/*
================================================================
=
= Function: US_Print
=
= Description:
=
= Prints a string in the current window. Newlines are supported.
=
================================================================
*/
void US_Print(const char *sorg)
{
    char c;
    char *sstart = strdup(sorg);
    char *s = sstart;
    char *se = NULL;
    U16 w;
    U16 h;

    /* loop unitl we are at the end of the string passed to us */
    while (*s != '\0')
    {
        /* if the string contains \n split the string into two */
        /* zero terminated strings */
        se = s;
        while (((c = *se) != 0) && (c != '\n'))
        {
            se++;
        }

        *se = '\0';

        /* print the string to screen */
        VW_MeasurePropString(s,&w,&h);
        px = PrintX;
        py = PrintY;
        VWB_DrawPropString(s);

        s = se;

        /* check to see if this is the first of the two strings to print  */
        /* if it make sure screen position is correct for this string */
        if(c != '\0')
        {
            *se = c;
            s++;
            PrintX = WindowX;
            PrintY += h;
        }
        else
        {
            PrintX += w;
        }
    }

    free(sstart);
}

/*
================================================================
=
= Function: US_CPrint
=
= Description:
=
= Prints a string centered in the current window Newlines
= are supported.
=
================================================================
*/

void US_CPrint(const char *sorg)
{
    char c;
    char *sstart = strdup(sorg);
    char *s = sstart;
    char *se;

    /* loop unitl we are at the end of the string passed to us */
    while (*s != '\0')
    {
        /* if the string contains \n split the string into two */
        /* zero terminated strings */
        se = s;
        while (((c = *se) !=0) && (c != '\n'))
        {
            se++;
        }

        *se = '\0';

        /* print the string to screen */
        US_CPrintLine(s);

        s = se;
        if(c != '\0')
        {
            *se = c;
            s++;
        }
    }

    free(sstart);
}

/*
================================================================
=
= Function: US_CPrintLine
=
= Description:
=
= Prints a string centered on the current line and advances to
= the next line. Newlines are not supported.
=
================================================================
*/
static void US_CPrintLine(const char *s)
{
    U16 w;
    U16 h;

    VW_MeasurePropString(s,&w,&h);

    if(w > WindowW)
    {
        printf("US_CPrintLine() - String exceeds width \n");
        while(1){}; /* hang system */
    }

    px = WindowX + ((WindowW - w) / 2);
    py = PrintY;
    VWB_DrawPropString(s);
    PrintY += h;
}

/*
================================================================
=
= Function: US_InitRndT
=
= Description:
=
= Initializes the pseudo random number generator.
= If randomize is true, the seed will be initialized depending
= on the current time
=
================================================================
*/
void US_InitRndT(S32 randomize)
{
    if(randomize != 0)
    {
        rndindex = (GetTimeMS() >> 4) & 0xff;
    }
    else
    {
        rndindex = 0;
    }
}

/*
================================================================
=
= Function: US_RndT
=
= Description:
=
= Returns the next 8-bit pseudo random number
=
================================================================
*/
U8 US_RndT(void)
{
    rndindex = (rndindex+1) & 0xff;
    return rndtable[rndindex];
}

/*
================================================================
=
= Function: US_LineInput
=
= Description:
=
= Print savegame name into save game screen menu
=
================================================================
*/
U8 US_LineInput(S32 x,S32 y,char *buf, S32 maxchars,S32 maxwidth)
{
    char    string[MAXSTRING];
    char    episode[5] = "";
    char    map[5] = "";
    ControlInfo ci;
    U32     lastBlinkTime;
    U8      tick = 0;
    U8      savename = 0;

    strcpy(string, DS_USERNAME);
    /* convert number to string */
    sprintf(episode,"%d",gamestate.episode);
    sprintf(map,"%d",gamestate.mapon);
    /* add to string */
    strcat(string,"_E");
    strcat(string,episode);
    strcat(string,"M");
    strcat(string,map);

    px = x;
    py = y;

    /* draw string into screen */
    VWB_DrawPropString(string);

    x = px;
    y = py;
    lastBlinkTime = GetTimeCount();

    UpdateScreen(0);

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

        IN_ReadControl(&ci);

        if((GetTimeCount() - lastBlinkTime) >= 10)
        {
            switch(tick)
            {
                case 0:
                    VL_Bar(x, y, 8, 9, BKGDCOLOR);
                break;

                case 1:
                    PrintX = x;
                    PrintY = y;
                    US_Print("|");
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
        strcpy(buf,string);
        savename = 1;
    }

    return savename;
}
