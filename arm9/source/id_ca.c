#include <fat.h>
#include <nds.h>
#include <stdio.h>
#include <stdlib.h>
#include "types.h"
#include "wl_def.h"
#include "id_ca.h"
#include "audiowl6.h"
#include "gfxv_apo.h"
#include "id_vl.h"

/* macro defines */

#define BUFFERSIZE 0x1000
#define BLOCK      64
#define MASKBLOCK  128
#define ORIG_SOUNDCOMMON_SIZE 6
#define ORIG_INSTRUMENT_SIZE 16
#define ORIG_ADLIBSOUND_SIZE (ORIG_SOUNDCOMMON_SIZE + ORIG_INSTRUMENT_SIZE + 2)
#define ORIG_MAPTYPE_SIZE 38
#define NUMMAPS         60
#define NEARTAG         0xa7
#define FARTAG          0xa8

/* external variables */

extern pictabletype *pictable;

/* local variables */

static const char aheadname[] = "/GAMES/WOLF/AUDIOHED.";
static const char afilename[] = "/GAMES/WOLF/AUDIOT.";
static const char gdictname[] = "/GAMES/WOLF/VGADICT.";
static const char gheadname[] = "/GAMES/WOLF/VGAHEAD.";
static const char gfilename[] = "/GAMES/WOLF/VGAGRAPH.";
static const char mheadname[] = "/GAMES/WOLF/MAPHEAD.";

static S32 * audiostarts = NULL;           /* array of offsets in audio / audiot */
static FILE * audiohandle = NULL;          /* handle to AUDIOT / AUDIO */
static FILE * grhandle = NULL;             /* handle to VGAGRAPH file */
static FILE * maphandle = NULL;            /* handle to MAPTEMP / GAMEMAPS */

static huffnode grhuffman[255];
static S32 grstarts[NUMCHUNKS + 1];
static S32 chunkcomplen;
static S32 chunkexplen;
static S32 bufferseg[BUFFERSIZE/4];
static U16 RLEWtag;
static maptype* mapheaderseg[NUMMAPS];

/* global variables */

U8 * audiosegs[NUMSNDCHUNKS] = {NULL};
U8 * grsegs[NUMCHUNKS] = {NULL};
U16 * mapsegs[MAPPLANES] = {NULL};
char extension[5];
char graphext[5];
char audioext[5];
S32  numEpisodesMissing = -1;
S32  mapon;

/* local prototypes */

static void CAL_GetGrChunkLength(S32 chunk);
static S32 GrFilePos(const S32 idx);
static void CAL_HuffExpand(U8 *source, U8 *dest, S32 length, huffnode *hufftable);
static void CAL_ExpandGrChunk(S32 chunk, S32 *source);
static U8 CA_LoadFile(const char *filename, memptr *ptr);
static void CA_CacheAdlibSoundChunk(S32 chunk);
static void CA_CannotOpen(const char *string);
static inline U32 READLONGWORD(U8 *ptr);
static void CAL_SetupMapFile(void);
static void CAL_SetupGrFile(void);
static void CAL_SetupAudioFile(void);
static void CA_RLEWexpand(U16 *source, U16 *dest, S32 length, U16 rlewtag);
static void CAL_CarmackExpand(U8 *source, U16 *dest, S32 length);

/* 
================================================================
=
= Function: READWORD
=
= Description:
=
= swap two bytes around
=
================================================================
*/

inline U16 READWORD(U8 *ptr)
{
    U16 val = (ptr[0] | (ptr[1] << 8));
    return val;
}

/* 
================================================================
=
= Function: READLONGWORD
=
= Description:
=
= swap four bytes around
=
================================================================
*/
static inline U32 READLONGWORD(U8 *ptr)
{
    U32 val = (ptr[0] | (ptr[1] << 8) | (ptr[2] << 16) | (ptr[3] << 24));
    return val;
}

/* 
================================================================
=
= Function: CAL_SetupGrFile
=
= Description:
=
= loads huffman dictionary into memory, loads and swaps graphics
= chunks offsets into memory. Uncompresses pictable data and stores
= in memory then opens file handle to graphics data chunks.
=
================================================================
*/

static void CAL_SetupGrFile(void)
{
    S32 headersize;
    char fname[30];
    FILE *handle = NULL;
    U8 *compseg = NULL;
    U8 *data = NULL;
    S32 * i;

    /* load VGAdict.ext (huffman dictionary for graphics files) */

    strcpy(fname,gdictname);
    strcat(fname,graphext);
    
    handle = fopen(fname, "rb");
    if (handle == NULL)
    {
        CA_CannotOpen(fname);
    }

    fread(grhuffman,1,sizeof(grhuffman),handle);
    fclose(handle);
    handle = NULL;

    /* load the data offsets from VGAhead.ext */
    strcpy(fname,gheadname);
    strcat(fname,graphext);
    
    handle = fopen(fname, "rb");
    if (handle == NULL)
    {
        CA_CannotOpen(fname);
    }
    
    fseek(handle, 0, SEEK_END);
    headersize = ftell(handle);
    fseek(handle, 0, SEEK_SET);
    
    /* allocate memory for temp buffer with size of offset data*/
    data = (U8 *) malloc(headersize);
    CheckMallocResult(data);
    
    /* read offset data into temp buffer */
    fread(data,1,headersize,handle);
    fclose(handle);
    handle = NULL;
    
    /*swap around bytes in offset data and store result in grstarts array */
    for (i = grstarts; i != (grstarts + (sizeof(grstarts) / sizeof(S32))) ; ++i)
    {
        const S32 val = (S32)((data[0] | (data[1] << 8) | (data[2] << 16)) & 0x00FFFFFF);
        *i = ((val == 0x00FFFFFF) ? -1 : val);
        data += 3;
    }
    
    /* Open the graphics file, leaving it open until the game is finished */
    strcpy(fname,gfilename);
    strcat(fname,graphext);
    
    grhandle = fopen(fname, "rb");
    if (grhandle == NULL)
    {
        CA_CannotOpen(fname);
    }
    
    /* allocate memory for pictable buffer */
    pictable = (pictabletype *) malloc(NUMPICS*sizeof(pictabletype));
    CheckMallocResult(pictable);
    /* position file pointer to start of compressed pictable data */
    CAL_GetGrChunkLength(STRUCTPIC);       
    /* allocate memory for compressed pictable data */
    compseg = (U8 *) malloc(chunkcomplen);
    CheckMallocResult(compseg);
    /* fill the compseg buffer with compressed data*/
    fread(compseg,1,chunkcomplen,grhandle);
    /* fill the pictable buffer with uncompressed data */
    CAL_HuffExpand(compseg, (U8*)pictable, NUMPICS * sizeof(pictabletype), grhuffman);
    free(compseg);
}

/* 
================================================================
=
= Function: CAL_SetupAudioFile
=
= Description:
=
= loads audio chunk offsets into audiostarts buffer and opens
= file handle to audio data.
=
================================================================
*/

static void CAL_SetupAudioFile(void)
{
    char fname[30];
    void * ptr;

    /* load audiohed.ext (offsets for audio file) */

    strcpy(fname,aheadname);
    strcat(fname,audioext);

   
    if (CA_LoadFile(fname, &ptr) == 0)
    {
        CA_CannotOpen(fname);
    }

    audiostarts = (S32*)ptr;


    /* open the data file */

    strcpy(fname,afilename);
    strcat(fname,audioext);

    audiohandle = fopen(fname, "rb");
    if (audiohandle == NULL)
    {
        CA_CannotOpen(fname);
    }
}

/* 
================================================================
=
= Function: CA_CannotOpen
=
= Description:
=
= tells the user which file cant be opened and the hangs the
= system (giving the user time to read the message).
=
================================================================
*/

static void CA_CannotOpen(const char *string)
{
    char str[40];

    strcpy(str,"Can't open ");
    strcat(str,string);
    strcat(str,"!\n");
    
    printf("%s",str);
    while(1){} /* hang system */
}

/*
==========================
=
= CA_LoadFile
=
= Allocate space for and load a file
=
==========================
*/

static U8 CA_LoadFile (const char *filename, memptr *ptr)
{
    S32 size;

    FILE * handle = fopen(filename, "rb");
    if(handle == NULL)
    {
        return 0;
    }
    

    fseek(handle, 0, SEEK_END);
    size = ftell(handle);
    fseek(handle, 0, SEEK_SET);
    
    *ptr = malloc(size);
    CheckMallocResult(*ptr);
    
    if(fread(*ptr,1,size,handle) < size)
    {
        fclose (handle);
        return 0;
    }
    fclose (handle);
    
    return 1;
}

/* 
================================================================
=
= Function: CA_UncacheAudioChunk
=
= Description:
=
= Uncaches the requested audio chunk from memory
=
================================================================
*/

void CA_UncacheAudioChunk(S32 chunk)
{

    if(audiosegs[chunk] != NULL) 
    {
        free(audiosegs[chunk]); 
        audiosegs[chunk]=NULL;
    }

}


/* 
================================================================
=
= Function: CA_CacheAudioChunk
=
= Description:
=
= Caches the requested music audio chunk into memory
=
================================================================
*/

S32 CA_CacheAudioChunk(S32 chunk)
{
    S32 pos = audiostarts[chunk];
    S32 size = audiostarts[chunk+1]-pos;

    if(audiosegs[chunk] != NULL)
    {
        return size;                        /* already in memory */
    }

    audiosegs[chunk]=(U8 *) malloc(size);
    CheckMallocResult(audiosegs[chunk]);

    fseek(audiohandle,pos,SEEK_SET);
    fread(audiosegs[chunk],1,size,audiohandle);

    return size;
}

/* 
================================================================
=
= Function: CA_CacheAdlibSoundChunk
=
= Description:
=
= Caches the requested sound effects audio chunk into memory
=
================================================================
*/
static void CA_CacheAdlibSoundChunk(S32 chunk)
{
    S32 pos = audiostarts[chunk];
    S32 size = audiostarts[chunk+1]-pos;
    
    if(audiosegs[chunk] != NULL)
    {
        return;                        /* already in memory */
    }
    
    /* read raw data into bufferseg */
    fseek(audiohandle,pos,SEEK_SET);
    fread(bufferseg,1,(ORIG_ADLIBSOUND_SIZE - 1),audiohandle);    /* without data[1] */
    
    /* malloc some memory for adlib struct (plus any padding) */
    AdLibSound *sound = (AdLibSound *) malloc(size + sizeof(AdLibSound) - ORIG_ADLIBSOUND_SIZE);
    CheckMallocResult(sound);
    
    /* put raw data from buffer into adlib struct */
    U8 *ptr = (U8 *) bufferseg;
    sound->common.length = READLONGWORD(ptr);
    ptr += 4;
    sound->common.priority = READWORD(ptr);
    ptr += 2;
    sound->inst.mChar = *ptr++;
    sound->inst.cChar = *ptr++;
    sound->inst.mScale = *ptr++;
    sound->inst.cScale = *ptr++;
    sound->inst.mAttack = *ptr++;
    sound->inst.cAttack = *ptr++;
    sound->inst.mSus = *ptr++;
    sound->inst.cSus = *ptr++;
    sound->inst.mWave = *ptr++;
    sound->inst.cWave = *ptr++;
    sound->inst.nConn = *ptr++;
    sound->inst.voice = *ptr++;
    sound->inst.mode = *ptr++;
    sound->inst.unused[0] = *ptr++;
    sound->inst.unused[1] = *ptr++;
    sound->inst.unused[2] = *ptr++;
    sound->block = *ptr++;
    
    /* read data variable into adlib struct*/
    fread(sound->data,1,(size - ORIG_ADLIBSOUND_SIZE + 1),audiohandle);    /* + 1 because of byte data[1] */ 

    /* store pointer to new adlib struct in audioseg */
    audiosegs[chunk]=(U8 *) sound;
}

/* 
================================================================
=
= Function: CheckMallocResult
=
= Description:
=
= checks malloc request assigned memory if not tell user 
= something is wrong !
=
================================================================
*/

void CheckMallocResult(void *ptr)
{
    if(ptr == NULL)
    {
        printf("malloc failed to assign memory \n");
        while(1){} /* hang system */
    }
}

/* 
================================================================
=
= Function: CA_Shutdown
=
= Description:
=
= Return all allocated memory to heap and close file handles
=
================================================================
*/

void CA_Shutdown(void)
{
    S32 i;

    for(i=0; i<NUMSNDCHUNKS; i++)
    {
        CA_UncacheAudioChunk(i);
    }

    if(audiostarts != NULL)
    {
        free(audiostarts);
    }
    
    if(audiohandle != NULL)
    {
        fclose(audiohandle);
    }
    
    for(i=0; i<NUMCHUNKS; i++)
    {
        CA_UnCacheGrChunk(i);
    }
    
    free(pictable);
    
    if(grhandle != NULL)
    {
        fclose(grhandle);
    }
}

/* 
================================================================
=
= Function: CAL_GetGrChunkLength
=
= Description:
=
= Gets the length of an explicit length chunk (not tiles)
= The file pointer is positioned so the compressed data can be read in next.
=
================================================================ 
*/

static void CAL_GetGrChunkLength (S32 chunk)
{
    fseek(grhandle,GrFilePos(chunk),SEEK_SET);              /* move file pointer to required chunk via lookup of offset table*/
    fread(&chunkexplen,1,sizeof(chunkexplen),grhandle);     /* read the expected chunk length */
    chunkcomplen = GrFilePos(chunk+1)-GrFilePos(chunk)-4;   /* return chunk length minus 4 header bytes*/
}

/* 
================================================================
=
= Function: GrFilePos
=
= Description:
=
= returns the offset within file for requested graphics chunk
=
================================================================ 
*/

static S32 GrFilePos(const S32 idx)
{
    if(idx > (sizeof(grstarts) / sizeof(S32)))
    {
        printf("requested graphics chunk offset out of range \n");
        while(1){}; /*hang system*/
    }
    else
    {
        return grstarts[idx];
    }
}

/*
============================================================================
=
= Function: CAL_HuffExpand
=
= Description:
=
= Uncompresses source data into dest memory using hufftable
= (COMPRESSION routines, see JHUFF.C for more)
=
============================================================================
*/

static void CAL_HuffExpand(U8 *source, U8 *dest, S32 length, huffnode *hufftable)
{
    U8 *end;
    huffnode *headptr; 
    huffnode *huffptr;
    S32 written = 0;
    U8 val = *source++;
    U8 mask = 1;
    U16 nodeval;

    if((length == 0) || (dest == NULL))
    {
        printf("length or dest is null! \n");
        while(1){};     /* hang system */
    }

    headptr = hufftable+254;        /* head node is always node 254 */
    end = dest + length;
    huffptr = headptr;
    
    while(1)
    {
        if((val & mask) == 0)
        {
            nodeval = huffptr->bit0;
        }
        else
        {
            nodeval = huffptr->bit1;
        }
        
        if(mask==0x80)
        {
            val = *source++;
            mask = 1;
        }
        else 
        {
            mask <<= 1;
        }

        if(nodeval<256)
        {
            *dest++ = (U8) nodeval;
            written++;
            huffptr = headptr;
            if(dest >= end)
            {
                break;
            }
        }
        else
        {
            huffptr = hufftable + (nodeval - 256);
        }
    }
}

/* 
================================================================
=
= Function: CA_CacheGrChunk
=
= Description:
=
= Makes sure a given chunk is in memory, loading it if needed
=
================================================================
*/

void CA_CacheGrChunk(S32 chunk)
{
    S32 pos;
    S32 compressed;
    S32 *source = NULL;
    S32 next;

    if (grsegs[chunk])
    {
        return;         /* already in memory */
    }

    pos = GrFilePos(chunk);
    if (pos < 0)                /* $FFFFFFFF start is a sparse tile */
    {
        return;
    }
    
    next = chunk + 1;
    
    /* skip past any sparse tiles */
    while(GrFilePos(next) == -1)
    {
        next++;
    }
    
    /* store compressed chunk length */
    compressed = GrFilePos(next)-pos;
    
    /* set file pointer to compressed chunk required */
    fseek(grhandle,pos,SEEK_SET);
    
    /* load the chunk into a buffer, either the miscbuffer if it fits, or allocate a larger buffer */
    if (compressed <= BUFFERSIZE)
    {
        fread(bufferseg,1,compressed,grhandle);
        source = bufferseg;
    }
    else
    {
        source = (S32 *) malloc(compressed);
        CheckMallocResult(source);
        fread(source,1,compressed,grhandle);
    }
    
    CAL_ExpandGrChunk(chunk,source);

    /* if we used the heap to store compressed chunk then return memory */
    if(compressed > BUFFERSIZE)
    {
        free(source);
    }
    
}

/*
================================================================
=
= Function: CA_CacheMap
=
= Description:
=
= WOLF: This is specialized for a 64*64 map size, cache map
= data into mapsegs planes
=
================================================================
*/
void CA_CacheMap(S32 mapnum)
{
    S32     pos;
    S32     compressed;
    S32     plane;
    U16     *dest;
    memptr  bigbufferseg;
    U32     size;
    U16     *source;
    U16     *buffer2seg;
    S32     expanded;
    
    mapon = mapnum;
    
    /* load the planes into the allready allocated buffers */
    size = MAPAREA*2;
    
    for(plane = 0; plane<MAPPLANES; plane++)
    {
        /* get the start position of this maps plane */
        /* from the map header array in memory */
        pos = mapheaderseg[mapnum]->planestart[plane];
        compressed = mapheaderseg[mapnum]->planelength[plane];
        dest = mapsegs[plane];
        
        fseek(maphandle,pos,SEEK_SET);
        
        /* grab some memory for the uncompressed file*/
        if (compressed<=BUFFERSIZE)
        {
            source = (U16 *) bufferseg;
        }
        else
        {
            bigbufferseg = malloc(compressed);
            CheckMallocResult(bigbufferseg);
            source = (U16 *) bigbufferseg;
        }

        fread(source,1,compressed,maphandle);
        
        /* unhuffman, then un-RLEW (run length encoding) */
        /* The huffman'd chunk has a two byte expanded length first */
        /* The resulting RLEW chunk also does, even though it's not */
        /* really needed */
        expanded = *source;
        
        source++;
        buffer2seg = (U16 *) malloc(expanded);
        CheckMallocResult(buffer2seg);
        CAL_CarmackExpand((U8 *) source, buffer2seg,expanded);
        CA_RLEWexpand((buffer2seg + 1),dest,size,RLEWtag);
        free(buffer2seg);
        
        if(compressed>BUFFERSIZE)
        {
            free(bigbufferseg);
        }
    }
}

/*
================================================================
=
= Function: CAL_CarmackExpand
=
= Description:
=
= Length is the length of the EXPANDED data
=
================================================================
*/
static void CAL_CarmackExpand(U8 *source, U16 *dest, S32 length)
{
    U16 ch;
    U16 chhigh;
    U16 count;
    U16 offset;
    U8 *inptr;
    U16 *copyptr; 
    U16 *outptr;
    
    length /= 2;

    inptr = (U8 *) source;
    outptr = dest;
    
    while(length > 0)
    {
        ch = READWORD(inptr);
        inptr += 2;
        chhigh = (ch >> 8) & 0x00FF;
        
        if(chhigh == NEARTAG)
        {
            count = ch & 0xff;
            if(count == 0)
            {                       /* have to insert a word containing the tag byte */
                ch |= *inptr++;
                *outptr++ = ch;
                length--;
            }
            else
            {
                offset = *inptr++;
                copyptr = outptr - offset;
                length -= count;
                if(length < 0)
                {
                    return;
                }
                
                while(count--)
                {
                    *outptr++ = *copyptr++;
                }
            }
        }
        else if(chhigh == FARTAG)
        {
            count = ch & 0xff;
            if(count == 0)
            {                       /* have to insert a word containing the tag byte */
                ch |= *inptr++;
                *outptr++ = ch;
                length --;
            }
            else
            {
                offset = READWORD(inptr);
                inptr += 2;
                copyptr = dest + offset;
                length -= count;
                if(length < 0)
                {
                    return;
                }
                
                while(count--)
                {
                    *outptr++ = *copyptr++;
                }
            }
        }
        else
        {
            *outptr++ = ch;
            length --;
        }
    }
}

/*
================================================================
=
= Function: CA_RLEWexpand
=
= Description:
=
= expand run length encoded data from source into dest, length
= is the EXPANDED length. rlewtag value is used as a flag to
= indicate that the data following it has been compressed.
=
================================================================
*/
static void CA_RLEWexpand(U16 *source, U16 *dest, S32 length, U16 rlewtag)
{
    U16 value;
    U16 count;
    U16 i;
    U16 *end = dest + length / 2;
    
    /* start expanding run length encoded data */
    do
    {
        value = *source++;
        
        /* check to see if we need to uncompress data */
        if (value != rlewtag)
        {
            /* data is already uncompressed so just store it as is*/
            *dest++=value;
        }
        else
        {
            /* data is run length encoded so un-RLE it !*/
            count = *source++;
            value = *source++;
            for (i=1;i<=count;i++)
            {
                *dest++ = value;
            }
        }
    }while(dest<end);
}

/*
================================================================
=
= Function: CA_CacheScreen
=
= Description:
=
= Decompresses a chunk from disk straight onto the screen
=
================================================================
*/

void CA_CacheScreen(S32 chunk)
{
    S32    pos;
    S32    compressed;
    S32    expanded;
    memptr bigbufferseg;
    S32    *source;
    S32    next;
    U8     *pic;
    
    pos = GrFilePos(chunk);
    
    next = chunk + 1;
    
    /* skip past any sparse tiles */
    while (GrFilePos(next) == -1)
    {
        next++;
    }
    
    /* store compressed chunk length */
    compressed = GrFilePos(next)-pos;
    
    /* set file pointer to compressed chunk required */
    fseek(grhandle,pos,SEEK_SET);
    
    /* load the chunk into a buffer */
    bigbufferseg = malloc(compressed);
    CheckMallocResult(bigbufferseg);
    fread(bigbufferseg,1,compressed,grhandle);
    
    source = (S32 *) bigbufferseg;
   
    expanded = *source++;
    
    /* allocate final space, decompress it, and free bigbuffer */
    /* Sprites need to have shifts made and various other junk */
    
    pic = (U8 *) malloc(64000);
    CheckMallocResult(pic);
    CAL_HuffExpand((U8 *) source, pic, expanded, grhuffman);
    
    VL_MemToScreen(pic, 320, 200, 0, 0);
    
    free(pic);
    free(bigbufferseg);
}

/* 
================================================================
=
= Function: CA_UnCacheGrChunk
=
= Description:
=
= Uncache a given graphics chunk from memory
=
================================================================
*/
void CA_UnCacheGrChunk(S32 chunk)
{
    if(grsegs[chunk] != NULL) 
    {
        free(grsegs[chunk]);
        grsegs[chunk]=NULL;
    }
}

/*
============================================================================
=
= Function: CAL_ExpandGrChunk
=
= Description:
=
= Does whatever is needed with a pointer to a compressed chunk
=
============================================================================
*/

static void CAL_ExpandGrChunk (S32 chunk, S32 *source)
{
    S32    expanded;

    if((chunk >= STARTTILE8) && (chunk < STARTEXTERNS))
    {
        /* expanded sizes of tile8/16/32 are implicit */
        if(chunk < STARTTILE8M)          /* tile 8s are all in one chunk! */
        {
            expanded = BLOCK*NUMTILE8;
        }
        else if(chunk < STARTTILE16)
        {
            expanded = MASKBLOCK*NUMTILE8M;
        }
        else if(chunk < STARTTILE16M)    /* all other tiles are one/chunk */
        {
            expanded = BLOCK*4;
        }
        else if(chunk < STARTTILE32)
        {
            expanded = MASKBLOCK*4;
        }
        else if(chunk < STARTTILE32M)
        {
            expanded = BLOCK*16;
        }
        else
        {
            expanded = MASKBLOCK*16;
        }
    }
    else
    {
        /* everything else has an explicit size longword */
        expanded = *source++;
    }
    
    /*  allocate final space, decompress it, and free bigbuffer */
    /*  Sprites need to have shifts made and various other junk */
    
    grsegs[chunk] = (U8 *) malloc(expanded);
    CheckMallocResult(grsegs[chunk]);
    CAL_HuffExpand((U8 *) source, grsegs[chunk], expanded, grhuffman);
}

/*
======================
=
= Function: CA_LoadAllSounds
=
= Description:
=
= Purges all sounds, then loads all new ones.
=
======================
*/

void CA_LoadAllSounds(void)
{
    U32 i;
    U32 start = STARTADLIBSOUNDS;
    
    /* uncach all loaded sounds */
    for (i=0;i<NUMSOUNDS;i++,start++)
    {
        CA_UncacheAudioChunk(start);
    }
    
    start = STARTADLIBSOUNDS;
    
    /* load all sounds */
    for (i=0;i<NUMSOUNDS;i++,start++)
    {
        CA_CacheAdlibSoundChunk(start);
    }
}

/*
============================================================================
=
= Function: CAL_SetupMapFile
=
= Description:
=
= loads map file data
=
============================================================================
*/
static void CAL_SetupMapFile(void)
{
    char fname[30];
    S32 i;
    S32 j;
    S32 k;
    S32 pos;
    FILE * handle = NULL;
    U8 *tinf = NULL;
    U16 length = 0;
    U8 *ptr = NULL;
    
    /* load maphead.ext (offsets and tileinfo for map file) */
    strcpy(fname,mheadname);
    strcat(fname,extension);
    
    handle = fopen(fname, "rb");
    
    if(handle == NULL)
    {
        CA_CannotOpen(fname);
    }
    
    /* obtain file size */
    fseek (handle , 0 , SEEK_END);
    length = ftell(handle);
    rewind(handle);
    /* read in the map file header */    
    tinf = (U8 *) malloc(length);
    CheckMallocResult(tinf);
    fread(tinf,1,length,handle);
    
    fclose(handle);
    handle = NULL;
    RLEWtag = ((mapfiletype *) tinf)->RLEWtag;
    
    /* open the data file */
    strcpy(fname,"/GAMES/WOLF/GAMEMAPS.");
    strcat(fname, extension);
    maphandle = fopen(fname, "rb");
    
    if(maphandle == NULL)
    {
        CA_CannotOpen(fname);
    }
    
    /* load all map header */
    for (i=0, j=2; i<NUMMAPS ;i++, j+=4)
    {
        pos = (S32)((tinf[j + 3] << 24)  | (tinf[j + 2] << 16) | (tinf[j + 1] << 8) | tinf[j]);
        
        if(pos < 0)
        {
            continue;   /* $FFFFFFFF start is a sparse map */
        }

        mapheaderseg[i]=(maptype *) malloc(sizeof(maptype));
        CheckMallocResult(mapheaderseg[i]);
        
        /* read raw data into bufferseg */
        fseek(maphandle,pos,SEEK_SET);
        fread(bufferseg,1,(ORIG_MAPTYPE_SIZE),maphandle);   
        
        /* put raw data from buffer into map struct */
        ptr = (U8 *) bufferseg;
        
        mapheaderseg[i]->planestart[0] = READLONGWORD(ptr);
        ptr += 4;
        mapheaderseg[i]->planestart[1] = READLONGWORD(ptr);
        ptr += 4;
        mapheaderseg[i]->planestart[2] = READLONGWORD(ptr);
        ptr += 4;
        mapheaderseg[i]->planelength[0] = READWORD(ptr);
        ptr += 2;
        mapheaderseg[i]->planelength[1] = READWORD(ptr);
        ptr += 2;
        mapheaderseg[i]->planelength[2] = READWORD(ptr);
        ptr += 2;
        mapheaderseg[i]->width = READWORD(ptr);
        ptr += 2;
        mapheaderseg[i]->height = READWORD(ptr);
        ptr += 2;
        for(k = 0; k < 16; k++)
        {
            mapheaderseg[i]->name[k] = *ptr++;
        }
    }
    
    free(tinf);
    
    /* allocate space for 2 64*64 planes */
    for(i = 0; i < MAPPLANES; i++)
    {
        mapsegs[i] = (U16 *) malloc(MAPAREA*2);
        CheckMallocResult(mapsegs[i]);
    }
}

/*
============================================================================
=
= Function: CA_Startup
=
= Description:
=
= Open all files and load in headers
=
============================================================================
*/
void CA_Startup(void)
{

    CAL_SetupMapFile();
    CAL_SetupGrFile();
    CAL_SetupAudioFile();

    mapon = -1;
}


