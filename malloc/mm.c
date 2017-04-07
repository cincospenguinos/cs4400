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
#define ALIGN(size) (((size) + (ALIGNMENT-1)) & ~(ALIGNMENT-1))
#define PAGE_ALIGN(size) (((size) + (mem_pagesize()-1)) & ~(mem_pagesize()-1))

#define DEFAULT_PAGE_SIZE PAGE_ALIGN(2 * mem_pagesize())

typedef struct page_header {
  struct page_header *next;
} page_header;

typedef size_t block_header;

// Some defs to help us navigate and manage things
#define BLOCK_SIZE(blk)(*blk & 0xfffffffffffffff8)
#define ALLOCATED(blk)(*blk & 0x01)
#define TERMINATOR(blk)(BLOCK_SIZE(blk) == 0 && ALLOCATED(blk))

#define FIRST_BLOCK(page) ((block_header*)&page[1])
#define NEXT_BLOCK(blk)((block_header*)&blk[(BLOCK_SIZE(blk) + 8) / sizeof(block_header)]) // +8 for header
#define PAYLOAD_OF(blk)((void*) &blk[1])
// Functions to make our lives easier
static page_header* new_page(size_t, page_header*);
static void set_block(block_header*, size_t, int);
static block_header* allocate_block(block_header*, size_t);

// Functions to make debugging easier
static void mem_check();
static void wr();

static char buffer[32];
static int page_count = 0;
static int block_count = 0;

// The start of our list
static page_header *first_page;

/* 
 * mm_init - initialize the malloc package.
 */
int mm_init(void)
{
  first_page = new_page(DEFAULT_PAGE_SIZE, NULL);
  page_count = 0;
  
  if(first_page == NULL)
    return -1;
  
  mem_check();
  return 0;
}

/* 
 * mm_malloc - Allocate a block by using bytes from current_avail,
 *     grabbing a new page if necessary.
 */
void* mm_malloc(size_t size)
{
  size = ALIGN(size) + 16; // To ensure space for the headers
  void *mem; // What we will be returning
  page_header *pg = first_page;

  while(pg != NULL) {
    block_header *blk = FIRST_BLOCK(pg);

    while(!(TERMINATOR(blk))){
      if(BLOCK_SIZE(blk) > size){
	mem = PAYLOAD_OF(allocate_block(blk, size));
	return mem;
      }


      blk = NEXT_BLOCK(blk);
    }

    pg = pg->next;
  }

  mem_check();
  return NULL;
}

/*
 * mm_free - Freeing a block does nothing.
 */
void mm_free(void *ptr)
{
  // TODO
  mem_check();
}

/* Returns the page header of the new page */
static page_header* new_page(size_t size, page_header *old) {
  size = PAGE_ALIGN(size);
  void *ptr = mem_map(size);

  if (ptr == NULL)
    return NULL;

  page_header *next = &((page_header*)ptr)[0];

  if (old != NULL)
    old->next = next;

  next->next = NULL;

  block_header *first = FIRST_BLOCK(next);
  set_block(first, size - 16, 0); // -16 for page_header and terminating block
  block_header *term = NEXT_BLOCK(first);
  set_block(term, 0, 1);

  if(!(TERMINATOR(NEXT_BLOCK(first)))){
    sprintf(buffer, "new_page does not size properly!\n");
    wr();
    exit(1);
  }

  sprintf(buffer, "Terminator at %p\n", term);
  wr();

  // Debugging
  page_count++;
  block_count++;

  return next;
}

static void set_block(block_header *hdr, size_t size, int alloc) {
  *hdr = size;
  *hdr += alloc;
}

/* Allocates the block provided with the size provided */
static block_header* allocate_block(block_header *hdr, size_t size){
  size_t old_size = BLOCK_SIZE(hdr);
  set_block(hdr, size, 1);

  block_header *nxt = NEXT_BLOCK(hdr);
  set_block(nxt, old_size - BLOCK_SIZE(hdr) - 8, 0); // Why did we have to subtract 8?

  if(!(TERMINATOR(NEXT_BLOCK(nxt)))){
    sprintf(buffer, "nxt does not point to terminator!\n");
    wr();
    sprintf(buffer, "points to %p\n", NEXT_BLOCK(nxt));
    wr();
    exit(1);
  }

  block_count++;
  mem_check();

  return hdr;
}


/*
 * Debugging functions
 */

static void wr(){
  write(1, buffer, strlen(buffer));
}

static void error_info(){
  page_header *pg = first_page;
  int b = 0;
  while(pg != NULL){
    sprintf(buffer, "PAGE %p\n", pg);
    wr();
    
    block_header *blk = FIRST_BLOCK(pg);
    while(!(TERMINATOR(blk)) && b <= block_count){
      sprintf(buffer, "\t%p\n", blk);
      wr();
      blk = NEXT_BLOCK(blk);
      b++;
    }

    pg = pg->next;
  }
}

static void mem_check(){
  page_header *pg = first_page;
  int pages = 0;
  int blocks = 0;

  while(pg != NULL) {
    if(pages > page_count){
      sprintf(buffer, "Page count error!\n");
      wr();
      exit(1);
    }

    block_header *blk = FIRST_BLOCK(pg);

    while(!(TERMINATOR(blk))){

      // Ensure we don't pass the terminator
      if(blocks > block_count){
	sprintf(buffer, "Block count error!\n");
	wr();
	exit(1);
      }

      // Ensure that we are aligned properly
      if(((long)(blk)) % 16 != 8){
	sprintf(buffer, "Block alignment error!\n");
	wr();
	sprintf(buffer, "Block at %p\n", blk);
	wr();
	error_info();
	exit(1);
      }


      block_count++;
      blk = NEXT_BLOCK(blk);
    }

    pg = pg->next;
    pages++;
  }
}
