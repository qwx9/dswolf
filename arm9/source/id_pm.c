#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "types.h"
#include "id_ca.h"

/* macro defines */

/* None */

/* external variables */

extern char extension[5];

/* local variables */

/* None */

/* global variables */

U8 PMSoundInfoPagePadded = 0;
S32 ChunksInFile;
S32 PMSpriteStart;
S32 PMSoundStart;

U32 *PMPageData;
size_t PMPageDataSize;
U8 **PMPages;

/* local prototypes */

static void PM_CannotOpen(const char *string);

/*
================================================================
=
= Function: PM_Startup
=
= Description:
=
= Load sprite and digital sound effect data into memory
=
================================================================
*/
void PM_Startup(void)
{
    FILE *file = NULL;
    char fname[30] = "/GAMES/WOLF/VSWAP.";
    U32 dataStart;
    S32 i;
    long fileSize;
    long pageDataSize;
    S32 alignPadding = 0;
    U32 offs;
    U8 *ptr;
    U32 *pageOffsets;
    U16 *pageLengths;
    U32 size;

    strcat(fname,extension);

    file = fopen(fname, "rb");
    if(file == NULL)
    {
        PM_CannotOpen(fname);
    }

    ChunksInFile = 0;
    fread(&ChunksInFile,1,sizeof(U16), file);
    PMSpriteStart = 0;
    fread(&PMSpriteStart,1, sizeof(U16), file);
    PMSoundStart = 0;
    fread(&PMSoundStart,1, sizeof(U16), file);

    /* read in page offsets */
    pageOffsets = (U32 *) malloc((ChunksInFile + 1) * sizeof(U32));
    CheckMallocResult(pageOffsets);
    fread(pageOffsets,ChunksInFile, sizeof(U32), file);

    /* read in page lengths */
    pageLengths = (U16 *) malloc(ChunksInFile * sizeof(U16));
    CheckMallocResult(pageLengths);
    fread(pageLengths,ChunksInFile,sizeof(U16), file);

    fseek(file, 0, SEEK_END);
    fileSize = ftell(file);
    pageDataSize = fileSize - pageOffsets[0];

    if(pageDataSize > (size_t) -1)
    {
        printf("The page file \"%s\" is too large!", fname);
        while(1){};     /* hang system */
    }

    pageOffsets[ChunksInFile] = fileSize;

    dataStart = pageOffsets[0];

    /* Check that all pageOffsets are valid */
    for(i = 0; i < ChunksInFile; i++)
    {
        if(pageOffsets[i] == 0)
        {
            continue;   /* sparse page */
        }

        if((pageOffsets[i] < dataStart) || (pageOffsets[i] >= (size_t) fileSize))
        {
            printf("Illegal page offset for page %i: %u", i, pageOffsets[i]);
            while(1){};     /* hang system */
        }
    }

    /* Calculate total amount of padding needed for sprites and sound info page */
    for(i = PMSpriteStart; i < PMSoundStart; i++)
    {
        if(pageOffsets[i] == 0)
        {
            continue;   /* sparse page */
        }

        offs = pageOffsets[i] - dataStart + alignPadding;

        if((offs & 1) == 1)
        {
            alignPadding++;
        }
    }

    if((pageOffsets[ChunksInFile - 1] - dataStart + alignPadding) & 1)
    {
        alignPadding++;
    }

    PMPageDataSize = (size_t) pageDataSize + alignPadding;
    PMPageData = (U32 *) malloc(PMPageDataSize);
    CheckMallocResult(PMPageData);

    PMPages = (U8 **) malloc((ChunksInFile + 1) * sizeof(U8 *));
    CheckMallocResult(PMPages);

    /* Load pages and initialize PMPages pointers */
    ptr = (U8 *) PMPageData;

    for(i = 0; i < ChunksInFile; i++)
    {
        if(((i >= PMSpriteStart) && (i < PMSoundStart)) || (i == (ChunksInFile - 1)))
        {
            size_t offs = ptr - (U8 *) PMPageData;

            /* pad with zeros to make it 2-byte aligned */
            if((offs & 1) == 1)
            {
                *ptr++ = 0;
                if(i == (ChunksInFile - 1))
                {
                    PMSoundInfoPagePadded = 1;
                }
            }
        }

        PMPages[i] = ptr;

        if(pageOffsets[i] == 0)
        {
            continue;               /* sparse page */
        }

        /* Use specified page length, when next page is sparse page. */
        /* Otherwise, calculate size from the offset difference between this and the next page. */
        if(pageOffsets[i + 1] == 0)
        {
            size = pageLengths[i];
        }
        else
        {
            size = pageOffsets[i + 1] - pageOffsets[i];
        }

        fseek(file, pageOffsets[i], SEEK_SET);
        fread(ptr, size, 1, file);
        ptr += size;
    }

    /* last page points after page buffer */
    PMPages[ChunksInFile] = ptr;

    free(pageLengths);
    free(pageOffsets);
    fclose(file);
}

/*
================================================================
=
= Function: PM_CannotOpen
=
= Description:
=
= tells the user which file cant be opened and the hangs the
= system (giving the user time to read the message).
=
================================================================
*/

static void PM_CannotOpen(const char *string)
{
    char str[40];

    strcpy(str,"Can't open ");
    strcat(str,string);
    strcat(str,"!\n");

    printf("%s",str);
    while(1){} /* hang system */
}

/*
================================================================
=
= Function: PM_Shutdown
=
= Description:
=
= Free memory used for PM data
=
================================================================
*/
void PM_Shutdown(void)
{
    free(PMPages);
    free(PMPageData);
}
