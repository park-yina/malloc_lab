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

static void* extend_heap(size_t word) {
    
    char* bp;
    size_t temp = 0;
    if (word % 2 == 0) {
        temp =word*SINGLE_SIZE ;
    }
    else {
        temp = (word + 1) * SINGLE_SIZE;
    }
    if ((long)(bp = mem_sbrk(temp)) == -1)return NULL;
    PUT(HDRP(bp), PACK(temp, 0));
    PUT(FTRP(bp), PACK(temp, 0));
    PUT(HDRP(NEXT_BLKP(bp)), PACK(0, 1));
    return coalesce(bp);
}
static void* heap_listp;
static char* result;
static void* coalesce(void* bp) {
    size_t pre_alloc = GET_ALLOC(FTRP(PREV_BLKP(bp)));
    size_t next_alloc = GET_ALLOC(HDRP(NEXT_BLKP(bp)));
    size_t size = GET_SIZE(HDRP(bp));
    if (pre_alloc && next_alloc) {
        return bp;
        //이전과 다음이 모두 사용중인 경우이다.
    }
    else if (pre_alloc && !next_alloc) {
        size += GET_SIZE(HDRP(NEXT_BLKP(bp)));
        PUT(HDRP(bp), PACK(size, 0));
        PUT(FTRP(bp), PACK(size, 0));
    }
    else if (!pre_alloc && next_alloc) {
        size += GET_SIZE(HDRP(PREV_BLKP(bp)));
        PUT(HDRP(PREV_BLKP(bp)), PACK(size, 0));
        bp = PREV_BLKP(bp);
    }
    //저 위에 속하지 않는 경우라면 당연히 양쪽을 모두 다 써도 괜찮은 경우니까 사이즈도 그만큼 뿔어나야함
    else {
        size += GET_SIZE(HDRP(PREV_BLKP(bp))) + GET_SIZE(HDRP(NEXT_BLKP(bp)));
        PUT(HDRP(PREV_BLKP(bp)), PACK(size, 0);
        PUT(HDRP(NEXT_BLKP(bp)), PACK(size, 0));
        bp = PREV_BLKP(bp);
    }
    return bp;
}
/* 
 * mm_init - initialize the malloc package.
 */
int mm_init(void)
{
    if ((heap_listp = mem_sbrk(4 * SINGLE_SIZE)) == (void*)-1)
        return -1;
    PUT(heap_listp, 0);
    PUT(heap_listp + (1 * SINGLE_SIZE), PACK(DOUBLE_SIZE, 1));
    PUT(heap_listp + (2 * SINGLE_SIZE), PACK(DOUBLE_SIZE, 1));
    PUT(heap_listp + (3 * SINGLE_SIZE), PACK(0, 1));
    heap_listp += (2 * SINGLE_SIZE);
    if (extend_heap(CHUNKSIZE / SINGLE_SIZE) == NULL)
        return -1;
    result = (char*)heap_listp;
    return 0;
}

/* 
 * mm_malloc - Allocate a block by incrementing the brk pointer.
 *     Always allocate a block whose size is a multiple of the alignment.
 */
void *mm_malloc(size_t size)
{
    int newsize = ALIGN(size + SIZE_T_SIZE);
    void *p = mem_sbrk(newsize);
    if (p == (void *)-1)
	return NULL;
    else {
        *(size_t *)p = size;
        return (void *)((char *)p + SIZE_T_SIZE);
    }
}

/*
 * mm_free - Freeing a block does nothing.
 */
void mm_free(void *ptr)
{
}

/*
 * mm_realloc - Implemented simply in terms of mm_malloc and mm_free
 */
void *mm_realloc(void *ptr, size_t size)
{
    void *oldptr = ptr;
    void *newptr;
    size_t copySize;
    
    newptr = mm_malloc(size);
    if (newptr == NULL)
      return NULL;
    copySize = *(size_t *)((char *)oldptr - SIZE_T_SIZE);
    if (size < copySize)
      copySize = size;
    memcpy(newptr, oldptr, copySize);
    mm_free(oldptr);
    return newptr;
}
