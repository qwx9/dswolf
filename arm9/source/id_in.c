#include <nds.h>
#include <stdio.h>
#include <stdlib.h>
#include "types.h"
#include "id_in.h"
#include "id_time.h"

/* macro defines */

/* None */

/* external variables */

extern U8 ExitMenu;

/* local variables */

/* None */

/* global variables */

/* None */

/* local prototypes */

/* None */

/*
================================================================
=
= Function: IN_ReadControl
=
= Description:
=
= Reads the device associated with the specified
= player and fills in the control info struct
=
================================================================
*/
void IN_ReadControl(ControlInfo *info)
{
    U32 keys_down;
    U32 keys_held;

    /* Do DS input housekeeping */
    scanKeys();

    /* read all keys that are pressed down */
    keys_down = keysDown();
    keys_held = keysHeld();

    if(((keys_down & KEY_UP) == KEY_UP) || ((keys_held & KEY_UP) == KEY_UP))
    {
        info->dir = dir_North;
    }

    if(((keys_down & KEY_DOWN) == KEY_DOWN) || ((keys_held & KEY_DOWN) == KEY_DOWN))
    {
        info->dir = dir_South;
    }

    if(((keys_down & KEY_RIGHT) == KEY_RIGHT) || ((keys_held & KEY_RIGHT) == KEY_RIGHT))
    {
        info->dir = dir_East;
    }

    if(((keys_down & KEY_LEFT) == KEY_LEFT) || ((keys_held & KEY_LEFT) == KEY_LEFT))
    {
       info->dir = dir_West;
    }

    /* (quick hack menu code should handle is) in menu dont register held down keys */
    if(ExitMenu == 0)
    {

        if((keys_down & KEY_A) == KEY_A)
        {
            info->button0 = 1;
        }

        if((keys_down & KEY_B) == KEY_B)
        {
            info->button1 = 1;
        }

        if((keys_down & KEY_X) == KEY_X)
        {
            info->button2 = 1;
        }

        if((keys_down & KEY_Y) == KEY_Y)
        {
            info->button3 = 1;
        }

        if((keys_down & KEY_R) == KEY_R)
        {
            info->straferight = 1;
        }

        if((keys_down & KEY_L) == KEY_L)
        {
            info->strafeleft = 1;
        }

    }
    else /* in game allow held down keys to be used */
    {

        if(((keys_down & KEY_A) == KEY_A) || ((keys_held & KEY_A) == KEY_A))
        {
            info->button0 = 1;
        }

        if(((keys_down & KEY_B) == KEY_B) || ((keys_held & KEY_B) == KEY_B))
        {
            info->button1 = 1;
        }

        if(((keys_down & KEY_X) == KEY_X) || ((keys_held & KEY_X) == KEY_X))
        {
            info->button2 = 1;
        }

        if(((keys_down & KEY_Y) == KEY_Y) || ((keys_held & KEY_Y) == KEY_Y))
        {
            info->button3 = 1;
        }

        if(((keys_down & KEY_R) == KEY_R) || ((keys_held & KEY_R) == KEY_R))
        {
            info->straferight = 1;
        }

        if(((keys_down & KEY_L) == KEY_L) || ((keys_held & KEY_L) == KEY_L))
        {
            info->strafeleft = 1;
        }
    }

    if((keys_down & KEY_START) == KEY_START)
    {
        info->pause = 1;
    }

    if((keys_down & KEY_SELECT) == KEY_SELECT)
    {
        info->esc = 1;
    }
}

/*
================================================================
=
= Function: IN_Ack
=
= Description:
=
= wait here until button is pressed
=
================================================================
*/
void IN_Ack(void)
{
    ControlInfo ci;

    /* clear button struct before entering loop */
    ci.button0 = 0;
    ci.button1 = 0;
    ci.button2 = 0;
    ci.button3 = 0;
    ci.dir = dir_None;
    ci.straferight = 0;
    ci.strafeleft = 0;
    ci.pause = 0;
    ci.esc = 0;

    /* wait here until button is pressed */
    do
    {
        Delay_ms(10);
        IN_ReadControl(&ci);
    }while((ci.dir == dir_None) && (ci.button0 == 0) &&  (ci.button1 == 0) && (ci.button2 == 0) && (ci.button3 == 0)
                && (ci.straferight == 0) &&  (ci.strafeleft == 0) && (ci.pause == 0) && (ci.esc == 0));

}

/*
================================================================
=
= Function: IN_UserInput
=
= Description:
=
= Waits for the specified delay time (in ticks) or the
= user pressing a key or a mouse button. If the clear flag is set, it
= then either clears the key or waits for the user to let the mouse
= button up.
=
================================================================
*/
U8 IN_UserInput(U32 delay)
{
    U32 lasttime;

    lasttime = GetTimeCount();

    do
    {
        if(IN_CheckAck() == 1)
        {
            return 1;
        }

        Delay_ms(10);

    }while((GetTimeCount() - lasttime) < delay);

    return 0;
}

/*
================================================================
=
= Function: IN_CheckAck
=
= Description:
=
= see if something has been pressed
=
================================================================
*/
U8 IN_CheckAck(void)
{
    ControlInfo ci;

    /* clear button struct before calling read function */
    ci.button0 = 0;
    ci.button1 = 0;
    ci.button2 = 0;
    ci.button3 = 0;
    ci.dir = dir_None;
    ci.straferight = 0;
    ci.strafeleft = 0;
    ci.pause = 0;
    ci.esc = 0;

    IN_ReadControl(&ci);

    if((ci.dir == dir_None) && (ci.button0 == 0) &&  (ci.button1 == 0) && (ci.button2 == 0) && (ci.button3 == 0)
            && (ci.straferight == 0) &&  (ci.strafeleft == 0) && (ci.pause == 0) && (ci.esc == 0))
    {
        return 0;
    }
    else
    {
        return 1; /* key pressed */
    }
}
