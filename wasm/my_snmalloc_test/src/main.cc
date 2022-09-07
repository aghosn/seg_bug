#include <iostream>
#include <sys/mman.h>
#include <cstdlib>
#include <snmalloc/snmalloc.h>

#define PAGE_SIZE 0x1000
#define NB_PAGES 4
#define ALLOC_SIZE (PAGE_SIZE * NB_PAGES)

void* mmap_region()
{
  void* result = mmap(NULL, ALLOC_SIZE, PROT_READ|PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1 , 0);
  if (result == MAP_FAILED)
    abort();
  return result;
}

int main(void)
{
  std::cout << "Hello world!" << std::endl;
  void* my_region = mmap_region();

  //  TODO: 
  //  I want snmalloc to register my_region and manage allocations to/from it.
  //  Ideally, I'd like to have an allocator object with the API:
  //  class Allocator {
  //    void* alloc(size_t size);
  //    void dealloc(void* ptr);
  //  };
  //  Allocator should go through snmalloc to alloc/dealloc objects from my_region. 
  //
  //  HOW DO I DO THAT WITHOUT WRITTING 10K lines of code?
  return 0;
}
