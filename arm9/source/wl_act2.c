#include <nds.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "types.h"
#include "audiowl6.h"
#include "id_us.h"
#include "id_sd.h"
#include "wl_def.h"
#include "wl_state.h"
#include "wl_act2.h"
#include "wl_act1.h"
#include "wl_agent.h"
#include "wl_play.h"

/* macro defines */

#define NUMENEMIES  22
#define SPDDOG      1500
#define RUNSPEED    6000
#define ICONARROWS  90
#define MAPSPOT(x,y,plane) (mapsegs[plane][((y)<<mapshift)+(x)])
#define PROJECTILESIZE  0xc000l
#define LABS(x)    ((S32)(x)>0?(x):-(x))
#define PROJSIZE    0x2000
#define BJRUNSPEED  2048
#define BJJUMPSPEED 680

/* external variables */

extern U8 loadedgame;
extern gametype gamestate;
extern U8 tilemap[MAPSIZE][MAPSIZE];
extern U16 * mapsegs[MAPPLANES];
extern objtype *newobj;
extern objtype *actorat[MAPSIZE][MAPSIZE];
extern objtype *player;
extern U32 tics;
extern doorobj_t doorobjlist[MAXDOORS];
extern U8 areabyplayer[NUMAREAS];
extern S32 thrustspeed;
extern objtype *objfreelist;
extern fixed sintable[ANGLES+ANGLES/4];
extern fixed *costable;
extern S32 mapon;
extern exit_t playstate;

/* global variables */

/* guards */
statetype s_grdstand = {1,SPR_GRD_S_1,0,(statefunc)T_Stand,NULL,&s_grdstand};

statetype s_grdpath1  = {1,SPR_GRD_W1_1,20,(statefunc)T_Path,NULL,&s_grdpath1s};
statetype s_grdpath1s = {1,SPR_GRD_W1_1,5,NULL,NULL,&s_grdpath2};
statetype s_grdpath2  = {1,SPR_GRD_W2_1,15,(statefunc)T_Path,NULL,&s_grdpath3};
statetype s_grdpath3  = {1,SPR_GRD_W3_1,20,(statefunc)T_Path,NULL,&s_grdpath3s};
statetype s_grdpath3s = {1,SPR_GRD_W3_1,5,NULL,NULL,&s_grdpath4};
statetype s_grdpath4  = {1,SPR_GRD_W4_1,15,(statefunc)T_Path,NULL,&s_grdpath1};

statetype s_grdpain   = {2,SPR_GRD_PAIN_1,10,NULL,NULL,&s_grdchase1};
statetype s_grdpain1  = {2,SPR_GRD_PAIN_2,10,NULL,NULL,&s_grdchase1};

statetype s_grdshoot1 = {0,SPR_GRD_SHOOT1,20,NULL,NULL,&s_grdshoot2};
statetype s_grdshoot2 = {0,SPR_GRD_SHOOT2,20,NULL,(statefunc)T_Shoot,&s_grdshoot3};
statetype s_grdshoot3 = {0,SPR_GRD_SHOOT3,20,NULL,NULL,&s_grdchase1};

statetype s_grdchase1  = {1,SPR_GRD_W1_1,10,(statefunc)T_Chase,NULL,&s_grdchase1s};
statetype s_grdchase1s = {1,SPR_GRD_W1_1,3,NULL,NULL,&s_grdchase2};
statetype s_grdchase2  = {1,SPR_GRD_W2_1,8,(statefunc)T_Chase,NULL,&s_grdchase3};
statetype s_grdchase3  = {1,SPR_GRD_W3_1,10,(statefunc)T_Chase,NULL,&s_grdchase3s};
statetype s_grdchase3s = {1,SPR_GRD_W3_1,3,NULL,NULL,&s_grdchase4};
statetype s_grdchase4  = {1,SPR_GRD_W4_1,8,(statefunc)T_Chase,NULL,&s_grdchase1};

statetype s_grddie1 = {0,SPR_GRD_DIE_1,15,NULL,(statefunc)A_DeathScream,&s_grddie2};
statetype s_grddie2 = {0,SPR_GRD_DIE_2,15,NULL,NULL,&s_grddie3};
statetype s_grddie3 = {0,SPR_GRD_DIE_3,15,NULL,NULL,&s_grddie4};
statetype s_grddie4 = {0,SPR_GRD_DEAD,0,NULL,NULL,&s_grddie4};

/* officers */
statetype s_ofcstand = {1,SPR_OFC_S_1,0,(statefunc)T_Stand,NULL,&s_ofcstand};

statetype s_ofcpath1  = {1,SPR_OFC_W1_1,20,(statefunc)T_Path,NULL,&s_ofcpath1s};
statetype s_ofcpath1s = {1,SPR_OFC_W1_1,5,NULL,NULL,&s_ofcpath2};
statetype s_ofcpath2  = {1,SPR_OFC_W2_1,15,(statefunc)T_Path,NULL,&s_ofcpath3};
statetype s_ofcpath3  = {1,SPR_OFC_W3_1,20,(statefunc)T_Path,NULL,&s_ofcpath3s};
statetype s_ofcpath3s = {1,SPR_OFC_W3_1,5,NULL,NULL,&s_ofcpath4};
statetype s_ofcpath4  = {1,SPR_OFC_W4_1,15,(statefunc)T_Path,NULL,&s_ofcpath1};

statetype s_ofcpain   = {2,SPR_OFC_PAIN_1,10,NULL,NULL,&s_ofcchase1};
statetype s_ofcpain1  = {2,SPR_OFC_PAIN_2,10,NULL,NULL,&s_ofcchase1};

statetype s_ofcshoot1 = {0,SPR_OFC_SHOOT1,6,NULL,NULL,&s_ofcshoot2};
statetype s_ofcshoot2 = {0,SPR_OFC_SHOOT2,20,NULL,(statefunc)T_Shoot,&s_ofcshoot3};
statetype s_ofcshoot3 = {0,SPR_OFC_SHOOT3,10,NULL,NULL,&s_ofcchase1};

statetype s_ofcchase1  = {1,SPR_OFC_W1_1,10,(statefunc)T_Chase,NULL,&s_ofcchase1s};
statetype s_ofcchase1s = {1,SPR_OFC_W1_1,3,NULL,NULL,&s_ofcchase2};
statetype s_ofcchase2  = {1,SPR_OFC_W2_1,8,(statefunc)T_Chase,NULL,&s_ofcchase3};
statetype s_ofcchase3  = {1,SPR_OFC_W3_1,10,(statefunc)T_Chase,NULL,&s_ofcchase3s};
statetype s_ofcchase3s = {1,SPR_OFC_W3_1,3,NULL,NULL,&s_ofcchase4};
statetype s_ofcchase4  = {1,SPR_OFC_W4_1,8,(statefunc)T_Chase,NULL,&s_ofcchase1};

statetype s_ofcdie1 = {0,SPR_OFC_DIE_1,11,NULL,(statefunc)A_DeathScream,&s_ofcdie2};
statetype s_ofcdie2 = {0,SPR_OFC_DIE_2,11,NULL,NULL,&s_ofcdie3};
statetype s_ofcdie3 = {0,SPR_OFC_DIE_3,11,NULL,NULL,&s_ofcdie4};
statetype s_ofcdie4 = {0,SPR_OFC_DIE_4,11,NULL,NULL,&s_ofcdie5};
statetype s_ofcdie5 = {0,SPR_OFC_DEAD,0,NULL,NULL,&s_ofcdie5};

/* mutant */
statetype s_mutstand = {1,SPR_MUT_S_1,0,(statefunc)T_Stand,NULL,&s_mutstand};

statetype s_mutpath1  = {1,SPR_MUT_W1_1,20,(statefunc)T_Path,NULL,&s_mutpath1s};
statetype s_mutpath1s = {1,SPR_MUT_W1_1,5,NULL,NULL,&s_mutpath2};
statetype s_mutpath2  = {1,SPR_MUT_W2_1,15,(statefunc)T_Path,NULL,&s_mutpath3};
statetype s_mutpath3  = {1,SPR_MUT_W3_1,20,(statefunc)T_Path,NULL,&s_mutpath3s};
statetype s_mutpath3s = {1,SPR_MUT_W3_1,5,NULL,NULL,&s_mutpath4};
statetype s_mutpath4  = {1,SPR_MUT_W4_1,15,(statefunc)T_Path,NULL,&s_mutpath1};

statetype s_mutpain   = {2,SPR_MUT_PAIN_1,10,NULL,NULL,&s_mutchase1};
statetype s_mutpain1  = {2,SPR_MUT_PAIN_2,10,NULL,NULL,&s_mutchase1};

statetype s_mutshoot1 = {0,SPR_MUT_SHOOT1,6,NULL,(statefunc)T_Shoot,&s_mutshoot2};
statetype s_mutshoot2 = {0,SPR_MUT_SHOOT2,20,NULL,NULL,&s_mutshoot3};
statetype s_mutshoot3 = {0,SPR_MUT_SHOOT3,10,NULL,(statefunc)T_Shoot,&s_mutshoot4};
statetype s_mutshoot4 = {0,SPR_MUT_SHOOT4,20,NULL,NULL,&s_mutchase1};

statetype s_mutchase1  = {1,SPR_MUT_W1_1,10,(statefunc)T_Chase,NULL,&s_mutchase1s};
statetype s_mutchase1s = {1,SPR_MUT_W1_1,3,NULL,NULL,&s_mutchase2};
statetype s_mutchase2  = {1,SPR_MUT_W2_1,8,(statefunc)T_Chase,NULL,&s_mutchase3};
statetype s_mutchase3  = {1,SPR_MUT_W3_1,10,(statefunc)T_Chase,NULL,&s_mutchase3s};
statetype s_mutchase3s = {1,SPR_MUT_W3_1,3,NULL,NULL,&s_mutchase4};
statetype s_mutchase4  = {1,SPR_MUT_W4_1,8,(statefunc)T_Chase,NULL,&s_mutchase1};

statetype s_mutdie1    = {0,SPR_MUT_DIE_1,7,NULL,(statefunc)A_DeathScream,&s_mutdie2};
statetype s_mutdie2    = {0,SPR_MUT_DIE_2,7,NULL,NULL,&s_mutdie3};
statetype s_mutdie3    = {0,SPR_MUT_DIE_3,7,NULL,NULL,&s_mutdie4};
statetype s_mutdie4    = {0,SPR_MUT_DIE_4,7,NULL,NULL,&s_mutdie5};
statetype s_mutdie5    = {0,SPR_MUT_DEAD,0,NULL,NULL,&s_mutdie5};

/* SS */
statetype s_ssstand = {1,SPR_SS_S_1,0,(statefunc)T_Stand,NULL,&s_ssstand};

statetype s_sspath1  = {1,SPR_SS_W1_1,20,(statefunc)T_Path,NULL,&s_sspath1s};
statetype s_sspath1s = {1,SPR_SS_W1_1,5,NULL,NULL,&s_sspath2};
statetype s_sspath2  = {1,SPR_SS_W2_1,15,(statefunc)T_Path,NULL,&s_sspath3};
statetype s_sspath3  = {1,SPR_SS_W3_1,20,(statefunc)T_Path,NULL,&s_sspath3s};
statetype s_sspath3s = {1,SPR_SS_W3_1,5,NULL,NULL,&s_sspath4};
statetype s_sspath4  = {1,SPR_SS_W4_1,15,(statefunc)T_Path,NULL,&s_sspath1};

statetype s_sspain   = {2,SPR_SS_PAIN_1,10,NULL,NULL,&s_sschase1};
statetype s_sspain1  = {2,SPR_SS_PAIN_2,10,NULL,NULL,&s_sschase1};

statetype s_ssshoot1 = {0,SPR_SS_SHOOT1,20,NULL,NULL,&s_ssshoot2};
statetype s_ssshoot2 = {0,SPR_SS_SHOOT2,20,NULL,(statefunc)T_Shoot,&s_ssshoot3};
statetype s_ssshoot3 = {0,SPR_SS_SHOOT3,10,NULL,NULL,&s_ssshoot4};
statetype s_ssshoot4 = {0,SPR_SS_SHOOT2,10,NULL,(statefunc)T_Shoot,&s_ssshoot5};
statetype s_ssshoot5 = {0,SPR_SS_SHOOT3,10,NULL,NULL,&s_ssshoot6};
statetype s_ssshoot6 = {0,SPR_SS_SHOOT2,10,NULL,(statefunc)T_Shoot,&s_ssshoot7};
statetype s_ssshoot7 = {0,SPR_SS_SHOOT3,10,NULL,NULL,&s_ssshoot8};
statetype s_ssshoot8 = {0,SPR_SS_SHOOT2,10,NULL,(statefunc)T_Shoot,&s_ssshoot9};
statetype s_ssshoot9 = {0,SPR_SS_SHOOT3,10,NULL,NULL,&s_sschase1};

statetype s_sschase1  = {1,SPR_SS_W1_1,10,(statefunc)T_Chase,NULL,&s_sschase1s};
statetype s_sschase1s = {1,SPR_SS_W1_1,3,NULL,NULL,&s_sschase2};
statetype s_sschase2  = {1,SPR_SS_W2_1,8,(statefunc)T_Chase,NULL,&s_sschase3};
statetype s_sschase3  = {1,SPR_SS_W3_1,10,(statefunc)T_Chase,NULL,&s_sschase3s};
statetype s_sschase3s = {1,SPR_SS_W3_1,3,NULL,NULL,&s_sschase4};
statetype s_sschase4  = {1,SPR_SS_W4_1,8,(statefunc)T_Chase,NULL,&s_sschase1};

statetype s_ssdie1    = {0,SPR_SS_DIE_1,15,NULL,(statefunc)A_DeathScream,&s_ssdie2};
statetype s_ssdie2    = {0,SPR_SS_DIE_2,15,NULL,NULL,&s_ssdie3};
statetype s_ssdie3    = {0,SPR_SS_DIE_3,15,NULL,NULL,&s_ssdie4};
statetype s_ssdie4    = {0,SPR_SS_DEAD,0,NULL,NULL,&s_ssdie4};

/* dogs */
statetype s_dogpath1  = {1,SPR_DOG_W1_1,20,(statefunc)T_Path,NULL,&s_dogpath1s};
statetype s_dogpath1s = {1,SPR_DOG_W1_1,5,NULL,NULL,&s_dogpath2};
statetype s_dogpath2  = {1,SPR_DOG_W2_1,15,(statefunc)T_Path,NULL,&s_dogpath3};
statetype s_dogpath3  = {1,SPR_DOG_W3_1,20,(statefunc)T_Path,NULL,&s_dogpath3s};
statetype s_dogpath3s = {1,SPR_DOG_W3_1,5,NULL,NULL,&s_dogpath4};
statetype s_dogpath4  = {1,SPR_DOG_W4_1,15,(statefunc)T_Path,NULL,&s_dogpath1};

statetype s_dogjump1  = {0,SPR_DOG_JUMP1,10,NULL,NULL,&s_dogjump2};
statetype s_dogjump2  = {0,SPR_DOG_JUMP2,10,NULL,(statefunc)T_Bite,&s_dogjump3};
statetype s_dogjump3  = {0,SPR_DOG_JUMP3,10,NULL,NULL,&s_dogjump4};
statetype s_dogjump4  = {0,SPR_DOG_JUMP1,10,NULL,NULL,&s_dogjump5};
statetype s_dogjump5  = {0,SPR_DOG_W1_1,10,NULL,NULL,&s_dogchase1};

statetype s_dogchase1  = {1,SPR_DOG_W1_1,10,(statefunc)T_DogChase,NULL,&s_dogchase1s};
statetype s_dogchase1s = {1,SPR_DOG_W1_1,3,NULL,NULL,&s_dogchase2};
statetype s_dogchase2  = {1,SPR_DOG_W2_1,8,(statefunc)T_DogChase,NULL,&s_dogchase3};
statetype s_dogchase3  = {1,SPR_DOG_W3_1,10,(statefunc)T_DogChase,NULL,&s_dogchase3s};
statetype s_dogchase3s = {1,SPR_DOG_W3_1,3,NULL,NULL,&s_dogchase4};
statetype s_dogchase4  = {1,SPR_DOG_W4_1,8,(statefunc)T_DogChase,NULL,&s_dogchase1};

statetype s_dogdie1   = {0,SPR_DOG_DIE_1,15,NULL,(statefunc)A_DeathScream,&s_dogdie2};
statetype s_dogdie2   = {0,SPR_DOG_DIE_2,15,NULL,NULL,&s_dogdie3};
statetype s_dogdie3   = {0,SPR_DOG_DIE_3,15,NULL,NULL,&s_dogdead};
statetype s_dogdead   = {0,SPR_DOG_DEAD,15,NULL,NULL,&s_dogdead};

/* hans */
statetype s_bossstand = {0,SPR_BOSS_W1,0,(statefunc)T_Stand,NULL,&s_bossstand};

statetype s_bosschase1  = {0,SPR_BOSS_W1,10,(statefunc)T_Chase,NULL,&s_bosschase1s};
statetype s_bosschase1s = {0,SPR_BOSS_W1,3,NULL,NULL,&s_bosschase2};
statetype s_bosschase2  = {0,SPR_BOSS_W2,8,(statefunc)T_Chase,NULL,&s_bosschase3};
statetype s_bosschase3  = {0,SPR_BOSS_W3,10,(statefunc)T_Chase,NULL,&s_bosschase3s};
statetype s_bosschase3s = {0,SPR_BOSS_W3,3,NULL,NULL,&s_bosschase4};
statetype s_bosschase4  = {0,SPR_BOSS_W4,8,(statefunc)T_Chase,NULL,&s_bosschase1};

statetype s_bossdie1    = {0,SPR_BOSS_DIE1,15,NULL,(statefunc)A_DeathScream,&s_bossdie2};
statetype s_bossdie2    = {0,SPR_BOSS_DIE2,15,NULL,NULL,&s_bossdie3};
statetype s_bossdie3    = {0,SPR_BOSS_DIE3,15,NULL,NULL,&s_bossdie4};
statetype s_bossdie4    = {0,SPR_BOSS_DEAD,0,NULL,NULL,&s_bossdie4};

statetype s_bossshoot1  = {0,SPR_BOSS_SHOOT1,30,NULL,NULL,&s_bossshoot2};
statetype s_bossshoot2  = {0,SPR_BOSS_SHOOT2,10,NULL,(statefunc)T_Shoot,&s_bossshoot3};
statetype s_bossshoot3  = {0,SPR_BOSS_SHOOT3,10,NULL,(statefunc)T_Shoot,&s_bossshoot4};
statetype s_bossshoot4  = {0,SPR_BOSS_SHOOT2,10,NULL,(statefunc)T_Shoot,&s_bossshoot5};
statetype s_bossshoot5  = {0,SPR_BOSS_SHOOT3,10,NULL,(statefunc)T_Shoot,&s_bossshoot6};
statetype s_bossshoot6  = {0,SPR_BOSS_SHOOT2,10,NULL,(statefunc)T_Shoot,&s_bossshoot7};
statetype s_bossshoot7  = {0,SPR_BOSS_SHOOT3,10,NULL,(statefunc)T_Shoot,&s_bossshoot8};
statetype s_bossshoot8  = {0,SPR_BOSS_SHOOT1,10,NULL,NULL,&s_bosschase1};

/* gretel */
statetype s_gretelstand  = {1,SPR_GRETEL_W1,0,(statefunc)T_Stand,NULL,&s_gretelstand};

statetype s_gretelchase1  = {0,SPR_GRETEL_W1,10,(statefunc)T_Chase,NULL,&s_gretelchase1s};
statetype s_gretelchase1s = {0,SPR_GRETEL_W1,3,NULL,NULL,&s_gretelchase2};
statetype s_gretelchase2  = {0,SPR_GRETEL_W2,8,(statefunc)T_Chase,NULL,&s_gretelchase3};
statetype s_gretelchase3  = {0,SPR_GRETEL_W3,10,(statefunc)T_Chase,NULL,&s_gretelchase3s};
statetype s_gretelchase3s = {0,SPR_GRETEL_W3,3,NULL,NULL,&s_gretelchase4};
statetype s_gretelchase4  = {0,SPR_GRETEL_W4,8,(statefunc)T_Chase,NULL,&s_gretelchase1};

statetype s_greteldie1  = {0,SPR_GRETEL_DIE1,15,NULL,(statefunc)A_DeathScream,&s_greteldie2};
statetype s_greteldie2  = {0,SPR_GRETEL_DIE2,15,NULL,NULL,&s_greteldie3};
statetype s_greteldie3  = {0,SPR_GRETEL_DIE3,15,NULL,NULL,&s_greteldie4};
statetype s_greteldie4  = {0,SPR_GRETEL_DEAD,0,NULL,NULL,&s_greteldie4};

statetype s_gretelshoot1 = {0,SPR_GRETEL_SHOOT1,30,NULL,NULL,&s_gretelshoot2};
statetype s_gretelshoot2 = {0,SPR_GRETEL_SHOOT2,10,NULL,(statefunc)T_Shoot,&s_gretelshoot3};
statetype s_gretelshoot3 = {0,SPR_GRETEL_SHOOT3,10,NULL,(statefunc)T_Shoot,&s_gretelshoot4};
statetype s_gretelshoot4 = {0,SPR_GRETEL_SHOOT2,10,NULL,(statefunc)T_Shoot,&s_gretelshoot5};
statetype s_gretelshoot5 = {0,SPR_GRETEL_SHOOT3,10,NULL,(statefunc)T_Shoot,&s_gretelshoot6};
statetype s_gretelshoot6 = {0,SPR_GRETEL_SHOOT2,10,NULL,(statefunc)T_Shoot,&s_gretelshoot7};
statetype s_gretelshoot7 = {0,SPR_GRETEL_SHOOT3,10,NULL,(statefunc)T_Shoot,&s_gretelshoot8};
statetype s_gretelshoot8 = {0,SPR_GRETEL_SHOOT1,10,NULL,NULL,&s_gretelchase1};

/* gift */
statetype s_giftstand = {0,SPR_GIFT_W1,0,(statefunc)T_Stand,NULL,&s_giftstand};

statetype s_giftchase1  = {0,SPR_GIFT_W1,10,(statefunc)T_Gift,NULL,&s_giftchase1s};
statetype s_giftchase1s = {0,SPR_GIFT_W1,3,NULL,NULL,&s_giftchase2};
statetype s_giftchase2  = {0,SPR_GIFT_W2,8,(statefunc)T_Gift,NULL,&s_giftchase3};
statetype s_giftchase3  = {0,SPR_GIFT_W3,10,(statefunc)T_Gift,NULL,&s_giftchase3s};
statetype s_giftchase3s = {0,SPR_GIFT_W3,3,NULL,NULL,&s_giftchase4};
statetype s_giftchase4  = {0,SPR_GIFT_W4,8,(statefunc)T_Gift,NULL,&s_giftchase1};

statetype s_giftdie1  = {0,SPR_GIFT_W1,1,NULL,(statefunc)A_DeathScream,&s_giftdie2};
statetype s_giftdie2  = {0,SPR_GIFT_W1,10,NULL,NULL,&s_giftdie3};
statetype s_giftdie3  = {0,SPR_GIFT_DIE1,10,NULL,NULL,&s_giftdie4};
statetype s_giftdie4  = {0,SPR_GIFT_DIE2,10,NULL,NULL,&s_giftdie5};
statetype s_giftdie5  = {0,SPR_GIFT_DIE3,10,NULL,NULL,&s_giftdie6};
statetype s_giftdie6  = {0,SPR_GIFT_DEAD,20,NULL,(statefunc)A_StartDeathCam,&s_giftdie6};

statetype s_giftshoot1 = {0,SPR_GIFT_SHOOT1,30,NULL,NULL,&s_giftshoot2};
statetype s_giftshoot2 = {0,SPR_GIFT_SHOOT2,10,NULL,(statefunc)T_GiftThrow,&s_giftchase1};

/* fat */
statetype s_fatstand = {0,SPR_FAT_W1,0,(statefunc)T_Stand,NULL,&s_fatstand};

statetype s_fatchase1  = {0,SPR_FAT_W1,10,(statefunc)T_Fat,NULL,&s_fatchase1s};
statetype s_fatchase1s = {0,SPR_FAT_W1,3,NULL,NULL,&s_fatchase2};
statetype s_fatchase2  = {0,SPR_FAT_W2,8,(statefunc)T_Fat,NULL,&s_fatchase3};
statetype s_fatchase3  = {0,SPR_FAT_W3,10,(statefunc)T_Fat,NULL,&s_fatchase3s};
statetype s_fatchase3s = {0,SPR_FAT_W3,3,NULL,NULL,&s_fatchase4};
statetype s_fatchase4  = {0,SPR_FAT_W4,8,(statefunc)T_Fat,NULL,&s_fatchase1};

statetype s_fatdie1  = {0,SPR_FAT_W1,1,NULL,(statefunc)A_DeathScream,&s_fatdie2};
statetype s_fatdie2  = {0,SPR_FAT_W1,10,NULL,NULL,&s_fatdie3};
statetype s_fatdie3  = {0,SPR_FAT_DIE1,10,NULL,NULL,&s_fatdie4};
statetype s_fatdie4  = {0,SPR_FAT_DIE2,10,NULL,NULL,&s_fatdie5};
statetype s_fatdie5  = {0,SPR_FAT_DIE3,10,NULL,NULL,&s_fatdie6};
statetype s_fatdie6  = {0,SPR_FAT_DEAD,20,NULL,(statefunc)A_StartDeathCam,&s_fatdie6};

statetype s_fatshoot1 = {0,SPR_FAT_SHOOT1,30,NULL,NULL,&s_fatshoot2};
statetype s_fatshoot2 = {0,SPR_FAT_SHOOT2,10,NULL,(statefunc)T_GiftThrow,&s_fatshoot3};
statetype s_fatshoot3 = {0,SPR_FAT_SHOOT3,10,NULL,(statefunc)T_Shoot,&s_fatshoot4};
statetype s_fatshoot4 = {0,SPR_FAT_SHOOT4,10,NULL,(statefunc)T_Shoot,&s_fatshoot5};
statetype s_fatshoot5 = {0,SPR_FAT_SHOOT3,10,NULL,(statefunc)T_Shoot,&s_fatshoot6};
statetype s_fatshoot6 = {0,SPR_FAT_SHOOT4,10,NULL,(statefunc)T_Shoot,&s_fatchase1};

/* schabb */
statetype s_schabbstand = {0,SPR_SCHABB_W1,0,(statefunc)T_Stand,NULL,&s_schabbstand};

statetype s_schabbchase1  = {0,SPR_SCHABB_W1,10,(statefunc)T_Schabb,NULL,&s_schabbchase1s};
statetype s_schabbchase1s = {0,SPR_SCHABB_W1,3,NULL,NULL,&s_schabbchase2};
statetype s_schabbchase2  = {0,SPR_SCHABB_W2,8,(statefunc)T_Schabb,NULL,&s_schabbchase3};
statetype s_schabbchase3  = {0,SPR_SCHABB_W3,10,(statefunc)T_Schabb,NULL,&s_schabbchase3s};
statetype s_schabbchase3s = {0,SPR_SCHABB_W3,3,NULL,NULL,&s_schabbchase4};
statetype s_schabbchase4  = {0,SPR_SCHABB_W4,8,(statefunc)T_Schabb,NULL,&s_schabbchase1};

statetype s_schabbdie1  = {0,SPR_SCHABB_W1,10,NULL,(statefunc)A_DeathScream,&s_schabbdie2};
statetype s_schabbdie2  = {0,SPR_SCHABB_W1,10,NULL,NULL,&s_schabbdie3};
statetype s_schabbdie3  = {0,SPR_SCHABB_DIE1,10,NULL,NULL,&s_schabbdie4};
statetype s_schabbdie4  = {0,SPR_SCHABB_DIE2,10,NULL,NULL,&s_schabbdie5};
statetype s_schabbdie5  = {0,SPR_SCHABB_DIE3,10,NULL,NULL,&s_schabbdie6};
statetype s_schabbdie6  = {0,SPR_SCHABB_DEAD,20,NULL,(statefunc)A_StartDeathCam,&s_schabbdie6};

statetype s_schabbshoot1 = {0,SPR_SCHABB_SHOOT1,30,NULL,NULL,&s_schabbshoot2};
statetype s_schabbshoot2 = {0,SPR_SCHABB_SHOOT2,10,NULL,(statefunc)T_SchabbThrow,&s_schabbchase1};

statetype s_needle1 = {0,SPR_HYPO1,6,(statefunc)T_Projectile,NULL,&s_needle2};
statetype s_needle2 = {0,SPR_HYPO2,6,(statefunc)T_Projectile,NULL,&s_needle3};
statetype s_needle3 = {0,SPR_HYPO3,6,(statefunc)T_Projectile,NULL,&s_needle4};
statetype s_needle4 = {0,SPR_HYPO4,6,(statefunc)T_Projectile,NULL,&s_needle1};

/* fake hitler */
statetype s_fakestand   = {0,SPR_FAKE_W1,0,(statefunc)T_Stand,NULL,&s_fakestand};

statetype s_fakechase1  = {0,SPR_FAKE_W1,10,(statefunc)T_Fake,NULL,&s_fakechase1s};
statetype s_fakechase1s = {0,SPR_FAKE_W1,3,NULL,NULL,&s_fakechase2};
statetype s_fakechase2  = {0,SPR_FAKE_W2,8,(statefunc)T_Fake,NULL,&s_fakechase3};
statetype s_fakechase3  = {0,SPR_FAKE_W3,10,(statefunc)T_Fake,NULL,&s_fakechase3s};
statetype s_fakechase3s = {0,SPR_FAKE_W3,3,NULL,NULL,&s_fakechase4};
statetype s_fakechase4  = {0,SPR_FAKE_W4,8,(statefunc)T_Fake,NULL,&s_fakechase1};

statetype s_fakedie1    = {0,SPR_FAKE_DIE1,10,NULL,(statefunc)A_DeathScream,&s_fakedie2};
statetype s_fakedie2    = {0,SPR_FAKE_DIE2,10,NULL,NULL,&s_fakedie3};
statetype s_fakedie3    = {0,SPR_FAKE_DIE3,10,NULL,NULL,&s_fakedie4};
statetype s_fakedie4    = {0,SPR_FAKE_DIE4,10,NULL,NULL,&s_fakedie5};
statetype s_fakedie5    = {0,SPR_FAKE_DIE5,10,NULL,NULL,&s_fakedie6};
statetype s_fakedie6    = {0,SPR_FAKE_DEAD,0,NULL,NULL,&s_fakedie6};

statetype s_fakeshoot1  = {0,SPR_FAKE_SHOOT,8,NULL,(statefunc)T_FakeFire,&s_fakeshoot2};
statetype s_fakeshoot2  = {0,SPR_FAKE_SHOOT,8,NULL,(statefunc)T_FakeFire,&s_fakeshoot3};
statetype s_fakeshoot3  = {0,SPR_FAKE_SHOOT,8,NULL,(statefunc)T_FakeFire,&s_fakeshoot4};
statetype s_fakeshoot4  = {0,SPR_FAKE_SHOOT,8,NULL,(statefunc)T_FakeFire,&s_fakeshoot5};
statetype s_fakeshoot5  = {0,SPR_FAKE_SHOOT,8,NULL,(statefunc)T_FakeFire,&s_fakeshoot6};
statetype s_fakeshoot6  = {0,SPR_FAKE_SHOOT,8,NULL,(statefunc)T_FakeFire,&s_fakeshoot7};
statetype s_fakeshoot7  = {0,SPR_FAKE_SHOOT,8,NULL,(statefunc)T_FakeFire,&s_fakeshoot8};
statetype s_fakeshoot8  = {0,SPR_FAKE_SHOOT,8,NULL,(statefunc)T_FakeFire,&s_fakeshoot9};
statetype s_fakeshoot9  = {0,SPR_FAKE_SHOOT,8,NULL,NULL,&s_fakechase1};

statetype s_fire1       = {0,SPR_FIRE1,6,NULL,(statefunc)T_Projectile,&s_fire2};
statetype s_fire2       = {0,SPR_FIRE2,6,NULL,(statefunc)T_Projectile,&s_fire1};

/* mech hitler */
statetype s_mechastand  = {0,SPR_MECHA_W1,0,(statefunc)T_Stand,NULL,&s_mechastand};

statetype s_mechachase1  = {0,SPR_MECHA_W1,10,(statefunc)T_Chase,(statefunc)A_MechaSound,&s_mechachase1s};
statetype s_mechachase1s = {0,SPR_MECHA_W1,6,NULL,NULL,&s_mechachase2};
statetype s_mechachase2  = {0,SPR_MECHA_W2,8,(statefunc)T_Chase,NULL,&s_mechachase3};
statetype s_mechachase3  = {0,SPR_MECHA_W3,10,(statefunc)T_Chase,(statefunc)A_MechaSound,&s_mechachase3s};
statetype s_mechachase3s = {0,SPR_MECHA_W3,6,NULL,NULL,&s_mechachase4};
statetype s_mechachase4  = {0,SPR_MECHA_W4,8,(statefunc)T_Chase,NULL,&s_mechachase1};

statetype s_mechadie1    = {0,SPR_MECHA_DIE1,10,NULL,(statefunc)A_DeathScream,&s_mechadie2};
statetype s_mechadie2    = {0,SPR_MECHA_DIE2,10,NULL,NULL,&s_mechadie3};
statetype s_mechadie3    = {0,SPR_MECHA_DIE3,10,NULL,(statefunc)A_HitlerMorph,&s_mechadie4};
statetype s_mechadie4    = {0,SPR_MECHA_DEAD,0,NULL,NULL,&s_mechadie4};

statetype s_mechashoot1 = {0,SPR_MECHA_SHOOT1,30,NULL,NULL,&s_mechashoot2};
statetype s_mechashoot2 = {0,SPR_MECHA_SHOOT2,10,NULL,(statefunc)T_Shoot,&s_mechashoot3};
statetype s_mechashoot3 = {0,SPR_MECHA_SHOOT3,10,NULL,(statefunc)T_Shoot,&s_mechashoot4};
statetype s_mechashoot4 = {0,SPR_MECHA_SHOOT2,10,NULL,(statefunc)T_Shoot,&s_mechashoot5};
statetype s_mechashoot5 = {0,SPR_MECHA_SHOOT3,10,NULL,(statefunc)T_Shoot,&s_mechashoot6};
statetype s_mechashoot6 = {0,SPR_MECHA_SHOOT2,10,NULL,(statefunc)T_Shoot,&s_mechachase1};

/* real hitler */
statetype s_hitlerchase1  = {0,SPR_HITLER_W1,6,(statefunc)T_Chase,NULL,&s_hitlerchase1s};
statetype s_hitlerchase1s = {0,SPR_HITLER_W1,4,NULL,NULL,&s_hitlerchase2};
statetype s_hitlerchase2  = {0,SPR_HITLER_W2,2,(statefunc)T_Chase,NULL,&s_hitlerchase3};
statetype s_hitlerchase3  = {0,SPR_HITLER_W3,6,(statefunc)T_Chase,NULL,&s_hitlerchase3s};
statetype s_hitlerchase3s = {0,SPR_HITLER_W3,4,NULL,NULL,&s_hitlerchase4};
statetype s_hitlerchase4  = {0,SPR_HITLER_W4,2,(statefunc)T_Chase,NULL,&s_hitlerchase1};

statetype s_hitlerdie1  = {0,SPR_HITLER_W1,1,NULL,(statefunc)A_DeathScream,&s_hitlerdie2};
statetype s_hitlerdie2  = {0,SPR_HITLER_W1,10,NULL,NULL,&s_hitlerdie3};
statetype s_hitlerdie3  = {0,SPR_HITLER_DIE1,10,NULL,(statefunc)A_Slurpie,&s_hitlerdie4};
statetype s_hitlerdie4  = {0,SPR_HITLER_DIE2,10,NULL,NULL,&s_hitlerdie5};
statetype s_hitlerdie5  = {0,SPR_HITLER_DIE3,10,NULL,NULL,&s_hitlerdie6};
statetype s_hitlerdie6  = {0,SPR_HITLER_DIE4,10,NULL,NULL,&s_hitlerdie7};
statetype s_hitlerdie7  = {0,SPR_HITLER_DIE5,10,NULL,NULL,&s_hitlerdie8};
statetype s_hitlerdie8  = {0,SPR_HITLER_DIE6,10,NULL,NULL,&s_hitlerdie9};
statetype s_hitlerdie9  = {0,SPR_HITLER_DIE7,10,NULL,NULL,&s_hitlerdie10};
statetype s_hitlerdie10 = {0,SPR_HITLER_DEAD,20,NULL,(statefunc)A_StartDeathCam,&s_hitlerdie10};

statetype s_hitlershoot1 = {0,SPR_HITLER_SHOOT1,30,NULL,NULL,&s_hitlershoot2};
statetype s_hitlershoot2 = {0,SPR_HITLER_SHOOT2,10,NULL,(statefunc)T_Shoot,&s_hitlershoot3};
statetype s_hitlershoot3 = {0,SPR_HITLER_SHOOT3,10,NULL,(statefunc)T_Shoot,&s_hitlershoot4};
statetype s_hitlershoot4 = {0,SPR_HITLER_SHOOT2,10,NULL,(statefunc)T_Shoot,&s_hitlershoot5};
statetype s_hitlershoot5 = {0,SPR_HITLER_SHOOT3,10,NULL,(statefunc)T_Shoot,&s_hitlershoot6};
statetype s_hitlershoot6 = {0,SPR_HITLER_SHOOT2,10,NULL,(statefunc)T_Shoot,&s_hitlerchase1};

/* ghosts */
statetype s_blinkychase1 = {0,SPR_BLINKY_W1,10,(statefunc)T_Ghosts,NULL,&s_blinkychase2};
statetype s_blinkychase2 = {0,SPR_BLINKY_W2,10,(statefunc)T_Ghosts,NULL,&s_blinkychase1};

statetype s_inkychase1   = {0,SPR_INKY_W1,10,(statefunc)T_Ghosts,NULL,&s_inkychase2};
statetype s_inkychase2   = {0,SPR_INKY_W2,10,(statefunc)T_Ghosts,NULL,&s_inkychase1};

statetype s_pinkychase1  = {0,SPR_PINKY_W1,10,(statefunc)T_Ghosts,NULL,&s_pinkychase2};
statetype s_pinkychase2  = {0,SPR_PINKY_W2,10,(statefunc)T_Ghosts,NULL,&s_pinkychase1};

statetype s_clydechase1  = {0,SPR_CLYDE_W1,10,(statefunc)T_Ghosts,NULL,&s_clydechase2};
statetype s_clydechase2  = {0,SPR_CLYDE_W2,10,(statefunc)T_Ghosts,NULL,&s_clydechase1};


/* rocket / smoke */

statetype s_rocket = {1,SPR_ROCKET_1,3,(statefunc)T_Projectile,(statefunc)A_Smoke,&s_rocket};

statetype s_smoke1 = {0,SPR_SMOKE_1,3,NULL,NULL,&s_smoke2};
statetype s_smoke2 = {0,SPR_SMOKE_2,3,NULL,NULL,&s_smoke3};
statetype s_smoke3 = {0,SPR_SMOKE_3,3,NULL,NULL,&s_smoke4};
statetype s_smoke4 = {0,SPR_SMOKE_4,3,NULL,NULL,NULL};

statetype s_boom1 = {0,SPR_BOOM_1,6,NULL,NULL,&s_boom2};
statetype s_boom2 = {0,SPR_BOOM_2,6,NULL,NULL,&s_boom3};
statetype s_boom3 = {0,SPR_BOOM_3,6,NULL,NULL,NULL};

/* BJ victory */

statetype s_bjrun1   = {0,SPR_BJ_W1,12,(statefunc)T_BJRun,NULL,&s_bjrun1s};
statetype s_bjrun1s  = {0,SPR_BJ_W1,3, NULL,NULL,&s_bjrun2};
statetype s_bjrun2   = {0,SPR_BJ_W2,8,(statefunc)T_BJRun,NULL,&s_bjrun3};
statetype s_bjrun3   = {0,SPR_BJ_W3,12,(statefunc)T_BJRun,NULL,&s_bjrun3s};
statetype s_bjrun3s  = {0,SPR_BJ_W3,3, NULL,NULL,&s_bjrun4};
statetype s_bjrun4   = {0,SPR_BJ_W4,8,(statefunc)T_BJRun,NULL,&s_bjrun1};

statetype s_bjjump1  = {0,SPR_BJ_JUMP1,14,(statefunc)T_BJJump,NULL,&s_bjjump2};
statetype s_bjjump2  = {0,SPR_BJ_JUMP2,14,(statefunc)T_BJJump,(statefunc)T_BJYell,&s_bjjump3};
statetype s_bjjump3  = {0,SPR_BJ_JUMP3,14,(statefunc)T_BJJump,NULL,&s_bjjump4};
statetype s_bjjump4  = {0,SPR_BJ_JUMP4,300,NULL,(statefunc)T_BJDone,&s_bjjump4};

/* local variables */

static S16 starthitpoints[4][NUMENEMIES] =
{
    /* BABY MODE */
    {
        25,   /* guards */
        50,   /* officer */
        100,  /* SS */
        1,    /* dogs */
        850,  /* Hans */
        850,  /* Schabbs */
        200,  /* fake hitler */
        800,  /* mecha hitler */
        45,   /* mutants */
        25,   /* ghosts */
        25,   /* ghosts */
        25,   /* ghosts */
        25,   /* ghosts */

        850,  /* Gretel */
        850,  /* Gift */
        850,  /* Fat */
        5,    /* en_spectre, */
        1450, /* en_angel, */
        850,  /* en_trans, */
        1050, /* en_uber, */
        950,  /* en_will, */
        1250  /* en_death */
    },
    /* DON'T HURT ME MODE */
    {
        25,   /* guards */
        50,   /* officer */
        100,  /* SS */
        1,    /* dogs */
        950,  /* Hans */
        950,  /* Schabbs */
        300,  /* fake hitler */
        950,  /* mecha hitler */
        55,   /* mutants */
        25,   /* ghosts */
        25,   /* ghosts */
        25,   /* ghosts */
        25,   /* ghosts */

        950,  /* Gretel */
        950,  /* Gift   */
        950,  /* Fat    */
        10,   /* en_spectre, */
        1550, /* en_angel,   */
        950,  /* en_trans,   */
        1150, /* en_uber,    */
        1050, /* en_will,    */
        1350  /* en_death    */
    },
    /* BRING 'EM ON MODE */
    {
        25,   /* guards */
        50,   /* officer */
        100,  /* SS */
        1,    /* dogs */

        1050, /* Hans */
        1550, /* Schabbs */
        400,  /* fake hitler */
        1050, /* mecha hitler */

        55,   /* mutants */
        25,   /* ghosts */
        25,   /* ghosts */
        25,   /* ghosts */
        25,   /* ghosts */

        1050, /* Gretel */
        1050, /* Gift   */
        1050, /* Fat    */
        15,   /* en_spectre, */
        1650, /* en_angel,   */
        1050, /* en_trans,   */
        1250, /* en_uber,    */
        1150, /* en_will,    */
        1450  /* en_death    */
    },
    /* DEATH INCARNATE MODE */
    {
        25,   /* guards  */
        50,   /* officer */
        100,  /* SS      */
        1,    /* dogs    */

        1200, /* Hans         */
        2400, /* Schabbs      */
        500,  /* fake hitler  */
        1200, /* mecha hitler */

        65,   /* mutants */
        25,   /* ghosts  */
        25,   /* ghosts  */
        25,   /* ghosts  */
        25,   /* ghosts  */

        1200, /* Gretel     */
        1200, /* Gift       */
        1200, /* Fat        */
        25,   /* en_spectre, */
        2000, /* en_angel,   */
        1200, /* en_trans,   */
        1400, /* en_uber,    */
        1300, /* en_will,    */
        1600  /* en_death    */
    }
};


/* local prototypes */

static void SelectPathDir(objtype *ob);
static void SelectRunDir(objtype *ob);
static U8 ProjectileTryMove(objtype *ob);

/*
================================================================
=
= Function: SpawnStand
=
= Description:
=
=
=
================================================================
*/

void SpawnStand(enemy_t which, S32 tilex, S32 tiley, S32 dir)
{
    U16 *map;
    U16 tile;

    switch (which)
    {
        case en_guard:
            SpawnNewObj(tilex,tiley,&s_grdstand);
            newobj->speed = SPDPATROL;
            if(loadedgame == 0)
            {
                gamestate.killtotal++;
            }
        break;

        case en_officer:
            SpawnNewObj(tilex,tiley,&s_ofcstand);
            newobj->speed = SPDPATROL;
            if(loadedgame == 0)
            {
                gamestate.killtotal++;
            }
        break;

        case en_mutant:
            SpawnNewObj(tilex,tiley,&s_mutstand);
            newobj->speed = SPDPATROL;
            if(loadedgame == 0)
            {
                gamestate.killtotal++;
            }
        break;

        case en_ss:
            SpawnNewObj(tilex,tiley,&s_ssstand);
            newobj->speed = SPDPATROL;
            if(loadedgame == 0)
            {
                gamestate.killtotal++;
            }
        break;

        default:
            /* do nothing */
        break;
    }

    map = mapsegs[0]+(tiley<<mapshift)+tilex;
    tile = *map;

    if(tile == AMBUSHTILE)
    {
        tilemap[tilex][tiley] = 0;

        if (*(map+1) >= AREATILE)
        {
            tile = *(map+1);
        }

        if (*(map-MAPWIDTH) >= AREATILE)
        {
            tile = *(map-MAPWIDTH);
        }

        if (*(map+MAPWIDTH) >= AREATILE)
        {
            tile = *(map+MAPWIDTH);
        }

        if ( *(map-1) >= AREATILE)
        {
            tile = *(map-1);
        }

        *map = tile;
        newobj->areanumber = tile-AREATILE;
        newobj->flags |= FL_AMBUSH;
    }

    newobj->obclass = (classtype)(guardobj + which);
    newobj->hitpoints = starthitpoints[gamestate.difficulty][which];
    newobj->dir = (dirtype)(dir * 2);
    newobj->flags |= FL_SHOOTABLE;
}

/*
================================================================
=
= Function: SpawnPatrol
=
= Description:
=
=
=
================================================================
*/

void SpawnPatrol(enemy_t which, S32 tilex, S32 tiley, S32 dir)
{
    switch (which)
    {
        case en_guard:
            SpawnNewObj (tilex,tiley,&s_grdpath1);
            newobj->speed = SPDPATROL;
            if(loadedgame == 0)
            {
                gamestate.killtotal++;
            }
        break;

        case en_officer:
            SpawnNewObj (tilex,tiley,&s_ofcpath1);
            newobj->speed = SPDPATROL;
            if(loadedgame == 0)
            {
                gamestate.killtotal++;
            }
        break;

        case en_ss:
            SpawnNewObj (tilex,tiley,&s_sspath1);
            newobj->speed = SPDPATROL;
            if(loadedgame == 0)
            {
                gamestate.killtotal++;
            }
        break;

        case en_mutant:
            SpawnNewObj (tilex,tiley,&s_mutpath1);
            newobj->speed = SPDPATROL;
            if(loadedgame == 0)
            {
                gamestate.killtotal++;
            }
        break;

        case en_dog:
            SpawnNewObj (tilex,tiley,&s_dogpath1);
            newobj->speed = SPDDOG;
            if (loadedgame == 0)
            {
                gamestate.killtotal++;
            }
        break;

        default:
            /* do nothing */
        break;
    }

    newobj->obclass = (classtype)(guardobj+which);
    newobj->dir = (dirtype)(dir*2);
    newobj->hitpoints = starthitpoints[gamestate.difficulty][which];
    newobj->distance = TILEGLOBAL;
    newobj->flags |= FL_SHOOTABLE;
    newobj->active = ac_yes;

    actorat[newobj->tilex][newobj->tiley] = NULL;    /* don't use original spot */

    switch (dir)
    {
        case 0:
            newobj->tilex++;
        break;

        case 1:
            newobj->tiley--;
        break;

        case 2:
            newobj->tilex--;
        break;

        case 3:
            newobj->tiley++;
        break;
    }

    actorat[newobj->tilex][newobj->tiley] = newobj;
}

/*
================================================================
=
= Function: T_Stand
=
= Description:
=
= standard AI routine for all standing enemies
=
================================================================
*/
void T_Stand(objtype *ob)
{
    SightPlayer(ob);
}

/*
================================================================
=
= Function: T_Path
=
= Description:
=
= standard AI routine for all walking enemies
=
================================================================
*/
void T_Path(objtype *ob)
{
    S32    move;

    /* can actor see player ? */
    if(SightPlayer(ob) == 1)
    {
        return; /* yes and therefore has been put into combat frame */
    }

    if (ob->dir == nodir)
    {
        SelectPathDir(ob);
        if (ob->dir == nodir)
        {
            return;    /* all movement is blocked */
        }
    }

    /* set maximum movement for this actor */
    move = ob->speed*tics;

    /* loop until actor has moved the desired amount */
    while (move)
    {
        /* is actor standing by a door */
        if (ob->distance < 0)
        {
            /* yes then open the door */
            OpenDoor (-ob->distance-1);

            /* waiting for a door to open */
            if (doorobjlist[-ob->distance-1].action != dr_open)
            {
                return;
            }

            ob->distance = TILEGLOBAL;    /* go ahead, the door is now open */
            TryWalk(ob);
        }

        /* if actor only needs to move a small amount, move him and exit */
        if (move < ob->distance)
        {
            MoveObj (ob,move);
            break;
        }


        if (ob->tilex>MAPSIZE || ob->tiley>MAPSIZE)
        {
            iprintf("T_Path hit a wall at %u,%u, dir %u",ob->tilex,ob->tiley,ob->dir);
            while(1){ /* hang system */};
        }

        /* reached goal tile, so select another one            */
        /* fix position to account for round off during moving */
        ob->x = ((S32)ob->tilex<<TILESHIFT)+TILEGLOBAL/2;
        ob->y = ((S32)ob->tiley<<TILESHIFT)+TILEGLOBAL/2;
        move -= ob->distance;

        SelectPathDir(ob);

        if (ob->dir == nodir)
        {
            return;    /* all movement is blocked */
        }
    }
}

/*
================================================================
=
= Function: SelectPathDir
=
= Description:
=
= Select actor path
=
================================================================
*/
static void SelectPathDir(objtype *ob)
{
    U32 spot;

    spot = MAPSPOT(ob->tilex,ob->tiley,1)-ICONARROWS;

    if(spot<8)
    {
        /* new direction */
        ob->dir = (dirtype)(spot);
    }

    ob->distance = TILEGLOBAL;

    if (!TryWalk (ob))
    {
        ob->dir = nodir;
    }
}

/*
================================================================
=
= Function: A_DeathScream
=
= Description:
=
= Play death scream sound effect
=
================================================================
*/
void A_DeathScream(objtype *ob)
{

    S32 sounds[9] =
    {
        DEATHSCREAM1SND,
        DEATHSCREAM2SND,
        DEATHSCREAM3SND,
        DEATHSCREAM4SND,
        DEATHSCREAM5SND,
        DEATHSCREAM7SND,
        DEATHSCREAM8SND,
        DEATHSCREAM9SND
    };

#ifndef SHAREWARE
    if(mapon==9 && !US_RndT())
    {
        switch(ob->obclass)
        {
            case mutantobj:
            case guardobj:
            case officerobj:
            case ssobj:
            case dogobj:
                PlaySoundLocActor(DEATHSCREAM6SND,ob);
                return;
            break;

            default:
                /* do nothing */
            break;
        }
    }
#endif

    switch (ob->obclass)
    {
        case mutantobj:
            PlaySoundLocActor(AHHHGSND,ob);
        break;

        case guardobj:
#ifndef SHAREWARE
            PlaySoundLocActor(sounds[US_RndT()%8],ob);
#else
            PlaySoundLocActor(sounds[US_RndT()%2],ob);
#endif
        break;

        case officerobj:
            PlaySoundLocActor(NEINSOVASSND,ob);
        break;

        case ssobj:
            PlaySoundLocActor(LEBENSND,ob);
        break;

        case dogobj:
            PlaySoundLocActor(DOGDEATHSND,ob);
        break;

        case bossobj:
            SD_PlaySound(MUTTISND);
        break;

        case schabbobj:
            SD_PlaySound(MEINGOTTSND);
        break;
        case fakeobj:
            SD_PlaySound(HITLERHASND);
        break;

        case mechahitlerobj:
            SD_PlaySound(SCHEISTSND);
        break;

        case realhitlerobj:
            SD_PlaySound(EVASND);
        break;

        case gretelobj:
            SD_PlaySound(MEINSND);
        break;

        case giftobj:
            SD_PlaySound(DONNERSND);
        break;

        case fatobj:
            SD_PlaySound(ROSESND);
        break;

        default:
            /* do nothing */
        break;
    }

}

/*
================================================================
=
= Function: A_StartDeathCam
=
= Description:
=
= Play actor death cam
=
================================================================
*/
void A_StartDeathCam(objtype *ob)
{
    if(gamestate.victoryflag)
    {
        playstate = ex_victorious;    /* exit castle tile */
        return;
    }
}

/*
================================================================
=
= Function: A_Slurpie
=
= Description:
=
= Play SLURPIESND sound effect
=
================================================================
*/
void A_Slurpie(objtype *ob)
{
    SD_PlaySound(SLURPIESND);
}

/*
================================================================
=
= Function: SpawnGift
=
= Description:
=
=
=
================================================================
*/
void SpawnGift(S32 tilex, S32 tiley)
{
    if(SD_GetDigiDevice() != sds_Off)
    {
        s_giftdie2.tictime = 140;
    }
    else
    {
        s_giftdie2.tictime = 5;
    }

    SpawnNewObj (tilex,tiley,&s_giftstand);

    newobj->speed = SPDPATROL;
    newobj->obclass = giftobj;
    newobj->hitpoints = starthitpoints[gamestate.difficulty][en_gift];
    newobj->dir = nodir;
    newobj->flags |= FL_SHOOTABLE|FL_AMBUSH;

    if(loadedgame == 0)
    {
        gamestate.killtotal++;
    }
}

/*
================================================================
=
= Function: SpawnFat
=
= Description:
=
=
=
================================================================
*/
void SpawnFat(S32 tilex, S32 tiley)
{
    if (SD_GetDigiDevice() != sds_Off)
    {
        s_fatdie2.tictime = 140;
    }
    else
    {
        s_fatdie2.tictime = 5;
    }

    SpawnNewObj (tilex,tiley,&s_fatstand);

    newobj->speed = SPDPATROL;
    newobj->obclass = fatobj;
    newobj->hitpoints = starthitpoints[gamestate.difficulty][en_fat];
    newobj->dir = nodir;
    newobj->flags |= FL_SHOOTABLE|FL_AMBUSH;

    if(loadedgame == 0)
    {
        gamestate.killtotal++;
    }
}

/*
================================================================
=
= Function: SpawnSchabbs
=
= Description:
=
=
=
================================================================
*/
void SpawnSchabbs(S32 tilex, S32 tiley)
{
    if (SD_GetDigiDevice() != sds_Off)
    {
        s_schabbdie2.tictime = 140;
    }
    else
    {
        s_schabbdie2.tictime = 5;
    }

    SpawnNewObj (tilex,tiley,&s_schabbstand);

    newobj->speed = SPDPATROL;
    newobj->obclass = schabbobj;
    newobj->hitpoints = starthitpoints[gamestate.difficulty][en_schabbs];
    newobj->dir = nodir;
    newobj->flags |= FL_SHOOTABLE|FL_AMBUSH;

    if(loadedgame == 0)
    {
        gamestate.killtotal++;
    }
}

/*
================================================================
=
= Function: SpawnFakeHitler
=
= Description:
=
=
=
================================================================
*/
void SpawnFakeHitler(S32 tilex, S32 tiley)
{
    if (SD_GetDigiDevice() != sds_Off)
    {
        s_hitlerdie2.tictime = 140;
    }
    else
    {
        s_hitlerdie2.tictime = 5;
    }

    SpawnNewObj (tilex,tiley,&s_fakestand);

    newobj->speed = SPDPATROL;
    newobj->obclass = fakeobj;
    newobj->hitpoints = starthitpoints[gamestate.difficulty][en_fake];
    newobj->dir = nodir;
    newobj->flags |= FL_SHOOTABLE|FL_AMBUSH;

    if(loadedgame == 0)
    {
        gamestate.killtotal++;
    }
}

/*
================================================================
=
= Function: SpawnHitler
=
= Description:
=
=
=
================================================================
*/
void SpawnHitler(S32 tilex, S32 tiley)
{
    if(SD_GetDigiDevice() != sds_Off)
    {
        s_hitlerdie2.tictime = 140;
    }
    else
    {
        s_hitlerdie2.tictime = 5;
    }

    SpawnNewObj(tilex,tiley,&s_mechastand);

    newobj->speed = SPDPATROL;
    newobj->obclass = mechahitlerobj;
    newobj->hitpoints = starthitpoints[gamestate.difficulty][en_hitler];
    newobj->dir = nodir;
    newobj->flags |= FL_SHOOTABLE|FL_AMBUSH;

    if(loadedgame == 0)
    {
        gamestate.killtotal++;
    }
}

/*
================================================================
=
= Function: SpawnGretel
=
= Description:
=
=
=
================================================================
*/
void SpawnGretel(S32 tilex, S32 tiley)
{
    SpawnNewObj (tilex,tiley,&s_gretelstand);

    newobj->speed = SPDPATROL;
    newobj->obclass = gretelobj;
    newobj->hitpoints = starthitpoints[gamestate.difficulty][en_gretel];
    newobj->dir = nodir;
    newobj->flags |= FL_SHOOTABLE|FL_AMBUSH;

    if(loadedgame == 0)
    {
        gamestate.killtotal++;
    }
}

/*
================================================================
=
= Function: SpawnBoss
=
= Description:
=
=
=
================================================================
*/
void SpawnBoss(S32 tilex, S32 tiley)
{
    SpawnNewObj(tilex,tiley,&s_bossstand);

    newobj->speed = SPDPATROL;
    newobj->obclass = bossobj;
    newobj->hitpoints = starthitpoints[gamestate.difficulty][en_boss];
    newobj->dir = nodir;
    newobj->flags |= FL_SHOOTABLE|FL_AMBUSH;

    if(loadedgame == 0)
    {
        gamestate.killtotal++;
    }
}

/*
================================================================
=
= Function: SpawnGhosts
=
= Description:
=
=
=
================================================================
*/
void SpawnGhosts (S32 which, S32 tilex, S32 tiley)
{
    switch(which)
    {
        case en_blinky:
            SpawnNewObj(tilex,tiley,&s_blinkychase1);
        break;

        case en_clyde:
            SpawnNewObj(tilex,tiley,&s_clydechase1);
        break;

        case en_pinky:
            SpawnNewObj(tilex,tiley,&s_pinkychase1);
        break;

        case en_inky:
            SpawnNewObj(tilex,tiley,&s_inkychase1);
        break;

        default:
            /* do nothing */
        break;
    }

    newobj->obclass = ghostobj;
    newobj->speed = SPDDOG;
    newobj->dir = east;
    newobj->flags |= FL_AMBUSH;

    if(loadedgame == 0)
    {
        gamestate.killtotal++;
        gamestate.killcount++;
    }
}

/*
================================================================
=
= Function: SpawnDeadGuard
=
= Description:
=
=
=
================================================================
*/
void SpawnDeadGuard(S32 tilex, S32 tiley)
{
    SpawnNewObj(tilex,tiley,&s_grddie4);
    newobj->flags |= FL_NONMARK;    /* walk through moving enemy fix */
    newobj->obclass = inertobj;
}

/*
================================================================
=
= Function: T_Ghosts
=
= Description:
=
=
=
================================================================
*/
void T_Ghosts(objtype *ob)
{
    S32 move;

    if (ob->dir == nodir)
    {
        SelectChaseDir (ob);

        if (ob->dir == nodir)
        {
            return;    /* object is blocked in */
        }
    }

    move = ob->speed*tics;

    while (move)
    {
        if (move < ob->distance)
        {
            MoveObj (ob,move);
            break;
        }

        /* reached goal tile, so select another one            */
        /* fix position to account for round off during moving */
        ob->x = ((int32_t)ob->tilex<<TILESHIFT)+TILEGLOBAL/2;
        ob->y = ((int32_t)ob->tiley<<TILESHIFT)+TILEGLOBAL/2;

        move -= ob->distance;

        SelectChaseDir (ob);

        if (ob->dir == nodir)
        {
            return;    /* object is blocked in */
        }
    }
}

/*
================================================================
=
= Function: T_Chase
=
= Description:
=
= Actor chase player AI routine
=
================================================================
*/
void T_Chase(objtype *ob)
{
    S32 move,target;
    S32 dx,dy,dist,chance;
    U8 dodge;

    if(gamestate.victoryflag == 1)
    {
        return;
    }

    dodge = 0;

    if(CheckLine(ob))      /* got a shot at player? */
    {
        ob->hidden = 0;
        /* tile distance between player and actor */
        dx = abs(ob->tilex - player->tilex);
        dy = abs(ob->tiley - player->tiley);
        /* work out the longest distance between player and actor */
        dist = (dx > dy) ? dx : dy;

        /* calaculate the chance of actor attacking based on distance */
        if(dist != 0)
        {
            chance = (tics<<4)/dist;
        }
        else
        {
            chance = 300;
        }

        if(dist == 1)
        {
            target = abs(ob->x - player->x);
            if(target < 0x14000l)
            {
                target = abs(ob->y - player->y);
                if (target < 0x14000l)
                {
                    chance = 300;
                }
            }
        }

        if(US_RndT() < chance)
        {
            /* go into attack frame */
            switch (ob->obclass)
            {
                case guardobj:
                    NewState(ob,&s_grdshoot1);
                break;

                case officerobj:
                    NewState(ob,&s_ofcshoot1);
                break;

                case mutantobj:
                    NewState(ob,&s_mutshoot1);
                break;

                case ssobj:
                    NewState(ob,&s_ssshoot1);
                break;

                case bossobj:
                    NewState(ob,&s_bossshoot1);
                break;

                case gretelobj:
                    NewState(ob,&s_gretelshoot1);
                break;

                case mechahitlerobj:
                    NewState(ob,&s_mechashoot1);
                break;

                case realhitlerobj:
                    NewState(ob,&s_hitlershoot1);
                break;

                default:
                    /* do nothing */
                break;
            }

            return;
        }

        /* dont attack, dodge instead this time round */
        dodge = 1;
    }
    else
    {
        ob->hidden = 1;
    }

    if(ob->dir == nodir)
    {
        if(dodge == 1)
        {
            SelectDodgeDir(ob);
        }
        else
        {
            SelectChaseDir(ob);
        }

        if (ob->dir == nodir)
        {
            return;    /* object is blocked in */
        }
    }

    /* set maximum movement for this actor */
    move = ob->speed*tics;

    /* loop until actor has moved the desired amount */
    while(move)
    {
        /* is actor standing by a door */
        if (ob->distance < 0)
        {
            /* yes then open the door */
            OpenDoor(-ob->distance-1);

            /* waiting for a door to open */
            if (doorobjlist[-ob->distance-1].action != dr_open)
            {
                return;
            }

            ob->distance = TILEGLOBAL;    /* go ahead, the door is now open */

            TryWalk(ob);
        }

        /* if actor only needs to move a small amount, move him and exit */
        if(move < ob->distance)
        {
            MoveObj(ob,move);
            break;
        }

        /* reached goal tile, so select another one            */
        /* fix position to account for round off during moving */
        ob->x = ((S32)ob->tilex<<TILESHIFT)+TILEGLOBAL/2;
        ob->y = ((S32)ob->tiley<<TILESHIFT)+TILEGLOBAL/2;

        move -= ob->distance;

        if(dodge == 1)
        {
            SelectDodgeDir(ob);
        }
        else
        {
            SelectChaseDir(ob);
        }

        if(ob->dir == nodir)
        {
            return;    /* object is blocked in */
        }
    }
}

/*
================================================================
=
= Function: T_Shoot
=
= Description:
=
= Try to damage the player, based on skill level and player's
= speed
=
================================================================
*/
void T_Shoot(objtype *ob)
{
    S32    dx;
    S32    dy;
    S32    dist;
    S32    hitchance;
    S32    damage;

    hitchance = 128;

    if(areabyplayer[ob->areanumber] == 0)
    {
        return;
    }

    if(CheckLine(ob))    /* player is not behind a wall */
    {
        dx = abs(ob->tilex - player->tilex);
        dy = abs(ob->tiley - player->tiley);
        dist = dx>dy ? dx:dy;

        if (ob->obclass == ssobj || ob->obclass == bossobj)
        {
            dist = dist*2/3;    /* ss are better shots */
        }

        if(thrustspeed >= RUNSPEED)
        {
            if (ob->flags&FL_VISABLE)
            {
                hitchance = 160-dist*16;    /* player can see to dodge */
            }
            else
            {
                hitchance = 160-dist*8;
            }
        }
        else
        {
            if(ob->flags&FL_VISABLE)
            {
                hitchance = 256-dist*16;    /* player can see to dodge */
            }
            else
            {
                hitchance = 256-dist*8;
            }
        }

        /* see if the shot was a hit */

        if (US_RndT() < hitchance)
        {
            if(dist<2)
            {
                damage = US_RndT()>>2;
            }
            else if (dist<4)
            {
                damage = US_RndT()>>3;
            }
            else
            {
                damage = US_RndT()>>4;
            }

            TakeDamage (damage,ob);
        }
    }

    switch(ob->obclass)
    {
        case ssobj:
            PlaySoundLocActor(SSFIRESND,ob);
        break;

        case giftobj:
        case fatobj:
            PlaySoundLocActor(MISSILEFIRESND,ob);
        break;

        case mechahitlerobj:
        case realhitlerobj:
        case bossobj:
            PlaySoundLocActor(BOSSFIRESND,ob);
        break;

        case schabbobj:
            PlaySoundLocActor(SCHABBSTHROWSND,ob);
        break;

        case fakeobj:
            PlaySoundLocActor(FLAMETHROWERSND,ob);
        break;

        default:
            PlaySoundLocActor(NAZIFIRESND,ob);
        break;
    }
}

/*
================================================================
=
= Function: T_DogChase
=
= Description:
=
=
=
================================================================
*/
void T_DogChase(objtype *ob)
{
    S32    move;
    S32    dx;
    S32    dy;

    if(ob->dir == nodir)
    {
        SelectDodgeDir(ob);

        if (ob->dir == nodir)
        {
            return;    /* object is blocked in */
        }
    }

    /* set maximum movement for this actor */
    move = ob->speed*tics;

    /* loop until actor has moved the desired amount */
    while(move)
    {
        /* check for byte range */
        dx = player->x - ob->x;

        if (dx<0)
        {
            dx = -dx;
        }

        dx -= move;

        if (dx <= MINACTORDIST)
        {
            dy = player->y - ob->y;

            if (dy<0)
            {
                dy = -dy;
            }

            dy -= move;

            if (dy <= MINACTORDIST)
            {
                NewState(ob,&s_dogjump1);
                return;
            }
        }

        /* if actor only needs to move a small amount, move him and exit */
        if (move < ob->distance)
        {
            MoveObj (ob,move);
            break;
        }

        /* reached goal tile, so select another one            */
        /* fix position to account for round off during moving */
        ob->x = ((S32)ob->tilex<<TILESHIFT)+TILEGLOBAL/2;
        ob->y = ((S32)ob->tiley<<TILESHIFT)+TILEGLOBAL/2;

        move -= ob->distance;

        SelectDodgeDir (ob);

        if (ob->dir == nodir)
        {
            return;    /* object is blocked in */
        }
    }
}

/*
================================================================
=
= Function: T_Bite
=
= Description:
=
=
=
================================================================
*/
void T_Bite(objtype *ob)
{
    S32    dx;
    S32    dy;

    PlaySoundLocActor(DOGATTACKSND,ob);

    dx = player->x - ob->x;

    if (dx<0)
    {
        dx = -dx;
    }

    dx -= TILEGLOBAL;

    if (dx <= MINACTORDIST)
    {
        dy = player->y - ob->y;

        if (dy<0)
        {
            dy = -dy;
        }

        dy -= TILEGLOBAL;

        if (dy <= MINACTORDIST)
        {
            if(US_RndT()<180)
            {
                TakeDamage(US_RndT()>>4,ob);
                return;
            }
        }
    }
}

/*
================================================================
=
= Function: T_Gift
=
= Description:
=
=
=
================================================================
*/
void T_Gift(objtype *ob)
{
    S32 move;
    S32 dx;
    S32 dy;
    S32 dist;
    U8 dodge;

    dodge = 0;
    /* tile distance between player and actor */
    dx = abs(ob->tilex - player->tilex);
    dy = abs(ob->tiley - player->tiley);
    /* work out the longest distance between player and actor */
    dist = dx>dy ? dx : dy;

    /* got a shot at player? */
    if(CheckLine(ob) == 1)
    {
        ob->hidden = 0;

        if((unsigned) US_RndT() < (tics<<3) && objfreelist)
        {
            /* go into attack frame */
            NewState (ob,&s_giftshoot1);
            return;
        }

        dodge = 1;
    }
    else
    {
        ob->hidden = 1;
    }

    if(ob->dir == nodir)
    {
        if(dodge)
        {
            SelectDodgeDir(ob);
        }
        else
        {
            SelectChaseDir (ob);
        }

        if (ob->dir == nodir)
        {
            return;    /* object is blocked in */
        }
    }

    move = ob->speed*tics;

    while(move)
    {
        if (ob->distance < 0)
        {
            /* waiting for a door to open */
            OpenDoor (-ob->distance-1);

            if (doorobjlist[-ob->distance-1].action != dr_open)
            {
                return;
            }

            ob->distance = TILEGLOBAL;    /* go ahead, the door is now open */
            TryWalk(ob);
        }

        if (move < ob->distance)
        {
            MoveObj (ob,move);
            break;
        }

        /* reached goal tile, so select another one            */
        /* fix position to account for round off during moving */
        ob->x = ((S32)ob->tilex<<TILESHIFT)+TILEGLOBAL/2;
        ob->y = ((S32)ob->tiley<<TILESHIFT)+TILEGLOBAL/2;

        move -= ob->distance;

        if(dist <4)
        {
            SelectRunDir(ob);
        }
        else if(dodge)
        {
            SelectDodgeDir(ob);
        }
        else
        {
            SelectChaseDir(ob);
        }

        if (ob->dir == nodir)
        {
            return;   /* object is blocked in */
        }
    }
}

/*
================================================================
=
= Function: T_GiftThrow
=
= Description:
=
=
=
================================================================
*/
void T_GiftThrow(objtype *ob)
{
    S32    deltax;
    S32    deltay;
    float  angle;
    S32    iangle;

    deltax = player->x - ob->x;
    deltay = ob->y - player->y;

    angle = (float) atan2((float) deltay, (float) deltax);

    if (angle<0)
    {
        angle = (float) (M_PI*2+angle);
    }

    iangle = (S32)(angle/(M_PI*2)*ANGLES);

    GetNewActor();
    newobj->state = &s_rocket;
    newobj->ticcount = 1;

    newobj->tilex = ob->tilex;
    newobj->tiley = ob->tiley;
    newobj->x = ob->x;
    newobj->y = ob->y;
    newobj->obclass = rocketobj;
    newobj->dir = nodir;
    newobj->angle = iangle;
    newobj->speed = 0x2000l;
    newobj->flags = FL_NEVERMARK;
    newobj->active = ac_yes;

    PlaySoundLocActor (MISSILEFIRESND,newobj);
}

/*
================================================================
=
= Function: SelectRunDir
=
= Description:
=
= Run Away from player
=
================================================================
*/
static void SelectRunDir(objtype *ob)
{
    S32 deltax;
    S32 deltay;
    dirtype d[3];
    dirtype tdir;

    deltax=player->tilex - ob->tilex;
    deltay=player->tiley - ob->tiley;

    if(deltax < 0)
    {
        d[1]= east;
    }
    else
    {
        d[1]= west;
    }

    if(deltay < 0)
    {
        d[2]=south;
    }
    else
    {
        d[2]=north;
    }

    if(abs(deltay)>abs(deltax))
    {
        tdir=d[1];
        d[1]=d[2];
        d[2]=tdir;
    }

    ob->dir=d[1];

    if(TryWalk(ob) == 1)
    {
        return;     /*either moved forward or attacked*/
    }

    ob->dir=d[2];

    if(TryWalk(ob) == 1)
    {
        return;
    }

    /* there is no direct path to the player, so pick another direction */

    if (US_RndT()>128)      /*randomly determine direction of search*/
    {
        for (tdir=north; tdir<=west; tdir=(dirtype)(tdir+1))
        {
            ob->dir=tdir;

            if(TryWalk(ob) == 1)
            {
                return;
            }
        }
    }
    else
    {
        for(tdir=west; tdir>=north; tdir=(dirtype)(tdir-1))
        {
            ob->dir=tdir;

            if(TryWalk(ob) == 1)
            {
                return;
            }
        }
    }

    ob->dir = nodir;    /* can't move */
}

/*
================================================================
=
= Function: A_Smoke
=
= Description:
=
= create smoke sprite
=
================================================================
*/
void A_Smoke(objtype *ob)
{
    GetNewActor();

    newobj->state = &s_smoke1;
    newobj->ticcount = 6;

    newobj->tilex = ob->tilex;
    newobj->tiley = ob->tiley;
    newobj->x = ob->x;
    newobj->y = ob->y;
    newobj->obclass = inertobj;
    newobj->active = ac_yes;

    newobj->flags = FL_NEVERMARK;
}

/*
================================================================
=
= Function: T_Projectile
=
= Description:
=
=
=
================================================================
*/
void T_Projectile(objtype *ob)
{
    S32 deltax;
    S32 deltay;
    S32 damage;
    S32 speed;

    speed = (S32)ob->speed*tics;

    deltax = FixedMul(speed,costable[ob->angle]);
    deltay = -FixedMul(speed,sintable[ob->angle]);

    if(deltax > 0x10000l)
    {
        deltax = 0x10000l;
    }

    if(deltay > 0x10000l)
    {
        deltay = 0x10000l;
    }

    ob->x += deltax;
    ob->y += deltay;

    deltax = LABS(ob->x - player->x);
    deltay = LABS(ob->y - player->y);

    if(!ProjectileTryMove(ob))
    {
        if (ob->obclass == rocketobj)
        {
            PlaySoundLocActor(MISSILEHITSND,ob);
            ob->state = &s_boom1;
        }
        else
        {
            ob->state = NULL;    /* mark for removal */
        }

        return;
    }

    if(deltax < PROJECTILESIZE && deltay < PROJECTILESIZE)
    {
        /* hit the player */
        switch (ob->obclass)
        {
            case needleobj:
                damage = (US_RndT() >>3) + 20;
            break;

            case rocketobj:
            case hrocketobj:
            case sparkobj:
                damage = (US_RndT() >>3) + 30;
            break;

            case fireobj:
                damage = (US_RndT() >>3);
            break;

            default:
                damage = (US_RndT() >>3);
            break;
        }

        TakeDamage (damage,ob);
        ob->state = NULL;    /* mark for removal */
        return;
    }

    ob->tilex = (short)(ob->x >> TILESHIFT);
    ob->tiley = (short)(ob->y >> TILESHIFT);
}

/*
================================================================
=
= Function: ProjectileTryMove
=
= Description:
=
= returns true if move ok
=
================================================================
*/
static U8 ProjectileTryMove(objtype *ob)
{
    S32      xl,yl,xh,yh,x,y;
    objtype *check;

    xl = (ob->x-PROJSIZE) >> TILESHIFT;
    yl = (ob->y-PROJSIZE) >> TILESHIFT;

    xh = (ob->x+PROJSIZE) >> TILESHIFT;
    yh = (ob->y+PROJSIZE) >> TILESHIFT;

    /* check for solid walls */
    for (y=yl;y<=yh;y++)
    {
        for (x=xl;x<=xh;x++)
        {
            check = actorat[x][y];

            if (check && !ISPOINTER(check))
            {
                return 0;
            }
        }
    }

    return 1;
}

/*
================================================================
=
= Function: T_Fat
=
= Description:
=
=
=
================================================================
*/
void T_Fat (objtype *ob)
{
    S32 move;
    S32 dx;
    S32 dy;
    S32 dist;
    U8 dodge;

    dodge = 0;
    dx = abs(ob->tilex - player->tilex);
    dy = abs(ob->tiley - player->tiley);
    dist = dx>dy ? dx : dy;

    /* got a shot at player? */
    if(CheckLine(ob) == 1)
    {
        ob->hidden = 0;

        if((unsigned)US_RndT() < (tics<<3) && objfreelist)
        {
            /* go into attack frame */
            NewState(ob,&s_fatshoot1);
            return;
        }
        dodge = 1;
    }
    else
    {
        ob->hidden = true;
    }

    if(ob->dir == nodir)
    {
        if(dodge == 1)
        {
            SelectDodgeDir(ob);
        }
        else
        {
            SelectChaseDir(ob);
        }

        if(ob->dir == nodir)
        {
            return;    /* object is blocked in */
        }
    }

    move = ob->speed*tics;

    while (move)
    {
        if (ob->distance < 0)
        {

            OpenDoor (-ob->distance-1);

            /* waiting for a door to open */
            if (doorobjlist[-ob->distance-1].action != dr_open)
            {
                return;
            }

            ob->distance = TILEGLOBAL;    /* go ahead, the door is now open */
            TryWalk(ob);
        }

        if (move < ob->distance)
        {
            MoveObj (ob,move);
            break;
        }

        /* reached goal tile, so select another one            */
        /* fix position to account for round off during moving */
        ob->x = ((int32_t)ob->tilex<<TILESHIFT)+TILEGLOBAL/2;
        ob->y = ((int32_t)ob->tiley<<TILESHIFT)+TILEGLOBAL/2;

        move -= ob->distance;

        if (dist <4)
        {
            SelectRunDir(ob);
        }
        else if (dodge)
        {
            SelectDodgeDir(ob);
        }
        else
        {
            SelectChaseDir(ob);
        }

        if (ob->dir == nodir)
        {
            return;    /* object is blocked in */
        }
    }
}

/*
================================================================
=
= Function: T_Schabb
=
= Description:
=
=
=
================================================================
*/
void T_Schabb(objtype *ob)
{
    S32 move;
    S32 dx;
    S32 dy;
    S32 dist;
    U8 dodge;

    dodge = 0;
    dx = abs(ob->tilex - player->tilex);
    dy = abs(ob->tiley - player->tiley);
    dist = dx>dy ? dx : dy;

    /* got a shot at player? */
    if(CheckLine(ob) == 1)
    {
        ob->hidden = 0;
        if((unsigned) US_RndT() < (tics<<3) && objfreelist)
        {
            /* go into attack frame */
            NewState (ob,&s_schabbshoot1);
            return;
        }
        dodge = 1;
    }
    else
    {
        ob->hidden = 1;
    }

    if (ob->dir == nodir)
    {
        if(dodge == 1)
        {
            SelectDodgeDir (ob);
        }
        else
        {
            SelectChaseDir (ob);
        }

        if(ob->dir == nodir)
        {
            return;    /* object is blocked in */
        }
    }

    move = ob->speed*tics;

    while (move)
    {
        if (ob->distance < 0)
        {
            OpenDoor (-ob->distance-1);

            /* waiting for a door to open */
            if (doorobjlist[-ob->distance-1].action != dr_open)
            {
                return;
            }
            ob->distance = TILEGLOBAL;    /* go ahead, the door is now open */
            TryWalk(ob);
        }

        if (move < ob->distance)
        {
            MoveObj (ob,move);
            break;
        }

        /* reached goal tile, so select another one             */
        /* fix position to account for round off during moving  */
        ob->x = ((int32_t)ob->tilex<<TILESHIFT)+TILEGLOBAL/2;
        ob->y = ((int32_t)ob->tiley<<TILESHIFT)+TILEGLOBAL/2;

        move -= ob->distance;

        if (dist <4)
        {
            SelectRunDir(ob);
        }
        else if (dodge)
        {
            SelectDodgeDir (ob);
        }
        else
        {
            SelectChaseDir (ob);
        }

        if (ob->dir == nodir)
        {
            return;    /* object is blocked in */
        }
    }
}

/*
================================================================
=
= Function: T_SchabbThrow
=
= Description:
=
=
=
================================================================
*/
void T_SchabbThrow(objtype *ob)
{
    S32   deltax;
    S32   deltay;
    float angle;
    S32   iangle;

    deltax = player->x - ob->x;
    deltay = ob->y - player->y;

    angle = (float) atan2((float) deltay, (float) deltax);

    if(angle<0)
    {
        angle = (float) (M_PI*2+angle);
    }

    iangle = (S32)(angle/(M_PI*2)*ANGLES);

    GetNewActor ();
    newobj->state = &s_needle1;
    newobj->ticcount = 1;

    newobj->tilex = ob->tilex;
    newobj->tiley = ob->tiley;
    newobj->x = ob->x;
    newobj->y = ob->y;
    newobj->obclass = needleobj;
    newobj->dir = nodir;
    newobj->angle = iangle;
    newobj->speed = 0x2000l;

    newobj->flags = FL_NEVERMARK;
    newobj->active = ac_yes;

    PlaySoundLocActor(SCHABBSTHROWSND,newobj);
}

/*
================================================================
=
= Function: T_Fake
=
= Description:
=
=
=
================================================================
*/
void T_Fake (objtype *ob)
{
    S32 move;

    /* got a shot at player? */
    if(CheckLine(ob) == 1)
    {
        ob->hidden = 0;

        if((unsigned) US_RndT() < (tics<<1) && objfreelist)
        {
            /* go into attack frame */
            NewState (ob,&s_fakeshoot1);
            return;
        }
    }
    else
    {
        ob->hidden = 1;
    }

    if (ob->dir == nodir)
    {
        SelectDodgeDir (ob);
        if (ob->dir == nodir)
        {
            return;    /* object is blocked in */
        }
    }

    move = ob->speed*tics;

    while (move)
    {
        if (move < ob->distance)
        {
            MoveObj (ob,move);
            break;
        }

        /* reached goal tile, so select another one            */
        /* fix position to account for round off during moving */
        ob->x = ((int32_t)ob->tilex<<TILESHIFT)+TILEGLOBAL/2;
        ob->y = ((int32_t)ob->tiley<<TILESHIFT)+TILEGLOBAL/2;

        move -= ob->distance;

        SelectDodgeDir (ob);

        if (ob->dir == nodir)
        {
            return;    /* object is blocked in */
        }
    }
}

/*
================================================================
=
= Function: T_Fake
=
= Description:
=
= create fire ball sprite if we can get a free object
=
================================================================
*/
void T_FakeFire (objtype *ob)
{
    S32   deltax;
    S32   deltay;
    float angle;
    S32   iangle;

    /* stop shooting if over MAXACTORS */
    if (!objfreelist)
    {
        NewState(ob,&s_fakechase1);
        return;
    }

    deltax = player->x - ob->x;
    deltay = ob->y - player->y;
    angle = (float) atan2((float) deltay, (float) deltax);

    if(angle<0)
    {
        angle = (float)(M_PI*2+angle);
    }

    iangle = (int) (angle/(M_PI*2)*ANGLES);

    GetNewActor ();
    newobj->state = &s_fire1;
    newobj->ticcount = 1;

    newobj->tilex = ob->tilex;
    newobj->tiley = ob->tiley;
    newobj->x = ob->x;
    newobj->y = ob->y;
    newobj->dir = nodir;
    newobj->angle = iangle;
    newobj->obclass = fireobj;
    newobj->speed = 0x1200l;
    newobj->flags = FL_NEVERMARK;
    newobj->active = ac_yes;

    PlaySoundLocActor (FLAMETHROWERSND,newobj);
}

/*
================================================================
=
= Function: A_MechaSound
=
= Description:
=
=
=
================================================================
*/
void A_MechaSound (objtype *ob)
{
    if(areabyplayer[ob->areanumber] == 1)
    {
        PlaySoundLocActor (MECHSTEPSND,ob);
    }
}

/*
================================================================
=
= Function: A_HitlerMorph
=
= Description:
=
=
=
================================================================
*/
void A_HitlerMorph (objtype *ob)
{
    S16 hitpoints[4]={500,700,800,900};

    SpawnNewObj (ob->tilex,ob->tiley,&s_hitlerchase1);
    newobj->speed = SPDPATROL*5;

    newobj->x = ob->x;
    newobj->y = ob->y;

    newobj->distance = ob->distance;
    newobj->dir = ob->dir;
    newobj->flags = ob->flags | FL_SHOOTABLE;
    newobj->flags &= ~FL_NONMARK;   /* hitler stuck with nodir fix */

    newobj->obclass = realhitlerobj;
    newobj->hitpoints = hitpoints[gamestate.difficulty];
}

/*
================================================================
=
= Function: SpawnBJVictory
=
= Description:
=
=
=
================================================================
*/
void SpawnBJVictory (void)
{
    SpawnNewObj(player->tilex,player->tiley+1,&s_bjrun1);
    newobj->x = player->x;
    newobj->y = player->y;
    newobj->obclass = bjobj;
    newobj->dir = north;
    newobj->temp1 = 6;    /* tiles to run forward */
}

/*
================================================================
=
= Function: T_BJRun
=
= Description:
=
=
=
================================================================
*/
void T_BJRun (objtype *ob)
{
    S32    move;

    move = BJRUNSPEED*tics;

    while (move)
    {
        if (move < ob->distance)
        {
            MoveObj (ob,move);
            break;
        }


        ob->x = ((int32_t)ob->tilex<<TILESHIFT)+TILEGLOBAL/2;
        ob->y = ((int32_t)ob->tiley<<TILESHIFT)+TILEGLOBAL/2;
        move -= ob->distance;

        SelectPathDir (ob);

        if ( !(--ob->temp1) )
        {
            NewState (ob,&s_bjjump1);
            return;
        }
    }
}

/*
================================================================
=
= Function: T_BJJump
=
= Description:
=
=
=
================================================================
*/
void T_BJJump (objtype *ob)
{
    S32    move;

    move = BJJUMPSPEED*tics;
    MoveObj (ob,move);
}

/*
================================================================
=
= Function: T_BJYell
=
= Description:
=
=
=
================================================================
*/
void T_BJYell (objtype *ob)
{
    PlaySoundLocActor(YEAHSND,ob);
}

/*
================================================================
=
= Function: T_BJDone
=
= Description:
=
= exit castle tile
=
================================================================
*/
void T_BJDone (objtype *ob)
{
    playstate = ex_victorious;    /* exit castle tile */
}
