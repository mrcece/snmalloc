#pragma once

#if (defined(__FreeBSD__) || defined(__OpenBSD__)) && !defined(_KERNEL)
#  include "../ds/bits.h"
#  include "../mem/allocconfig.h"

#  include <stdio.h>
#  include <strings.h>
#  include <sys/mman.h>

namespace snmalloc
{
  class PALBSD
  {
  public:
    /**
     * Bitmap of PalFeatures flags indicating the optional features that this
     * PAL supports.
     */
    static constexpr uint64_t pal_features = 0;
    static void error(const char* const str)
    {
      puts(str);
      abort();
    }

    /// Notify platform that we will not be using these pages
    void notify_not_using(void* p, size_t size) noexcept
    {
      assert(bits::is_aligned_block<OS_PAGE_SIZE>(p, size));
      madvise(p, size, MADV_FREE);
    }

    /// Notify platform that we will be using these pages
    template<ZeroMem zero_mem>
    void notify_using(void* p, size_t size) noexcept
    {
      assert(
        bits::is_aligned_block<OS_PAGE_SIZE>(p, size) || (zero_mem == NoZero));
      if constexpr (zero_mem == YesZero)
      {
        zero(p, size);
      }
      else
      {
        UNUSED(size);
        UNUSED(p);
      }
    }

    /// OS specific function for zeroing memory
    template<bool page_aligned = false>
    void zero(void* p, size_t size) noexcept
    {
      if (page_aligned || bits::is_aligned_block<OS_PAGE_SIZE>(p, size))
      {
        assert(bits::is_aligned_block<OS_PAGE_SIZE>(p, size));
        void* r = mmap(
          p,
          size,
          PROT_READ | PROT_WRITE,
          MAP_PRIVATE | MAP_ANONYMOUS | MAP_FIXED,
          -1,
          0);

        if (r != MAP_FAILED)
          return;
      }

      bzero(p, size);
    }

    template<bool committed>
    void* reserve(const size_t* size) noexcept
    {
      size_t request = *size;

      void* p = mmap(
        nullptr,
        request,
        PROT_READ | PROT_WRITE,
        MAP_PRIVATE | MAP_ANONYMOUS,
        -1,
        0);

      if (p == MAP_FAILED)
        error("Out of memory");

      return p;
    }
  };
} // namespace snmalloc
#endif
