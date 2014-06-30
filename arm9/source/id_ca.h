#ifndef ID_CA_H
#define ID_CA_H

extern void CA_UncacheAudioChunk(S32 chunk);
extern S32 CA_CacheAudioChunk(S32 chunk);
extern void CheckMallocResult(void *ptr);
extern void CA_Shutdown(void);
extern void CA_CacheGrChunk(S32 chunk);
extern void CA_LoadAllSounds(void);
extern void CA_UnCacheGrChunk(S32 chunk);
extern void CA_CacheScreen(S32 chunk);
extern void CA_Startup(void);
extern void CA_CacheMap(S32 mapnum);
extern inline U16 READWORD(U8 *ptr);

#endif