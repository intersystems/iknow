#include "PoolAllocator.h"

using namespace iknow::base;

Pool* Pool::Default() {
  static Pool pool;
  return &pool;
}

size_t Pool::high_water_ = 0;


