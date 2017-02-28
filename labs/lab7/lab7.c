#include <stdlib.h>
#include <stdio.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <elf.h>

/* Given the in-memory ELF header pointer as `ehdr` and a section
   header pointer as `shdr`, returns a pointer to the memory that
   contains the in-memory content of the section */
#define AT_SEC(ehdr, shdr) ((void *)(ehdr) + (shdr)->sh_offset)

static void print_section_names(Elf64_Ehdr* ehdr);
static void check_for_shared_object(Elf64_Ehdr *ehdr);
static void fail(char *reason, int err_code);
void print_symbols(Elf64_Ehdr* ehdr);
Elf64_Shdr* section_by_name(Elf64_Ehdr *ehdr, char* name);

int main(int argc, char **argv) 
{
  int fd;
  size_t len;
  void *p;
  Elf64_Ehdr *ehdr;

  if (argc != 2)
    fail("expected one file on the command line", 0);

  /* Open the shared-library file */
  fd = open(argv[1], O_RDONLY);
  if (fd == -1)
    fail("could not open file", errno);

  /* Find out how big the file is: */
  len = lseek(fd, 0, SEEK_END);

  /* Map the whole file into memory: */
  p = mmap(NULL, len, PROT_READ, MAP_PRIVATE, fd, 0);
  if (p == (void*)-1)
    fail("mmap failed", errno);

  /* Since the ELF file starts with an ELF header, the in-memory image
     can be cast to a `Elf64_Ehdr *` to inspect it: */
  ehdr = (Elf64_Ehdr *)p;

  /* Check that we have the right kind of file: */
  check_for_shared_object(ehdr);

  /* Part 1 */
  //print_section_names(ehdr);

  Elf64_Shdr *ptr = section_by_name(ehdr, ".text");
  printf("%p\n", ptr);

  return 0;
}


void print_section_names(Elf64_Ehdr* ehdr)
{
  // TODO: FIll in (part 1)
  unsigned int sh_offset = ehdr->e_shoff;
  Elf64_Shdr* shdrs = (void*)ehdr + sh_offset;

  // Offset from beginning of file                                       
  unsigned long shstrtab_offset = shdrs[ehdr->e_shstrndx].sh_offset;

  // Raw address of shstrtab section (beginning of file + offset)             
  char* strings = (void*)ehdr + shstrtab_offset;
  char *tmp = strings;
  int i;
  for (i = 0; i < ehdr->e_shnum; i++)
    printf("%s\n", strings + shdrs[i].sh_name);
}

Elf64_Shdr* section_by_name(Elf64_Ehdr *ehdr, char* name)
{
  // TODO: Fill in on your own time.
  //       This will be tremendously useful on assignment 4.
  Elf64_Shdr *section = NULL;

  unsigned int sh_offset = ehdr->e_shoff;
  Elf64_Shdr *shdrs = (void*)ehdr + sh_offset;
  unsigned long shstrtab_offset = shdrs[ehdr->e_shstrndx].sh_offset;

  char* strings = (void*)ehdr + shstrtab_offset;

  int i;
  for(i = 0; i < ehdr->e_shnum; i++){
    if(strcmp(strings + shdrs[i].sh_name, name) == 0){
      section = &shdrs[i];
      break;
    }
  }

  return section;
}

// Just do a little bit of error-checking.
// Make sure we're dealing with an ELF file.
static void check_for_shared_object(Elf64_Ehdr *ehdr) 
{
  if ((ehdr->e_ident[EI_MAG0] != ELFMAG0)
      || (ehdr->e_ident[EI_MAG1] != ELFMAG1)
      || (ehdr->e_ident[EI_MAG2] != ELFMAG2)
      || (ehdr->e_ident[EI_MAG3] != ELFMAG3))
    fail("not an ELF file", 0);

  if (ehdr->e_ident[EI_CLASS] != ELFCLASS64)
    fail("not a 64-bit ELF file", 0);

  if (ehdr->e_type != ET_DYN)
    fail("not a shared-object file", 0);
}

static void fail(char *reason, int err_code) 
{
  fprintf(stderr, "%s (%d)\n", reason, err_code);
  exit(1);
}
