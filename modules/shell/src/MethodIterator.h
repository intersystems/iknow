#ifndef IKNOW_SHELL_METHOD_ITERATOR_H_
#define IKNOW_SHELL_METHOD_ITERATOR_H_
#include <iterator>

/*
  The MethodIterator template is designed to convert COS-style NextKey/GetKey (i.e. $ORDER)
  collection iteration into an STL iterator. It does this through a pair of pointers-to-member-function.
  Its chief limitation is in maintaining a copy of the returned value of the Get* method for the
  dereference operators: The original value cannot be affected, so this is effectively a const iterator.
  (But I didn't make it a const iterator because CacheList, one of the main values we're interested in iterating,
  lacks useful const lookup methods).

  An even more clever implementation could perhaps return an object that behaves as a reference, e.g.
  a CacheRef<ValT> with a SetMemberFunc available to handle operator=. Unneeded here, though, as
  language model loading is read-only.

  The initially constructed iterator is the 'end' or null iterator, with a default constructed
  key type (typically 0 or ""). Increment it to get a "begin" iterator on the collection.
*/
namespace iknow {
  namespace shell {
    template<typename ValT, typename ClassT, typename KeyT = int>
    class MethodIterator : public std::iterator<std::forward_iterator_tag, ValT> {
    public:
      typedef KeyT (ClassT::*NextMemberFunc)(KeyT k);
      typedef ValT (ClassT::*GetMemberFunc)(KeyT k);
      MethodIterator(ClassT* collection, NextMemberFunc next, GetMemberFunc get) :
	collection_(collection),
	next_(next),
	get_(get),
	key_(KeyT()),
	val_(ValT()) {}
      //Default construction is null (req. by forward iterator)
      MethodIterator() :
	collection_(0),
	next_(0),
	get_(0),
	key_(KeyT()),
	val_(ValT()){}
      bool operator==(const MethodIterator& other) const {
	//comparing iterators from different containers is undefined,
	//so just check the key.
	return key_ == other.key_;
      }
      bool operator!=(const MethodIterator& other) const {
	return !(*this == other);
      }
      //Note that these operators are returning
      //a reference to a *copy* of the method returned
      //value.
      ValT& operator*() const {
	return val_;
      }
      ValT* operator->() const {
	return &val_;
      }
      //recommended by the ParaShift FAQ for readability:
#define CALL_MEMBER_FN(object,ptrToMember)  ((object).*(ptrToMember))
      MethodIterator& operator++() {
	key_ = CALL_MEMBER_FN(*collection_, next_)(key_);
	if (key_) val_ = CALL_MEMBER_FN(*collection_, get_)(key_);
	return *this;
      }
      MethodIterator operator++(int) {
	MethodIterator old(*this);
	++(*this);
	return old;
      }
    private:
      ClassT* collection_; //Must be a pointer to allow null creation.
      NextMemberFunc next_; //function to get next key
      GetMemberFunc get_; //function to read @ key
      KeyT key_; //current key
      mutable ValT val_; //current val
    };
  }
}


#endif //IKNOW_SHELL_METHOD_ITERATOR_H_
