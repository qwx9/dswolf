#include <nds.h>
#include <stdio.h>
#include "types.h"
#include "audiowl6.h"
#include "id_sd.h"
#include "id_time.h"
#include "id_ca.h"
#include "id_pm.h"

/* macro defines */

#define OPL_REG_WAVEFORM_ENABLE   0x01
#define OPL_REG_TIMER1            0x02
#define OPL_REG_TIMER_CTRL        0x04
#define OPL_REG_FM_MODE           0x08
#define OPL_REGS_TREMOLO          0x20
#define OPL_REGS_LEVEL            0x40
#define OPL_REGS_ATTACK           0x60
#define OPL_REGS_SUSTAIN          0x80
#define OPL_REGS_FREQ_1           0xA0
#define OPL_REGS_FREQ_2           0xB0
#define OPL_REG_EFFECTS           0xBD
#define OPL_REGS_FEEDBACK         0xC0
#define OPL_REGS_WAVEFORM         0xE0

#define SQ_MAX_TRACKS             8
#define ORIGSAMPLERATE            7042

/* external variables */

extern U8 * audiosegs[NUMSNDCHUNKS];
extern S32 PMSoundStart;
extern U8 PMSoundInfoPagePadded;

/* local variables */

static U8 sqActive = 0;
static U16 *sqHack,*sqHackPtr,sqHackLen,sqHackSeqLen;
static U32 sqHackTime;
static U32 alTimeCount;
static U8 **SoundTable = NULL;
static U16 SoundPriority;
static soundnames SoundNumber;
static U8 * alSound;
static Instrument alZeroInst;
static U32 alLengthLeft;
static U32 alBlock;
static SDMode  SoundMode = sdm_Off;
static SMMode  MusicMode = smm_Off;
static SDSMode DigiMode = sds_Off;
static digiinfo *DigiList = NULL;
static U8 *SoundBuffers[STARTMUSIC - STARTDIGISOUNDS];
static U16 NumDigi;
static S32 NDS_ChannelHandle = -1;

/* global variables */

S16 DigiMap[LASTSOUND];

/* local prototypes */

static void alOut(U32 reg, U32 value);
static U8 SDL_DetectAdLib(void);
static void SD_MusicOn(void);
static void SDL_SetupDigi(void);
static void SDL_StartDevice(void);
static void SDL_StartAL(void);
static void SDL_AlSetFXInst(Instrument *inst);
static void SDL_ALPlaySound(AdLibSound *sound);
static void SDL_ALStopSound(void);
static void SDL_ALService(void);
static void SDL_ALSoundService(void);
static void SDL_SoundFinished(void);
static void SDL_ShutDevice(void);
static void SDL_ShutAL(void);
static S32 SD_PlayDigitized(U16 which);
void SDL_t2Service(void);

/*
================================================================
=
= Function: SDL_t2Service
=
= Description:
=
= every 700hz this function is called from a timer interrupt
= it calls two function one to update music if needed and
= the other to update sound effects if needed.
=
================================================================
*/
void SDL_t2Service(void)
{
    static  U16 count = 1;

    /* update music if required */
    SDL_ALService();

    if((count % 5) == 0)
    {
        /* update sound effect if required */
        SDL_ALSoundService();
    }

    count++;
}

/*
================================================================
=
= Function: SDL_ALService
=
= Description:
=
= every 700hz this function is called from a timer interrupt
= when called it will check to see if the adlib music emulation
= needs updating.
=
================================================================
*/

static void SDL_ALService(void)
{
    U8  a,v;
    U16 w;

    if(sqActive == 0)
    {
        return;
    }

    while (sqHackLen && (sqHackTime <= alTimeCount))
    {
        w = *sqHackPtr++;
        sqHackTime = alTimeCount + *sqHackPtr++;
        a = (U8)(w & 0x00ff);
        v = (u8)((w & 0xff00) >> 8);
        alOut(a,v);
        sqHackLen -= 4;
    }

    alTimeCount++;

    if (!sqHackLen)
    {
        sqHackPtr = (U16 *)sqHack;
        sqHackLen = sqHackSeqLen;
        alTimeCount = sqHackTime = 0;
    }
}

/*
================================================================
=
= Function: SDL_ALSoundService
=
= Description:
=
= every 700hz this function is called from a timer interrupt
= when called it will check to see if the adlib sound effect
= emulation needs updating.
=
================================================================
*/
static void SDL_ALSoundService(void)
{
    U8    s;

    if(alSound != NULL)
    {
        s = *alSound++;

        if(s == 0)
        {
            alOut(OPL_REGS_FREQ_2,0);
        }
        else
        {
            alOut(OPL_REGS_FREQ_1,s);
            alOut(OPL_REGS_FREQ_2,alBlock);
        }

        if((--alLengthLeft) == 0)
        {
            alSound = NULL;
            alOut(OPL_REGS_FREQ_2,0);
            SoundNumber = 0;
            SoundPriority = 0;
        }
    }
}

/*
================================================================
=
= Function: SD_Startup
=
= Description:
=
= Detects all additional sound hardware and installs my ISR
=
================================================================
*/
void SD_Startup(void)
{

    /* reset adlib emulated chip */
    SDL_DetectAdLib();

    /* calls the SDL_ALService function 700 times per second. */
    timerStart(2, ClockDivider_1024, TIMER_FREQ_1024(700), SDL_t2Service);

    alTimeCount = 0;

    /* init sound effects variables */
    SD_SetSoundMode(sdm_AdLib);

    /* set music device */
    SD_SetMusicMode(smm_AdLib);

    /* set digital sound device */
    SD_SetDigiDevice(sds_SoundBlaster);

    /* init digital sound variables */
    SDL_SetupDigi();
}

/*
================================================================
=
= Function: SDL_DetectAdLib
=
= Description:
=
= Determines if there's an AdLib (or SoundBlaster
= emulating an AdLib) present
=
================================================================
*/
static U8 SDL_DetectAdLib(void)
{
    U8    status1,status2;
    S32    i;

    alOut(OPL_REG_TIMER_CTRL,0x60); /* Reset T1 & T2 */
    alOut(OPL_REG_TIMER_CTRL,0x80); /* Reset IRQ */
    status1 = 0x00;                 /* No status register hardcode correct response */
    alOut(OPL_REG_TIMER1,0xff);     /* Set timer 1 */
    alOut(OPL_REG_TIMER_CTRL,0x21); /* Start timer 1 */

    Delay_ms(10);                   /* wait for timer1 to overflow */

    status2 = 0xC0;                 /* No status register hardcode correct response */
    alOut(OPL_REG_TIMER_CTRL,0x60); /* Reset T1 & T2 */
    alOut(OPL_REG_TIMER_CTRL,0x80); /* Reset IRQ */

    /* test to see if adlib card is present */
    if (((status1 & 0xe0) == 0x00) && ((status2 & 0xe0) == 0xc0))
    {
        for (i = 1;i <= 0xf5;i++)   /* Zero all the registers (reset chip) */
        {
            alOut(i,0);
        }

        alOut(OPL_REG_WAVEFORM_ENABLE,0x20);    /* Set WSE=1 */
        alOut(OPL_REG_FM_MODE,0);               /* Set CSM=0 & SEL=0 (select FM music mode) */

        return(1);
    }
    else
    {
        return(0);
    }
}

/*
================================================================
=
= Function: SD_StartMusic
=
= Description:
=
= Start playing requested audio chunk
=
================================================================
*/

void SD_StartMusic(S32 chunk)
{
    SD_MusicOff();

    if (MusicMode == smm_AdLib)
    {
        S32 chunkLen = CA_CacheAudioChunk(chunk);

        sqHack = (U16 *)(void *) audiosegs[chunk];     /* alignment is correct */

        if(*sqHack == 0)
        {
            sqHackLen = sqHackSeqLen = chunkLen;
        }
        else
        {
            sqHackLen = sqHackSeqLen = *sqHack++;
            sqHackPtr = sqHack;
            sqHackTime = 0;
            alTimeCount = 0;
            SD_MusicOn();
        }
    }
}

/*
================================================================
=
= Function: SD_MusicOn
=
= Description:
=
= Sets a sqActive to true which allows the adlib interrupt handler
= to start processing adlib sound.
=
================================================================
*/

static void SD_MusicOn(void)
{
    sqActive = 1;
}

/*
================================================================
=
= Function: SD_MusicOff
=
= Description:
=
= Clear active adlib sound channels (ie switch them off)
=
================================================================
*/

S16 SD_MusicOff(void)
{
    U16 i;

    sqActive = 0;

    if(MusicMode == smm_AdLib)
    {
        alOut(OPL_REG_EFFECTS,0);

        for (i = 0;i <= SQ_MAX_TRACKS;i++)
        {
            alOut((OPL_REGS_FREQ_2 + i),0);
        }
    }

    /* return current position in song */
    return (S16) (sqHackPtr-sqHack);
}

/*
================================================================
=
= Function: SD_Shutdown
=
= Description:
=
= Shut down all sound generation on the adlib
=
================================================================
*/
void SD_Shutdown(void)
{
    SD_MusicOff();
}

/*
================================================================
=
= Function: SD_PlaySound
=
= Description:
=
= plays the specified sound effect
=
================================================================
*/
U8 SD_PlaySound(soundnames sound)
{
    SoundCommon *s;

    /* if sound effects are off do nothing */
    if((DigiMode == sds_Off) && (SoundMode == sdm_Off))
    {
        return 0;
    }

    s = (SoundCommon *) SoundTable[sound];

    if(s == NULL)
    {
        printf("SD_PlaySound() - Uncached sound \n");
        while(1){}; /* hang system */
    }

    if((DigiMode != sds_Off) && (DigiMap[sound] != -1))
    {
        /* play digital sound */
        SD_PlayDigitized(DigiMap[sound]);
        return(0);
    }

    /* only play fx if sound is on ! */
    if (SoundMode == sdm_Off)
    {
        return 0;
    }

    if(s->length == 0)
    {
        printf("SD_PlaySound() - Zero length sound \n");
        while(1){}; /* hang system */
    }

    if(s->priority < SoundPriority)
    {
        return 0;
    }

    SDL_ALPlaySound((AdLibSound *)s);

    SoundNumber = sound;
    SoundPriority = s->priority;

    return 0;
}

/*
================================================================
=
= Function: SD_PlayDigitized
=
= Description:
=
= Play digital sound
=
================================================================
*/
static S32 SD_PlayDigitized(U16 which)
{
    if(DigiMode == sds_Off)
    {
        return 0;
    }

    if(which >= NumDigi)
    {
        printf("SD_PlayDigitized: bad sound number %i", which);
        while(1){}; /* hang system */
    }

    NDS_ChannelHandle = soundPlaySample(SoundBuffers[which], SoundFormat_8Bit, DigiList[which].length , ORIGSAMPLERATE, 127, 64, false, 0);

    return 0;
}

/*
================================================================
=
= Function: PlaySoundLocGlobal
=
= Description:
=
= Currently only plays sound should really set pan before,
= playing sound to give stero effect.
=
================================================================
*/
void PlaySoundLocGlobal(U16 s,fixed gx,fixed gy)
{
    //SetSoundLoc(gx, gy);
    //SD_PositionSound(leftchannel, rightchannel);

    SD_PlaySound((soundnames) s);
}

/*
================================================================
=
= Function: SD_StopDigitized
=
= Description:
=
= stops the current digital sound playing if it hasnt finished
=
================================================================
*/
void SD_StopDigitized (void)
{
    if((DigiMode == sds_Off) || (NDS_ChannelHandle == -1))
    {
        return;
    }

    soundKill(NDS_ChannelHandle);

    NDS_ChannelHandle = -1;
}

/*
================================================================
=
= Function: SDL_SetupDigi
=
= Description:
=
= Called at start up to initalize digital sound variables
=
================================================================
*/
static void SDL_SetupDigi(void)
{
    U32 i;
    S32 lastPage;
    U32 size = 0;
    S32 page;

    /* Correct padding enforced by PM_Startup() */
    U16 *soundInfoPage = (U16 *) (void *) PM_GetPage(ChunksInFile-1);
    NumDigi = (U16) PM_GetPageSize(ChunksInFile - 1) / 4;

    DigiList = (digiinfo *) malloc(NumDigi * sizeof(digiinfo));

    for(i = 0; i < NumDigi; i++)
    {
        /* Calculate the size of the digi from the sizes of the pages */
        /* between the start page and the start page of the next sound */
        DigiList[i].startpage = soundInfoPage[i * 2];

        /* test to see if startpage is within a valid range */
        if((S32) DigiList[i].startpage >= ChunksInFile - 1)
        {
            /* no more valid digi sounds after this point */
            /* so set the number of valid sounds to i */
            NumDigi = i;
            break;
        }

        /* workout last page of this sound */
        if(i < NumDigi - 1)
        {
            lastPage = soundInfoPage[i * 2 + 2];
            if((lastPage == 0) || (lastPage + PMSoundStart > ChunksInFile - 1))
            {
                lastPage = ChunksInFile - 1;
            }
            else
            {
                lastPage += PMSoundStart;
            }
        }
        else
        {
            lastPage = ChunksInFile - 1;
        }

        /* work out the size of sound (size from startpage to lastpage) */
        for(page = PMSoundStart + DigiList[i].startpage; page < lastPage; page++)
        {
            size += PM_GetPageSize(page);
        }

        /* Don't include padding of sound info page, if padding was added */
        if((lastPage == (ChunksInFile - 1)) && (PMSoundInfoPagePadded == 1))
        {
            size--;
        }

        if(((size & 0xffff0000) != 0) && ((size & 0xffff) < soundInfoPage[i * 2 + 1]))
        {
            size -= 0x10000;
        }

        size = (size & 0xffff0000) | soundInfoPage[i * 2 + 1];

        /* finally store sound length */
        DigiList[i].length = size;

    }

    for(i = 0; i < LASTSOUND; i++)
    {
        DigiMap[i] = -1;
    }
}

/*
================================================================
=
= Function: SD_PrepareSound
=
= Description:
=
= Called at start up to store sound data into a buffer so it
= can be accessed quickly
=
================================================================
*/
void SD_PrepareSound(S32 which)
{
    U32 i;
    S8  temp;

    if(DigiList == NULL)
    {
        printf("SD_PrepareSound(%i): DigiList not initialized!\n", which);
        while(1){ /* hang system */ };
    }

    S32 page = DigiList[which].startpage;
    S32 size = DigiList[which].length;

    U8 *origsamples = PM_GetSound(page);
    if(origsamples + size >= PM_GetEnd())
    {
        printf("SD_PrepareSound(%i): Sound reaches out of page file!\n", which);
        while(1){ /* hang system */ };
    }

    /* create buffer to store sound data */
    U8 *wavebuffer = (U8 *) malloc(size);

    if(wavebuffer == NULL)
    {
        printf("Unable to allocate wave buffer for sound %i!\n", which);
        while(1){ /* hang system */ };
    }

    /* convert from unsigned PCM to signed PCM */
    for(i = 0; i < size ; i++)
    {
        temp = (S8)origsamples[i];   /* convert to signed value ie 0x80 = -128 */
        wavebuffer[i] = temp + 128;  /* center around zero ie -128 + 128 = 0 */
    }

    /* store pointer to this sound data*/
    SoundBuffers[which] = wavebuffer;
}

/*
================================================================
=
= Function: SD_SetSoundMode
=
= Description:
=
= Sets sound effects variables
=
================================================================
*/
U8 SD_SetSoundMode(SDMode mode)
{

    SD_StopSound();

    SoundTable = &audiosegs[STARTADLIBSOUNDS];

    if(mode != SoundMode)
    {
        SDL_ShutDevice();
        SoundMode = mode;
        SDL_StartDevice();
    }

    return(1);
}

/*
================================================================
=
= Function: SD_StopSound
=
= Description:
=
= if a sound is playing, stops it
=
================================================================
*/
void SD_StopSound(void)
{
    if(SoundMode == sdm_AdLib)
    {
        SDL_ALStopSound();
    }

    SDL_SoundFinished();
}

/*
================================================================
=
= Function: SDL_SoundFinished
=
= Description:
=
= reset sound effect variables
=
================================================================
*/
static void SDL_SoundFinished(void)
{
    SoundNumber   = (soundnames)0;
    SoundPriority = 0;
}

/*
================================================================
=
= Function: SDL_ShutDevice
=
= Description:
=
= turns off whatever device was being used for sound fx
=
================================================================
*/
static void SDL_ShutDevice(void)
{
    if(SoundMode == sdm_AdLib)
    {
        SDL_ShutAL();
    }

    SoundMode = sdm_Off;
}

/*
================================================================
=
= Function: SDL_ShutAL
=
= Description:
=
= Shuts down the AdLib card for sound effects
=
================================================================
*/
static void SDL_ShutAL(void)
{
    alSound = 0;
    alOut(OPL_REG_EFFECTS,0);
    alOut(OPL_REGS_FREQ_2,0);
    /*clear struct before use */
    alZeroInst.mChar = 0;
    alZeroInst.cChar = 0;
    alZeroInst.mScale = 0;
    alZeroInst.cScale = 0;
    alZeroInst.mAttack = 0;
    alZeroInst.cAttack = 0;
    alZeroInst.mSus = 0;
    alZeroInst.cSus = 0;
    alZeroInst.mWave = 0;
    alZeroInst.cWave = 0;
    SDL_AlSetFXInst(&alZeroInst);
}

/*
================================================================
=
= Function: SD_GetSoundMode
=
= Description:
=
= returns sound effects mode
=
================================================================
*/
SDMode SD_GetSoundMode(void)
{
    return(SoundMode);
}

/*
================================================================
=
= Function: SD_SetMusicMode
=
= Description:
=
= sets the device to use for background music
=
================================================================
*/
U8 SD_SetMusicMode(SMMode mode)
{
    if(MusicMode == smm_AdLib)
    {
        SD_MusicOff();
    }

    while(SD_MusicPlaying() != 0)
    {
        Delay_ms(5);
    }

    MusicMode = mode;

    return(1);
}

/*
================================================================
=
= Function: SD_MusicPlaying
=
= Description:
=
= returns true if music is currently playing, false if not
=
================================================================
*/
U8 SD_MusicPlaying(void)
{
    U8 result;

    switch(MusicMode)
    {
        case smm_AdLib:
            result = sqActive;
            break;
        default:
            result = 0;
            break;
    }

    return(result);
}

/*
================================================================
=
= Function: SD_GetMusicMode
=
= Description:
=
= returns device use for music
=
================================================================
*/
SMMode SD_GetMusicMode(void)
{
    return(MusicMode);
}

/*
================================================================
=
= Function: SD_SetDigiDevice
=
= Description:
=
= sets the device to use digital effects
=
================================================================
*/
void SD_SetDigiDevice(SDSMode mode)
{
    if(mode == DigiMode)
    {
        return;
    }

    DigiMode = mode;
}

/*
================================================================
=
= Function: SD_GetDigiDevice
=
= Description:
=
= returns the device used digital effects
=
================================================================
*/
SDSMode SD_GetDigiDevice(void)
{
    return(DigiMode);
}

/*
================================================================
=
= Function: SD_WaitSoundDone
=
= Description:
=
= waits until the current sound is done playing
=
================================================================
*/
void SD_WaitSoundDone(void)
{
    while(SD_SoundPlaying() != 0)
    {
        Delay_ms(5);
    }
}

/*
================================================================
=
= Function: SD_SoundPlaying
=
= Description:
=
= returns the sound number that's playing, or 0 if no sound
= is playing
=
================================================================
*/
U16 SD_SoundPlaying(void)
{
    U8 result = 0;

    if(SoundMode == sdm_AdLib)
    {
        result = alSound? 1 : 0;
    }

    if(result == 1)
    {
        return(SoundNumber);
    }
    else
    {
        return(0);
    }
}

/*
================================================================
=
= Function: SDL_StartDevice
=
= Description:
=
= turns on sound fx
=
================================================================
*/
static void SDL_StartDevice(void)
{
    if(SoundMode == sdm_AdLib)
    {
        SDL_StartAL();
    }

    SoundNumber = (soundnames) 0;
    SoundPriority = 0;
}

/*
================================================================
=
= Function: SDL_StartAL
=
= Description:
=
= Starts up the AdLib card for sound effects
=
================================================================
*/
static void SDL_StartAL(void)
{
    alOut(OPL_REG_EFFECTS,0);

    SDL_AlSetFXInst(&alZeroInst);
}

/*
================================================================
=
= Function: SDL_AlSetFXInst
=
= Description:
=
= Set sound effect
=
================================================================
*/
static void SDL_AlSetFXInst(Instrument *inst)
{
    U8 c,m;

    m = 0;      /* modulator cell for channel 0 */
    c = 3;      /* carrier cell for channel 0 */
    alOut((m + OPL_REGS_TREMOLO),inst->mChar);
    alOut((m + OPL_REGS_LEVEL),inst->mScale);
    alOut((m + OPL_REGS_ATTACK),inst->mAttack);
    alOut((m + OPL_REGS_SUSTAIN),inst->mSus);
    alOut((m + OPL_REGS_WAVEFORM),inst->mWave);
    alOut((c + OPL_REGS_TREMOLO),inst->cChar);
    alOut((c + OPL_REGS_LEVEL),inst->cScale);
    alOut((c + OPL_REGS_ATTACK),inst->cAttack);
    alOut((c + OPL_REGS_SUSTAIN),inst->cSus);
    alOut((c + OPL_REGS_WAVEFORM),inst->cWave);

    alOut(OPL_REGS_FEEDBACK,0);
}

/*
================================================================
=
= Function: SDL_ALPlaySound
=
= Description:
=
= Plays the specified sound on the AdLib card
=
================================================================
*/
static void SDL_ALPlaySound(AdLibSound *sound)
{
    Instrument    *inst;
    U8            *data;

    SDL_ALStopSound();

    alLengthLeft = sound->common.length;
    data = sound->data;
    alBlock = ((sound->block & 7) << 2) | 0x20;
    inst = &sound->inst;

    if((inst->mSus | inst->cSus) == 0)
    {
        printf("SDL_ALPlaySound() - Bad instrument \n");
        while(1){}; /* hang system */
    }

    SDL_AlSetFXInst(inst);
    alSound = (U8 *)data;
}

/*
================================================================
=
= Function: SD_ContinueMusic
=
= Description:
=
= Restart music from startoffs within song
=
================================================================
*/
void SD_ContinueMusic(S32 chunk, S32 startoffs)
{
    S32 i;
    S32 chunkLen;
    U8 reg;
    U8 val;

    SD_MusicOff();

    if(MusicMode == smm_AdLib)
    {
        chunkLen = CA_CacheAudioChunk(chunk);
        sqHack = (U16 *)(void *) audiosegs[chunk];     /* alignment is correct */

        if(*sqHack == 0)
        {
            sqHackLen = sqHackSeqLen = chunkLen;
        }
        else
        {
            sqHackLen = sqHackSeqLen = *sqHack++;
        }

        sqHackPtr = sqHack;

        if(startoffs >= sqHackLen)
        {
            printf("SD_StartMusic: Illegal startoffs provided!");
            while(1){ /* hang system */ };
        }

        /* fast forward to correct position        */
        /* (needed to reconstruct the instruments) */

        for(i = 0; i < startoffs; i += 2)
        {
            reg = *(U8 *)sqHackPtr;
            val = *(((U8 *)sqHackPtr) + 1);

            if((reg >= 0xb1) && (reg <= 0xb8))
            {
                val &= 0xdf;           /* disable play note flag */
            }
            else if(reg == 0xbd)
            {
                val &= 0xe0;           /* disable drum flags */
            }

            alOut(reg,val);
            sqHackPtr += 2;
            sqHackLen -= 4;
        }

        sqHackTime = 0;
        alTimeCount = 0;

        SD_MusicOn();
    }
}

/*
================================================================
=
= Function: SDL_ALStopSound
=
= Description:
=
= Turns off any sound effects playing through the AdLib card
=
================================================================
*/
static void SDL_ALStopSound(void)
{
    alSound = NULL;
    alOut(OPL_REGS_FREQ_2, 0);
}

/*
================================================================
=
= Function: alOut
=
= Description:
=
= Write a value to a adlib register
=
================================================================
*/

static void alOut(U32 reg, U32 value)
{
    fifoSendValue32( FIFO_USER_01, ( reg << 8 ) | value );
    Delay_ms(1);
}

