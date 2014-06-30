#ifndef WL_ACT2_H
#define WL_ACT2_H

extern void SpawnStand(enemy_t which, S32 tilex, S32 tiley, S32 dir);
extern void SpawnDeadGuard(S32 tilex, S32 tiley);
extern void SpawnPatrol(enemy_t which, S32 tilex, S32 tiley, S32 dir);
extern void SpawnBoss(S32 tilex, S32 tiley);
extern void SpawnGretel(S32 tilex, S32 tiley);
extern void SpawnGift(S32 tilex, S32 tiley);
extern void T_Stand(objtype *ob);
extern void T_Path(objtype *ob);
extern void A_DeathScream(objtype *ob);
extern void A_StartDeathCam(objtype *ob);
extern void SpawnFat(S32 tilex, S32 tiley);
extern void SpawnSchabbs(S32 tilex, S32 tiley);
extern void SpawnFakeHitler(S32 tilex, S32 tiley);
extern void A_Slurpie (objtype *ob);
extern void SpawnHitler(S32 tilex, S32 tiley);
extern void SpawnGhosts(S32 which, S32 tilex, S32 tiley);
extern void T_Ghosts(objtype *ob);
extern void T_Chase(objtype *ob);
extern void T_Shoot (objtype *ob);
extern void T_DogChase(objtype *ob);
extern void T_Bite(objtype *ob);
extern void T_Gift(objtype *ob);
extern void T_GiftThrow(objtype *ob);
extern void A_Smoke(objtype *ob);
extern void T_Projectile(objtype *ob);
extern void T_Fat (objtype *ob);
extern void T_Schabb(objtype *ob);
extern void T_SchabbThrow(objtype *ob);
extern void T_Fake(objtype *ob);
extern void T_FakeFire(objtype *ob);
extern void A_MechaSound (objtype *ob);
extern void A_HitlerMorph (objtype *ob);
extern void SpawnBJVictory (void);
extern void T_BJRun (objtype *ob);
extern void T_BJJump (objtype *ob);
extern void T_BJDone (objtype *ob);
extern void T_BJYell (objtype *ob);

/* guards */
extern  statetype s_grdstand;

extern  statetype s_grdpath1;
extern  statetype s_grdpath1s;
extern  statetype s_grdpath2;
extern  statetype s_grdpath3;
extern  statetype s_grdpath3s;
extern  statetype s_grdpath4;

extern  statetype s_grdpain;
extern  statetype s_grdpain1;

extern  statetype s_grdshoot1;
extern  statetype s_grdshoot2;
extern  statetype s_grdshoot3;

extern  statetype s_grdchase1;
extern  statetype s_grdchase1s;
extern  statetype s_grdchase2;
extern  statetype s_grdchase3;
extern  statetype s_grdchase3s;
extern  statetype s_grdchase4;

extern  statetype s_grddie1;
extern  statetype s_grddie2;
extern  statetype s_grddie3;
extern  statetype s_grddie4;

/* officers */
extern  statetype s_ofcstand;

extern  statetype s_ofcpath1;
extern  statetype s_ofcpath1s;
extern  statetype s_ofcpath2;
extern  statetype s_ofcpath3;
extern  statetype s_ofcpath3s;
extern  statetype s_ofcpath4;

extern  statetype s_ofcpain;
extern  statetype s_ofcpain1;

extern  statetype s_ofcshoot1;
extern  statetype s_ofcshoot2;
extern  statetype s_ofcshoot3;

extern  statetype s_ofcchase1;
extern  statetype s_ofcchase1s;
extern  statetype s_ofcchase2;
extern  statetype s_ofcchase3;
extern  statetype s_ofcchase3s;
extern  statetype s_ofcchase4;

extern  statetype s_ofcdie1;
extern  statetype s_ofcdie2;
extern  statetype s_ofcdie3;
extern  statetype s_ofcdie4;
extern  statetype s_ofcdie5;

/* mutant */
extern  statetype s_mutstand;

extern  statetype s_mutpath1;
extern  statetype s_mutpath1s;
extern  statetype s_mutpath2;
extern  statetype s_mutpath3;
extern  statetype s_mutpath3s;
extern  statetype s_mutpath4;

extern  statetype s_mutpain;
extern  statetype s_mutpain1;

extern  statetype s_mutshoot1;
extern  statetype s_mutshoot2;
extern  statetype s_mutshoot3;
extern  statetype s_mutshoot4;

extern  statetype s_mutchase1;
extern  statetype s_mutchase1s;
extern  statetype s_mutchase2;
extern  statetype s_mutchase3;
extern  statetype s_mutchase3s;
extern  statetype s_mutchase4;

extern  statetype s_mutdie1;
extern  statetype s_mutdie2;
extern  statetype s_mutdie3;
extern  statetype s_mutdie4;
extern  statetype s_mutdie5;

/* SS */
extern  statetype s_ssstand;

extern  statetype s_sspath1;
extern  statetype s_sspath1s;
extern  statetype s_sspath2;
extern  statetype s_sspath3;
extern  statetype s_sspath3s;
extern  statetype s_sspath4;

extern  statetype s_sspain;
extern  statetype s_sspain1;

extern  statetype s_ssshoot1;
extern  statetype s_ssshoot2;
extern  statetype s_ssshoot3;
extern  statetype s_ssshoot4;
extern  statetype s_ssshoot5;
extern  statetype s_ssshoot6;
extern  statetype s_ssshoot7;
extern  statetype s_ssshoot8;
extern  statetype s_ssshoot9;

extern  statetype s_sschase1;
extern  statetype s_sschase1s;
extern  statetype s_sschase2;
extern  statetype s_sschase3;
extern  statetype s_sschase3s;
extern  statetype s_sschase4;

extern  statetype s_ssdie1;
extern  statetype s_ssdie2;
extern  statetype s_ssdie3;
extern  statetype s_ssdie4;

/* dogs */
extern  statetype s_dogpath1;
extern  statetype s_dogpath1s;
extern  statetype s_dogpath2;
extern  statetype s_dogpath3;
extern  statetype s_dogpath3s;
extern  statetype s_dogpath4;

extern  statetype s_dogjump1;
extern  statetype s_dogjump2;
extern  statetype s_dogjump3;
extern  statetype s_dogjump4;
extern  statetype s_dogjump5;

extern  statetype s_dogchase1;
extern  statetype s_dogchase1s;
extern  statetype s_dogchase2;
extern  statetype s_dogchase3;
extern  statetype s_dogchase3s;
extern  statetype s_dogchase4;

extern  statetype s_dogdie1;
extern  statetype s_dogdie2;
extern  statetype s_dogdie3;
extern  statetype s_dogdead;

/* hans */
extern  statetype s_bossstand;

extern  statetype s_bosschase1;
extern  statetype s_bosschase1s;
extern  statetype s_bosschase2;
extern  statetype s_bosschase3;
extern  statetype s_bosschase3s;
extern  statetype s_bosschase4;

extern  statetype s_bossdie1;
extern  statetype s_bossdie2;
extern  statetype s_bossdie3;
extern  statetype s_bossdie4;

extern  statetype s_bossshoot1;
extern  statetype s_bossshoot2;
extern  statetype s_bossshoot3;
extern  statetype s_bossshoot4;
extern  statetype s_bossshoot5;
extern  statetype s_bossshoot6;
extern  statetype s_bossshoot7;
extern  statetype s_bossshoot8;

/* gretel */
extern  statetype s_gretelstand;

extern  statetype s_gretelchase1;
extern  statetype s_gretelchase1s;
extern  statetype s_gretelchase2;
extern  statetype s_gretelchase3;
extern  statetype s_gretelchase3s;
extern  statetype s_gretelchase4;

extern  statetype s_greteldie1;
extern  statetype s_greteldie2;
extern  statetype s_greteldie3;
extern  statetype s_greteldie4;

extern  statetype s_gretelshoot1;
extern  statetype s_gretelshoot2;
extern  statetype s_gretelshoot3;
extern  statetype s_gretelshoot4;
extern  statetype s_gretelshoot5;
extern  statetype s_gretelshoot6;
extern  statetype s_gretelshoot7;
extern  statetype s_gretelshoot8;

/* gift */
extern  statetype s_giftstand;

extern  statetype s_giftchase1;
extern  statetype s_giftchase1s;
extern  statetype s_giftchase2;
extern  statetype s_giftchase3;
extern  statetype s_giftchase3s;
extern  statetype s_giftchase4;

extern  statetype s_giftdie1;
extern  statetype s_giftdie2;
extern  statetype s_giftdie3;
extern  statetype s_giftdie4;
extern  statetype s_giftdie5;
extern  statetype s_giftdie6;

extern  statetype s_giftshoot1;
extern  statetype s_giftshoot2;

/* fat */
extern  statetype s_fatstand;

extern  statetype s_fatchase1;
extern  statetype s_fatchase1s;
extern  statetype s_fatchase2;
extern  statetype s_fatchase3;
extern  statetype s_fatchase3s;
extern  statetype s_fatchase4;

extern  statetype s_fatdie1;
extern  statetype s_fatdie2;
extern  statetype s_fatdie3;
extern  statetype s_fatdie4;
extern  statetype s_fatdie5;
extern  statetype s_fatdie6;

extern  statetype s_fatshoot1;
extern  statetype s_fatshoot2;
extern  statetype s_fatshoot3;
extern  statetype s_fatshoot4;
extern  statetype s_fatshoot5;
extern  statetype s_fatshoot6;

/* schabb */
extern  statetype s_schabbstand;

extern  statetype s_schabbchase1;
extern  statetype s_schabbchase1s;
extern  statetype s_schabbchase2;
extern  statetype s_schabbchase3;
extern  statetype s_schabbchase3s;
extern  statetype s_schabbchase4;

extern  statetype s_schabbdie1;
extern  statetype s_schabbdie2;
extern  statetype s_schabbdie3;
extern  statetype s_schabbdie4;
extern  statetype s_schabbdie5;
extern  statetype s_schabbdie6;

extern  statetype s_schabbshoot1;
extern  statetype s_schabbshoot2;

extern  statetype s_needle1;
extern  statetype s_needle2;
extern  statetype s_needle3;
extern  statetype s_needle4;

/* fake hitler */
extern  statetype s_fakestand;

extern  statetype s_fakechase1;
extern  statetype s_fakechase1s;
extern  statetype s_fakechase2;
extern  statetype s_fakechase3;
extern  statetype s_fakechase3s;
extern  statetype s_fakechase4;

extern  statetype s_fakedie1;
extern  statetype s_fakedie2;
extern  statetype s_fakedie3;
extern  statetype s_fakedie4;
extern  statetype s_fakedie5;
extern  statetype s_fakedie6;

extern  statetype s_fakeshoot1;
extern  statetype s_fakeshoot2;
extern  statetype s_fakeshoot3;
extern  statetype s_fakeshoot4;
extern  statetype s_fakeshoot5;
extern  statetype s_fakeshoot6;
extern  statetype s_fakeshoot7;
extern  statetype s_fakeshoot8;
extern  statetype s_fakeshoot9;

extern  statetype s_fire1;
extern  statetype s_fire2;

/* mech hitler */

extern  statetype s_mechachase1;
extern  statetype s_mechachase1s;
extern  statetype s_mechachase2;
extern  statetype s_mechachase3;
extern  statetype s_mechachase3s;
extern  statetype s_mechachase4;

extern  statetype s_mechadie1;
extern  statetype s_mechadie2;
extern  statetype s_mechadie3;
extern  statetype s_mechadie4;

extern  statetype s_mechashoot1;
extern  statetype s_mechashoot2;
extern  statetype s_mechashoot3;
extern  statetype s_mechashoot4;
extern  statetype s_mechashoot5;
extern  statetype s_mechashoot6;

/* real hitler */

extern  statetype s_hitlerchase1;
extern  statetype s_hitlerchase1s;
extern  statetype s_hitlerchase2;
extern  statetype s_hitlerchase3;
extern  statetype s_hitlerchase3s;
extern  statetype s_hitlerchase4;

extern  statetype s_hitlerdie1;
extern  statetype s_hitlerdie2;
extern  statetype s_hitlerdie3;
extern  statetype s_hitlerdie4;
extern  statetype s_hitlerdie5;
extern  statetype s_hitlerdie6;
extern  statetype s_hitlerdie7;
extern  statetype s_hitlerdie8;
extern  statetype s_hitlerdie9;
extern  statetype s_hitlerdie10;

extern  statetype s_hitlershoot1;
extern  statetype s_hitlershoot2;
extern  statetype s_hitlershoot3;
extern  statetype s_hitlershoot4;
extern  statetype s_hitlershoot5;
extern  statetype s_hitlershoot6;

/* ghost */
extern  statetype s_blinkychase1;
extern  statetype s_blinkychase2;
extern  statetype s_inkychase1;
extern  statetype s_inkychase2;
extern  statetype s_pinkychase1;
extern  statetype s_pinkychase2;
extern  statetype s_clydechase1;
extern  statetype s_clydechase2;

/* rocket smoke */
extern  statetype s_rocket;

extern  statetype s_smoke1;
extern  statetype s_smoke2;
extern  statetype s_smoke3;
extern  statetype s_smoke4;

extern  statetype s_boom2;
extern  statetype s_boom3;

/* BJ victory */
extern  statetype s_bjrun1;
extern  statetype s_bjrun1s;
extern  statetype s_bjrun2;
extern  statetype s_bjrun3;
extern  statetype s_bjrun3s;
extern  statetype s_bjrun4;

extern  statetype s_bjjump1;
extern  statetype s_bjjump2;
extern  statetype s_bjjump3;
extern  statetype s_bjjump4;

#endif