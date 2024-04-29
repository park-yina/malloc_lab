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
/* single word (4) or double word (8) alignment */
#define DOUBLE_SIZE 8
#define SINGLE_SIZE 4
#define CHUNKSIZE (1<<12)

#define MAX(x,y) ((x)>(y)? (x):(y))

#define PACK(size,alloc) ((size)|(alloc))
#define GET(p) (*(unsigned int *)(p))
#define PUT(p,val) (*(unsigned int *)(p)=(val))
#define GET_SIZE(p) (GET(p)& ~0X7)
#define GET_ALLOC(p) (GET(p) & 0X1)
#define HDRP(bp) ((char *)(bp)-SINGLE_SIZE)
#define FTRP(bp) ((char *)(bp)+GET_SIZE(HDRP(bp))-DOUBLE_SIZE)
#define NEXT_BLKP(bp) ((char *)(bp)+GET_SIZE(((char *)(bp)-SINGLE_SIZE)))
#define PREV_BLKP(bp) ((char *)(bp)-GET_SIZE(((char *)(bp)-DOUBLE_SIZE)))
/* rounds up to the nearest multiple of ALIGNMENT */
#define ALIGN(size) (((size) + (DOUBLE_SIZE-1)) & ~0x7)
#define SIZE_T_SIZE (ALIGN(sizeof(size_t)))
static void* heap_listp;
static void* coalesce(void* bp)
{
    size_t prev_alloc = GET_ALLOC(FTRP(PREV_BLKP(bp)));
    size_t next_alloc = GET_ALLOC(HDRP(NEXT_BLKP(bp)));
    size_t size = GET_SIZE(HDRP(bp));

    if (prev_alloc && next_alloc) {            /* Case 1 */
        return bp;
    }

    else if (prev_alloc && !next_alloc) {      /* Case 2 */
        size += GET_SIZE(HDRP(NEXT_BLKP(bp)));
        PUT(HDRP(bp), PACK(size, 0));
        PUT(FTRP(bp), PACK(size, 0));
    }

    else if (!prev_alloc && next_alloc) {      /* Case 3 */
        size += GET_SIZE(HDRP(PREV_BLKP(bp)));
        PUT(FTRP(bp), PACK(size, 0));
        PUT(HDRP(PREV_BLKP(bp)), PACK(size, 0));
        bp = PREV_BLKP(bp);
    }

    else {                                     /* Case 4 */
        size += GET_SIZE(HDRP(PREV_BLKP(bp))) +
            GET_SIZE(FTRP(NEXT_BLKP(bp)));
        PUT(HDRP(PREV_BLKP(bp)), PACK(size, 0));
        PUT(FTRP(NEXT_BLKP(bp)), PACK(size, 0));
        bp = PREV_BLKP(bp);
    }
    return bp;
}
static void* extend_heap(size_t words) {
    char* bp;
    size_t size = (words % 2) ? (words + 1) * SINGLE_SIZE : words * SINGLE_SIZE;
    if ((long)(bp = mem_sbrk(size)) == -1)
        return NULL;
    PUT(HDRP(bp), PACK(size, 0));
    PUT(FTRP(bp), PACK(size, 0));
    PUT(HDRP(NEXT_BLKP(bp)), PACK(0, 1));
    return coalesce(bp);
}
extern int mm_init(void) {
    if ((heap_listp = mem_sbrk(8 * SINGLE_SIZE)) == (void*)-1)
        return -1;

    PUT(heap_listp, PACK(0, 1));                                  // Prologue header
    PUT(heap_listp + (1 * SINGLE_SIZE), PACK(DOUBLE_SIZE, 1));    // Prologue footer
    PUT(heap_listp + (2 * SINGLE_SIZE), PACK(DOUBLE_SIZE, 1));    // Epilogue header
    PUT(heap_listp + (3 * SINGLE_SIZE), PACK(0, 1));              // Epilogue footer

    heap_listp += (2 * SINGLE_SIZE);

    if (extend_heap(CHUNKSIZE / SINGLE_SIZE) == NULL)
        return -1;

    return 0;
}

static void place(void* bp, size_t newsize);
static void* find_fit(size_t asize);


extern void* mm_malloc(size_t size)
{
    size_t asize;
    size_t extendsize;
    char* bp;

    if (size <= 0)
        return NULL;

    if (size <= DOUBLE_SIZE)
        asize = 2 * DOUBLE_SIZE;
    else
        asize = DOUBLE_SIZE * ((size + DOUBLE_SIZE + DOUBLE_SIZE - 1) / DOUBLE_SIZE); // 8배수로 올림 처리

    if ((bp = find_fit(asize)) != NULL)
    {
        place(bp, asize);
        return bp;
    }

    extendsize = MAX(asize, CHUNKSIZE);
    if ((bp = extend_heap(extendsize / SINGLE_SIZE)) == NULL)
        return NULL;

    place(bp, asize);
    return bp;
}

extern void mm_free(void* ptr) {
    if (ptr == NULL) // NULL 포인터를 free하려는 경우 무시합니다.
        return;
    size_t size = GET_SIZE(HDRP(ptr));
    PUT(HDRP(ptr), PACK(size, 0));
    PUT(FTRP(ptr), PACK(size, 0));
    coalesce(ptr);
}

extern void* mm_realloc(void* ptr, size_t size)
{
    if (ptr == NULL) // 포인터가 NULL인 경우 할당만 수행
        return mm_malloc(size);

    if (size <= 0)
    {
        mm_free(ptr);
        return NULL;
    }

    void* newptr = mm_malloc(size); // 새로 할당한 블록의 포인터
    if (newptr == NULL)
        return NULL; // 할당 실패

    size_t copySize = GET_SIZE(HDRP(ptr)) - DOUBLE_SIZE;
    if (size < copySize)
        copySize = size;

    memcpy(newptr, ptr, copySize); // 새 블록으로 데이터 복사

    /* 기존 블록 반환 */
    mm_free(ptr);

    return newptr;
}

static void* find_fit(size_t asize) {
    void* bp;
    for (bp = heap_listp; GET_SIZE(HDRP(bp)) > 0; bp = NEXT_BLKP(bp)) {
        if (!GET_ALLOC(HDRP(bp)) && (asize <= GET_SIZE(HDRP(bp)))) {//공간이 크거나 같다는건 데이터가 들어갈 수 있다는 뜻임
            return bp;

        }
    }
    return NULL;
}

static void place(void* bp, size_t asize) {
    size_t new_size = GET_SIZE(HDRP(bp));
    if ((new_size - asize) >= (2 * DOUBLE_SIZE)) {
        PUT(HDRP(bp), PACK(asize, 1));
        PUT(FTRP(bp), PACK(asize, 1));
        PUT(HDRP(NEXT_BLKP(bp)), PACK(new_size - asize, 0));
        PUT(FTRP(NEXT_BLKP(bp)), PACK(new_size - asize, 0));
    }
    else {
        PUT(HDRP(bp), PACK(new_size, 1));
        PUT(FTRP(bp), PACK(new_size, 1));
    }
}
