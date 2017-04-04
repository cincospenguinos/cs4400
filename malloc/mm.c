/*
 * mm-naive.c - The least memory-efficient malloc package.
 * 
 * In this naive approach, a block is allocated by allocating a
 * new page as needed.  A block is pure payload. There are no headers or
 * footers.  Blocks are never coalesced or reused.
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

/* always use 16-byte alignment */
#define ALIGNMENT 16

/* number of starting chunks */
#define STARTING_CHUNK_SIZE 4096

/* rounds up to the nearest multiple of ALIGNMENT */
#define ALIGN(size) (((size) + (ALIGNMENT-1)) & ~(ALIGNMENT-1))

/* rounds up to the nearest multiple of mem_pagesize() */
#define PAGE_ALIGN(size) (((size) + (mem_pagesize()-1)) & ~(mem_pagesize()-1))

typedef struct chunk_header {
  struct chunk_header *next_chunk;
} chunk_header;

typedef size_t block_header;

/* Functions and definitions for block_headers */ // TODO: Maybe make this a function?
#define GET_BLK_HDR(payload) ((block_header) ((char *) (payload)  - sizeof(block_header)));

int size_of_block(block_header*);
int allocated(block_header*);
void set_block(int allocated, size_t size, block_header *header_ptr);

void *current_avail = NULL;
int current_avail_size = 0;

chunk_header *first_chunk;
//block_header *first_block;

static char buffer[32]; // For debugging

/* Returns a new chunk with the appropriate header */
chunk_header* get_new_chunk(size_t size);

/* Validates whether or not the memory is setup properly */
void validate_memory();

/* Prints to STDOUT */
void write_info(const char[]);

/* 
 * mm_init - initialize the malloc package.
 */
int mm_init(void)
{
  first_chunk = get_new_chunk(PAGE_ALIGN(STARTING_CHUNK_SIZE));

  current_avail_size = PAGE_ALIGN(STARTING_CHUNK_SIZE);
  current_avail = mem_map(current_avail_size);
  
  if(current_avail == NULL)
    return -1;

  return 0;
}

chunk_header* get_new_chunk(size_t size){
  size = PAGE_ALIGN(size);
  void *chunk = mem_map(size);
  
  if(chunk == NULL)
    return NULL;
  
  chunk_header *c_hdr = &((chunk_header*) chunk)[0];

  block_header *first = &((block_header*) chunk)[1];
  set_block(0, size - 24, first);

  block_header *last = ((block_header*) chunk)[size / sizeof(block_header) - 1];
  set_block(1, 0, last);

  return c_hdr;
}

/* 
 * mm_malloc - Allocate a block by using bytes from current_avail,
 *     grabbing a new page if necessary.
 */
void *mm_malloc(size_t size)
{
  static char buffer[] = "                    ";
  sprintf(buffer, "%p\n", &first_chunk);
  //write_info(buffer);

  int newsize = ALIGN(size);
  void *p;
  
  if (current_avail_size < newsize) {
    current_avail_size = PAGE_ALIGN(newsize);
    current_avail = mem_map(current_avail_size);
    
    if (current_avail == NULL)
      return NULL;

    //static char buffer[] = "             \n";
    //sprintf(buffer, "%p\n", current_avail);
    //write_info(buffer);
  }

  p = current_avail;
  current_avail += newsize;
  current_avail_size -= newsize;

  validate_memory();
  
  return p;
}

/*
 * mm_free - Freeing a block does nothing.
 */
void mm_free(void *ptr)
{

  validate_memory();
}

void validate_memory(){
  // TODO: This
}

void write_info(const char str[]){
  write(1, str, strlen(str));
}

int size_of_block(block_header* hdr){
  return *hdr & 0xfffffff8;
}

int allocated(block_header* hdr){
  return *hdr & 0x1;
}

void set_block(int allocated, size_t size, block_header *hdr){
  *hdr = (block_header) size;
  *hdr += allocated;
}
