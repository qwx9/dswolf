#include <nds.h>
#include <stdio.h>
#include "types.h"
#include "wl_def.h"
#include "id_vl.h"
#include "gfxv_apo.h"
#include "id_time.h"
#include "wl_menu.h"
#include "id_ca.h"
#include "id_pm.h"

/* macro defines */

#define RGB(r, g, b) {(r)*255/63, (g)*255/63, (b)*255/63, 0}
#define SPRITESCALEFACTOR   2
#define REDCOLOUR     4
#define REDSTEPS      8
#define WHITESTEPS    20

/* external variables */

extern U8 signon[];
extern U8 * grsegs[NUMCHUNKS];
extern U32 screenofs;
extern S32 viewwidth;
extern S32 viewheight;
extern gametype gamestate;
extern S32 mapon;
extern S32 postx;
extern U8  *postsource;

/* local variables */

static vga_colour redshifts[NUMREDSHIFTS][256];
static vga_colour whiteshifts[NUMWHITESHIFTS][256];
static U32 rndmask;
static U32 rndbits_y;
static U8 *screen = NULL;
static vga_colour palette1[256];
static vga_colour palette2[256];
static vga_colour curpal[256];
static vga_colour vga_24bit_pal[] =
{
    #include "wolfpal.inc"
};

static U8 vgaCeiling[]=
{
 0x1d,0x1d,0x1d,0x1d,0x1d,0x1d,0x1d,0x1d,0x1d,0xbf,
 0x4e,0x4e,0x4e,0x1d,0x8d,0x4e,0x1d,0x2d,0x1d,0x8d,
 0x1d,0x1d,0x1d,0x1d,0x1d,0x2d,0xdd,0x1d,0x1d,0x98,

 0x1d,0x9d,0x2d,0xdd,0xdd,0x9d,0x2d,0x4d,0x1d,0xdd,
 0x7d,0x1d,0x2d,0x2d,0xdd,0xd7,0x1d,0x1d,0x1d,0x2d,
 0x1d,0x1d,0x1d,0x1d,0xdd,0xdd,0x7d,0xdd,0xdd,0xdd
};

/*
===========================================================================
=
= It uses maximum-length Linear Feedback Shift Registers (LFSR) counters.
= You can find a list of them with lengths from 3 to 168 at:
= http://www.xilinx.com/support/documentation/application_notes/xapp052.pdf
= Many thanks to Xilinx for this list!!!
=
===========================================================================
*/

/* XOR masks for the pseudo-random number sequence starting with n=17 bits */
static const U32 rndmasks[] = {
                    /* n    XNOR from (starting at 1, not 0 as usual) */
    0x00012000,     /* 17   17,14       */
    0x00020400,     /* 18   18,11       */
    0x00040023,     /* 19   19,6,2,1    */
    0x00090000,     /* 20   20,17       */
    0x00140000,     /* 21   21,19       */
    0x00300000,     /* 22   22,21       */
    0x00420000,     /* 23   23,18       */
    0x00e10000,     /* 24   24,23,22,17 */
    0x01200000,     /* 25   25,22      (this is enough for 8191x4095) */
};

/* global variables */

PrintConsole bottomScreen;
S16 px;
S16 py;
S32 *wallheight = NULL;
U8 screenfaded;
U8 palshifted = 0;

/* local prototypes */

static void VL_GetPalette(vga_colour *palette);
static void VL_FillPalette(S32 red, S32 green, S32 blue);
static void VL_SetPalette(vga_colour *palette, U8 forceupdate);
static S32 log2_ceil(U32 x);

/*
================================================================
=
= Function: VL_SetVGAPlaneMode
=
= Description:
=
= Setup NDS screen options and locate screen buffer memory
=
================================================================
*/
void VL_SetVGAPlaneMode(void)
{
    /* set top screen to mode 5 double buffer */
    videoSetMode(MODE_5_2D|DISPLAY_BG3_ACTIVE);	    /* BG3 only - extended rotation */
    vramSetBankA(VRAM_A_MAIN_BG_0x06000000);        /* same as VRAM_A_MAIN_BG */
    vramSetBankB(VRAM_B_MAIN_BG_0x06020000);        /* use second bank for main screen - 256 KiB */

    /* set top screen scaling */
    REG_BG3CNT = BG_BMP8_512x512;                   /* BG3 Control register, 8 bits */
    REG_BG3PA = (SCREENWIDTH * 256)/256;            /* Scale X (320 / 256 = 1.25) */
    REG_BG3PB = 0;                                  /* BG X rotation (0 = none) */
    REG_BG3PC = 0;                                  /* BG Y rotation (0 = none) */
    REG_BG3PD = (SCREENHEIGHT * 256)/192;          /* Scale Y (200 / 192 = 1.0416) */
    REG_BG3X = 0;
    REG_BG3Y = 0;

    /* Disable LED blinking if the passcard does not do it for us (DSX) */
    ledBlink(0);

    /* set bottom screen to mode 2 single bufffer*/
    videoSetModeSub(MODE_0_2D|DISPLAY_BG0_ACTIVE);
    vramSetBankC(VRAM_C_SUB_BG);

    /* clear bottom screen and use as console */
    REG_BG0CNT_SUB = BG_MAP_BASE(31);
    BG_PALETTE_SUB[255] = RGB15(31,31,31);
    consoleInit(&bottomScreen,3, BgType_Text4bpp, BgSize_T_256x256, 31, 0, false, true);

    /* clear top screen */
    memset(BG_GFX, 0, 512 * 512 * 2);

    swiWaitForVBlank();
    consoleClear();

    /* get memory for screen buffer */
    screen = (U8 *) malloc((SCREENWIDTH * SCREENHEIGHT));
    CheckMallocResult(screen);

    /* get memory to hold ray slice heights*/
    wallheight = (S32 *) malloc(SCREENWIDTH * sizeof(S32));
    CheckMallocResult(wallheight);

    /* populate current screen palette */
    memcpy(curpal, vga_24bit_pal, sizeof(vga_colour) * 256);
}

/*
================================================================
=
= Function: InitRedShifts
=
= Description:
=
= Setup red and white palette shift arrays
=
================================================================
*/
void InitRedShifts(void)
{
    vga_colour *workptr, *baseptr;
    S32 i, j, delta;

    /* fade through intermediate frames */
    for (i = 1; i <= NUMREDSHIFTS; i++)
    {
        workptr = redshifts[i - 1];
        baseptr = vga_24bit_pal;

        for (j = 0; j <= 255; j++)
        {
            delta = 256 - baseptr->r;
            workptr->r = baseptr->r + delta * i / REDSTEPS;
            delta = -baseptr->g;
            workptr->g = baseptr->g + delta * i / REDSTEPS;
            delta = -baseptr->b;
            workptr->b = baseptr->b + delta * i / REDSTEPS;
            baseptr++;
            workptr++;
        }
    }

    for (i = 1; i <= NUMWHITESHIFTS; i++)
    {
        workptr = whiteshifts[i - 1];
        baseptr = vga_24bit_pal;

        for (j = 0; j <= 255; j++)
        {
            delta = 256 - baseptr->r;
            workptr->r = baseptr->r + delta * i / WHITESTEPS;
            delta = 248 - baseptr->g;
            workptr->g = baseptr->g + delta * i / WHITESTEPS;
            delta = 0-baseptr->b;
            workptr->b = baseptr->b + delta * i / WHITESTEPS;
            baseptr++;
            workptr++;
        }
    }
}

/*
================================================================
=
= Function: SignonScreen
=
= Description:
=
= copy signon screen pic into screen buffer memory
=
================================================================
*/
void SignonScreen(void)
{
    U32 i;

    /* copy signon pic into screen buffer */
    for(i = 0; i < (SCREENWIDTH * SCREENHEIGHT) ;i++)
    {
        screen[i] = signon[i];
    }
}

/*
================================================================
=
= Function: UpdateScreen
=
= Description:
=
= update NDS screen buffer with new image and also update
= palette if required.
=
================================================================
*/
void UpdateScreen(U8 SetPalette)
{
    U32 i;
    S32 h;
    S32 step = 512;
    U8 *destmain = (U8 *)BG_GFX;
    U8 *srcmain = screen;

     /* dma screen buffer into nds video memory */
    for (h = SCREENHEIGHT; h > 0; h--)
    {
        dmaCopy(srcmain, destmain, SCREENWIDTH);
        destmain += step;
        srcmain += SCREENWIDTH;
    }

    if(SetPalette == 1)
    {
        u8 r, g, b;

        for(i = 0; i < 256; i++)
        {
            r = curpal[i].r;
            g = curpal[i].g;
            b = curpal[i].b;

            BG_PALETTE[i]=RGB8(r,g,b);
        }
    }
}

/*
================================================================
=
= Function: VL_Bar
=
= Description:
=
= Draws a rectangle on screen
=
================================================================
*/
void VL_Bar(S32 x, S32 y, S32 width, S32 height, S32 color)
{
    if(((x + width) > SCREENWIDTH)
        && ((y + height) > SCREENHEIGHT))
    {
        printf("VL_Bar: Destination rectangle out of bounds! \n");
        while(1){}; /* hang system */
    }

    U8 *dest = screen + y * SCREENWIDTH + x;

    while((height--) != 0)
    {
        memset(dest, color, width);
        dest += SCREENWIDTH;
    }
}

/*
================================================================
=
= Function: VL_Hlin
=
= Description:
=
= draws a horizontal line
=
================================================================
*/

void VL_Hlin (U32 x, U32 y, U32 width, S32 color)
{
    if(((x + width) > SCREENWIDTH)
        && (y > SCREENHEIGHT))
    {
        printf("VL_Hlin: Destination out of bounds! \n");
        while(1){}; /* hang system */
    }

    U8 *dest = screen + y * SCREENWIDTH + x;
    memset(dest, color, width);
}

/*
================================================================
=
= Function: VL_Vlin
=
= Description:
=
= draws a vertical line
=
================================================================
*/

void VL_Vlin (S32 x, S32 y, S32 height, S32 color)
{
    if((x > SCREENWIDTH)
        && ((y + height) > SCREENHEIGHT))
    {
        printf("VL_Vlin: Destination out of bounds! \n");
        while(1){}; /* hang system */
    }

    U8 *dest = screen + y * SCREENWIDTH + x;

    while((height--) != 0)
    {
        *dest = color;
        dest += SCREENWIDTH;
    }
}

/*
================================================================
=
= Function: VL_GetPixel
=
= Description:
=
= get the colour of a pixel on screen
=
================================================================
*/
U8 VL_GetPixel(S16 x, S16 y)
{
    U8 col;

    if((x > SCREENWIDTH) && (y > SCREENHEIGHT))
    {
        printf(" VL_GetPixel: Destination out of bounds! \n");
        while(1){}; /* hang system */
    }

    col = screen[(y * SCREENWIDTH) + x];

    return col;
}

/*
================================================================
=
= Function: VL_MemToScreen
=
= Description:
=
= Draws a graphics picture chunk from memory into screen buffer
=
================================================================
*/
void VL_MemToScreen(U8 *source, U32 width, U32 height, S32 destx, S32 desty)
{
    if(((destx + width) > SCREENWIDTH)
        &&((desty + height) > SCREENHEIGHT))
    {
        printf("VL_MemToScreen: Destination of picture out of bounds! \n");
        while(1){}; /* hang system */
    }

    U32 j;
    U32 i;
    U32 scj;
    U32 sci;
    U8 col;

    for(j=0,scj=0; j<height; j++, scj++)
    {
        for(i=0,sci=0; i<width; i++, sci++)
        {
            col = (U8)source[(j*(width>>2)+(i>>2))+(i&3)*(width>>2)*height];
            screen[(scj+desty)* SCREENWIDTH + sci + destx] = col;
        }
    }
}

/*
================================================================
=
= Function: VWB_DrawPropString
=
= Description:
=
= draws string to screen
=
================================================================
*/
void VWB_DrawPropString(const char* string)
{
    fontstruct  *font;
    S16         width, step, height;
    U8          *source, *dest;
    U8          ch;
    S16         i;
    U8          fontnumber;
    U8          fontcolor;

    fontnumber = GetFontNum();
    fontcolor  = GetFontColor();
    font = (fontstruct *) grsegs[STARTFONT+fontnumber];
    height = font->height;
    dest = screen + (py * SCREENWIDTH + px);

    while((ch = (U8)*string++) != 0)
    {
        width = step = font->width[ch];
        source = ((U8 *)font)+font->location[ch];

        while((width--) != 0)
        {
            for(i=0;i<height;i++)
            {
                if(source[(i*step)] != 0)
                {
                    dest[(i*SCREENWIDTH)] = fontcolor;
                }
            }

            source++;
            px++;
            dest++;
        }
    }
}

/*
================================================================
=
= Function: VL_Shutdown
=
= Description:
=
= Return screen buffer to heap before shut down
=
================================================================
*/
void VL_Shutdown(void)
{
    if(screen != NULL)
    {
        free(screen);
    }
}


/*
================================================================
=
= Function: VL_FadeOut
=
= Description:
=
= Fades the current palette to the given color in the given number of steps
=
================================================================
*/
void VL_FadeOut(S32 start, S32 end, S32 red, S32 green, S32 blue, S32 steps)
{
    S32         i,j,orig,delta;
    vga_colour   *origptr, *newptr;

    red = red * 255 / 63;
    green = green * 255 / 63;
    blue = blue * 255 / 63;

    Delay_ms(10);
    VL_GetPalette(palette1);
    memcpy(palette2, palette1, sizeof(vga_colour) * 256);


    /* fade through intermediate frames */
    for (i=0;i<steps;i++)
    {
        origptr = &palette1[start];
        newptr = &palette2[start];
        for (j=start;j<=end;j++)
        {
            orig = origptr->r;
            delta = red-orig;
            newptr->r = orig + delta * i / steps;
            orig = origptr->g;
            delta = green-orig;
            newptr->g = orig + delta * i / steps;
            orig = origptr->b;
            delta = blue-orig;
            newptr->b = orig + delta * i / steps;
            origptr++;
            newptr++;
        }

        VL_SetPalette(palette2, 1);
        Delay_ms(10);
    }

    /* final color */
    VL_FillPalette(red,green,blue);

    screenfaded = 1;
}

/*
================================================================
=
= Function: VL_FadeIn
=
= Description:
=
= Fades the current palette to orginal color in the given number of steps
=
================================================================
*/
void VL_FadeIn (S32 start, S32 end,S32 steps)
{
    S32 i,j,delta;

    Delay_ms(10);
    VL_GetPalette(palette1);
    memcpy(palette2, palette1, sizeof(vga_colour) * 256);

    /* fade through intermediate frames */
    for (i=0;i<steps;i++)
    {
        for (j=start;j<=end;j++)
        {
            delta = vga_24bit_pal[j].r-palette1[j].r;
            palette2[j].r = palette1[j].r + delta * i / steps;
            delta = vga_24bit_pal[j].g-palette1[j].g;
            palette2[j].g = palette1[j].g + delta * i / steps;
            delta = vga_24bit_pal[j].b-palette1[j].b;
            palette2[j].b = palette1[j].b + delta * i / steps;
        }

        Delay_ms(10);
        VL_SetPalette(palette2, 1);
    }


    /* final color */
    VL_SetPalette (vga_24bit_pal, 1);

    screenfaded = 0;
}

/*
================================================================
=
= Function: VL_GetPalette
=
= Description:
=
= copy orginal palette into supplied memory buffer
=
================================================================
*/
static void VL_GetPalette(vga_colour *palette)
{
    memcpy(palette, curpal, sizeof(vga_colour) * 256);
}

/*
================================================================
=
= Function: SetPalette
=
= Description:
=
= flash screen white or red by changing screen palette
=
================================================================
*/
void SetPalette(pal_type Pal, S32 ShiftAmount, U8 Update)
{
    if(Pal == red_pal)
    {
        VL_SetPalette(redshifts[ShiftAmount - 1],Update);
    }
    else if(Pal == white_pal)
    {
        VL_SetPalette(whiteshifts[ShiftAmount - 1],Update);
    }
}

/*
================================================================
=
= Function: VL_SetPalette
=
= Description:
=
= changes screen palette to requested one
=
================================================================
*/
static void VL_SetPalette(vga_colour *palette, U8 forceupdate)
{
    S32 i;

    memcpy(curpal, palette, sizeof(vga_colour) * 256);

    if(forceupdate == 1)
    {
        for(i = 0; i < 256; i++)
        {
          u8 r, g, b;
          r = curpal[i].r;
          g = curpal[i].g;
          b = curpal[i].b;

          BG_PALETTE[i]=RGB8(r,g,b);
        }
    }
}

/*
================================================================
=
= Function: VL_FillPalette
=
= Description:
=
= Fill a palette with a given colour
=
================================================================
*/
static void VL_FillPalette(S32 red, S32 green, S32 blue)
{
    S32 i;
    vga_colour pal[256];

    for(i=0; i<256; i++)
    {
        pal[i].r = red;
        pal[i].g = green;
        pal[i].b = blue;
    }

    VL_SetPalette(pal, 1);
}

/*
================================================================
=
= Function: VGAClearScreen
=
= Description:
=
= clear player visable screen (not status bar) to selected
= ceiling / floor colour
=
================================================================
*/
void VGAClearScreen(void)
{
    /* select ceiling colour based on episode / level */
    U8 ceiling = vgaCeiling[gamestate.episode*10+mapon];

    S32 y;
    U8 *ptr = screen;
    ptr += screenofs;

    /* colour ceiling */
    for(y = 0; y < viewheight / 2; y++, ptr += SCREENWIDTH)
    {
        memset(ptr, ceiling, viewwidth);
    }

    /* colour floor */
    for(; y < viewheight; y++, ptr += SCREENWIDTH)
    {
        memset(ptr, 0x19, viewwidth);
    }
}

/*
================================================================
=
= Function: log2_ceil
=
= Description:
=
= Returns the number of bits needed to represent the given value
=
================================================================
*/
static S32 log2_ceil(U32 x)
{
    S32 n = 0;
    U32 v = 1;

    while(v < x)
    {
        n++;
        v <<= 1;
    }

    return n;
}

/*
================================================================
=
= Function: VH_Startup
=
= Description:
=
= call from main init random number array for FizzleFade
=
================================================================
*/
void VL_Startup(void)
{
    S32 rndbits_x = log2_ceil(SCREENWIDTH);

    rndbits_y = log2_ceil(SCREENHEIGHT);

    S32 rndbits = rndbits_x + rndbits_y;

    if(rndbits < 17)
    {
        rndbits = 17;       /* no problem, just a bit slower */
    }
    else if(rndbits > 25)
    {
        rndbits = 25;       /* fizzle fade will not fill whole screen */
    }

    rndmask = rndmasks[rndbits - 17];
}

/*
================================================================
=
= Function: FinishPaletteShifts
=
= Description:
=
= Resets palette to normal if needed
=
================================================================
*/

void FinishPaletteShifts(void)
{
    if(palshifted == 1)
    {
        palshifted = 0;
        VL_SetPalette (vga_24bit_pal, 1);
    }
}

/*
================================================================
=
= Function: FizzleFade
=
= Description:
=
= randomly fade the screen to red
=
================================================================
*/
void FizzleFade(S32 x1, S32 y1, U32 width, U32 height, U32 frames)
{
    S32 lastrndval = 0;
    s32 rndval;
    U32 frame;
    U32 p;
    U32 pixperframe;
    U32 x;
    U32 y;

    /* work out how many pixel to turn red per frame */
    pixperframe = width * height / frames;

    /* get current tic count */
    frame = GetTimeCount();

    /* loop until screen is completely red */
    do
    {
        rndval = lastrndval;

        /* turn some pixels red */
        for(p = 0; p < pixperframe; p++)
        {
            /* seperate random value into x/y pair */
            x = rndval >> rndbits_y;
            y = rndval & ((1 << rndbits_y) - 1);

            /* advance to next random element */
            rndval = (rndval >> 1) ^ (rndval & 1 ? 0 : rndmask);

            if(x >= width || y >= height)
            {
                if(rndval == 0)     /* entire sequence has been completed */
                {
                    UpdateScreen(0);
                    return;
                }

                p--;
                continue;
            }

            /* turn one pixel red */
            screen[((y1 + y) * SCREENWIDTH) + x1 + x] = REDCOLOUR;

            if(rndval == 0) /* entire sequence has been completed */
            {
                UpdateScreen(0);
                return;
            }
        }

        lastrndval = rndval;

        UpdateScreen(0);

        frame++;
        Delay(frame - GetTimeCount());        /* don't go too fast */

    }while (1);
}

/*
================================================================
=
= Function: ScalePost
=
= Description:
=
= scales and draws the texture slice for the wall hit to screen
=
================================================================
*/
void ScalePost(void)
{
    S32 ywcount, yoffs, yw, yd, yendoffs;
    U8  col;

    /* get the wall height for the last pixx and scale by >> 3 */
    ywcount = yd = wallheight[postx] >> 3;

    if(yd <= 0)
    {
        yd = 100;
    }

    /* put y screen offset at the right location so that the */
    /* wall height will be draw correctly into the screen    */
    yoffs = (viewheight / 2 - ywcount) * SCREENWIDTH;

    /* make sure we dont get a negative y screen offset */
    if(yoffs < 0)
    {
        yoffs = 0;
    }

    /* add the x screen offset ie the start location of wall slice */
    yoffs += postx;

    /* work out where the bottom of the wall slice should be */
    yendoffs = viewheight / 2 + ywcount - 1;

    yw = TEXTURESIZE - 1;

    while(yendoffs >= viewheight)
    {
        ywcount -= TEXTURESIZE / 2;

        while(ywcount <= 0)
        {
            ywcount += yd;
            yw--;
        }
        yendoffs--;
    }

    /* no texture to display so return*/
    if(yw < 0)
    {
        return;
    }

    /* get the first byte of the texture to display */
    col = postsource[yw];

    /* work out the coord's of the end of the wall slice */
    yendoffs = yendoffs * SCREENWIDTH + postx;

    /* draw wall slice texture to screen */
    while(yoffs <= yendoffs)
    {
        /* draw from the bottom of the slice upwards */
        screen[(yendoffs + screenofs)] = col;

        ywcount -= TEXTURESIZE/2;

        /* scale the wall texture up / down depending on */
        /* size difference between splice height and texture height */
        if(ywcount <= 0)
        {
            do
            {
                ywcount += yd;
                yw--;
            }while(ywcount <= 0);

            if(yw < 0)
            {
                break;
            }

            col = postsource[yw];
        }

        /* reduce the offset by one y width */
        yendoffs -= SCREENWIDTH;
    }
}

/*
================================================================
=
= Function: ScaleShape
=
= Description:
=
= draw shapenum to screen but scaled according to view height
= and view width
=
================================================================
*/

void ScaleShape(S32 xcenter, S32 shapenum, U32 height, U32 flags)
{
    t_compshape *shape;
    U32 scale,pixheight;
    U32 starty,endy;
    U16 *cmdptr;
    U8 *cline;
    U8 *line;
    U8 *vmem;
    S32 actx,i,upperedge;
    S16 newstart;
    S32 scrstarty,screndy,lpix,rpix,pixcnt,ycnt;
    U32 j;
    U8 col;

    shape = (t_compshape *) PM_GetSprite(shapenum);

    scale = height>>3;     /* low three bits are fractional */

    if(scale == 0)
    {
        return;   /* too close or far away */
    }

    pixheight = scale * SPRITESCALEFACTOR;
    actx = xcenter - scale;
    upperedge = viewheight/2-scale;

    cmdptr = (U16 *) shape->dataofs;

    for(i = shape->leftpix, pixcnt = i*pixheight, rpix=(pixcnt>>6)+actx; i <= shape->rightpix ; i++, cmdptr++)
    {
        lpix = rpix;

        if(lpix>=viewwidth)
        {
            break;
        }

        pixcnt += pixheight;
        rpix = (pixcnt>>6)+actx;

        if((lpix!=rpix) && (rpix>0))
        {
            if(lpix < 0)
            {
                lpix=0;
            }

            if(rpix > viewwidth)
            {
                rpix = viewwidth;
                i = shape->rightpix+1;
            }

            cline = (U8 *)shape + *cmdptr;

            while(lpix<rpix)
            {
                if(wallheight[lpix] <= (S32)height)
                {
                    line=cline;

                    while((endy = READWORD(line)) != 0)
                    {
                        line += 2;
                        endy >>= 1;
                        newstart = READWORD(line);
                        line += 2;
                        starty = READWORD(line) >> 1;
                        line += 2;
                        j=starty;
                        ycnt=j*pixheight;
                        screndy=(ycnt>>6)+upperedge;

                        if(screndy<0)
                        {
                            vmem = screen + lpix + screenofs;
                        }
                        else
                        {
                            vmem = screen + screndy * SCREENWIDTH + lpix + screenofs;
                        }

                        for(;j<endy;j++)
                        {
                            scrstarty=screndy;
                            ycnt+=pixheight;
                            screndy=(ycnt>>6)+upperedge;

                            if((scrstarty!=screndy) && (screndy>0))
                            {
                                col=((U8 *)shape)[newstart+j];

                                if(scrstarty<0)
                                {
                                    scrstarty=0;
                                }

                                if(screndy>viewheight)
                                {
                                    screndy=viewheight;
                                    j=endy;
                                }

                                while(scrstarty<screndy)
                                {
                                    *vmem = col;
                                    vmem += SCREENWIDTH;
                                    scrstarty++;
                                }
                            }
                        }
                    }
                }

                lpix++;
            }
        }
    }
}

/*
================================================================
=
= Function: SimpleScaleShape
=
= Description:
=
= draw shapenum to screen but scaled according to view height
= and view width
=
================================================================
*/
void SimpleScaleShape(S32 xcenter, S32 shapenum, U32 height)
{
    t_compshape   *shape;
    U16 *cmdptr;
    U8 *cline;
    U8 *line;
    U8 *vmem;
    U32 scale;
    U32 pixheight;
    U32 endy;
    U32 starty;
    U32 j;
    S32 ycnt;
    S32 screndy;
    S32 actx;
    S32 upperedge;
    S32 i;
    S32 pixcnt;
    S32 rpix;
    S32 lpix;
    S32 scrstarty;
    S16 newstart;
    U8 col;

    shape = (t_compshape *) PM_GetSprite(shapenum);

    scale = height >> 1;

    pixheight = scale * SPRITESCALEFACTOR;

    actx = xcenter - scale;
    upperedge = viewheight/2 - scale;

    cmdptr = shape->dataofs;

    for(i = shape->leftpix, pixcnt = i*pixheight, rpix = (pixcnt>>6)+actx ; i <= shape->rightpix ; i++,cmdptr++)
    {
        lpix = rpix;

        if(lpix>=viewwidth)
        {
            break;
        }

        pixcnt += pixheight;

        rpix = (pixcnt>>6) + actx;

        if((lpix != rpix) && (rpix > 0))
        {
            if(lpix<0)
            {
                lpix=0;
            }

            if(rpix > viewwidth)
            {
                rpix = viewwidth;
                i = shape->rightpix+1;
            }

            cline = (U8 *)shape + *cmdptr;

            while(lpix<rpix)
            {
                line = cline;

                while((endy = READWORD(line)) != 0)
                {
                    line += 2;
                    endy >>= 1;
                    newstart = READWORD(line);
                    line += 2;
                    starty = READWORD(line) >> 1;
                    line += 2;
                    j = starty;
                    ycnt = j*pixheight;
                    screndy = (ycnt>>6)+upperedge;

                    if(screndy < 0)
                    {
                        vmem = screen + lpix + screenofs;
                    }
                    else
                    {
                        vmem = screen + screndy* SCREENWIDTH + lpix + screenofs;
                    }

                    for(;j<endy;j++)
                    {
                        scrstarty = screndy;
                        ycnt += pixheight;
                        screndy = (ycnt>>6)+upperedge;

                        if((scrstarty != screndy) && (screndy > 0))
                        {
                            col = ((U8 *)shape)[newstart+j];

                            if(scrstarty < 0)
                            {
                                scrstarty = 0;
                            }

                            if(screndy>viewheight)
                            {
                                screndy = viewheight;
                                j = endy;
                            }

                            while(scrstarty < screndy)
                            {
                                *vmem = col;
                                vmem += SCREENWIDTH;
                                scrstarty++;
                            }
                        }
                    }
                }

                lpix++;
            }
        }
    }
}
