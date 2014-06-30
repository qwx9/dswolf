#ifndef WL_AGENT_H
#define WL_AGENT_H

extern void DrawScore(void);
extern void DrawWeapon(void);
extern void DrawKeys(void);
extern void DrawAmmo(void);
extern void DrawLevel(void);
extern void DrawLives(void);
extern void DrawHealth(void);
extern void DrawFace(void);
extern void SpawnPlayer(S32 tilex, S32 tiley, S32 dir);
extern void T_Player(objtype *ob);
extern void T_Attack(objtype *ob);
extern void GetBonus(statobj_t *check);
extern void GivePoints(S32 points);
extern void TakeDamage (S32 points,objtype *attacker);
extern void Thrust(S32 angle, S32 speed);

#endif