#ifndef ID_PM_H
#define ID_PM_H

extern void PM_Shutdown(void);
extern void PM_Startup(void);

extern S32 ChunksInFile;
extern S32 PMSpriteStart;
extern S32 PMSoundStart;

/* ChunksInFile+1 pointers to page starts.               */
/* The last pointer points one byte after the last page. */
extern U8 **PMPages;

static inline U8 *PM_GetPage(S32 page)
{
    if((page < 0) || (page >= ChunksInFile))
    {
        printf("PM_GetPage: Tried to access illegal page: %i", page);
        while(1){} /* hang system */
    }

    return PMPages[page];
}

static inline U8 *PM_GetEnd()
{
    return PMPages[ChunksInFile];
}

static inline U8 *PM_GetTexture(S32 wallpic)
{
    return PM_GetPage(wallpic);
}

static inline U16 *PM_GetSprite(S32 shapenum)
{
        /* correct alignment is enforced by PM_Startup() */
        return (U16 *) (void *) PM_GetPage(PMSpriteStart + shapenum);
}

static inline U32 PM_GetPageSize(S32 page)
{
    if((page < 0) || (page >= ChunksInFile))
    {
        printf("PM_GetPageSize: Tried to access illegal page: %i", page);
        while(1){} /* hang system */
    }

    return (U32) (PMPages[page + 1] - PMPages[page]);
}

static inline U8 *PM_GetSound(S32 soundpagenum)
{
    return PM_GetPage(PMSoundStart + soundpagenum);
}

#endif
