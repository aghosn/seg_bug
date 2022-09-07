#include <iostream>
#include <sys/mman.h>
#include <cstdlib>

#include <snmalloc/snmalloc.h>

void* mmap_region()
{
  void* result = mmap(NULL, 0x1000, PROT_READ|PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1 , 0);
  if (result == MAP_FAILED)
    abort();
  return result;
}

int main(void)
{
  std::cout << "Hello world!" << std::endl;
  return 0;
}
