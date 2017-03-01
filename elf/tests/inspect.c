#include <stdlib.h>
#include <stdio.h>
#include <string.h>
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
#define FUNC_ARRAY_SIZE 100

/* These were defined for me */
static void check_for_shared_object(Elf64_Ehdr *ehdr);
static void fail(char *reason, int err_code);

/* Structure to hold data on functions */
typedef struct function {
  char *name;
  char *variables[5];
  unsigned int var_count = 0;
}function;

/* My functions */
static void print_functions();
static void get_all_functions(Elf64_Ehdr*);
static void get_dyn_vars(Elf64_Ehdr*);
static function create_func(char*);
static void get_all_vars(Elf64_Ehdr*);

// Variables I care about
static Elf64_Shdr *dynsym;
static Elf64_Shdr *dynstr;
static Elf64_Shdr *text;

static function funcs[FUNC_ARRAY_SIZE];
static unsigned int func_index = 0;

int main(int argc, char **argv) {
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

  /* Add a call to your work here */
  get_dyn_vars(ehdr);
  get_all_functions(ehdr);
  get_all_vars(ehdr);
  print_functions();

  return 0;
}

/**
 * Grabs the addresses of .dynstr and .dynsym
 */
static void get_dyn_vars(Elf64_Ehdr *ehdr){
  Elf64_Shdr* section_headers = (void*)ehdr + ehdr->e_shoff; // All the section headers
  char *section_names = (void*)ehdr + section_headers[ehdr->e_shstrndx].sh_offset; // section names
  int i;
  for(i = 0; i < ehdr->e_shnum; i++){
    char *name = section_names + section_headers[i].sh_name;

    if(strcmp(name, ".dynsym") == 0){
      //printf("Found .dynsym.\n");
      dynsym = &section_headers[i];
    } else if(strcmp(name, ".dynstr") == 0){
      //printf("Found .dynstr\n");
      dynstr = &section_headers[i];
    } else if(strcmp(name, ".text") == 0){
      text = &section_headers[i];
    }
  }
}

static void print_functions(){
  int i;
  for(i = 0; i < func_index; i++){
    printf("%s\n", funcs[i].name);
  }
}

static void get_all_functions(Elf64_Ehdr *ehdr){
  Elf64_Sym *syms = AT_SEC(ehdr, dynsym);
  char *strs = AT_SEC(ehdr, dynstr);
  int i, count = dynsym->sh_size / sizeof(Elf64_Sym);

  for (i = 0; i < count; i++) {
    Elf64_Sym sym = syms[i];
    if(ELF64_ST_TYPE(sym.st_info) == STT_FUNC && sym.st_size > 0){
      //printf("%s\n", strs + sym.st_name);
      funcs[func_index] = create_func((char*)(strs + sym.st_name));
      func_index++;
    }
  }
}

static function create_func(char *name){
  function f = { name, 0 };
  return f;
}

static void get_all_vars(Elf64_Ehdr* ehdr){
  /**
   * GAMEPLAN
   *
   * So we need to go into the code section itself and take a look around
   * to figure out what vars we use and stuff. And then we will add them
   * to our collection.
   */
}

//////////////// HELPERS ///////////////////////

static void check_for_shared_object(Elf64_Ehdr *ehdr) {
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

static void fail(char *reason, int err_code) {
  fprintf(stderr, "%s (%d)\n", reason, err_code);
  exit(1);
}
