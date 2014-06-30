#ifndef ID_SD_H
#define ID_SD_H

extern S16  SD_MusicOff(void);
extern void SD_Startup(void);
extern void SD_StartMusic(S32 chunk);
extern void SD_Shutdown(void);
extern U8 SD_PlaySound(soundnames sound);
extern SDMode SD_GetSoundMode(void);
extern SMMode SD_GetMusicMode(void);
extern SDSMode SD_GetDigiDevice(void);
extern U8 SD_SetSoundMode(SDMode mode);
extern void SD_WaitSoundDone(void);
extern U16 SD_SoundPlaying(void);
extern void SD_SetDigiDevice(SDSMode mode);
extern U8 SD_SetMusicMode(SMMode mode);
extern void SD_StopSound(void);
extern U8 SD_MusicPlaying(void);
extern void SD_ContinueMusic(S32 chunk, S32 startoffs);
extern void SD_PrepareSound(S32 which);
extern void SD_StopDigitized (void);
extern void PlaySoundLocGlobal(U16 s,fixed gx,fixed gy);

#endif