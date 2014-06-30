#ifndef ID_TIME_H
#define ID_TIME_H

extern void InitTimer(void);
extern void Delay_ms(U32 ms);
extern U32 GetTimeCount(void);
extern U32 GetTimeMS(void);

static inline void Delay(U32 wolfticks)
{
    if(wolfticks > 0)
    {
        Delay_ms(wolfticks * 100 / 7);
    }
}

#endif