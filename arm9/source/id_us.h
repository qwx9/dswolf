#ifndef ID_US_H
#define ID_US_H

extern void US_Print(const char *sorg);
extern void US_CPrint(const char *sorg);
extern void US_InitRndT(S32 randomize);
extern U8 US_RndT(void);
extern U8 US_LineInput(S32 x,S32 y,char *buf, S32 maxchars,S32 maxwidth);

#endif