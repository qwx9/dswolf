#ifndef WL_MENU_H
#define WL_MENU_H

extern S16 StartCPMusic(S32 song);
extern void IntroScreen(void);
extern void CacheLump(S32 lumpstart, S32 lumpend);
extern void DrawStripes(S32 y);
extern void DrawWindow(S32 x, S32 y, S32 w, S32 h, S32 wcolor);
extern void ClearMScreen(void);
extern void DrawMenu(CP_iteminfo * item_i, CP_itemtype * items);
extern S32 HandleMenu(CP_iteminfo * item_i, CP_itemtype * items, void (*pt2function) (S32 w));
extern void SetFontNum(U8 FontNum);
extern U8 GetFontNum(void);
extern void SetFontColor(U8 f, U8 b);
extern U8 GetFontColor(void);
extern void UnCacheLump(S32 lumpstart, S32 lumpend);
extern void CheckForEpisodes(void);
extern void US_ControlPanel(ScanCode scancode);
extern S32 CP_ViewScores(S32 temp);
extern S32 CP_Sound(S32 temp);
extern void Message(const char *string);
extern S32 CP_ChangeView(S32 temp);
extern S32 CP_ReadThis(S32 temp);
extern void FreeMusic(void);
extern U8 GetBackColour(void);
extern void TicDelay(S32 count);
extern S32 CP_Control(S32 temp);
extern S32 CustomControls(S32 temp);
extern void FixupCustom(S32 w);
extern S32 CP_NewGame(S32 temp);
extern S32 CP_LoadGame(S32 quick);
extern S32 CP_SaveGame (S32 quick);
extern void SetupSaveGames(void);

#endif