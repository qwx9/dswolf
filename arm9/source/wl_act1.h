#ifndef WL_ACT1_H
#define WL_ACT1_H

extern void InitDoorList(void);
extern void InitStaticList(void);
extern void SpawnDoor(S32 tilex, S32 tiley, U8 vertical, S32 lock);
extern void SpawnStatic(S32 tilex, S32 tiley, S32 type);
extern void OperateDoor(U8 door);
extern void MoveDoors(void);
extern void InitAreas(void);
extern void OpenDoor (U8 door);
extern void PlaceItemType(S32 itemtype, S32 tilex, S32 tiley);
void PushWall   (int, int, int);
void MovePWalls (void);

#endif
