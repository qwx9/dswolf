#ifndef ID_VH_H
#define ID_VH_H

extern void VWB_DrawPic(S32 x, S32 y, S32 chunknum);
extern void VWB_Hlin(S32 x1, S32 x2, S32 y, S32 color);
extern void VWB_Vlin(S32 y1, S32 y2, S32 x, S32 color);
extern void VW_MeasurePropString(const char *string, U16 *width, U16 *height);

#endif