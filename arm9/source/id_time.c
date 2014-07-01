#include <nds.h>
#include "types.h"
#include "id_time.h"

/* macro defines */

#define timers2ms(tlow,thigh) ((tlow>>5)+(thigh<<11))
#define TICRATE     70
#define TIMER_TOP   0x08000000

/* external variables */

/* None */

/* local variables */

static U32 lastCount;
static U32 lastTime;

/* global variables */

/* None */

/* local prototypes */

/* None */

/*
================================================================
=
= Function: InitTimer
=
= Description:
=
= Start timer 0 / 1 pair in free running mode
=
================================================================
*/

void InitTimer(void)
{
    /* initialize timer */
    TIMER0_DATA=0;
    TIMER1_DATA=0;
    TIMER0_CR=TIMER_DIV_1024 | TIMER_ENABLE;
    TIMER1_CR=TIMER_CASCADE | TIMER_ENABLE;

    lastCount = 0;
    lastTime = 0;
}

/*
================================================================
=
= Function: Delay_ms
=
= Description:
=
= ms Delay function (blocking) using timer 0 & 1
=
================================================================
*/

void Delay_ms(U32 ms)
{
    U32 now;
    now=timers2ms(TIMER0_DATA, TIMER1_DATA);
    while((u32)timers2ms(TIMER0_DATA, TIMER1_DATA)<now+ms)
    {   /* wait */  };
}

/*
================================================================
=
= Function: GetTimeCount
=
= Description:
=
= returns time in 1/70th second tics
=
================================================================
*/
U32 GetTimeCount(void)
{
    U32 ticks;
    U32 delta;

    ticks = timers2ms(TIMER0_DATA, TIMER1_DATA);

    /* handle roll over */
    if(ticks < lastCount)
    {
        delta = (U32)(TIMER_TOP - lastTime);
        ticks += delta;
    }

    lastCount = ticks;

    return((ticks * TICRATE) / 1000);
}

/*
================================================================
=
= Function: GetTimeMS
=
= Description:
=
= Same as GetTimeCount, but returns time in milliseconds
=
================================================================
*/
U32 GetTimeMS(void)
{
    U32 ticks;
    U32 delta;

    ticks = timers2ms(TIMER0_DATA, TIMER1_DATA);

    /* handle roll over */
    if(ticks < lastTime)
    {
        delta = (U32)(TIMER_TOP - lastTime);
        ticks += delta;
    }

    lastTime = ticks;

    return ticks;
}
