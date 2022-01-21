#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <sys/mman.h>
#include <errno.h>

#define TRUE 1
#define FALSE 0
#define HEAD (sizeof(struct taken))
#define MIN(size) (((size)>(16))?(size):(16))
#define LIMIT(size) (MIN(0) + HEAD + size)
#define MAGIC(memory) ((struct taken*)memory-1)
#define HIDE(block) (void*)((struct taken*)block+1)
#define ALIGN 8
#define ARENA (64*1024)

struct head *arena = NULL;
struct head *flist;

struct head{
    u_int16_t bfree; //2 bytes, status of block before
    u_int16_t bsize; //2 bytes, size of block before
    u_int16_t free; //2 bytes, the status of the block
    u_int16_t size; //2 bytes, the size(max 2^16=64KiByte)
    struct head *next; //8 bytes pointer
    struct head *prev; //8 bytes pointer
};

struct taken{
    u_int16_t bfree; //2 bytes, status of block before
    u_int16_t bsize; //2 bytes, size of block before
    u_int16_t free; //2 bytes, the status of the block
    u_int16_t size; //2 bytes, the size(max 2^16=64KiByte)
};

struct head *after(struct head *block){
    return (struct head*)(((char*)block) + block->size + HEAD);
}
struct head *before(struct head *block){
    return (struct head*)(((char*)block) - (block->bsize + HEAD));
}

//returns pointer to the splitted block
struct head *split(struct head *block, int size){
    int rsize = block->size - (size + HEAD);
    block->size = rsize;

    struct head *splt = after(block);
    splt->bsize = rsize;
    splt->bfree = block->free;
    splt->size = size;
    splt->free = TRUE; //this will be set to false in find() method after inserted the free block.

    struct head *aft = after(splt);
    aft->bsize = size;
    
    return splt;
}


struct head *new(){
    if(arena != NULL){
        printf("One arena already allocated \n");
        return NULL;
    }
    /*mmap(void addr, size_t length, int prot, int flags, int fd, off_t offset)
        addr = NULL -> kernel choses the page-aligned address at which to create the mapping
        prot -> desired memory protection of the mapping
        flags-> wether updates to the mappings are visible to other processes mapping the same region
    */
    struct head *new = mmap(NULL, ARENA, PROT_READ | PROT_WRITE, MAP_PRIVATE |MAP_ANONYMOUS, -1, 0);
    if(new == MAP_FAILED){
        printf("mmap failed: error %d\n", errno);
        return NULL;
    }
    
    //Make room for head and dummy/sentinel
    uint size = ARENA - (2*HEAD);
    
    new->bfree = FALSE;
    new->bsize = 0;
    new->free = TRUE;
    new->size = size;

    struct head *sentinel = after(new);
    sentinel->bfree = TRUE;
    sentinel->bsize = size;
    sentinel->free = FALSE;
    sentinel->size = 0;

    arena = (struct head*)new;
    return new;
}

void detach(struct head *block){
    if(block->next != NULL){
        block->next->prev = block->prev;
    }
    if(block->prev != NULL){
        block->prev->next = block->next;
    }else{
        flist = block->next;
    }
    
}

void insert(struct head *block){
    block->next = flist;
    block->prev = NULL;
    if(flist != NULL){
        flist->prev = block;
    }
    flist = block;
}

int adjust(int request){

    int size = MIN(request);
    int rsize = size % ALIGN;
    if(rsize != 0)
        size = size + ALIGN - rsize;
    return size;
}

struct head *find(int size){
    if(flist == NULL){
        flist = new();
        if(flist == NULL){
            return NULL;
        }
    }
    struct head *block = flist;
    while(block != NULL && (block->size < size)){
        block = block->next;
    }
    if(block == NULL){
        return NULL;
    }
    detach(block);
    if(block->size >= LIMIT(size)){ //if we could split

        struct head *new = split(block, size); //split the block
        insert(block); //...and put the rest in the freelist
        new->free = FALSE; //mark the found block as taken
        struct head *aft = after(new); //..and make sure...
        aft->bfree = FALSE;//... to update the block after the taken block
        return new;
    }//If not split
    block->free = FALSE;
    struct head *afterBlock = after(block);
    afterBlock->bfree = FALSE;
    return block;  
}

struct head *merge(struct head *block){
    struct head *aft = after(block);
    if(block->bfree){
        struct head *befBlock = before(block);
        detach(befBlock);
        int newsize = befBlock->size + block->size + HEAD;
        befBlock->size = newsize;
        aft->bsize = newsize;
        block = befBlock;
    }
    if(aft->free){

        detach(aft);
        int aftsize = aft->size + block->size + HEAD;
        block->size = aftsize;
        struct head *nxtaft = after(block);
        nxtaft->bsize = aftsize;
    }
    return block;
}

void *dalloc(size_t request){
    if(request <= 0){
        return NULL;
    }
    int size = adjust(request);
    struct head *taken = find(size);

    if(taken == NULL){
        return NULL;
    }else{
        return HIDE(taken); //return a pointer to the beginning of the data segment, HIDE the header.
    }
}

void dfree(void *memory){
    if(memory != NULL){
        struct head *block = MAGIC(memory);
        block = merge(block);
        struct head *aft = after(block);
        block->free = TRUE;
        aft->bfree = TRUE;
        insert(block);
    }
    return;
}


void initializeArena(){
    flist = NULL;
    arena = NULL;
    struct head *firstArena = new();
    insert(firstArena);
    //printf("Initialized the Arena\n"); 
}

void sanity(){
    struct head *block = arena;
    struct head *afterBlock = after(block);
    uint tot = ARENA;
    uint16_t bf = block->bfree;
    uint16_t bs = block->bsize;
    uint16_t sizeOfArena = 0;
    uint16_t sizeOfFreeArena = 0;
    printf("Checking arena...\n");
    while((block->free != FALSE) || (block->size != 0)){
        if(bf != block->bfree){
            printf("Fields bfree does not match, %d != %d\n", bf, block->bfree);
            exit(1);
        }
        if(bs != block->bsize){
            printf("Fields bsize does not match, %d != %d\n", bs, block->bsize);
            exit(1);
        }
        if((block->free == TRUE) && (afterBlock->free == TRUE)){
            printf("Two blocks next to each other are free\n");
            printf("block: %p\nafterblock: %p", block, afterBlock);
            exit(1);
        }
        if(block->free == TRUE)
            sizeOfFreeArena += block->size;
        sizeOfArena += HEAD + block->size;
        bf = block->free;
        bs = block->size;
        block = after(block);
        afterBlock = after(block);
    }
    uint total = sizeOfArena + HEAD;
    if(total != tot){
        printf("Size of Arena differ, %d != %d\n", total, tot);
        exit(1);
    }

    printf("Sanity check for arena done, and approved!\n");

    printf("Checking flist...\n");
    uint16_t sizeOfList = 0;
    block = flist;
    struct head *beforeBlock = block->prev;
    while(block != NULL){

        sizeOfList += block->size;
        if((block->size % ALIGN) != 0){
            printf("There is a block in flist that is not a multiple of 8\n");
            printf("Location of block is %p", block);
            exit(1);
        }
        if(block->prev != beforeBlock){
            printf("Wrong previous!\n");
            printf("block->prev: %p, beforeBlock: %p\n", block->prev, beforeBlock);
            exit(1);
        }

        beforeBlock = block;
        block = block->next;
    }
    printf("Sanity check for flist done, and approved!\n");
    printf("Compare the size of flist and arena:\n");
    if(sizeOfList != sizeOfFreeArena){
        printf("Error, size is not the same! %d != %d\n", sizeOfList, sizeOfFreeArena);
        exit(1);
    }else{
        printf("Sizes are the same, %d == %d\n", sizeOfList, sizeOfFreeArena);
    }
    printf("Sanity check for the sizes, approved!\n\n\n");
}

void checkSizeOfFreeList(){
    uint16_t freeSpace = 0;
    struct head *block = flist;
    printf("Space freed at (%p)\t %d\n", block, block->size);
    while(block != NULL){
        freeSpace += block->size;
        block = block->next;
    }
    printf("Space allocated: %lu\n", (1024*64-(2*HEAD))-freeSpace);
    printf("Total space free %d\n\n", freeSpace);
}
void checkFreeSpace(){

    struct head *block = flist;
    printf("Total space: %d\n\n", block->size);

}

void checkSizeOfBlocks(){
    int16_t freeSpace = 0;
    struct head *block = flist;
    while(block != NULL){
        freeSpace += block->size;
        block = block->next;
    }
    block = flist;
    printf("%d\n", freeSpace);
}

int checkLength(){
    int length = 0;
    if(flist == NULL){
        return length;
    }else{
        length++;
    }
    struct head *block = flist->next;
    
    while(block!=NULL){
        length ++;
        block = block->next;
    }
    return length;
}

int checkAvgLength(){
    int length = 0;
    int size = 0;
    if(flist == NULL){
        return length;
    }else{
        length++;
    }
    struct head *block = flist->next;
    
    while(block!=NULL){
        length ++;
        size += block->size;
        block = block->next;
    }
    return (size/length);
}

