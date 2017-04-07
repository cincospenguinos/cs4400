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
#define STARTING_CHUNK_SIZE 1 * 4096

/* rounds up to the nearest multiple of ALIGNMENT */
#define ALIGN(size) (((size) + (ALIGNMENT-1)) & ~(ALIGNMENT-1))

/* rounds up to the nearest multiple of mem_pagesize() */
#define PAGE_ALIGN(size) (((size) + (mem_pagesize()-1)) & ~(mem_pagesize()-1))

typedef struct chunk_header {
  struct chunk_header *next;
} chunk_header;

typedef size_t block_header;

/* Functions and definitions for block_headers */ // TODO: Maybe make this a function?
#define OVERHEAD sizeof(block_header)
#define GET_BLK_HDR(payload) ((block_header) ((char *) (payload)  - sizeof(block_header)));

static chunk_header* get_new_chunk(size_t, chunk_header*);
static block_header* first_block_in_chunk(chunk_header*);
static block_header* last_block_in_chunk(chunk_header*, size_t);
static block_header* next_block(block_header*);
static int terminating_block(block_header*);

static block_header* allocate_block(block_header*, size_t);

static int size_of_block(block_header*); // Size of a block
static int allocated(block_header*); // Whether or not the block is allocated
static void set_block(int, size_t, block_header*);

void *current_avail = NULL;
int current_avail_size = 0;

chunk_header *first_chunk; // The first chunk in our list

/* All of this is for debugging */

static char buffer[32]; // To print stuff
static unsigned int chunk_count = 0; // Number of chunks
static unsigned int block_count = 0; // Number of blocks
void validate_memory();
void write_info();
void error_info();

/* 
 * mm_init - initialize the malloc package.
 */
int mm_init(void)
{
  chunk_count = 0; // debugging

  first_chunk = get_new_chunk(PAGE_ALIGN(STARTING_CHUNK_SIZE), NULL);

  if(first_chunk == NULL)
    return -1;

  current_avail_size = PAGE_ALIGN(STARTING_CHUNK_SIZE);
  current_avail = mem_map(current_avail_size);
  
  if(current_avail == NULL)
    return -1;

  return 0;
}

static chunk_header* get_new_chunk(size_t size, chunk_header *old_chunk){
  size = PAGE_ALIGN(size);
  void *chunk = mem_map(size);
  
  if(chunk == NULL)
    return NULL;
  
  chunk_header *c_hdr = &((chunk_header*) chunk)[0];

  if (old_chunk != NULL)
    old_chunk->next = c_hdr;

  c_hdr->next = NULL;

  block_header *first = first_block_in_chunk(c_hdr);
  set_block(0, size - 16, first); // NOTE: the -16 = page header + block header

  block_header *last =  last_block_in_chunk(c_hdr, size);
  set_block(1, 0, last);

  sprintf(buffer, "Terminator at %p\n", last);
  write_info();

  chunk_count++;
  block_count++;

  return c_hdr;
}

/* 
 * mm_malloc - Allocate a block
 */
void *mm_malloc(size_t size)
{
  size = ALIGN(size);
  void *mem = NULL;
  chunk_header *hdr = first_chunk;

  while (hdr != NULL) {
    block_header *blk = first_block_in_chunk(hdr);

    while (!terminating_block(blk)) {
      if(size + OVERHEAD < size_of_block(blk)){
	// TODO: Insert memory here
	allocate_block(blk, size);
	break;
      }
      blk = next_block(blk);
    }

    hdr = hdr->next;

    if (mem != NULL) break;
  }

  
  ///// This is old stuff ///////
  int newsize = ALIGN(size);
  void *p;
  
  if (current_avail_size < newsize) {
    current_avail_size = PAGE_ALIGN(newsize);
    current_avail = mem_map(current_avail_size);
    
    if (current_avail == NULL)
      return NULL;
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
  // TODO
  validate_memory();
}

void validate_memory(){
  chunk_header *chunk_hdr = first_chunk;
  int chunks = 0;
  int blocks = 0;

  while(chunk_hdr != NULL){
    block_header *blk = first_block_in_chunk(chunk_hdr);

    while(!terminating_block(blk)){
      long addr = (long) blk[1];
      
      // Make sure all headers are aligned on 8
      if ((long)(blk) % 16 != 8){
	sprintf(buffer, "Block header alignment error!\n");
	write_info();
	long val = ((long)(blk)) % 16;
	
	sprintf(buffer, "%i was the result\n", val);
	write_info();
	sprintf(buffer, "%p was the address\n", blk);
	write_info();
	error_info();
	exit(1);
      }


      // Make sure payloads always have addresses that end in 16
      if(addr % 16 != 0){
	sprintf(buffer, "Alignment error!\n");
	write_info();
	sprintf(buffer, "expected 0; got %i \n", addr % 16);
	write_info();
	sprintf(buffer, "%i -> %i", size_of_block(blk), allocated(blk));
	write_info();
	exit(1);
      }

      // I wrote this test, but I'm not sure what it does. I should have
      // mentioned what exactly it was supposed to do
      /*
      if(size_of_block(blk) % 16 != 0){
	sprintf(buffer, "Size error!\n");
	write_info();
	sprintf(buffer, "Size was %d\n", size_of_block(blk));
	write_info();
	exit(1);
	}*/

      // Make sure that our block connection is correct
      if(blocks > block_count){
	// If this error fires, it could be due to overlooking the termination block
	// in the current chunk. 
	sprintf(buffer, "Block count error!\n");
	write_info();
	sprintf(buffer, "expected %i; counted %i\n", block_count, blocks);
	write_info();
	exit(1);
      }

      blk = next_block(blk);
      blocks++;
    }

    chunk_hdr = first_chunk->next;
    chunks++;
  }

  // Make sure all chunks are accounted for
  if(chunks != chunk_count){
    sprintf(buffer, "Wrong number of chunks!\n");
    write_info();
    sprintf(buffer, "Have %i; found %i\n", chunk_count, chunks);
    write_info();
    exit(1);
  }
}

/* Allocates the amount of space requested. Returns new block pointer */
static block_header* allocate_block(block_header *blk, size_t size) {
  block_header *next_header = &blk[size / sizeof(block_header)];

  block_header *herp = next_block(blk);
  sprintf(buffer, "Terminator? %d\n", terminating_block(herp));
  write_info();

  size_t old_size = size_of_block(blk);
  set_block(1, size, blk);

  size_t new_hdr_size = old_size - size;
  set_block(0, new_hdr_size, next_header);

  sprintf(buffer, "Old: %d\tNew: %d\tLeft: %d\t\n", old_size, size_of_block(blk), size_of_block(next_header));
  write_info();

  block_header *terminator = next_block(next_header);
  if(!terminating_block(terminator)){
    sprintf(buffer, "Error! terminating block is not there!\n");
    write_info();
    sprintf(buffer, "Calculated: %p\n", terminator);
    write_info();
    error_info();
    exit(1);
  }

  block_count++;

  return blk;
}

void write_info(){
  write(1, buffer, strlen(buffer));
}

void error_info(){
  sprintf(buffer, "\\\\\\\\\\\\\\\\\\\n");
  write_info();
  sprintf(buffer, "Chunks: %i\n", chunk_count);
  write_info();
  sprintf(buffer, "Blocks: %i\n", block_count);
  write_info();
  sprintf(buffer, "\\\\\\\\\\\\\\\\\\\n");
  write_info();
}

static int size_of_block(block_header* hdr){
  return *hdr & 0xfffffff8;
}

static int allocated(block_header* hdr){
  return *hdr & 0x1;
}

static void set_block(int allocated, size_t size, block_header *hdr){
  *hdr = (block_header) size;
  *hdr += allocated;
}

static block_header* first_block_in_chunk(chunk_header *hdr){
  return (block_header*) &hdr[1];
}

static block_header* last_block_in_chunk(chunk_header *hdr, size_t chunk_size){
  return (block_header*) &hdr[chunk_size / sizeof(block_header) - 1];
}

static block_header* next_block(block_header *hdr){
  return &(hdr[size_of_block(hdr) / sizeof(block_header)]);
}

static int terminating_block(block_header *hdr){
  return (size_of_block(hdr) == 0) && allocated(hdr);
}
