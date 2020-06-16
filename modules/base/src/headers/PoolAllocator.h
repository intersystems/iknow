#ifndef IKNOW_BASE_POOLALLOCATOR_H_
#define IKNOW_BASE_POOLALLOCATOR_H_
#include <stddef.h>
#ifdef ISC_IRIS
#include <type_traits>
#endif
#include <algorithm>
#include <vector>
#include <iostream>
#include "IkExportBase.h"
#include "utlExceptionFrom.h"
namespace iknow {
  namespace base {

    class BASE_API Pool {
    public:
      static Pool* Default();
      void Clear() {
	offset_ = 0;
	max_size_ = 0;
	block_size_ = 0;
	for (Blocks::iterator i = pool_.begin(); i != pool_.end(); ++i) {
	  delete[] *i;
	}
	pool_.clear();
      }
      void Reset(size_t block_size, size_t max_size = static_cast<size_t>(-1)) {
	Clear();
	max_size_ = max_size;
	block_size_ = block_size;
	AddBlock();
      }
      template<typename T>
      void* Allocate(size_t n) {
#ifndef SOLARIS
	size_t request_size =  n * sizeof(T);
#else //Solaris's Rogue Wave STL already adds the T multiple
	size_t request_size = n;
#endif

	//It's often useful to examine the largest allocation request made so far.
#ifdef TRW_DEBUG_HIGHWATER
	if (request_size > high_water_) {
	  high_water_ = request_size;
	  std::cout << "\r\n New high water: " << high_water_ << "\r\n";
	  std::cout.flush();
	}
#endif //TRW_DEBUG_HIGHWATER

	//For requests larger than the block_size_, we allocate a custom block sized exactly,
	//then allocate another block for the next request. This will get inefficient if many
	//requests are larger than the block_size_ so should be used only for exceptionally large
	//allocations.
	if (request_size > block_size_) {
	  AddBlock(request_size);
	  void *out = static_cast<void*>(CurrentBlock());
	  AddBlock();
	  return out;
	}

	//Normal handling for sub-block size allocations.
#ifdef ISC_IRIS
	size_t alignment = std::alignment_of<T>::value;
#else
	size_t alignment = alignof(T);
#endif
	size_t padding = offset_ % alignment ? alignment - offset_ % alignment : 0;
	size_t chunk_size = padding + request_size;

	if (offset_ + chunk_size > block_size_) {
	  AddBlock();
	  return Allocate<T>(n);
	}

	void *out  = static_cast<void*>(CurrentBlock() + offset_ + padding);
	offset_ += chunk_size;
	return out;
      }
      size_t MaxSize() const {
	return max_size_;
      }
    private:
      Pool() : pool_(0), max_size_(0), block_size_(0), offset_(0) {}

      //0-arg AddBlock() adds a block of the default size.
      void AddBlock() {
	AddBlock(block_size_);
      }

      void AddBlock(size_t block_size) {
#ifdef HP
	//HP-UX aCC thinks the memory below is leaked.
	//Not clear why.
#pragma diag_suppress 20201 
#endif //HP
	char* block = new char[block_size];
	if (!block) {
	  throw ExceptionFrom<Pool>("new char[] returned a null pointer when allocating a block.");
	}
#ifdef HP
#pragma diag_default 20201
#endif //HP
	pool_.push_back(block);
	offset_ = 0;
      }
      char* CurrentBlock() {
	return pool_.back();
      }
      typedef std::vector<char*> Blocks;
      Blocks pool_;
      size_t max_size_;
      size_t block_size_;
      size_t offset_;
      static size_t high_water_;
    };

    template<typename T>
    class PoolAllocator {
    public:
      typedef size_t size_type;
      typedef ptrdiff_t difference_type;
      typedef T* pointer;
      typedef const T* const_pointer;
      typedef T& reference;
      typedef const T& const_reference;
      typedef T value_type;
      
      template<typename U>
      struct rebind {
	typedef PoolAllocator<U> other;
      };

      PoolAllocator() {}
      
      template<typename U>
      PoolAllocator(const PoolAllocator<U>&) {}

      pointer address(reference r) const { return &r; }
     
      const_pointer address(const_reference r) const { return &r; }

      pointer allocate(size_type n) {
	return allocate(n, 0);
      }

      pointer allocate(size_type n, void*) {
	Pool* pool = Pool::Default();
	return static_cast<pointer>(pool->Allocate<T>(n));
      }
      void deallocate(pointer, size_type) {
	//nop: deallocate when we reset the pool
      }

      void construct(pointer p, const T& val) {
	new(p) T(val);
      }

      void destroy(pointer p) {
	//Bug in VS2008 thinks p isn't used
#ifdef WIN32
	p = p;
#endif
	p->~T();
      }

      size_type max_size() const {
	return Pool::Default()->MaxSize() / sizeof(T);
      }
    
    //Sun's ancient STL implementation expects this overload
    size_type max_size(size_t size) const {
      return Pool::Default()->MaxSize() / size;
    }

      static void Reset(size_type size) {
	Pool::Default()->Reset(size);
      }
      
      static void Clear() {
	Pool::Default()->Clear();
      }
    };


    template<typename T>
    bool operator==(const PoolAllocator<T>&, const PoolAllocator<T>&) {
      return true;
    }

    template<typename T>
    bool operator!=(const PoolAllocator<T>&, const PoolAllocator<T>&) {
      return false;
    }

  }
}

#endif //IKNOW_BASE_POOLALLOCATOR_H_
