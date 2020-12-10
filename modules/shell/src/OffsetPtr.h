#ifndef IKNOW_SHELL_OFFSETPTR_H_
#define IKNOW_SHELL_OFFSETPTR_H_

//A class for treating offsets from a base pointer as pointers (typically,
//the base pointer is the base of the Cache shared memory heap in this process, i.e. mcom)
namespace iknow {
  namespace shell {
    extern const unsigned char* base_pointer;

    inline const unsigned char*& GetBasePointer() { 
      return base_pointer;
    }
    inline void SetBasePointer(const unsigned char* base) {
      //We actually get a slight performance bump (~1% gcc on my Core i7 Mac) by skipping the
      //write when it's not needed. This might be because the read is essentially free (we're
      //going to look at the base_pointer anyway, else why would we be setting it?) and avoiding
      //the write may be more cache-friendly.
      if (base_pointer != base) base_pointer = base;
    }

    //An RAII class for modifying the base pointer
    //and restoring it when exiting.
    //It neither sets it nor restores it if the value
    //does not need to change.
    class BasePointerFrame {
    public:
        BasePointerFrame(const unsigned char* base) : previous_base_(base_pointer) {
            if (base != previous_base_) base_pointer = base;
        }
        ~BasePointerFrame() {
            if (previous_base_ != base_pointer) base_pointer = previous_base_;
        }
    private:
        const unsigned char* previous_base_;
    };

    typedef unsigned long long OffsetT;

    template<typename T>
    class OffsetPtr {
    public:
      OffsetPtr(T* t) : offset_(reinterpret_cast<const unsigned char*>(const_cast<const T*>(t)) - GetBasePointer()) {}
      OffsetPtr(OffsetT offset) : offset_(offset) {}
      OffsetPtr() : offset_(0) {}
      T& operator*() const {
	    return *reinterpret_cast<T*>(GetBasePointer() + offset_);
      }
      T* operator->() const {
	    return reinterpret_cast<T*>(GetBasePointer() + offset_);
      }
      operator const T*() const {
	    return reinterpret_cast<const T*>(GetBasePointer() + offset_);
      }
      OffsetT offset() const {
	    return offset_;
      }
    private:
      OffsetT offset_;
    };
  }
}

#endif //IKNOW_SHELL_OFFSETPTR_H_
