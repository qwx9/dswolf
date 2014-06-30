#ifndef WL_DEF_H
#define WL_DEF_H

#define TEXTCOLOR           0x17
#define BKGDCOLOR           0x2d
#define READHCOLOR          0x47
#define MenuFadeOut()       VL_FadeOut(0,255,43,0,0,10)
#define INTROSONG           NAZI_NOR_MUS
#define TickBase            70      /* 70Hz per tick - used as a base for timer 2 */
#define VW_FadeIn()         VL_FadeIn(0,255,30);
#define VW_FadeOut()        VL_FadeOut(0,255,0,0,0,30);
#define MenuFadeIn()        VL_FadeIn(0,255,10)
#define SCREENWIDTH         320
#define SCREENHEIGHT        200
#define VIEWCOLOR           0x7f
#define STATUSLINES         40
#define CTL_X               24
#define CTL_Y               86
#define STARTAMMO           8
#define EXTRAPOINTS         40000
#define mapshift            6
#define MAPSIZE             (1<<mapshift)
#define MAPPLANES           2
#define MAPHEIGHT           MAPSIZE
#define MAPWIDTH            MAPSIZE
#define FINEANGLES          3600
#define ANGLES              360
#define TILESHIFT           16l
#define GLOBAL1             (1l<<16)
#define TILEGLOBAL          GLOBAL1
#define NORTH               0
#define MAXWALLTILES        64          /* max number of wall tiles */
#define MINDIST             (0x5800l)
#define FOCALLENGTH         (0x5700l)   /* in global coordinates */
#define TEXTURESHIFT        6
#define TEXTURESIZE         (1<<TEXTURESHIFT)
#define TEXTUREFROMFIXEDSHIFT 4
#define TEXTUREMASK         (TEXTURESIZE*(TEXTURESIZE-1))
#define STR_VS              "View Scores"
#define ClearMemory         SD_StopDigitized
#define AMBUSHTILE          106
#define MAXDOORS            64    /* max number of sliding doors */
#define AREATILE            107   /* first of NUMAREAS floor tiles */
#define ISPOINTER(x)        ((((uintptr_t)(x)) & ~0xffff) != 0)
#define MAPAREA             MAPSIZE*MAPSIZE
#define MAXSTATS            400   /* max number of lamps, bonus, etc */
#define NUMREDSHIFTS        6
#define NUMWHITESHIFTS      3
#define NUMAREAS            37
#define UNSIGNEDSHIFT       8
#define MINACTORDIST        0x10000l    /* minimum dist from player center */
#define PlaySoundLocActor(s,ob)    PlaySoundLocGlobal(s,(ob)->x,(ob)->y)
#define SPDPATROL           512
#define MAXACTORS           150         /* max number of nazis, etc / map */
#define LRpack              20    /* # of levels to store in endgame */
#define BASEMOVE            35
#define RUNMOVE             70

/* 
================================================================
=
= Function: FixedMul
=
= Description:
=
= multiply two S32 numbers (and round result up)
= then store result back into a S32
=
================================================================
*/
static inline fixed FixedMul(fixed a, fixed b)
{
	
    return (fixed)(((long long)a * b + 0x8000) >> 16);
} 

#endif