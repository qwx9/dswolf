#include <stdio.h>
#include <stdlib.h>
#include "types.h"
#include "id_vh.h"
#include "gfxv_apo.h"
#include "wl_menu.h"
#include "id_vl.h"

/* macro defines */

#define VW_Hlin(x,z,y,c)    VL_Hlin(x,y,(z)-(x)+1,c)
#define VW_Vlin(y,z,x,c)    VL_Vlin(x,y,(z)-(y)+1,c)

/* external variables */

extern U8 * grsegs[NUMCHUNKS];

/* local variables */

/* None */

/* global variables */

pictabletype *pictable = NULL;

/* local prototypes */

static void VWL_MeasureString(const char *string, U16 *width, U16 *height, fontstruct *font);

/* 
================================================================
=
= Function: VWB_DrawPic
=
= Description:
=
= draws the requested graphics picture chunk to screen (at x,y) 
=
================================================================ 
*/
void VWB_DrawPic(S32 x, S32 y, S32 chunknum)
{
    S32     picnum = chunknum - STARTPICS;
    U32     width;
    U32     height;

    /* make sure picture start on a location that is a multiple of 8*/
    x = (S32)((U32)x & 0Xfffffff8);

    width = pictable[picnum].width;
    height = pictable[picnum].height;

    VL_MemToScreen(grsegs[chunknum],width,height,x,y);
}

/* 
================================================================
=
= Function: VWB_Hlin
=
= Description:
=
= draws horizontal line from x1 to x2
=
================================================================ 
*/
void VWB_Hlin(S32 x1, S32 x2, S32 y, S32 color)
{
    VW_Hlin(x1,x2,y,color);
}

/* 
================================================================
=
= Function: VWB_Vlin
=
= Description:
=
= draws vertical line from y1 to y2
=
================================================================ 
*/
void VWB_Vlin(S32 y1, S32 y2, S32 x, S32 color)
{
    VW_Vlin(y1,y2,x,color);
}

/* 
================================================================
=
= Function:  VW_MeasurePropString
=
= Description:
=
= adds font used by string then passes info to string measure
= function
=
================================================================ 
*/
void VW_MeasurePropString(const char *string, U16 *width, U16 *height)
{
    U8 fontnumber = GetFontNum();
    VWL_MeasureString(string,width,height,(fontstruct *)grsegs[STARTFONT+fontnumber]);
}

/* 
================================================================
=
= Function: VWL_MeasureString
=
= Description:
=
= measures string length
=
================================================================ 
*/
static void VWL_MeasureString(const char *string, U16 *width, U16 *height, fontstruct *font)
{
    *height = font->height;
    
    for (*width = 0; *string != '\0'; string++)
    {
        *width += font->width[*((U8 *)string)];     /* proportional width */
    }
}