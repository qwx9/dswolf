#ifndef ID_CA_H
#define ID_CA_H

extern void SpawnNewObj(U32 tilex, U32 tiley, statetype *state);
extern U8 SightPlayer(objtype *ob);
extern void NewState(objtype *ob, statetype *state);
extern U8 CheckLine(objtype *ob);
extern void SelectDodgeDir(objtype *ob);
extern void SelectChaseDir(objtype *ob);
extern void MoveObj(objtype *ob, S32 move);
extern U8 TryWalk (objtype *ob);
extern void DamageActor (objtype *ob, U32 damage);

#endif