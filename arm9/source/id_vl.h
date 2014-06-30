#ifndef ID_VL_H
#define ID_VL_H

extern void VL_SetVGAPlaneMode(void);
extern void SignonScreen(void);
extern void UpdateScreen(U8 SetPalette);
extern void VL_Bar(S32 x, S32 y, S32 width, S32 height, S32 color);
extern void VL_Shutdown(void);
extern void VL_FadeOut(S32 start, S32 end, S32 red, S32 green, S32 blue, S32 steps);
extern void VL_FadeIn (S32 start, S32 end, S32 steps);
extern void VL_MemToScreen(U8 *source, U32 width, U32 height, S32 destx, S32 desty);
extern void VL_Vlin (S32 x, S32 y, S32 height, S32 color);
extern void VL_Hlin (U32 x, U32 y, U32 width, S32 color);
extern void VWB_DrawPropString(const char* string);
extern U8 VL_GetPixel(S16 x, S16 y);
extern void VGAClearScreen(void);
extern void ScalePost(void);
extern void SimpleScaleShape(S32 xcenter, S32 shapenum, U32 height);
extern void ScaleShape(S32 xcenter, S32 shapenum, U32 height, U32 flags);
extern void FinishPaletteShifts(void);
extern void FizzleFade(S32 x1, S32 y1, U32 width, U32 height, U32 frames);
extern void VL_Startup(void);
extern void InitRedShifts(void);
extern void SetPalette(pal_type Pal, S32 ShiftAmount, U8 Update);

#endif