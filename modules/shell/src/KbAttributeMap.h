#ifndef IKNOW_SHELL_KBATTRIBUTEMAP_H_
#define IKNOW_SHELL_KBATTRIBUTEMAP_H_
#include "StaticHashTable.h"
#include "IkTypes.h"
#include <string>
#include <algorithm>
#include <iterator>
#include <map>

namespace iknow {
  namespace shell {
    typedef unsigned short AttributeId;
    typedef iknow::base::String AttributeValue;
    typedef CountedBaseString StoredAttributeValue;
    struct MapBuilder {
      typedef StaticHash::Builder<AttributeValue, AttributeId> HashBuilder;
      MapBuilder(RawAllocator& allocator, size_t table_size) : allocator_(allocator), current_id_(0), builder_(table_size) {}
      void operator()(const StoredAttributeValue* val) {
	builder_.Insert(val, allocator_.Insert(current_id_++));
      }
      HashBuilder::TableT Build() {
	return builder_.Build(allocator_);
      }
      RawAllocator& allocator_;
      AttributeId current_id_;
      HashBuilder builder_;
    private:
        void operator=(const MapBuilder&);
    };
    struct ValueInserter {
      ValueInserter(RawAllocator& allocator) : allocator_(allocator) {}
      StoredAttributeValue* operator()(const AttributeValue& value) { 
	return allocator_.InsertString(value);
      }
      RawAllocator& allocator_;
    private:
        void operator=(const ValueInserter&);
    };

    class KbAttributeMap {
    public:
      template<typename IterT>
      KbAttributeMap(RawAllocator& allocator, const IterT& values_begin, const IterT& values_end) {
	size_t count = values_end - values_begin;
	//First insert all the values into the allocator block, keeping a vector of where they ended up.
	typedef std::vector<AttributeValuePtr> AttributeValues;
	AttributeValues inserted_attributes;
	inserted_attributes.reserve(count);
	std::transform(values_begin, values_end, std::back_inserter(inserted_attributes), ValueInserter(allocator));
	
	//Now build the table mapping values to their ID
	MapBuilder builder(allocator, inserted_attributes.size());
	for (AttributeValues::iterator i = inserted_attributes.begin(); i != inserted_attributes.end(); ++i) {
	  builder(*i);
	}
	value_to_id_map_ = allocator.Insert(builder.Build());
	
	//Finally, insert the ID-indexed range of value pointers.
	values_begin_ = allocator.InsertRange(inserted_attributes.begin(), inserted_attributes.end());
	values_end_ = values_begin_ + count;
      }
      const StoredAttributeValue* GetValue(AttributeId id) const  {
	return *(static_cast<const AttributeValuePtr*>(values_begin_) + id);
      }
      const AttributeId* GetId(const iknow::base::String& value) const {
	return value_to_id_map_->Lookup(value);
      }
    private:
      //A map from values to IDs.
      typedef StaticHash::Table<AttributeValue, AttributeId> ValueToIdMap;
      OffsetPtr<const ValueToIdMap> value_to_id_map_;
      //Values are stored in an ID-indexed array of pointers to counted strings.
      typedef OffsetPtr<StoredAttributeValue> AttributeValuePtr;
      OffsetPtr<const AttributeValuePtr> values_begin_;
      OffsetPtr<const AttributeValuePtr> values_end_;
    };
      
    class AttributeMapBuilder {
    public:
      AttributeMapBuilder() : next_id_(0) {}
      AttributeId Insert(const iknow::base::String& value) {
	ValueMap::iterator i = values_.find(value);
	if (i != values_.end()) return i->second; //already exists, return ID
	values_[value] = next_id_;
	return next_id_++;
      }
      KbAttributeMap ToAttributeMap(RawAllocator& allocator) const {
	//This is really an unnecessary copy, but simplifies the logic
	//and KB loading should not be performance-sensitive.
	std::vector<iknow::base::String> values;
	values.resize(next_id_);
	for (ValueMap::const_iterator i = values_.begin(); i != values_.end(); ++i) {
	  values[i->second] = i->first;
	}
	return KbAttributeMap(allocator, values.begin(), values.end());
      }
    private:
      typedef std::map<iknow::base::String, AttributeId> ValueMap;
      ValueMap values_;
      AttributeId next_id_;
    };


  }
}

#endif //IKNOW_SHELL_KBATTRIBUTEMAP_H_
