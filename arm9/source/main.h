#ifndef MAIN_H
#define MAIN_H

extern void ShowViewSize(S32 width);
extern void NewViewSize (S32 width);
extern void NewGame (S32 difficulty,S32 episode);
extern U8 SaveTheGame(FILE *file,S32 x,S32 y);
extern U8 LoadTheGame(FILE *file,S32 x,S32 y);

#endif