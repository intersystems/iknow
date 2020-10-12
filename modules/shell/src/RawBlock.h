#ifndef IKNOW_SHELL_RAWBLOCK_H_
#define IKNOW_SHELL_RAWBLOCK_H_
#include "utlExceptionFrom.h"
#include <algorithm>
#include <string>
#include <iterator>
#include <cstring>

#include "IkTypes.h"
#include "OffsetPtr.h"

//RawBlocks are an abstraction for storing a series of objects in a
//statically-allocated chunk of memory (like a pre-allocated piece of Cache
//shared memory)
namespace iknow {
  namespace shell {
    class Raw {
    public:
      Raw(unsigned char* begin, size_t size) : begin_(begin), size_(size) {
		  memset(begin, 0, size); // init for a reproducible binary model.
	  }
      unsigned char* Begin() const { return begin_; }
      size_t Size() const { return size_; }
    private:
      unsigned char* begin_;
      size_t size_;
    };
    //TODO: Consolidate duplication in RawString/RawBlock

    template<typename CharT>
    struct CountedString {
    public:
      typedef unsigned short size_t;
      static size_t MaxLength() {
	return static_cast<size_t>(-1);
      }
      typedef CharT char_t;
      typedef std::basic_string<char_t> StringT;
      size_t size; //the first element
      char_t c[1]; //character(s)
      operator StringT() const {
	return StringT((const char_t*)&c[0], (::size_t)size);
      }
      const char_t* data() const {
	return &c[0];
      }
      const char_t* begin() const {
	return data();
      }
      const char_t* end() const {
	return begin() + size;
      }
    };

    typedef CountedString<iknow::base::Char> CountedBaseString;
    typedef CountedString<char> CountedCharString;

    class RawAllocator {
    public:
      RawAllocator(const Raw& raw) : raw_(raw), allocated_(0) {}

      template<typename T>
      T* Insert(const T& t) {
	size_t new_allocated = allocated_ + AlignmentNeeded<T>() + SpaceRequired<T>();
	if (new_allocated > raw_.Size()) 
	  throw ExceptionFrom<RawAllocator>("RawAllocator has insufficient space for insertion.");
	allocated_ += AlignmentNeeded<T>(); //add bytes for alignment
	T* obj = new(raw_.Begin() + allocated_) T(t);
	allocated_ = new_allocated;
	return obj;
      }
#if defined(SOLARIS)
      //Sun Cstd uses raw pointers for iterators and doesn't support
      //iterator_traits.
      template<typename T>
      T* InsertRange(const T* begin, const T* end) {
	size_t new_allocated = allocated_ + AlignmentNeeded<T>() + ((end - begin) * SpaceRequired<T>());
	if (new_allocated > raw_.Size())
	  throw ExceptionFrom<RawAllocator>("RawAllocator has insufficient space for insertion.");
	allocated_ += AlignmentNeeded<T>(); //add bytes for alignment (only once)
	T* obj = reinterpret_cast<T*>(raw_.Begin() + allocated_);
	for (const T* i = begin; i != end; ++i) {
	  new(raw_.Begin() + allocated_) T(*i);
	  allocated_ += SpaceRequired<T>();  
	}
	return obj;
      }
#else
      //This used to just overload Insert, but the awful Sun Studio
      //compiler tries to instantiate the other template.
      template<typename TIter>
	typename std::iterator_traits<TIter>::value_type* InsertRange(TIter begin, TIter end) {
	typedef typename std::iterator_traits<TIter>::value_type T;
	size_t new_allocated = allocated_ + AlignmentNeeded<T>() + ((end - begin) * SpaceRequired<T>());
	if (new_allocated > raw_.Size())
	  throw ExceptionFrom<RawAllocator>("RawAllocator has insufficient space for insertion.");
	allocated_ += AlignmentNeeded<T>(); //add bytes for alignment (only once)
	T* obj = reinterpret_cast<T*>(raw_.Begin() + allocated_);
	for (TIter i = begin; i != end; ++i) {
	  new(raw_.Begin() + allocated_) T(*i);
	  allocated_ += SpaceRequired<T>();  
	}
	return obj;
      }
#endif
      template<typename StringT>
      CountedString<typename StringT::value_type>* InsertString(const StringT& str) {
	typedef typename StringT::value_type CharT;
	typedef CountedString<CharT> CountedStringT;
	size_t length = str.size();
	if (length > CountedStringT::MaxLength()) {
	  throw ExceptionFrom<RawAllocator>("Can't insert string into raw block. Too long.");
	}
	const CharT* begin = str.data();
	size_t new_allocated = allocated_ + AlignmentNeeded<CountedStringT>() + SpaceRequiredForString<CharT>(length);
	if (new_allocated > raw_.Size()) {
	  throw ExceptionFrom<RawAllocator>("RawAllocator has insufficient space for insertion.");
	}
	allocated_ += AlignmentNeeded<CountedStringT>();
	CountedStringT* counted_string = reinterpret_cast<CountedStringT*>(raw_.Begin() + allocated_);
	CharT* begin_copy = &counted_string->c[0];
	allocated_ += SpaceRequiredForString<CharT>(length);
	counted_string->size = static_cast<typename CountedStringT::size_t>(length);
	std::copy(begin, begin + length, begin_copy);
	return counted_string;
      }
      
      template<typename CharT>
      size_t SpaceRequiredForString(size_t length) {
		typedef CountedString<CharT> CountedStringT;
		if (length == 0) return AlignmentNeeded<typename CountedStringT::size_t>() + sizeof(CountedStringT);
		return AlignmentNeeded<typename CountedStringT::size_t>() + sizeof(CountedStringT) - (sizeof(CountedStringT().c)) + (sizeof(CountedStringT().c) * length);
      }
      template<typename T>
      size_t SpaceRequired() {
		return sizeof(T);
      }
      template<typename T>
      size_t AlignmentNeeded() {
		size_t alignment = alignof(T);
		return allocated_ % alignment ? alignment - allocated_ % alignment : 0;
      }

      size_t BytesUsed() { return allocated_; }
      size_t BytesRemaining() { return raw_.Size() - BytesUsed(); };
	
#ifdef INCLUDE_GENERATE_IMAGE_CODE
	  void generate_image(std::string& dir_name, std::string kb_name) { // data logging
		  std::string image_file(dir_name + "/kb_" + kb_name + "_data.c");
		  std::ofstream ofs = std::ofstream(image_file); // Do ##class(Util).OutputToFile(dir _ "/OneStateMap.inl")
		  if (ofs.is_open()) {
			  std::string data_name = "kb_" + kb_name + "_data[]";
			  ofs << "const unsigned char " << data_name << " = { // memory block representing KB data"; // fprintf(stream, "const unsigned char %s = { // memory block representing KB data", data_name.c_str());
			  ofs << hex; // change to hexadecimal format for writing
			  size_t data_size = BytesUsed();
			  size_t cnt = static_cast<size_t>(0);
			  for (; cnt < data_size - static_cast<size_t>(1); ++cnt) {
				  if (!(cnt % static_cast<size_t>(8))) ofs << "\n\t"; // fprintf(stream, "\n\t");
				  ofs << "0x" << (int)(*(raw_.Begin() + cnt)) << ", "; // fprintf(stream, "0x%x, ", *(raw_.Begin() + cnt));
			  }
			  ofs << "0x" << (int)(*(raw_.Begin() + cnt)) << "\n};\n"; // fprintf(stream, "0x%x\n};\n", *(raw_.Begin() + cnt));
			  ofs.close();
		  } else {
			  std::string error_message("Cannot open " + image_file + " for writing !");
			  std::cerr << error_message << std::endl;
			  throw ExceptionFrom<RawAllocator>(error_message);
		  }
	}
#endif
      
    private:
      Raw raw_;
      size_t allocated_;
    
    };
  }
}

#endif //IKNOW_SHELL_RAWBLOCK_H_
