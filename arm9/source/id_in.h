#ifndef ID_IN_H
#define ID_IN_H

extern void IN_ReadControl(ControlInfo *info);
extern void IN_Ack(void);
extern U8 IN_UserInput(U32 delay);
extern U8 IN_CheckAck(void);

#endif