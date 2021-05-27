#ifndef IKNOW_SHELL_STATICHASHTABLE_H_
#define IKNOW_SHELL_STATICHASHTABLE_H_
#include <utility>
#include <algorithm>
#include <functional>
#include <map>
#include "RawBlock.h"
#include "OffsetPtr.h"
#include "Utility.h"
#include <string>
#include <vector>
#include <cstring> //for memcmp

//A lookup-only hash table, meant to be based on the RawBlock abstraction.
//
// The Key/Value types are a little funky. Both have to something we can store in a RawAllocator,
// and in fact the Insert function only takes pointers because of this.
//
// The Key is a string type, but we insert a corresponding CountedString<X> of some variety, since, per above,
// it has to be in a RawAllocator already, and it has to be convertable into a regular
// string for hashing by boost::hash.
//
//
namespace iknow {
  namespace shell {
    namespace StaticHash {

      template<typename KeyT, typename ValueT>
      class Pair {
      public:
	typedef typename KeyT::value_type char_t;
	typedef CountedString<char_t> CountedStringT;
	OffsetPtr<const CountedStringT> first;
	OffsetPtr<const ValueT> second;
	Pair(const CountedStringT* f, const ValueT* s) : first(f), second(s) {}
      };

      template<typename KeyT, typename ValueT>
      struct match_first {
	typedef typename KeyT::value_type char_t;
	match_first(const KeyT& key) : key_(key) {}
	const KeyT& key_;
        bool operator()(const Pair<KeyT, ValueT>& pair) {
          return
	    key_.size() == pair.first->size &&
	    memcmp(key_.data(), pair.first->data(), key_.size() * sizeof(char_t)) == 0;
        }
      private:
      void operator=(const match_first&);
      };


      template<typename KeyT, typename ValueT, typename It>
      struct match_first_range {
	typedef typename KeyT::value_type char_t;
	match_first_range(const It begin, const It end) : begin_(begin), end_(end) {}
	const It begin_;
	const It end_;
        bool operator()(const Pair<KeyT, ValueT>& pair) {
          return
	    end_ - begin_ == pair.first->size &&
	    memcmp(begin_, pair.first->data(), (end_ - begin_) * sizeof(char_t)) == 0;
        }
      private:
        void operator=(const match_first_range&);
      };

      template<typename KeyT, typename ValueT>
      class Bucket {
      public:
	typedef Pair<KeyT,ValueT> PairT;
        Bucket(const PairT* begin, size_t size) : begin_(begin), end_(begin + size) {}
        template<typename It>
	const PairT* Find(const It begin, const It end) const {
	  return std::find_if(Begin(), End(), match_first_range<KeyT, ValueT, It>(begin, end));
	}
	const PairT* Find(const KeyT& key) const {
          return std::find_if(Begin(), End(), match_first<KeyT, ValueT>(key));
        }
        const PairT* Begin() const { return begin_; }
        const PairT* End() const { return end_; }
      private:
        OffsetPtr<const PairT> begin_;
        OffsetPtr<const PairT> end_;
      };

      template<typename KeyT, typename ValueT>
      class Table {
      public:
		typedef Bucket<KeyT,ValueT> BucketT;
		typedef Pair<KeyT,ValueT> PairT;
		Table() : begin_((iknow::shell::OffsetT)0), end_((iknow::shell::OffsetT)0) {}
		Table(const BucketT* begin, size_t size) : begin_(begin), end_(begin + size) {}
		Table(const Table& other) { // explicit copy constructor to avoid random bytes (due to type alignment).
			this->begin_ = other.begin_;
			this->end_ = other.end_;
		}
	//Look up a value by a range containing the key; useful when
	//you want to use, say, a substring of an existing string without building
	//a new one.
	template<typename It>
	const ValueT* Lookup(const It begin, const It end) const {
	  const BucketT* bucket = FindBucket(begin, end);
          if (bucket == End()) return 0;
	  const PairT* pair = bucket->Find(begin, end);
	  if (pair == bucket->End()) return 0;
	  return pair->second;
	} 
	const ValueT* Lookup(const KeyT& key) const {
          const BucketT* bucket = FindBucket(key);
          if (bucket == End()) return 0;
          const PairT* pair = bucket->Find(key);
          if (pair == bucket->End()) return 0;
          return pair->second;
        }
	template<typename It>
	const BucketT* FindBucket(const It begin, const It end) const {
	  if (Begin() == End()) return End(); //empty table
	  return Begin() + (stable_hash::hash_range(begin, end) % (End() - Begin()));
	}
        const BucketT* FindBucket(const KeyT& key) const {
          return FindBucket(key.begin(), key.end());
        }
        const BucketT* Begin() const { return begin_; }
        const BucketT* End() const { return end_; }
      private:
        OffsetPtr<const BucketT> begin_;
        OffsetPtr<const BucketT> end_;
      };

      template<typename KeyT, typename ValueT>
      class Builder {
      public:
	typedef typename KeyT::value_type char_t;
	typedef Bucket<KeyT, ValueT> BucketT;
	typedef Pair<KeyT, ValueT> PairT;
	typedef CountedString<char_t> CountedStringT;
	typedef Table<KeyT,ValueT> TableT;
	Builder(size_t bucket_count) : bucket_count_(bucket_count) {}
	void Insert(const CountedStringT* key, const ValueT* value) {
		map_.insert(typename Map::value_type(stable_hash::hash_range(key->begin(), key->end()) % bucket_count_, std::make_pair(key, value)));
	}
	TableT Build(RawAllocator& allocator) {
	  std::vector<PairT> pairs_vector;
	  pairs_vector.reserve(map_.size());
	  for (typename Map::const_iterator i = map_.begin(); i != map_.end(); ++i) {
	    pairs_vector.push_back(PairT(i->second.first, i->second.second));
	  }
	  const PairT* first_pair = 0;
	  for (typename std::vector<PairT>::const_iterator i = pairs_vector.begin(); i != pairs_vector.end(); ++i) {
	    const PairT* pair = allocator.Insert(*i); //replace with algorithm?
	    if (!first_pair) first_pair = pair; //save the first one.
	  }
	  const PairT* pair = first_pair;
	  const BucketT* first_bucket = 0;
	  for (size_t i=0; i < bucket_count_; ++i) {
	    size_t bucket_size = map_.count(i);
	    BucketT bucket(pair, bucket_size);
	    const BucketT* new_bucket = allocator.Insert(bucket);
	    if (!first_bucket) first_bucket = new_bucket;
	    pair += bucket_size;
	  }
	  return TableT(first_bucket, bucket_count_);
	}
      private:
	size_t bucket_count_;
	//map from hash keys to values
	typedef typename std::multimap<size_t, std::pair<const CountedStringT*,const ValueT*> > Map;
	Map map_;
      };
    }
  }
}


#endif //IKNOW_SHELL_STATICHASHTABLE_H_
