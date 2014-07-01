#include <nds.h>
#include <stdio.h>
#include <stdlib.h>
#include "types.h"
#include "wl_def.h"
#include "id_vl.h"
#include "id_pm.h"
#include "id_time.h"
#include "wl_agent.h"

/* macro defines */

/* the door is the last picture before the sprites */
#define DOORWALL            (PMSpriteStart-8)
#define NUMWEAPONS          4
#define MAXTICS             10
#define MAXVISABLE          250
#define ACTORSIZE           0x4000

/* external variables */

extern objtype *player;
extern fixed sintable[ANGLES+ANGLES/4];
extern S32 viewheight;
extern S32 viewwidth;
extern S16 *pixelangle;
extern fixed finetangent[FINEANGLES/4];
extern U8 tilemap[MAPSIZE][MAPSIZE];
extern S32 *wallheight;
extern S32 heightnumerator;
extern S32 focallength;
extern S32 PMSpriteStart;
extern gametype gamestate;
extern U32 tics;
extern U16 doorposition[MAXDOORS];    /* leading edge of door 0=closed, 0xffff = fully open */
extern doorobj_t doorobjlist[MAXDOORS];
extern U8 spotvis[MAPSIZE][MAPSIZE];
extern statobj_t    statobjlist[MAXSTATS];
extern statobj_t    *laststatobj;
extern S16 centerx;
extern S32 scale;

/* local variables */

static fixed    viewx;
static fixed    viewy;        /* the focal point */
static S16      viewangle;
static fixed    viewsin;
static fixed    viewcos;
static S16      midangle;
static S32      min_wallheight;

/* ray tracing variables */
static S16    focaltx;
static S16    focalty;
static S16    viewtx;
static S16    viewty;
static U32    xpartialup;
static U32    xpartialdown;
static U32    ypartialup;
static U32    ypartialdown;
static S32    pixx;
static S16    xtilestep;
static S16    ytilestep;
static S32    xintercept;
static S32    yintercept;
static S16    xtile;
static S16    ytile;
static U16    xspot;
static U16    yspot;
static S32    texdelta;
static U16    tilehit;

static S32    lastside;
static S32    lastintercept;
static S32    lasttilehit;
static S32    lasttexture;

static S16    weaponscale[NUMWEAPONS] = {SPR_KNIFEREADY, SPR_PISTOLREADY, SPR_MACHINEGUNREADY, SPR_CHAINREADY};

static S32    dirangle[9] = {0,ANGLES/8,2*ANGLES/8,3*ANGLES/8,4*ANGLES/8, 5*ANGLES/8,6*ANGLES/8,7*ANGLES/8,ANGLES};

/* global variables */

U16 horizwall[MAXWALLTILES];
U16 vertwall[MAXWALLTILES];
S32    postx;
U8     *postsource;
U32    lasttimecount;
visobj_t vislist[MAXVISABLE];
fixed *costable = sintable+(ANGLES/4);

/* local prototypes */

static void CalcViewVariables(void);
static void WallRefresh(void);
static void AsmRefresh(void);
static void HitHorizWall(void);
static void HitVertWall(void);
static S32 CalcHeight(void);
static void DrawPlayerWeapon(void);
static void HitVertDoor(void);
static void HitHorizDoor(void);
static void DrawScaleds(void);
static U8 TransformTile (S32 tx, S32 ty, S16 *dispx, S16 *dispheight);
static void TransformActor(objtype *ob);
static S32 CalcRotate (objtype *ob);

/*
================================================================
=
= Function: ThreeDRefresh
=
= Description:
=
= Update view frame
=
================================================================
*/
void ThreeDRefresh(void)
{

    memset(spotvis,0,MAPAREA);
    spotvis[player->tilex][player->tiley] = 1;    /* Detect all sprites over player fix */

    CalcViewVariables();

    /* clear last frame and paint ceiling / floor colour */
    VGAClearScreen();

    /* re-draw walls  */
    WallRefresh();

    /* draw all the scaled images */
    DrawScaleds();

    DrawPlayerWeapon();

    UpdateScreen(0);
}

/*
================================================================
=
= Function: CalcViewVariables
=
= Description:
=
= setup view variables before updating frame
=
================================================================
*/
static void CalcViewVariables(void)
{
    viewangle = player->angle;
    midangle = viewangle*(FINEANGLES/ANGLES);
    viewsin = sintable[viewangle];
    viewcos = costable[viewangle];

    /* convert players position into screen x ,y coords */
    viewx = player->x - FixedMul(focallength,viewcos);
    viewy = player->y + FixedMul(focallength,viewsin);

    /* set screen tile coords */
    focaltx = (S16)(viewx>>TILESHIFT);
    focalty = (S16)(viewy>>TILESHIFT);

    /* set players tile coords */
    viewtx = (S16)(player->x >> TILESHIFT);
    viewty = (S16)(player->y >> TILESHIFT);
}

/*
================================================================
=
= Function: WallRefresh
=
= Description:
=
= re-draw walls for this frame
=
================================================================
*/
static void WallRefresh(void)
{
    /* distance to left size of tile */
    xpartialdown = viewx&(TILEGLOBAL-1);
    /* distance to right side of tile*/
    xpartialup = TILEGLOBAL-xpartialdown;
    /* distance to top of tile */
    ypartialdown = viewy&(TILEGLOBAL-1);
    /* distance to bottom of tile */
    ypartialup = TILEGLOBAL-ypartialdown;

    min_wallheight = viewheight;
    lastside = -1;                  /* the first pixel is on a new wall */
    AsmRefresh();
    ScalePost();                    /* no more optimization on last post */
}

/*
================================================================
=
= Function: AsmRefresh
=
= Description:
=
= do ray casting stuff and then call functions to workout wall
= distances.
=
================================================================
*/
static void AsmRefresh(void)
{
    S32 xstep = 0;      /* holds distance to next x ray intercept */
    S32 ystep = 0;      /* holds distance to next y ray intercept */
    U32 xpartial = 0;   /* holds distance between player and tile side */
    U32 ypartial = 0;
    S16 angl = 0;

    for(pixx=0; pixx<viewwidth; pixx++)
    {
        angl = midangle + pixelangle[pixx];

        if(angl<0)
        {
            angl+= FINEANGLES;
        }

        if(angl>=3600)
        {
            angl-= FINEANGLES;
        }

        if(angl<900)
        {
            xtilestep = 1;
            ytilestep = -1;
            xstep = finetangent[900-1-angl];
            ystep =- finetangent[angl];
            xpartial = xpartialup;
            ypartial = ypartialdown;
        }
        else if(angl<1800)
        {
            xtilestep =- 1;
            ytilestep =- 1;
            xstep =- finetangent[angl-900];
            ystep =- finetangent[1800-1-angl];
            xpartial = xpartialdown;
            ypartial = ypartialdown;
        }
        else if(angl<2700)
        {
            xtilestep =- 1;
            ytilestep = 1;
            xstep =- finetangent[2700-1-angl];
            ystep = finetangent[angl-1800];
            xpartial = xpartialdown;
            ypartial = ypartialup;
        }
        else if(angl<3600)
        {
            xtilestep = 1;
            ytilestep = 1;
            xstep = finetangent[angl-2700];
            ystep = finetangent[3600-1-angl];
            xpartial = xpartialup;
            ypartial = ypartialup;
        }

        yintercept = FixedMul(ystep,xpartial)+viewy;
        xtile = focaltx+xtilestep;
        xspot = (U16)((xtile<<mapshift)+((U32)yintercept>>16));

        xintercept = FixedMul(xstep,ypartial)+viewx;
        ytile = focalty+ytilestep;
        yspot = (U16)((((U32)xintercept>>16)<<mapshift)+ytile);
        texdelta = 0;

        do
        {
            if(ytilestep == -1 && (yintercept>>16)<=ytile) goto horizentry;
            if(ytilestep == 1 && (yintercept>>16)>=ytile) goto horizentry;
vertentry:

            tilehit = ((U8 *)tilemap)[xspot];

            if(tilehit != 0)
            {
                /* check to see if we need to draw a door */
                if(tilehit & 0x80)
                {
                    S32 yintbuf = yintercept + (ystep>>1);
                    if((yintbuf>>16)!=(yintercept>>16))
                    {
                        goto passvert;
                    }

                    /* is door fully open ? */
                    if((U16)yintbuf < doorposition[(tilehit & 0x7f)])
                    {
                        goto passvert;
                    }

                    yintercept= yintbuf;
                    xintercept= (xtile<<TILESHIFT)|0x8000;
                    ytile = (S16) (yintercept >> TILESHIFT);
                    HitVertDoor();
                }
                else
                {
                    if(tilehit == 64)
                    {
                        /* do something */
                    }
                    else
                    {
                        xintercept=xtile<<TILESHIFT;
                        ytile = (S16) (yintercept >> TILESHIFT);
                        HitVertWall();
                    }
                }

                break;
            }
passvert:
            /* mark this tile visable as ray has just passed through it */
            *((U8 *)spotvis+xspot) = 1;
            xtile += xtilestep;
            yintercept += ystep;
            xspot = (U16)((xtile<<mapshift)+((U32)yintercept>>16));

        }while(1);

        continue;

        do
        {
            if(xtilestep == -1 && (xintercept>>16)<=xtile) goto vertentry;
            if(xtilestep == 1 && (xintercept>>16)>=xtile) goto vertentry;
horizentry:

            tilehit=((U8 *)tilemap)[yspot];
            if(tilehit != 0)
            {
                /* check to see if we need to draw a door */
                if(tilehit & 0x80)
                {
                    S32 xintbuf=xintercept + (xstep>>1);
                    if((xintbuf>>16)!=(xintercept>>16))
                    {
                        goto passhoriz;
                    }

                    /* is door fully open ? */
                    if((U16)xintbuf<doorposition[(tilehit & 0x7f)])
                    {
                        goto passhoriz;
                    }

                    xintercept = xintbuf;
                    yintercept = (ytile<<TILESHIFT) + 0x8000;
                    xtile = (S16) (xintercept >> TILESHIFT);
                    HitHorizDoor();
                }
                else
                {
                    if(tilehit == 64)
                    {
                        /* do something */
                    }
                    else
                    {
                        yintercept=ytile<<TILESHIFT;
                        xtile = (S16) (xintercept >> TILESHIFT);
                        HitHorizWall();
                    }
                }

                break;
            }
passhoriz:
            *((U8 *)spotvis+yspot) = 1;
            ytile+=ytilestep;
            xintercept+=xstep;
            yspot=(U16)((((U32)xintercept>>16)<<mapshift)+ytile);

        }while(1);
    }
}

/*
================================================================
=
= Function: HitHorizWall
=
= Description:
=
= tilehit bit 7 is 0, because it's not a door tile
= if bit 6 is 1 and the adjacent tile is a door tile, use door side pic
=
================================================================
*/
static void HitHorizWall(void)
{
    S32 wallpic;
    S32 texture;

    /* work out texture offset (texture slice to use) */
    texture = ((xintercept+texdelta)>>TEXTUREFROMFIXEDSHIFT) & TEXTUREMASK;

    if(ytilestep == -1)
    {
        yintercept += TILEGLOBAL;
    }
    else
    {
        texture = TEXTUREMASK - texture;
    }

    /* did we hit the same tile as the last time we entered into this function */
    if((lastside == 0) && (lastintercept == ytile) && (lasttilehit == tilehit) && !(lasttilehit & 0x40))
    {
        if((pixx & 0x03) && (texture == lasttexture))
        {
            ScalePost();
            postx = pixx;
            wallheight[pixx] = wallheight[pixx-1];
            return;
        }

        ScalePost();
        wallheight[pixx] = CalcHeight();
        postsource += texture-lasttexture;
        postx = pixx;
        lasttexture = texture;
        return;
    }

    if(lastside != -1)
    {
        ScalePost();
    }

    /* new wall */
    lastside = 0;
    lastintercept = ytile;
    lasttilehit = tilehit;
    lasttexture = texture;
    wallheight[pixx] = CalcHeight();
    postx = pixx;

    if((tilehit & 0x40) == 0x40)
    {
        /* check for adjacent doors */
        xtile = (S16)(xintercept>>TILESHIFT);

        if(tilemap[xtile][ytile-ytilestep] & 0x80)
        {
            wallpic = DOORWALL + 2;
        }
        else
        {
            wallpic = horizwall[tilehit & ~0x40];
        }
    }
    else
    {
        wallpic = horizwall[tilehit];
    }

    postsource = PM_GetTexture(wallpic) + texture;
}

/*
================================================================
=
= Function: HitVertWall
=
= Description:
=
= tilehit bit 7 is 0, because it's not a door tile
= if bit 6 is 1 and the adjacent tile is a door tile, use door side pic
=
================================================================
*/
static void HitVertWall(void)
{
    S32 wallpic;
    S32 texture;

    /* work out texture offset (texture slice to use) */
    texture = ((yintercept+texdelta)>>TEXTUREFROMFIXEDSHIFT)&TEXTUREMASK;

    if(xtilestep == -1)
    {
        texture = TEXTUREMASK-texture;
        xintercept += TILEGLOBAL;
    }

    /* did we hit the same tile as the last time we entered into this function */
    if((lastside == 1) && (lastintercept == xtile) && (lasttilehit == tilehit) && !(lasttilehit & 0x40))
    {
        if((pixx&3) && (texture == lasttexture))
        {
            ScalePost();
            postx = pixx;
            wallheight[pixx] = wallheight[pixx-1];
            return;
        }

        ScalePost();
        wallheight[pixx] = CalcHeight();
        postsource += texture-lasttexture;
        postx = pixx;
        lasttexture = texture;
        return;
    }

    if(lastside!=-1)
    {
        ScalePost();
    }

    lastside=1;
    lastintercept=xtile;
    lasttilehit=tilehit;
    lasttexture=texture;
    wallheight[pixx] = CalcHeight();
    postx = pixx;

    if (tilehit & 0x40)
    {   /* check for adjacent doors */
        ytile = (short)(yintercept>>TILESHIFT);

        if((tilemap[xtile-xtilestep][ytile] & 0x80) == 0x80)
        {
            wallpic = DOORWALL+3;
        }
        else
        {
            wallpic = vertwall[tilehit & ~0x40];
        }
    }
    else
    {
        wallpic = vertwall[tilehit];
    }

    postsource = PM_GetTexture(wallpic) + texture;
}

/*
================================================================
=
= Function: HitVertDoor
=
= Description:
=
= Tile hit was a door (bit 7 of tile was set) so handle drawing
= door here.
=
================================================================
*/

static void HitVertDoor(void)
{
    S32 doorpage;
    S32 doornum;
    S32 texture;

    /* get door number hit */
    doornum = tilehit & 0x7f;

    /* work out texture offset (texture slice to use) */
    texture = ((yintercept-doorposition[doornum])>>TEXTUREFROMFIXEDSHIFT)&TEXTUREMASK;

    /* did we hit the same tile as the last time we entered into this function */
    if(lasttilehit == tilehit)
    {
        if((pixx&3) && texture == lasttexture)
        {
            ScalePost();
            postx=pixx;
            wallheight[pixx] = wallheight[pixx-1];
            return;
        }

        ScalePost();
        wallheight[pixx] = CalcHeight();
        postsource+=texture-lasttexture;
        postx=pixx;
        lasttexture=texture;
        return;
    }

    if(lastside!=-1)
    {
        ScalePost();
    }

    lastside=2;
    lasttilehit=tilehit;
    lasttexture=texture;
    wallheight[pixx] = CalcHeight();
    postx = pixx;

    switch(doorobjlist[doornum].lock)
    {
        case dr_normal:
            doorpage = DOORWALL+1;
        break;

        case dr_lock1:
        case dr_lock2:
        case dr_lock3:
        case dr_lock4:
            doorpage = DOORWALL+7;
        break;

        case dr_elevator:
            doorpage = DOORWALL+5;
        break;

        default:
            doorpage = DOORWALL+1; /* stop compile warning */
        break;
    }

    postsource = PM_GetTexture(doorpage) + texture;
}

/*
================================================================
=
= Function: HitHorizDoor
=
= Description:
=
= Tile hit was a door (bit 7 of tile was set) so handle drawing
= door here.
=
================================================================
*/
static void HitHorizDoor(void)
{
    S32 doorpage;
    S32 doornum;
    S32 texture;

    /* get door number hit */
    doornum = tilehit&0x7f;
    /* work out texture offset (texture slice to use) */
    texture = ((xintercept-doorposition[doornum])>>TEXTUREFROMFIXEDSHIFT)&TEXTUREMASK;

    /* did we hit the same tile as the last time we entered into this function */
    if(lasttilehit==tilehit)
    {
        if((pixx&3) && texture == lasttexture)
        {
            ScalePost();
            postx=pixx;
            wallheight[pixx] = wallheight[pixx-1];
            return;
        }
        ScalePost();
        wallheight[pixx] = CalcHeight();
        postsource+=texture-lasttexture;
        postx=pixx;
        lasttexture=texture;
        return;
    }

    if(lastside!=-1)
    {
        ScalePost();
    }

    lastside=2;
    lasttilehit=tilehit;
    lasttexture=texture;
    wallheight[pixx] = CalcHeight();
    postx = pixx;

    switch(doorobjlist[doornum].lock)
    {
        case dr_normal:
            doorpage = DOORWALL;
        break;

        case dr_lock1:
        case dr_lock2:
        case dr_lock3:
        case dr_lock4:
            doorpage = DOORWALL+6;
        break;

        case dr_elevator:
            doorpage = DOORWALL+4;
        break;

        default:
            doorpage = DOORWALL; /* stop compile warning */
        break;
    }

    postsource = PM_GetTexture(doorpage) + texture;
}

/*
================================================================
=
= Function: TransformActor
=
= Description:
=
= Takes paramaters:
=   gx,gy               : globalx/globaly of point
=
= globals:
=   viewx,viewy         : point of view
=   viewcos,viewsin     : sin/cos of viewangle
=   scale               : conversion from global value to screen value
=
= sets:
=   screenx,transx,transy,screenheight: projected edge location and size
=
================================================================
*/

static void TransformActor (objtype *ob)
{
    fixed gx,gy,gxt,gyt,nx,ny;

    /* translate point to view centered coordinates */
    gx = ob->x-viewx;
    gy = ob->y-viewy;

    /* calculate newx */
    gxt = FixedMul(gx,viewcos);
    gyt = FixedMul(gy,viewsin);

    nx = gxt-gyt-ACTORSIZE;     /* fudge the shape forward a bit, because    */
                                 /* the midpoint could put parts of the shape */
                                 /* into an adjacent wall                     */

    /* calculate newy */
    gxt = FixedMul(gx,viewsin);
    gyt = FixedMul(gy,viewcos);
    ny = gyt+gxt;

    /* calculate perspective ratio */
    ob->transx = nx;
    ob->transy = ny;

    if (nx<MINDIST)                 /* too close, don't overflow the divide */
    {
        ob->viewheight = 0;
        return;
    }

    ob->viewx = (U16)(centerx + ny*scale/nx);

    /* calculate height (heightnumerator/(nx>>8)) */
    ob->viewheight = (U16)(heightnumerator/(nx>>8));
}

/*
================================================================
=
= Function: TransformTile
=
= Description:
=
= Takes paramaters:
=   tx,ty               : tile the object is centered in
=
= globals:
=   viewx,viewy         : point of view
=   viewcos,viewsin     : sin/cos of viewangle
=   scale               : conversion from global value to screen value
=
= sets:
=   screenx,transx,transy,screenheight: projected edge location and size
=
= Returns true if the tile is withing getting distance
=
================================================================
*/

static U8 TransformTile (S32 tx, S32 ty, S16 *dispx, S16 *dispheight)
{
    fixed gx,gy,gxt,gyt,nx,ny;

    /* translate point to view centered coordinates */
    gx = ((int32_t)tx<<TILESHIFT)+0x8000-viewx;
    gy = ((int32_t)ty<<TILESHIFT)+0x8000-viewy;

    /* calculate newx */
    gxt = FixedMul(gx,viewcos);
    gyt = FixedMul(gy,viewsin);
    nx = gxt-gyt-0x2000;            /* 0x2000 is size of object */

    /* calculate newy */
    gxt = FixedMul(gx,viewsin);
    gyt = FixedMul(gy,viewcos);
    ny = gyt+gxt;

    /* calculate height / perspective ratio */
    if (nx<MINDIST)                 /* too close, don't overflow the divide */
    {
        *dispheight = 0;
    }
    else
    {
        *dispx = (S16)(centerx + ny*scale/nx);
        *dispheight = (S16)(heightnumerator/(nx>>8));
    }

    /* see if it should be grabbed */
    if (nx<TILEGLOBAL && ny>-TILEGLOBAL/2 && ny<TILEGLOBAL/2)
    {
        return 1;
    }
    else
    {
        return 0;
    }
}

/*
================================================================
=
= Function: CalcRotate
=
= Description:
=
=
=
================================================================
*/
static S32 CalcRotate (objtype *ob)
{
    S32 angle;
    S32 viewangle;

    /* this isn't exactly correct, as it should vary by a trig value, */
    /* but it is close enough with only eight rotations               */

    viewangle = player->angle + (centerx - ob->viewx)/8;

    if (ob->obclass == rocketobj || ob->obclass == hrocketobj)
    {
        angle = (viewangle-180) - ob->angle;
    }
    else
    {
        angle = (viewangle-180) - dirangle[ob->dir];
    }

    angle+=ANGLES/16;

    while (angle>=ANGLES)
    {
        angle-=ANGLES;
    }

    while (angle<0)
    {
        angle+=ANGLES;
    }

    if (ob->state->rotate == 2)    /* 2 rotation pain frame            */
    {
        return 0;                   /* pain with shooting frame bugfix  */
    }

    return angle/(ANGLES/8);
}

/*
================================================================
=
= Function: CalcHeight
=
= Description:
=
= Calculates the height of xintercept,yintercept from viewx,viewy
=
================================================================
*/
static S32 CalcHeight(void)
{
    fixed z = FixedMul(xintercept - viewx, viewcos)
        - FixedMul(yintercept - viewy, viewsin);

    if(z < MINDIST)
    {
        z = MINDIST;
    }

    S32 height = heightnumerator / (z >> 8);

    if(height < min_wallheight)
    {
        min_wallheight = height;
    }

    return height;
}

/*
================================================================
=
= Function: DrawScaleds
=
= Description:
=
= Draws all objects that are visable
=
================================================================
*/
static void DrawScaleds(void)
{
    S32   i;
    S32   least;
    S32   numvisable;
    S32   height;
    visobj_t *visptr = NULL;
    visobj_t *visstep = NULL;
    visobj_t *farthest = NULL;
    statobj_t *statptr = NULL;

    U8    *tilespot;
    U8    *visspot;
    U32   spotloc;
    objtype   *obj;

    visptr = &vislist[0];

    /* place static objects */
    for (statptr = &statobjlist[0] ; statptr !=laststatobj ; statptr++)
    {
        visptr->shapenum = statptr->shapenum;

        if ((visptr->shapenum) == -1)
        {
            continue;    /* object has been deleted */
        }

        if(*(statptr->visspot) == 0)
        {
            continue;    /* not visable */
        }

        if (TransformTile (statptr->tilex,statptr->tiley,
            &visptr->viewx,&visptr->viewheight) && (statptr->flags & FL_BONUS))
        {
            GetBonus(statptr);

            if(statptr->shapenum == -1)
            {
                continue;    /* object has been taken */
            }
        }

        if(visptr->viewheight == 0)
        {
            continue;    /* to close to the object */
        }

        /* don't let it overflow */
        if (visptr < &vislist[MAXVISABLE-1])
        {
            visptr->flags = (S16) statptr->flags;
            visptr++;
        }
    }

    /* place active objects */

    for (obj = player->next ; obj != NULL ; obj = obj->next)
    {
        visptr->shapenum = obj->state->shapenum;

        if (visptr->shapenum == 0)
        {
            continue; /* no shape */
        }

        spotloc = (obj->tilex<<mapshift)+obj->tiley;   /* optimize: keep in struct? */
        visspot = &spotvis[0][0]+spotloc;
        tilespot = &tilemap[0][0]+spotloc;

        /* could be in any of the nine surrounding tiles */
        if (*visspot
            || ( *(visspot-1) && !*(tilespot-1) )
            || ( *(visspot+1) && !*(tilespot+1) )
            || ( *(visspot-65) && !*(tilespot-65) )
            || ( *(visspot-64) && !*(tilespot-64) )
            || ( *(visspot-63) && !*(tilespot-63) )
            || ( *(visspot+65) && !*(tilespot+65) )
            || ( *(visspot+64) && !*(tilespot+64) )
            || ( *(visspot+63) && !*(tilespot+63) ) )
        {
            obj->active = ac_yes;
            TransformActor(obj);

            if(obj->viewheight == 0)
            {
                continue;    /* too close or far away */
            }

            visptr->viewx = obj->viewx;
            visptr->viewheight = obj->viewheight;

            if (visptr->shapenum == -1)
            {
                visptr->shapenum = obj->temp1;  /* special shape */
            }

            if(obj->state->rotate == 1)
            {
                visptr->shapenum += CalcRotate(obj);
            }

            if (visptr < &vislist[MAXVISABLE-1])    /* don't let it overflow */
            {
                visptr->flags = (S16) obj->flags;
                visptr++;
            }

            obj->flags |= FL_VISABLE;
        }
        else
        {
            obj->flags &= ~FL_VISABLE;
        }
    }

    /* draw from back to front */
    numvisable = (S32) (visptr-&vislist[0]);

    if(numvisable == 0)
    {
        return;    /* no visable objects */
    }

    for (i = 0; i<numvisable; i++)
    {
        least = 32000;

        for (visstep = &vislist[0] ; visstep<visptr ; visstep++)
        {
            height = visstep->viewheight;

            if (height < least)
            {
                least = height;
                farthest = visstep;
            }
        }

        /* draw farthest */
        ScaleShape(farthest->viewx, farthest->shapenum, farthest->viewheight, farthest->flags);

        farthest->viewheight = 32000;
    }
}

/*
================================================================
=
= Function: DrawPlayerWeapon
=
= Description:
=
= Draw the player's hands
=
================================================================
*/
static void DrawPlayerWeapon(void)
{
    S32 shapenum;

    if(gamestate.victoryflag)
    {
        return;
    }

    if(gamestate.weapon != wp_none)
    {
        shapenum = weaponscale[gamestate.weapon] + gamestate.weaponframe;
        SimpleScaleShape(viewwidth/2,shapenum,viewheight+1);
    }

}

/*
================================================================
=
= Function: CalcTics
=
= Description:
=
= calculate how many tic's per frame
=
================================================================
*/
void CalcTics(void)
{
    U32 newtime;

    /* calculate tics since last refresh for adaptive timing */
    if(lasttimecount > GetTimeCount())
    {
        lasttimecount = GetTimeCount();    /* if the game was paused a LONG time */
    }

    do
    {
        newtime = GetTimeCount();
        tics = newtime-lasttimecount;
    } while (!tics);                       /* make sure at least one tic passes */

    lasttimecount = newtime;

    if(tics > MAXTICS)
    {
        tics = MAXTICS;
    }
}
