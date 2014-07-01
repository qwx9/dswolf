/*---------------------------------------------------------------------------------

    default ARM7 core

    Copyright (C) 2005 - 2010
    Michael Noland (joat)
    Jason Rogers (dovoto)
    Dave Murphy (WinterMute)

    This software is provided 'as-is', without any express or implied
    warranty.  In no event will the authors be held liable for any
    damages arising from the use of this software.

    Permission is granted to anyone to use this software for any
    purpose, including commercial applications, and to alter it and
    redistribute it freely, subject to the following restrictions:

    1.  The origin of this software must not be misrepresented; you
        must not claim that you wrote the original software. If you use
        this software in a product, an acknowledgment in the product
        documentation would be appreciated but is not required.

    2.  Altered source versions must be plainly marked as such, and
        must not be misrepresented as being the original software.

    3.  This notice may not be removed or altered from any source
        distribution.

---------------------------------------------------------------------------------*/
#include <nds.h>
#include <dswifi7.h>
#include <maxmod7.h>

#define FIFO_ADLIB FIFO_USER_01

volatile bool exitflag = false;

extern void PutAdLibBuffer(int);
extern void AdlibEmulator();

/*-------------------------------------------------------------------------------*/
void AdLibHandler(u32 value, void * userdata)
{
    PutAdLibBuffer(value);
}

/*-------------------------------------------------------------------------------*/
void VblankHandler(void)
{
    Wifi_Update();
}

/*-------------------------------------------------------------------------------*/
void VcountHandler()
{
    inputGetAndSend();
}

/*-------------------------------------------------------------------------------*/
void powerButtonCB()
{

    exitflag = true;
}

/*-------------------------------------------------------------------------------*/
int main() {

    readUserSettings();

    irqInit();
    /* Start the RTC tracking IRQ */
    initClockIRQ();
    fifoInit();

    SetYtrigger(80);

    /* installWifiFIFO(); */
    installSoundFIFO();

    installSystemFIFO();

    fifoSetValue32Handler(FIFO_ADLIB, AdLibHandler, 0);

    TIMER0_CR = 0;
    irqSet(IRQ_VCOUNT, VcountHandler);

    irqEnable( IRQ_VBLANK | IRQ_VCOUNT | IRQ_NETWORK);

    REG_SOUNDCNT = SOUND_ENABLE;
    REG_MASTER_VOLUME = 127;

    setPowerButtonCB(powerButtonCB);

    AdlibEmulator();                /* We never return from here */

    /* Keep the ARM7 mostly idle */
    while (!exitflag)
    {
        if( 0 == (REG_KEYINPUT & (KEY_SELECT | KEY_START | KEY_L | KEY_R)))
        {
            exitflag = true;
        }
        swiWaitForVBlank();
    }

    return 0;
}
