#ifndef IKNOW_BASE_INDEXMAP_H_
#define IKNOW_BASE_INDEXMAP_H_
//A class template for a map which maintains integer ids and metadata.
//
//Originally intended to track entity occurrences in a format convenient for direct output.
// http://turbo.iscinternal.com/twiki/bin/view/ISC/IKnowDOInterfaceSpecs
#include <vector>

#include "utlExceptionFrom.h"
#include "PoolAllocator.h"
#include <unordered_map>

namespace iknow {
  namespace base {
    template<typename KeyT, typename MetadataT = void *, typename HashT = std::hash<KeyT>>
    class IndexMap {
    public:
      typedef size_t index_t;
    private:
      typedef std::vector<KeyT, PoolAllocator<KeyT> > DataVector;
      typedef std::vector<MetadataT, PoolAllocator<MetadataT> > MetadataVector;
      typedef typename std::unordered_map<KeyT, index_t, HashT> IdMap;
    public:
      typedef typename DataVector::const_iterator const_iterator_keys;
      typedef typename MetadataVector::const_iterator const_iterator_metadata;
      IndexMap() {}
      explicit IndexMap(size_t capacity) {
        data_vector_.reserve(capacity);
	metadata_vector_.reserve(capacity);
	id_map_.rehash(capacity);
      }
      index_t Insert(const KeyT& key) {
        typename IdMap::const_iterator i = id_map_.find(key);
        if (i != id_map_.end()) return i->second; //already exists, return index
          // doesn't exist, insert
          index_t id = data_vector_.size() + 1; //index ids are 1-based.
          id_map_.insert(typename IdMap::value_type(key, id));
          data_vector_.push_back(key);
          metadata_vector_.push_back(MetadataT());
          return id;
      }
      //This reference is only guaranteed valid while the IndexMap remains unmodified.
      MetadataT& GetMetadata(index_t id) {
        return metadata_vector_.at(id - 1);
      }
      const_iterator_keys begin_keys() const {
        return data_vector_.begin();
      }
      const_iterator_keys end_keys() const {
        return data_vector_.end();
      }
      const_iterator_metadata begin_metadata() const {
        return metadata_vector_.begin();
      }
      const_iterator_metadata end_metadata() const {
        return metadata_vector_.end();
      }
	  size_t Size() const {
        return id_map_.size();
	  }
    private:
      //Two separate vectors for data and metadata.
      //This simplifies iteration over one or the other
      //over a std::pair-based approach.
      DataVector data_vector_;
      MetadataVector metadata_vector_;
      IdMap id_map_;
    };
  }
}
#endif //IKNOW_BASE_INDEXMAP_H_
