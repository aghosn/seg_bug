#include <cstdint>
#include <iostream>
#include <sys/mman.h>
#include <cstdlib>
#include <snmalloc/snmalloc.h>
#include <snmalloc/backend/fixedglobalconfig.h>

#define PAGE_SIZE 0x1000
#define NB_PAGES 4
#define ALLOC_SIZE (1 << 28)

using namespace snmalloc;

using CustomGlobals = FixedRangeConfig<PALNoAlloc<DefaultPal>>;
using FixedAlloc = LocalAllocator<CustomGlobals>;


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
  DefaultPal::notify_using<NoZero>(my_region, ALLOC_SIZE);
  CustomGlobals::init(nullptr, my_region, ALLOC_SIZE);
  FixedAlloc allocator;

  void* object = allocator.alloc(sizeof(int));
  if (object == nullptr)
    abort();
  if ((uint64_t)(object) < (uint64_t)(my_region))
    abort();
  if ((uint64_t)(object) > ((uint64_t)(my_region) + ALLOC_SIZE))
    abort();
  return 0;
}
