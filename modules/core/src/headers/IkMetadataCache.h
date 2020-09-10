#ifndef IKNOW_CORE_IKMETADATACACHE_H_
#define IKNOW_CORE_IKMETADATACACHE_H_
#include <cstddef>
#include "IkTypes.h"
#include "utlExceptionFrom.h"

//Rather than perform the expensive virtual method and string hash table lookup/conversion
//we'll store frequently accessed knowledgebase metadata values here.

// This design is rather baroque, but it needs to fulfill a few requirements:
//
// #1, with a bullet: Performance. It's OK if the first access of a metadata value is slow, but
// some metadata values may be accessed repeatedly in tight loops. As currently implemented, repeated
// accesses have a cost of a few instructions at most.
// #2: Flexibility. Any key/values should be permitted, which means the interface with COS has to be
// in terms of strings.
// #3: Easy extendability. This is last, but still important. It should be relatively easy to add new
// metadata key/value pairs.

// The external interface to the metadata cache is simple. Create one by passing in a KB reference.
// Access values with GetValue(), which takes a template MetadataValue enum argument to specify the
// value to retrieve. The return type of this function is determined via a TMP type lookup:
// MetadataTraits<kSomeValue>::type. There's also a static DefaultValue() which returns the
// value the implementation will use if no metadata value for that key is specified in the KB.

// The implementation is largely a set of member function template specializations for each
// value, created by the METADATA_VALUE_INFO macro.

// The process for adding access to a new metadata value (presumably already defined in
// at least one KB's metadata.csv file):
// 
// 1) Add a value name to the MetadataValue enum.
// 2) Add a field to IkMetadataCache to hold the cached value.
// 3) Add a call to the METADATA_VALUE_INFO macro with appropriate parameters.
// 4) Add a call to the LoadValue<...>() function for the new enum value name.
// 5) (Optional) If the new value has a previously unused type, add a specialization
// of ConvertValue<> to IkMetadataCache.cpp.

namespace iknow {
  namespace core {
    class IkKnowledgebase;

    enum MetadataValue {
      kConceptsToMergeMax,
	  kNonRelevantsToMergeMax,
      kChainPattern,
      kModifiersOnLeft,
      kIsJapanese,
      kRuleBasedPaths,
      kHasEntityVectors,
      kEntityVectorScanDirection,
      kLanguageCode,
	  kP1,
	  kP2,
	  kP3,
	  kP4,
	  kPScale,
	  kPathConstruction,
	  kRegexSplitter,
      kFuriganaHandling
    };

    template<MetadataValue V>
    struct MetadataTraits;

    enum ChainPattern {
      kCRCPattern,
      kCCRPattern
    };

    enum ScanDirection {
      kRightToLeft,
      kLeftToRight
    };
    
	enum PathConstruction {
		kCRCSequence, // default
		kPathRelevant
	};
    /*
    ** https://github.com/intersystems/iknow/issues/31
    ** Furigana handling made flexible: "on"=handle as originally implemented (default), "off"=no special treatment
    */
    enum FuriganaHandling {
        kFuriDefault, // as originally implemented
        kFuriOff
    };
    class IkMetadataCache {
    public:
      explicit IkMetadataCache(const IkKnowledgebase& kb);

      template<MetadataValue V>
      typename MetadataTraits<V>::value_type GetValue() const;

      template<MetadataValue V>
      static typename MetadataTraits<V>::value_type DefaultValue();

    private:

      //Reads a string value from the KB for the given MetadataValue.
      template<MetadataValue V>
      static iknow::base::String ReadStringValue(const IkKnowledgebase& kb);

      //A non-templated version to use once we have the key from instantiating the above
      //function.
      static iknow::base::String ReadStringValue(const IkKnowledgebase& kb, const std::string& key);

      //Convert a value from a string to a value-specific type (e.g. "1" -> bool(true))
      template<typename ValT>
      static ValT ConvertValue(const iknow::base::String& val);

      //Reads the string value for the given MetadataValue from the KB using ReadStringValue().
      //If it is empty, return the default value, otherwise convert the returned string
      //to a value using an appropriate specialization of ConvertValue<>()
      template<MetadataValue V, typename ValT>
      static ValT ReadValue(const IkKnowledgebase& kb, const ValT& default_value) {
        iknow::base::String val(ReadStringValue<V>(kb));
        if (val.empty()) return default_value;
        return ConvertValue<ValT>(val);
      }

      //Sets the appropriate field of this cache to the given value.
      template<MetadataValue V>
      void SetValue(typename MetadataTraits<V>::value_type value);

      //Loads a string value from the KB and caches it in this object.
      template<MetadataValue V>
      void LoadValue(const IkKnowledgebase&) { //specialized below
        throw ExceptionFrom<IkMetadataCache>("Unknown metadata value requested.");
      }

      std::size_t concepts_to_merge_max_;
	  std::size_t nonrelevants_to_merge_max_;
      ChainPattern chain_pattern_;
      bool modifiers_on_left_;
      bool is_japanese_;
      bool rule_based_paths_;
      bool has_entity_vectors_;
      ScanDirection entity_vector_scan_direction_;
      iknow::base::String language_code_;
	  std::size_t p1_; // parameters for dominance calculation, see documentation
	  std::size_t p2_;
	  std::size_t p3_;
	  std::size_t p4_;
	  std::size_t pScale_;
	  PathConstruction path_construction_;
	  iknow::base::String regex_splitter_;
      FuriganaHandling furigana_handling_;
    };

#define METADATA_VALUE_INFO(id, key, type, field, default_value)	\
    template<> struct MetadataTraits<id> { typedef type value_type; };	\
    template<> inline type IkMetadataCache::GetValue<id>() const { return field; } \
    template<> inline void IkMetadataCache::SetValue<id>(type value) { field = value; } \
    template<> inline type IkMetadataCache::DefaultValue<id>()  { return default_value; } \
    template<> type IkMetadataCache::ConvertValue<type>(const iknow::base::String& val); \
    template<> inline iknow::base::String IkMetadataCache::ReadStringValue<id>(const IkKnowledgebase& kb) { \
      return ReadStringValue(kb, key);					\
    }									\
    template<> inline void IkMetadataCache::LoadValue<id>(const IkKnowledgebase& kb) { \
      SetValue<id>(ReadValue<id, type>(kb, default_value));		\
    }									

    //Don't forget to add the LoadValue to the constructor in IkMetadataCache.cpp

    METADATA_VALUE_INFO(kConceptsToMergeMax, "ConceptsToMergeMax", std::size_t, concepts_to_merge_max_, 6)
	METADATA_VALUE_INFO(kNonRelevantsToMergeMax, "NonRelevantsToMergeMax", std::size_t, nonrelevants_to_merge_max_, 4)
    METADATA_VALUE_INFO(kChainPattern, "ChainPattern", ChainPattern, chain_pattern_, kCRCPattern)
    METADATA_VALUE_INFO(kModifiersOnLeft, "ModifiersOnLeft", bool, modifiers_on_left_, true)
    METADATA_VALUE_INFO(kIsJapanese, "IsJapanese", bool, is_japanese_, false)
    METADATA_VALUE_INFO(kRuleBasedPaths, "RuleBasedPaths", bool, rule_based_paths_, false)
    METADATA_VALUE_INFO(kHasEntityVectors, "HasEntityVectors", bool, has_entity_vectors_, false)
    METADATA_VALUE_INFO(kEntityVectorScanDirection, "EntityVectorScanDirection", ScanDirection, entity_vector_scan_direction_, kLeftToRight)
    METADATA_VALUE_INFO(kLanguageCode, "LanguageCode", iknow::base::String, language_code_, iknow::base::SpaceString())
	METADATA_VALUE_INFO(kP1, "P1", std::size_t, p1_, 200)
	METADATA_VALUE_INFO(kP2, "P2", std::size_t, p2_, 50)
	METADATA_VALUE_INFO(kP3, "P3", std::size_t, p3_, 30) // default is 0.3, not 0.5
	METADATA_VALUE_INFO(kP4, "P4", std::size_t, p4_, 0)
	METADATA_VALUE_INFO(kPScale, "SCALE", std::size_t, pScale_, 100)
	METADATA_VALUE_INFO(kPathConstruction, "PathConstruction", PathConstruction, path_construction_, kCRCSequence)
	METADATA_VALUE_INFO(kRegexSplitter, "ValUnitRegexSplitter", iknow::base::String, regex_splitter_, iknow::base::String())
    METADATA_VALUE_INFO(kFuriganaHandling, "FuriganaHandling", FuriganaHandling, furigana_handling_, kFuriDefault)
#undef METADATA_VALUE_INFO

  }
}

#endif //IKNOW_CORE_IKMETADATACACHE_H_
