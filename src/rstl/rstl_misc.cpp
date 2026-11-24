#include "Kyoto/Alloc/CMemory.hpp"
#include "rstl/rc_ptr.hpp"
#include "rstl/rmemory_allocator.hpp"

int rstl::rc_ptr_private::sNull = 0x00FFFFFF;

void* rstl::rmemory_allocator::allocate(int size) {
  return size == 0 ? 0 : rs_new_line(36) unsigned char[size];
}
