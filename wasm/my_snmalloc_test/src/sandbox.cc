#include "snmalloc/mem/localalloc.h"
#include <iostream>
#include <snmalloc/snmalloc.h>

using namespace snmalloc;

void no_op_register_clean_up()
{
  SNMALLOC_CHECK(0 && "Should never be called!");
}

using NoOpPal = PALNoAlloc<DefaultPal>;

struct ArenaMap
{
  CapPtr<void, CBArena> arena_root;
  template<typename T = void, typename U, capptr_bounds B>
  SNMALLOC_FAST_PATH CapPtr<T, CBArena> capptr_amplify(CapPtr<U, B> r)
  {
    return Aal::capptr_rebound<T>(arena_root, r);
  }
};

using NoOpMemoryProvider = ChunkAllocator<NoOpPal, ArenaMap>;

using ExternalCoreAlloc = Allocator<NoOpMemoryProvider, SNMALLOC_DEFAULT_CHUNKMAP, false>;
using ExternalAlloc = LocalAllocator<ExternalCoreAlloc, no_op_register_clean_up>;



int main(void)
{
  std::cout << "Hello World" << std::endl;
  return 0;
}
