#ifndef TYPES_H
#define TYPES_H

#define SHAREWARE
#define MaxHighName 57

typedef unsigned char   U8;
typedef signed char     S8;
typedef unsigned short  U16;
typedef signed short    S16;
typedef unsigned int    U32;
typedef signed int      S32;
typedef S32             fixed;
typedef void * memptr;
typedef S32             ScanCode;

enum
{
    bt_nobutton=-1,
    bt_attack=0,
    bt_strafe,
    bt_run,
    bt_use,
    bt_readyknife,
    bt_readypistol,
    bt_readymachinegun,
    bt_readychaingun,
    bt_nextweapon,
    bt_prevweapon,
    bt_esc,
    bt_pause,
    bt_strafeleft,
    bt_straferight,
    bt_moveforward,
    bt_movebackward,
    bt_turnleft,
    bt_turnright,
    NUMBUTTONS
};

/* sprite constants */

enum
{
    SPR_DEMO,
    SPR_DEATHCAM,

    /* static sprites */

    SPR_STAT_0,SPR_STAT_1,SPR_STAT_2,SPR_STAT_3,
    SPR_STAT_4,SPR_STAT_5,SPR_STAT_6,SPR_STAT_7,

    SPR_STAT_8,SPR_STAT_9,SPR_STAT_10,SPR_STAT_11,
    SPR_STAT_12,SPR_STAT_13,SPR_STAT_14,SPR_STAT_15,

    SPR_STAT_16,SPR_STAT_17,SPR_STAT_18,SPR_STAT_19,
    SPR_STAT_20,SPR_STAT_21,SPR_STAT_22,SPR_STAT_23,

    SPR_STAT_24,SPR_STAT_25,SPR_STAT_26,SPR_STAT_27,
    SPR_STAT_28,SPR_STAT_29,SPR_STAT_30,SPR_STAT_31,

    SPR_STAT_32,SPR_STAT_33,SPR_STAT_34,SPR_STAT_35,
    SPR_STAT_36,SPR_STAT_37,SPR_STAT_38,SPR_STAT_39,

    SPR_STAT_40,SPR_STAT_41,SPR_STAT_42,SPR_STAT_43,
    SPR_STAT_44,SPR_STAT_45,SPR_STAT_46,SPR_STAT_47,

    /* guard */

    SPR_GRD_S_1,SPR_GRD_S_2,SPR_GRD_S_3,SPR_GRD_S_4,
    SPR_GRD_S_5,SPR_GRD_S_6,SPR_GRD_S_7,SPR_GRD_S_8,

    SPR_GRD_W1_1,SPR_GRD_W1_2,SPR_GRD_W1_3,SPR_GRD_W1_4,
    SPR_GRD_W1_5,SPR_GRD_W1_6,SPR_GRD_W1_7,SPR_GRD_W1_8,

    SPR_GRD_W2_1,SPR_GRD_W2_2,SPR_GRD_W2_3,SPR_GRD_W2_4,
    SPR_GRD_W2_5,SPR_GRD_W2_6,SPR_GRD_W2_7,SPR_GRD_W2_8,

    SPR_GRD_W3_1,SPR_GRD_W3_2,SPR_GRD_W3_3,SPR_GRD_W3_4,
    SPR_GRD_W3_5,SPR_GRD_W3_6,SPR_GRD_W3_7,SPR_GRD_W3_8,

    SPR_GRD_W4_1,SPR_GRD_W4_2,SPR_GRD_W4_3,SPR_GRD_W4_4,
    SPR_GRD_W4_5,SPR_GRD_W4_6,SPR_GRD_W4_7,SPR_GRD_W4_8,

    SPR_GRD_PAIN_1,SPR_GRD_DIE_1,SPR_GRD_DIE_2,SPR_GRD_DIE_3,
    SPR_GRD_PAIN_2,SPR_GRD_DEAD,

    SPR_GRD_SHOOT1,SPR_GRD_SHOOT2,SPR_GRD_SHOOT3,

    /* dogs */

    SPR_DOG_W1_1,SPR_DOG_W1_2,SPR_DOG_W1_3,SPR_DOG_W1_4,
    SPR_DOG_W1_5,SPR_DOG_W1_6,SPR_DOG_W1_7,SPR_DOG_W1_8,

    SPR_DOG_W2_1,SPR_DOG_W2_2,SPR_DOG_W2_3,SPR_DOG_W2_4,
    SPR_DOG_W2_5,SPR_DOG_W2_6,SPR_DOG_W2_7,SPR_DOG_W2_8,

    SPR_DOG_W3_1,SPR_DOG_W3_2,SPR_DOG_W3_3,SPR_DOG_W3_4,
    SPR_DOG_W3_5,SPR_DOG_W3_6,SPR_DOG_W3_7,SPR_DOG_W3_8,

    SPR_DOG_W4_1,SPR_DOG_W4_2,SPR_DOG_W4_3,SPR_DOG_W4_4,
    SPR_DOG_W4_5,SPR_DOG_W4_6,SPR_DOG_W4_7,SPR_DOG_W4_8,

    SPR_DOG_DIE_1,SPR_DOG_DIE_2,SPR_DOG_DIE_3,SPR_DOG_DEAD,
    SPR_DOG_JUMP1,SPR_DOG_JUMP2,SPR_DOG_JUMP3,

    /* ss */

    SPR_SS_S_1,SPR_SS_S_2,SPR_SS_S_3,SPR_SS_S_4,
    SPR_SS_S_5,SPR_SS_S_6,SPR_SS_S_7,SPR_SS_S_8,

    SPR_SS_W1_1,SPR_SS_W1_2,SPR_SS_W1_3,SPR_SS_W1_4,
    SPR_SS_W1_5,SPR_SS_W1_6,SPR_SS_W1_7,SPR_SS_W1_8,

    SPR_SS_W2_1,SPR_SS_W2_2,SPR_SS_W2_3,SPR_SS_W2_4,
    SPR_SS_W2_5,SPR_SS_W2_6,SPR_SS_W2_7,SPR_SS_W2_8,

    SPR_SS_W3_1,SPR_SS_W3_2,SPR_SS_W3_3,SPR_SS_W3_4,
    SPR_SS_W3_5,SPR_SS_W3_6,SPR_SS_W3_7,SPR_SS_W3_8,

    SPR_SS_W4_1,SPR_SS_W4_2,SPR_SS_W4_3,SPR_SS_W4_4,
    SPR_SS_W4_5,SPR_SS_W4_6,SPR_SS_W4_7,SPR_SS_W4_8,

    SPR_SS_PAIN_1,SPR_SS_DIE_1,SPR_SS_DIE_2,SPR_SS_DIE_3,
    SPR_SS_PAIN_2,SPR_SS_DEAD,

    SPR_SS_SHOOT1,SPR_SS_SHOOT2,SPR_SS_SHOOT3,

    /* mutant */

    SPR_MUT_S_1,SPR_MUT_S_2,SPR_MUT_S_3,SPR_MUT_S_4,
    SPR_MUT_S_5,SPR_MUT_S_6,SPR_MUT_S_7,SPR_MUT_S_8,

    SPR_MUT_W1_1,SPR_MUT_W1_2,SPR_MUT_W1_3,SPR_MUT_W1_4,
    SPR_MUT_W1_5,SPR_MUT_W1_6,SPR_MUT_W1_7,SPR_MUT_W1_8,

    SPR_MUT_W2_1,SPR_MUT_W2_2,SPR_MUT_W2_3,SPR_MUT_W2_4,
    SPR_MUT_W2_5,SPR_MUT_W2_6,SPR_MUT_W2_7,SPR_MUT_W2_8,

    SPR_MUT_W3_1,SPR_MUT_W3_2,SPR_MUT_W3_3,SPR_MUT_W3_4,
    SPR_MUT_W3_5,SPR_MUT_W3_6,SPR_MUT_W3_7,SPR_MUT_W3_8,

    SPR_MUT_W4_1,SPR_MUT_W4_2,SPR_MUT_W4_3,SPR_MUT_W4_4,
    SPR_MUT_W4_5,SPR_MUT_W4_6,SPR_MUT_W4_7,SPR_MUT_W4_8,

    SPR_MUT_PAIN_1,SPR_MUT_DIE_1,SPR_MUT_DIE_2,SPR_MUT_DIE_3,
    SPR_MUT_PAIN_2,SPR_MUT_DIE_4,SPR_MUT_DEAD,

    SPR_MUT_SHOOT1,SPR_MUT_SHOOT2,SPR_MUT_SHOOT3,SPR_MUT_SHOOT4,

    /* officer */

    SPR_OFC_S_1,SPR_OFC_S_2,SPR_OFC_S_3,SPR_OFC_S_4,
    SPR_OFC_S_5,SPR_OFC_S_6,SPR_OFC_S_7,SPR_OFC_S_8,

    SPR_OFC_W1_1,SPR_OFC_W1_2,SPR_OFC_W1_3,SPR_OFC_W1_4,
    SPR_OFC_W1_5,SPR_OFC_W1_6,SPR_OFC_W1_7,SPR_OFC_W1_8,

    SPR_OFC_W2_1,SPR_OFC_W2_2,SPR_OFC_W2_3,SPR_OFC_W2_4,
    SPR_OFC_W2_5,SPR_OFC_W2_6,SPR_OFC_W2_7,SPR_OFC_W2_8,

    SPR_OFC_W3_1,SPR_OFC_W3_2,SPR_OFC_W3_3,SPR_OFC_W3_4,
    SPR_OFC_W3_5,SPR_OFC_W3_6,SPR_OFC_W3_7,SPR_OFC_W3_8,

    SPR_OFC_W4_1,SPR_OFC_W4_2,SPR_OFC_W4_3,SPR_OFC_W4_4,
    SPR_OFC_W4_5,SPR_OFC_W4_6,SPR_OFC_W4_7,SPR_OFC_W4_8,

    SPR_OFC_PAIN_1,SPR_OFC_DIE_1,SPR_OFC_DIE_2,SPR_OFC_DIE_3,
    SPR_OFC_PAIN_2,SPR_OFC_DIE_4,SPR_OFC_DEAD,

    SPR_OFC_SHOOT1,SPR_OFC_SHOOT2,SPR_OFC_SHOOT3,

    /* ghosts */

    SPR_BLINKY_W1,SPR_BLINKY_W2,SPR_PINKY_W1,SPR_PINKY_W2,
    SPR_CLYDE_W1,SPR_CLYDE_W2,SPR_INKY_W1,SPR_INKY_W2,

    /* hans */

    SPR_BOSS_W1,SPR_BOSS_W2,SPR_BOSS_W3,SPR_BOSS_W4,
    SPR_BOSS_SHOOT1,SPR_BOSS_SHOOT2,SPR_BOSS_SHOOT3,SPR_BOSS_DEAD,

    SPR_BOSS_DIE1,SPR_BOSS_DIE2,SPR_BOSS_DIE3,

    /* schabbs */

    SPR_SCHABB_W1,SPR_SCHABB_W2,SPR_SCHABB_W3,SPR_SCHABB_W4,
    SPR_SCHABB_SHOOT1,SPR_SCHABB_SHOOT2,

    SPR_SCHABB_DIE1,SPR_SCHABB_DIE2,SPR_SCHABB_DIE3,SPR_SCHABB_DEAD,
    SPR_HYPO1,SPR_HYPO2,SPR_HYPO3,SPR_HYPO4,

    /* fake */

    SPR_FAKE_W1,SPR_FAKE_W2,SPR_FAKE_W3,SPR_FAKE_W4,
    SPR_FAKE_SHOOT,SPR_FIRE1,SPR_FIRE2,

    SPR_FAKE_DIE1,SPR_FAKE_DIE2,SPR_FAKE_DIE3,SPR_FAKE_DIE4,
    SPR_FAKE_DIE5,SPR_FAKE_DEAD,

    /* hitler */

    SPR_MECHA_W1,SPR_MECHA_W2,SPR_MECHA_W3,SPR_MECHA_W4,
    SPR_MECHA_SHOOT1,SPR_MECHA_SHOOT2,SPR_MECHA_SHOOT3,SPR_MECHA_DEAD,

    SPR_MECHA_DIE1,SPR_MECHA_DIE2,SPR_MECHA_DIE3,

    SPR_HITLER_W1,SPR_HITLER_W2,SPR_HITLER_W3,SPR_HITLER_W4,
    SPR_HITLER_SHOOT1,SPR_HITLER_SHOOT2,SPR_HITLER_SHOOT3,SPR_HITLER_DEAD,

    SPR_HITLER_DIE1,SPR_HITLER_DIE2,SPR_HITLER_DIE3,SPR_HITLER_DIE4,
    SPR_HITLER_DIE5,SPR_HITLER_DIE6,SPR_HITLER_DIE7,

    /* giftmacher */

    SPR_GIFT_W1,SPR_GIFT_W2,SPR_GIFT_W3,SPR_GIFT_W4,
    SPR_GIFT_SHOOT1,SPR_GIFT_SHOOT2,

    SPR_GIFT_DIE1,SPR_GIFT_DIE2,SPR_GIFT_DIE3,SPR_GIFT_DEAD,

    /* Rocket, smoke and small explosion */

    SPR_ROCKET_1,SPR_ROCKET_2,SPR_ROCKET_3,SPR_ROCKET_4,
    SPR_ROCKET_5,SPR_ROCKET_6,SPR_ROCKET_7,SPR_ROCKET_8,

    SPR_SMOKE_1,SPR_SMOKE_2,SPR_SMOKE_3,SPR_SMOKE_4,
    SPR_BOOM_1,SPR_BOOM_2,SPR_BOOM_3,

    /* gretel */
    SPR_GRETEL_W1,SPR_GRETEL_W2,SPR_GRETEL_W3,SPR_GRETEL_W4,
    SPR_GRETEL_SHOOT1,SPR_GRETEL_SHOOT2,SPR_GRETEL_SHOOT3,SPR_GRETEL_DEAD,

    SPR_GRETEL_DIE1,SPR_GRETEL_DIE2,SPR_GRETEL_DIE3,

    /* fat face */
    SPR_FAT_W1,SPR_FAT_W2,SPR_FAT_W3,SPR_FAT_W4,
    SPR_FAT_SHOOT1,SPR_FAT_SHOOT2,SPR_FAT_SHOOT3,SPR_FAT_SHOOT4,

    SPR_FAT_DIE1,SPR_FAT_DIE2,SPR_FAT_DIE3,SPR_FAT_DEAD,

    /* bj */
    SPR_BJ_W1,

    SPR_BJ_W2,SPR_BJ_W3,SPR_BJ_W4,
    SPR_BJ_JUMP1,SPR_BJ_JUMP2,SPR_BJ_JUMP3,SPR_BJ_JUMP4,


    /* player attack frames */
    SPR_KNIFEREADY,SPR_KNIFEATK1,SPR_KNIFEATK2,SPR_KNIFEATK3,
    SPR_KNIFEATK4,

    SPR_PISTOLREADY,SPR_PISTOLATK1,SPR_PISTOLATK2,SPR_PISTOLATK3,
    SPR_PISTOLATK4,

    SPR_MACHINEGUNREADY,SPR_MACHINEGUNATK1,SPR_MACHINEGUNATK2,MACHINEGUNATK3,
    SPR_MACHINEGUNATK4,

    SPR_CHAINREADY,SPR_CHAINATK1,SPR_CHAINATK2,SPR_CHAINATK3,
    SPR_CHAINATK4,

};

enum
{
    swapstates = -1,
    noaction = 0,
    gunattack,
    knifeattack,
    nextattack,
    nextchoice
};

enum menuitems
{
    newgame,
    soundmenu,
    control,
    loadgame,
    savegame,
    changeview,
    readthis,
    viewscores,
    backtodemo,
    quit
};

enum
{
    gd_baby,
    gd_easy,
    gd_medium,
    gd_hard
};

typedef enum
{
    dir_North,
    dir_NorthEast,
    dir_East,
    dir_SouthEast,
    dir_South,
    dir_SouthWest,
    dir_West,
    dir_NorthWest,
    dir_None
} Direction;

typedef enum
{
    ex_stillplaying,
    ex_completed,
    ex_died,
    ex_warped,
    ex_resetgame,
    ex_loadedgame,
    ex_victorious,
    ex_abort,
    ex_demodone,
    ex_secretlevel
} exit_t;

typedef enum
{
    wp_none = -1,
    wp_knife,
    wp_pistol,
    wp_machinegun,
    wp_chaingun,
} weapontype;

typedef enum {
    dr_normal,
    dr_lock1,
    dr_lock2,
    dr_lock3,
    dr_lock4,
    dr_elevator
} door_t;

typedef enum
{
    orig_pal = 0,
    cur_pal,
    red_pal,
    white_pal,
} pal_type;

typedef struct
{
  U8 r;
  U8 g;
  U8 b;
  U8 a;
} vga_colour;

typedef struct
{
    U16 bit0;
    U16 bit1;       /* 0-255 is a character, > is a pointer to a node */
} huffnode;

typedef struct
{
    S16 width;
    S16 height;
} pictabletype;

typedef struct
{
    S16 x;
    S16 y;
    S16 amount;
    S16 curpos;
    S16 indent;
} CP_iteminfo;

typedef struct
{
    S16 active;
    char string[36];
    S32 (* pt2function)(S32 temp1);
} CP_itemtype;

typedef struct
{
    S16 height;
    S16 location[256];
    S8  width[256];
} fontstruct;

typedef struct
{
    U8          button0;
    U8          button1;
    U8          button2;
    U8          button3;
    Direction   dir;
    U8          straferight;
    U8          strafeleft;
    U8          pause;
    U8          esc;
} CursorInfo;

typedef CursorInfo  ControlInfo;

typedef struct
{
    U32     length;
    U16     priority;
} SoundCommon;

typedef struct
{
    U8 mChar;
    U8 cChar;
    U8 mScale;
    U8 cScale;
    U8 mAttack;
    U8 cAttack;
    U8 mSus;
    U8 cSus;
    U8 mWave;
    U8 cWave;
    U8 nConn;

    /* These are only for Muse - these bytes are really unused */
    U8 voice;
    U8 mode;
    U8 unused[3];
} Instrument;

typedef struct times
{
    S32 time;
    char timestr[6];
} times_type;

typedef enum
{
    sdm_Off,
    sdm_PC,
    sdm_AdLib,
} SDMode;

typedef enum
{
    smm_Off,
    smm_AdLib
} SMMode;

typedef enum
{
    sds_Off,
    sds_PC,
    sds_SoundBlaster
} SDSMode;

typedef enum
{
    ac_badobject = -1,
    ac_no,
    ac_yes,
    ac_allways
} activetype;

typedef enum
{
    nothing,
    playerobj,
    inertobj,
    guardobj,
    officerobj,
    ssobj,
    dogobj,
    bossobj,
    schabbobj,
    fakeobj,
    mechahitlerobj,
    mutantobj,
    needleobj,
    fireobj,
    bjobj,
    ghostobj,
    realhitlerobj,
    gretelobj,
    giftobj,
    fatobj,
    rocketobj,

    spectreobj,
    angelobj,
    transobj,
    uberobj,
    willobj,
    deathobj,
    hrocketobj,
    sparkobj
} classtype;

typedef enum
{
    east,
    northeast,
    north,
    northwest,
    west,
    southwest,
    south,
    southeast,
    nodir
} dirtype;

typedef enum
{
    FL_SHOOTABLE        = 0x00000001,
    FL_BONUS            = 0x00000002,
    FL_NEVERMARK        = 0x00000004,
    FL_VISABLE          = 0x00000008,
    FL_ATTACKMODE       = 0x00000010,
    FL_FIRSTATTACK      = 0x00000020,
    FL_AMBUSH           = 0x00000040,
    FL_NONMARK          = 0x00000080,
    FL_FULLBRIGHT       = 0x00000100,
    /* next free bit is   0x00001000 */
} objflag_t;

typedef enum
{
    dr_open,
    dr_closed,
    dr_opening,
    dr_closing
} doortype;

typedef enum {
    none,
    block,
    bo_gibs,
    bo_alpo,
    bo_firstaid,
    bo_key1,
    bo_key2,
    bo_key3,
    bo_key4,
    bo_cross,
    bo_chalice,
    bo_bible,
    bo_crown,
    bo_clip,
    bo_clip2,
    bo_machinegun,
    bo_chaingun,
    bo_food,
    bo_fullheal,
    bo_25clip,
    bo_spear
} wl_stat_t;

typedef enum {
    en_guard,
    en_officer,
    en_ss,
    en_dog,
    en_boss,
    en_schabbs,
    en_fake,
    en_hitler,
    en_mutant,
    en_blinky,
    en_clyde,
    en_pinky,
    en_inky,
    en_gretel,
    en_gift,
    en_fat,
    en_spectre,
    en_angel,
    en_trans,
    en_uber,
    en_will,
    en_death
} enemy_t;

typedef enum {
    di_north,
    di_east,
    di_south,
    di_west
} controldir_t;

typedef struct
{
    SoundCommon     common;
    Instrument      inst;
    U8              block;
    U8              data[1];
} AdLibSound;

typedef struct
{
    char    name[MaxHighName + 1];
    S32     score;
    U16     completed;
    U16     episode;
} HighScore;

typedef struct
{
    S16 allowed[4];
} CustomCtrls;

typedef struct
{
    U16 RLEWtag;
    S32 headeroffsets[100];
} mapfiletype;

typedef struct
{
    S32    planestart[3];
    U16    planelength[3];
    U16    width;
    U16    height;
    char   name[16];
} maptype;

typedef struct
{
    S16       difficulty;
    S16       mapon;
    S32       oldscore,score,nextextra;
    S16       lives;
    S16       health;
    S16       ammo;
    U16       keys;
    weapontype  bestweapon,weapon,chosenweapon;

    S16       faceframe;
    S16       attackframe,attackcount,weaponframe;

    S16       episode,secretcount,treasurecount,killcount,
              secrettotal,treasuretotal,killtotal;
    S32       TimeCount;
    S32       killx,killy;
    U8        victoryflag;            /* set during victory animations */
} gametype;

typedef void (* statefunc) (void *);

typedef struct statestruct
{
    U8      rotate;
    S16     shapenum;           /* a shapenum of -1 means get from ob->temp1 */
    S16     tictime;
    void    (*think) (void *);
    void    (*action) (void *);
    struct  statestruct *next;
} statetype;

/* thinking actor structure */
typedef struct objstruct
{
    activetype  active;
    S16         ticcount;
    classtype   obclass;
    statetype   *state;

    U32         flags;              /* FL_SHOOTABLE, etc */

    S32         distance;           /* if negative, wait for that door to open */
    dirtype     dir;

    fixed       x,y;
    U16         tilex,tiley;
    U8          areanumber;

    S16         viewx;
    U16         viewheight;
    fixed       transx,transy;      /* in global coord */

    S16         angle;
    S16         hitpoints;
    S32         speed;

    S16         temp1,temp2,hidden;
    struct objstruct *next,*prev;
} objtype;

typedef struct
{
    U16 leftpix;
    U16 rightpix;
    U16 dataofs[64];               /* table data after dataofs[rightpix-leftpix+1] */
} t_compshape;

struct atkinf
{
    S8    tics;                     /* the amount of time to wait in attack function */
    S8    attack;                   /* attack is 1 for gun, 2 for knife */
    S8    frame;                    /* attack frames 1 = start 4 = finish */
};

typedef struct
{
    U32 startpage;
    U32 length;
} digiinfo;

/* door actor structure */
typedef struct doorstruct
{
    U8     tilex;
    U8     tiley;
    U8     vertical;
    U8     lock;
    doortype action;
    S16    ticcount;
} doorobj_t;

/* trivial actor structure */
typedef struct statstruct
{
    U8        tilex;
    U8        tiley;
    S16       shapenum;           /* if shapenum == -1 the obj has been removed */
    U8        *visspot;
    U32       flags;
    U8        itemnumber;
} statobj_t;

typedef struct statinfostruct
{
    S16        picnum;
    wl_stat_t  type;
    U32        specialFlags;     /* they are ORed to the statobj_t flags */
} statinfo_type;

typedef struct
{
    S16    viewx;
    S16    viewheight;
    S16    shapenum;
    S16    flags;    /* this must be changed to uint32_t, when you */
                     /* you need more than 16-flags for drawing */
} visobj_t;

/* WL_INTER */
typedef struct
{
    S32 kill;
    S32 secret;
    S32 treasure;
    S32 time;
} LRstruct;

#endif
