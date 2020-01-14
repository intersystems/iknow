#ifndef IKNOW_BASE_STRINGPOOL_H_
#define IKNOW_BASE_STRINGPOOL_H_
#include "IkTypes.h"
#include "utlExceptionFrom.h"
#include <vector>
#include <list>

namespace iknow {
  namespace base {
    class StringPool {
    public:
      StringPool(size_t count, size_t string_size) : index_(0), strings_(count, String(string_size, static_cast<Char>(0))) {}
      template<typename IterT>
      String* Allocate(IterT begin, IterT end) {
	    if (index_ == strings_.size()) {
		  // throw ExceptionFrom<StringPool>("Unable to allocate. StringPool full.");
		  stringlist_.push_back(String(begin, end)); // Store in list
		  return &(stringlist_.back());
	    }
        size_t size = end - begin;
        String* s = &(strings_[index_++]);
        if (size > s->capacity()) {
          s->resize(size);
        }
        if (!size) { 
          s->clear();
        } else {
          s->replace(0, s->size(), &*begin, size);
        }
        return s;
      }
      template<typename StringT>
      String* Allocate(const StringT& s) {
        return Allocate(s.begin(), s.end());
      }
      void Reset(size_t count, size_t string_size) {
        index_ = 0;
		stringlist_.clear();
        if (strings_.size() < count || strings_.capacity() > count * 2) strings_.resize(count, String(string_size, static_cast<Char>(0)));
        for (Strings::iterator i = strings_.begin(); i != strings_.end(); ++i) {
          if (i->capacity() < string_size || i->capacity() > string_size * 2) i->resize(string_size);
        }
      }
    private:
      size_t index_;
      typedef std::vector<String> Strings;
      Strings strings_;
	  typedef std::list<String> StringList; // fallback storage for memory allocation problems.
	  StringList stringlist_;
    };

    template<typename T>
    class VectorPool {
    public:
      typedef std::vector<T> Vector;
      typedef std::vector<Vector> Vectors;
      VectorPool(size_t count, size_t vec_size) : index_(0), vectors_(count) {
	for (typename Vectors::iterator i = vectors_.begin(); i != vectors_.end(); ++i) {
	  i->reserve(vec_size);
	}
      }
      void Reset(size_t count, size_t vec_size) {
	index_ = 0;
	vectors_.resize(count);
	for (typename Vectors::iterator i = vectors_.begin(); i != vectors_.end(); ++i) {
	  i->resize(0);
	  i->reserve(vec_size); //TODO: Check to be sure this doesn't reallocate.
	}
      }
      Vector* Allocate() {
	if (index_ == vectors_.size()) throw ExceptionFrom<VectorPool>("Unable to allocate. VectorPool full.");
	return &(vectors_[index_++]);
      }
    private:
      size_t index_;
      Vectors vectors_;
    };
  }
}
#endif //IKNOW_BASE_STRINGPOOL_H_
