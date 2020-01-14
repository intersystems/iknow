#ifndef IKNOW_BASE_FASTSET_H_
#define IKNOW_BASE_FASTSET_H_
#include <map>
#include <vector>
#include <algorithm>
#include <stdexcept>
#include <string>
#include <bitset>
#include <iterator>
//TODO: TRW, Fix permanently, probably by way of pImpl.
#ifdef _WIN32
#pragma warning( disable: 4251 )
#endif //_WIN32

//A set implementation designed for very fast membership checking on a fairly small domain
namespace iknow {
  namespace base {

    template<unsigned int capacity, typename IndexT = size_t>
    class FastSet {
    public:
      typedef std::bitset<capacity> BitSet;
      typedef IndexT Index;
      typedef std::vector<Index> IndexVector;
      
      
      static size_t Capacity() {
	return capacity;
      }
    
      static Index NPos() {
	return static_cast<Index>(-1);
      }
    
      FastSet() : population_(0) {}
    
      //returns true if the FastSet contains the item referred to
      //by the index value.
      bool Contains(Index index) const {
#ifdef SOLARIS
	//The crummy RogueWave implementation in Sun Studio 12 does not support
	//a const operator[]. This was apparently a late addition to the C++03 standard, while
	//Studio 12 is stuck at C++98.
	//.test() is suboptimal because it performs a range check, while we check all
	//label indexes at language model load time. This is *the* most executed bit of code
	//in the engine by a large margin, so must be as fast as possible.
	return bitset_.test(index);
#else  //!SOLARIS
	return bitset_[index];
#endif //!SOLARIS
      }

      bool Contains(const BitSet& values) const {
	return (values & bitset_) == values;
      }
     
      //A faster insertion that requires that the given index already
      //correspond to some object
      void InsertAtIndex(const Index index) {
	//if (index >= bitset_.size()) bitset_.resize((index + 4) * 2); //TODO: TRW, test other resize strategies
	//if (!bitset_[index]) ++population_;
	bitset_.set(index);
      }
      
      //Removes the item at the given index.
      void RemoveAtIndex(const Index index) {
	if (bitset_[index]) --population_;
	bitset_.reset(index);
      }

      //Inserts all values from the other fast set
      void InsertAll(const FastSet& other) {
	bitset_ |= other.bitset_;
      }

      template<typename SetT>
      void InsertAll(const SetT& other) {
	//Really, this is specific to SmallSet, and should be moved there
	for (const Index* i = other.immediates_begin(); i != other.immediates_end(); ++i) {
	  if (*i != SetT::NPos()) InsertAtIndex(*i);
	}
	if (!other.has_rest()) return;
	for (typename IndexVector::const_iterator i = other.rest_begin(); i != other.rest_end(); ++i) {
	  InsertAtIndex(*i);
	}
      }

      //return the number of items in this set (bits set to 1)
      //Index Size() const {
      //return population_;
      //}

      //returns the index of the item at pos in this set
      Index At(size_t pos) const {
	//probably a "cleaner" way to do this with algorithms, but this
	//should be cheap.
	size_t max = bitset_.size();
	for (size_t i=0; i < max; ++i) {
	  if (this->Contains(i)) {
	    if (pos == 0) return i;
	    --pos;
	  }
	}
	throw std::out_of_range("FastSet At() call couldn't find a value at that position.");
      }

      //removes all items from this set
      void Clear() {
	bitset_.reset();
	population_ = 0;
      }

      std::string DebugString() {
	std::string output;
	output.reserve(bitset_.size());
	for (size_t i = 0; i < bitset_.size(); ++i) {
	  output += bitset_[i] ? "1" : "0"; 
	}
	return output;
      }

    private:
      BitSet bitset_;
      Index population_;
    };


    //TODO: Could be speeded up by knowing the count and stopping when we hit it?
    template<size_t immediate_size, typename IndexT>
    class SmallSetIterator : public std::iterator<std::input_iterator_tag, IndexT> {
    public:
      SmallSetIterator(size_t offset, const IndexT* immediates,
		       const IndexT* rest_begin, const IndexT* rest_end) :
	offset_(offset), immediates_(immediates),
	rest_begin_(rest_begin), rest_end_(rest_end) {
	//The iterator always must point to a valid value or the end,
	//not an NPos "hole"
	AdvanceToValueOrEnd();
      }

      SmallSetIterator(const SmallSetIterator& other) :
      offset_(other.offset_), immediates_(other.immediates_),
      rest_begin_(other.rest_begin_), rest_end_(other.rest_end_) {}

      SmallSetIterator& operator=(const SmallSetIterator& other) {
	offset_ = other.offset_;
	immediates_ = other.immediates_;
	rest_begin_ = other.rest_begin_;
	rest_end_ = other.rest_end_;
      }

      bool operator==(const SmallSetIterator& other) const {
	//By the standard, only meaningful on the same sequence.
	return offset_ == other.offset_;
      }

      bool operator!=(const SmallSetIterator& other) const {
	return !(*this == other);
      }

      IndexT operator*() const {
	return *(GetPointer());
      }

      SmallSetIterator& operator++() {
	++offset_;
	//Advance until we're at the end or not the NPos "hole"
	AdvanceToValueOrEnd();
	return *this;
      }

      SmallSetIterator operator++(int) {
	SmallSetIterator orig(*this);
	++(*this);
	return orig;
      }

    private:
      static IndexT NPos() {
	return static_cast<IndexT>(-1);
      }

      bool AtEnd() const {
	return GetPointer() == rest_end_;
      }
      
      void AdvanceToValueOrEnd() {
	while (!AtEnd() && *(GetPointer()) == NPos()) {
	  ++offset_;
	}
      }

      const IndexT* GetPointer() const {
	if (offset_ < immediate_size) {
	  return immediates_ + offset_;
	}
	return rest_begin_ + (offset_ - immediate_size);
      }

      size_t offset_;
      const IndexT* immediates_;
      const IndexT* rest_begin_;
      const IndexT* rest_end_;
    };


    //Implements the same interface as FastSet, but uses a small, "immediate" array
    //of indices for storage, followed by an optional vector of more.
    template<unsigned int immediate_size = 4, typename IndexT = size_t>
    class SmallSet {
    public:
      typedef IndexT Index;
      typedef SmallSetIterator<immediate_size, IndexT> const_iter;
      static size_t Capacity() {
	//Effectively infinite, but leave some room at the high end
	//for "special" marker values.
	return static_cast<size_t>(-100);
      }
      static Index NPos() {
	return static_cast<Index>(-1);
      }
      SmallSet() : rest_(0) {
	std::fill(immediates_begin(), immediates_end(), NPos());
      }

      ~SmallSet() {
	if (rest_) delete rest_;
      }

      SmallSet(const SmallSet& other) : rest_(0) {
	std::copy(other.immediates_begin(), other.immediates_end(), immediates_begin());
	if (other.rest_) {
	  rest_ = new IndexVector(*other.rest_);
	}
      }

      SmallSet& operator=(const SmallSet& other) {
	if (this == &other) return *this;
	std::copy(other.immediates_begin(), other.immediates_end(), immediates_begin());
	if (rest_) {
	  delete rest_;
	  rest_ = 0;
	}
	if (other.rest_) {
	  rest_ = new IndexVector(*other.rest_);
	}
	return *this;
      }
      //TODO: A move constructor/assignment operator

      bool Contains(Index index) const {
	return
	  //In the immediate array?
	  (std::find(immediates_begin(), immediates_end(), index) != immediates_end()) ||
	  //In the rest, if present?
	  (rest_ && (std::find(rest_->begin(), rest_->end(), index) != rest_->end()));
      }

      void InsertAtIndex(Index index) {
	if (Contains(index)) return; //Optimize to avoid scanning twice?
	//Space in the immediate array?
	Index* empty_slot = std::find(immediates_begin(), immediates_end(), NPos());
	if (empty_slot != immediates_end()) {
	  *empty_slot = index;
	  return;
	}
	//Add to rest vector
	//Create rest vector if needed
	if (!rest_) {
	  rest_ = new IndexVector(1, index);
	  return;
	}
	rest_->push_back(index);
      }

      void RemoveAtIndex(Index index) {
	//In the immediate array?
	Index* slot = std::find(immediates_begin(), immediates_end(), index);
	if (slot != immediates_end()) {
	  *slot = -1;
	  return;
	}
	if (!rest_) return; //nowhere else to remove it
	typename IndexVector::iterator i = std::find(rest_->begin(), rest_->end(), index);
	if (i == rest_->end()) return; //not present, nop
	rest_->erase(i);
      }
      
      template<unsigned int other_size>
      void InsertAll(const SmallSet<other_size, Index>& other) {
	for (const Index* i = other.immediates_begin(); i != other.immediates_end(); ++i) {
	  if (*i != NPos()) InsertAtIndex(*i);
	}
	if (!other.has_rest()) return;
	for (typename IndexVector::const_iterator i = other.rest_begin(); i != other.rest_end(); ++i) {
	  InsertAtIndex(*i);
	}
      }
      
      bool Empty() const {
	for (const Index* i = immediates_begin(); i != immediates_end(); ++i) {
	  if (*i != NPos()) return false;
	}
	return !rest_ || rest_->empty();
      }

      Index Size() const {
	size_t npos_count = 0;
#ifdef SOLARIS
	//Again compensate for the ancient Rogue Wave STL implementation.
	std::count(immediates_begin(), immediates_end(), NPos(), npos_count);
#else
	npos_count = std::count(immediates_begin(), immediates_end(), NPos());
#endif
	return static_cast<Index>(immediate_size - npos_count +
				  //Size of rest, if present
				  (rest_ ? rest_->size() : 0));
      }
    
      Index At(size_t pos) const {
	for (size_t i = 0; i < immediate_size; ++i) {
	  if (immediates_[i] != NPos() && pos-- == 0) return immediates_[i];  
	}
	if (rest_) {
	  for (size_t i = 0; i < rest_->size(); ++i) {
	    //TODO: Don't need to check for NPos() here.
	    if ((*rest_)[i] != NPos() && pos-- == 0) return (*rest_)[i];
	  }
	}
	throw std::out_of_range("SmallSet At() call couldn't find a value at that position.");
      }
      
      void Clear() {
	std::fill(immediates_begin(), immediates_end(), NPos());
	if (rest_) {
	  delete rest_;
	  rest_ = 0;
	}
      }

      const_iter begin() const {
	if (has_rest()) {
	  return const_iter(0, immediates_begin(), &(*rest_begin()), &(*rest_begin()) + rest_->size());
	}
	else {
	  return const_iter(0, immediates_begin(), 0, 0);
	}
      }
      
      const_iter end() const {
	if (has_rest()) {
	  return const_iter(immediate_size + rest_->size(), immediates_begin(),
			    &(*rest_begin()), &(*rest_begin()) + rest_->size());
	}
	else {
	  return const_iter(immediate_size, immediates_begin(),
			    0, 0);
	}
      }

      Index* immediates_begin() {
	return &immediates_[0];
      }
      Index* immediates_end() {
	return &immediates_[immediate_size];
      }
      const Index* immediates_begin() const {
	return &immediates_[0];
      }
      const Index* immediates_end() const {
	return &immediates_[immediate_size];
      }
      bool has_rest() const {
	return rest_ ? !rest_->empty() : false;
      }
      typedef std::vector<Index> IndexVector;
      typename IndexVector::iterator rest_begin() {
	return rest_->begin();
      }
      typename IndexVector::iterator rest_end() {
	return rest_->end();
      }
      typename IndexVector::const_iterator rest_begin() const {
	return rest_->begin();
      } 
      typename IndexVector::const_iterator rest_end() const {
	return rest_->end();
      }
    private:

      Index immediates_[immediate_size];
      IndexVector* rest_;
    };
  }
}

#endif //IKNOW_BASE_FASTSET_H_
