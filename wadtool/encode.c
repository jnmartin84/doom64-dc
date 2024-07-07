/*
 * adapted from https://github.com/Erick194/D64TOOL/blob/main/src/Lzlib.cpp
 */


#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>     /* malloc, free, rand */
#include <fcntl.h>
#include <unistd.h>

#define WINDOW_SIZE	4096
#define LENSHIFT 4		// this must be log2(LOOKAHEAD_SIZE)
#define LOOKAHEAD_SIZE	(1<<LENSHIFT)

typedef struct node_struct node_t;

///////////////////////////////////////////////////////////////////////////
//       IMPORTANT: FOLLOWING STRUCTURE MUST BE 16 BYTES IN LENGTH       //
///////////////////////////////////////////////////////////////////////////

struct node_struct
{
    unsigned char *pointer;
    node_t *prev;
    node_t *next;
    int	pad;
};

typedef struct list_struct
{
    node_t *start;
    node_t *end;
} list_t;

static list_t hashtable[256]; // used for faster encoding
static node_t hashtarget[WINDOW_SIZE]; // what the hash points to

//
//  Adds a node to the hash table at the beginning of a particular index
//  Removes the node in its place before.
//

void addnode(unsigned char *pointer)
{
    list_t *list;
    int targetindex;
    node_t *target;

    targetindex = (uintptr_t) pointer & ( WINDOW_SIZE - 1 );

    // remove the target node at this index

    target = &hashtarget[targetindex];
    if (target->pointer)
    {
        list = &hashtable[*target->pointer];
        if (target->prev)
        {
            list->end = target->prev;
            target->prev->next = 0;
        }
        else
        {
            list->end = 0;
            list->start = 0;
        }
    }

    // add a new node to the start of the hashtable list

    list = &hashtable[*pointer];

    target->pointer = pointer;
    target->prev = 0;
    target->next = list->start;
    if (list->start) {
        list->start->prev = target;
    }
    else {
        list->end = target;
    }
    list->start = target;
}

unsigned char *encode(unsigned char *input, int inputlen, int *size);


unsigned char *encode(unsigned char *input, int inputlen, int *size)
{
    int putidbyte = 0;
    unsigned char *encodedpos;
    int encodedlen;
    int i;
    int len;
    int numbytes, numcodes;
    int codelencount;
    unsigned char *window;
    unsigned char *lookahead;
    unsigned char *idbyte;
    unsigned char *output, *ostart;
    node_t *hashp;
    int lookaheadlen;
    int samelen;

    // initialize the hash table to the occurences of bytes
    for (i=0 ; i<256 ; i++)
    {
        hashtable[i].start = 0;
        hashtable[i].end = 0;
    }

    // initialize the hash table target 
    for (i=0 ; i<WINDOW_SIZE ; i++)
    {
        hashtarget[i].pointer = 0;
        hashtarget[i].next = 0;
        hashtarget[i].prev = 0;
    }

    // create the output
    ostart = output = (unsigned char *) malloc((inputlen * 9)/8+1);

    // initialize the window & lookahead
    lookahead = window = input;

    numbytes = numcodes = codelencount = 0;

    while (inputlen > 0)
    {
        // set the window position and size
        window = lookahead - WINDOW_SIZE;
        if (window < input) { window = input; }

        // decide whether to allocate a new id byte
        if (!putidbyte)
        {
            idbyte = output++;
            *idbyte = 0;
        }
        putidbyte = (putidbyte + 1) & 7;

        // go through the hash table of linked lists to find the strings
        // starting with the first character in the lookahead

        encodedlen = 0;
        lookaheadlen = inputlen < LOOKAHEAD_SIZE ? inputlen : LOOKAHEAD_SIZE;

        hashp = hashtable[lookahead[0]].start;
        while (hashp)
        {
            samelen = 0;
            len = lookaheadlen;
            while (len-- && hashp->pointer[samelen] == lookahead[samelen]) {
                samelen++;
            }
            if (samelen > encodedlen)
            {
                encodedlen = samelen;
                encodedpos = hashp->pointer;
            }
            if (samelen == lookaheadlen) { break; }

            hashp = hashp->next;
        }

        // encode the match and specify the length of the encoding
        if (encodedlen >= 3)
        {
            *idbyte = (*idbyte >> 1) | 0x80;
            *output++ = ((lookahead-encodedpos-1) >> LENSHIFT);
            *output++ = ((lookahead-encodedpos-1) << LENSHIFT) | (encodedlen-1);
            numcodes++;
            codelencount+=encodedlen;
        } else { // or just store the unmatched byte
            encodedlen = 1;
            *idbyte = (*idbyte >> 1);
            *output++ = *lookahead;
            numbytes++;
        }

        // update the hash table as the window slides
        for (i = 0; i < encodedlen; i++) {
            addnode(lookahead++);
        }

        // reduce the input size
        inputlen -= encodedlen;

        /*
        // print pacifier dots
        pacifier -= encodedlen;
        if (pacifier<=0)
        {
            //fprintf(stdout, ".");
            pacifier += 10000;
        }
        */

    }

    // done with encoding- now wrap up

    if (inputlen != 0) {
        //fprintf(stdout, "warning: inputlen != 0\n");
    }

    // put the end marker on the file
    if (!putidbyte) {
        idbyte = output++;
        *idbyte = 1;
    }
    else {
        *idbyte = ((*idbyte >> 1) | 0x80) >> (7 - putidbyte);
    }

    *output++ = 0;
    *output++ = 0;

    *size = output - ostart;

    /*
    fprintf(stdout, "\nnum bytes = %d\n", numbytes);
    fprintf(stdout, "num codes = %d\n", numcodes);
    fprintf(stdout, "ave code length = %f\n", (double) codelencount/numcodes);
    fprintf(stdout, "size = %d\n", *size);
    */

    return ostart;
}

//
//  Return the size of compressed data
//

int decodedsize(unsigned char *input)
{
    int getidbyte = 0;
    int len;
    int pos;
    int i;
    //unsigned char *source;
    int idbyte;
    int accum = 0;

    while (1)
    {
        /*// get a new idbyte if necessary
        if (!getidbyte) { idbyte = *input++; }
        getidbyte = (getidbyte + 1) & 7;

        if (idbyte&1) {
            // decompress
            input++;
            len = *input++ & 0xf;
            if (!len) break;
            accum += len + 1;
        }
        else {
            accum++;
        }
        *input++;

        idbyte = idbyte >> 1;*/

        // get a new idbyte if necessary
        if (!getidbyte) { idbyte = *input++; }
        getidbyte = (getidbyte + 1) & 7;

        if (idbyte & 1)
        {
            // decompress
            pos = *input++ << LENSHIFT;
            pos = pos | (*input >> LENSHIFT);
            len = (*input++ & 0xf) + 1;
            if (len == 1) break;
            for (i = 0; i < len; i++) {
                accum++;
            }
        }
        else
        {
            accum++;
            *input++;
        }

        idbyte = idbyte >> 1;
    }

    return accum;
}

void decode(unsigned char *input, unsigned char *output)
{
    int getidbyte = 0;
    int len;
    int pos;
    int i;
    unsigned char *source;
    int idbyte;

    while (1)
    {
        // get a new idbyte if necessary
        if (!getidbyte) { idbyte = *input++; }
        getidbyte = (getidbyte + 1) & 7;

        if (idbyte&1)
        {
            // decompress
            pos = *input++ << LENSHIFT;
            pos = pos | (*input >> LENSHIFT);
            source = output - pos - 1;
            len = (*input++ & 0xf)+1;
            if (len==1) break;
            for (i = 0; i < len; i++) {
                *output++ = *source++;
            }
        } else {
            *output++ = *input++;
        }

        idbyte = idbyte >> 1;
    }
}
