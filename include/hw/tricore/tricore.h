#ifndef HW_TRICORE_H
#define HW_TRICORE_H

#include "exec/memory.h"

struct tricore_boot_info {
    uint64_t ram_size;
    const char *kernel_filename;
};

struct memory_region {
  const char *name;
  uint32_t start;
  uint32_t end;
  uint32_t size;
};
#endif
