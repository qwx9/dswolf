#ifndef WL_PLAY_H
#define WL_PLAY_H

extern void PlayLoop(void);
extern void StartMusic(void);
extern void ContinueMusic(S32 offs);
extern S32 StopMusic(void);
extern void InitActorList(void);
extern void StartBonusFlash(void);
extern void GetNewActor(void);
extern void StartDamageFlash (S32 damage);

#endif