/*
 * mm-naive.c - The fastest, least memory-efficient malloc package.
 *
 * In this naive approach, a block is allocated by simply incrementing
 * the brk pointer.  A block is pure payload. There are no headers or
 * footers.  Blocks are never coalesced or reused. Realloc is
 * implemented directly using mm_malloc and mm_free.
 *
 * NOTE TO STUDENTS: Replace this header comment with your own header
 * comment that gives a high level description of your solution.
 */
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <unistd.h>
#include <string.h>

#include "mm.h"
#include "memlib.h"

 /*********************************************************
  * NOTE TO STUDENTS: Before you do anything else, please
  * provide your team information in the following struct.
  ********************************************************/
team_t team = {
    /* Team name */
    "kraftom_Jungle_5",
    /* First member's full name */
    "Park-yina",
    /* First member's email address */
    "kln99988@naver.com",
    /* Second member's full name (leave blank if none) */
    "",
    /* Second member's email address (leave blank if none) */
    ""
};

/* single word (4) or double word (8) alignment */
#define DOUBLE_SIZE 8
#define SINGLE_SIZE 4
#define CHUNKSIZE (1<<12)

#define MAX(x,y) ((x)>(y)? (x):(y))

#define PACK(size,alloc) ((size)|(alloc))
#define GET(p) (*(unsigned int *)(p));
#define PUG(p,val) (*(unsigned int *)(p)=(val));
#define GET_SIZE(p) (GET(p)& ~0X7)
#define GET_ALLOC(p) (GET(p) & 0X1)
#define HDRP(bp) ((char *)(bp)-SINGLE_SIZE);
#define FTRP(bp) ((char *)(bp)+GET_SIZE(HDRP(bp))-DOUBLE_SIZE)
#define NEXT_BLKP(bp) ((char *)(bp)+GET_SIZE(((char *)(bp)-SINGLE_SIZE)))
#define PREV_BLKP(bp) ((char *)(bp)+GET_SIZE(((char *)(bp)-DOUBLE_SIZE)))
/* rounds up to the nearest multiple of ALIGNMENT */
#define ALIGN(size) (((size) + (DOUBLE_SIZE-1)) & ~0x7)


#define SIZE_T_SIZE (ALIGN(sizeof(size_t)))
static void place(void* bp, size_t newsize);
static void* find_fit(size_t asize);
static void* extend_heap(size_t word) {
    char* bp;
    size_t temp = 0;
    if (word % 2 == 0) {
        temp = word * SINGLE_SIZE;
    }
    else {
        temp = (word + 1) * SINGLE_SIZE;
    }
    if ((long)(bp = mem_sbrk(temp)) == -1) return NULL;
    PUT(HDRP(bp), PACK(temp, 0));
    PUT(FTRP(bp), PACK(temp, 0));
    PUT(HDRP(NEXT_BLKP(bp)), PACK(0, 1));
    return coalesce(bp);
}

void* mm_malloc(size_t size) {
    size_t asize;
    size_t extendsize;
    char* bp;
    if (size == 0) return NULL;
    if (size <= DOUBLE_SIZE)
        asize = 2 * DOUBLE_SIZE;
    else
        asize = DOUBLE_SIZE * ((size + (DOUBLE_SIZE)+(DOUBLE_SIZE - 1)) / DOUBLE_SIZE);
    if ((bp = find_fit(asize)) != NULL) {
        place(bp, asize);
        return bp;
    }
    extendsize = MAX(asize, CHUNKSIZE);
    if ((bp = extend_heap(extendsize / SINGLE_SIZE)) == NULL)
        return NULL;
    place(bp, asize);
    return bp;
}

void mm_free(void* ptr) {
    if (!ptr) return;
    size_t size = GET_SIZE(HDRP(ptr));
    PUT(HDRP(ptr), PACK(size, 0));
    PUT(FTRP(ptr), PACK(size, 0));
    coalesce(ptr);
}

void* mm_realloc(void* ptr, size_t size) {
    if (!ptr) return mm_malloc(size);
    if (size == 0) {
        mm_free(ptr);
        return NULL;
    }
    void* oldptr = ptr;
    void* newptr;
    size_t copySize;

    newptr = mm_malloc(size);
    if (newptr == NULL)
        return NULL;
    copySize = *(size_t*)((char*)oldptr - SIZE_T_SIZE);
    if (size < copySize)
        copySize = size;
    memcpy(newptr, oldptr, copySize);
    mm_free(oldptr);
    return newptr;
}

static void* find_fit(size_t asize) {
    void* bp;
    for (bp = heap_listp; GET_SIZE(HDRP(bp)) > 0; bp = NEXT_BLKP(bp)) {
        if (!GET_ALLOC(HDRP(bp)) && (asize <= GET_SIZE(HDRP(bp)))) {
            return bp;
        }
    }
    return NULL;
}
