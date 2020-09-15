#include "SharedMemoryKnowledgebase.h"
#include "Scanner.h"
#include "IkStringEncoding.h"
#include "utlExceptionFrom.h"
#include "MethodIterator.h"
#include "IkMetadataCache.h"
#include <algorithm>
#include <iterator>
#include <functional>
#include <string>

#include <vector>
#include <map>
#include <utility>

using namespace iknow::base;
using namespace iknow::core;
using namespace iknow::shell;
using namespace iknow::shell::StaticHash;
using std::string;
using std::for_each;
using std::count;
using std::vector;
using std::pair;
using std::map;
using std::back_inserter;
using std::transform;
using namespace iknow::csvdata;

typedef map<String, FastLabelSet::Index> LabelIndexMap;

//A functor to convert a raw CacheList into a Kb* type
template<typename KbT>
class RawListToKb {
public:
	KbT operator()(CacheList& list);
};

class WithAllocator {
protected:
	WithAllocator(RawAllocator& allocator) : allocator_(allocator) {}
	RawAllocator& GetAllocator() { return allocator_; }
private:
	RawAllocator& allocator_;
	void operator=(const WithAllocator&);
};

class WithLabelMap {
protected:
	WithLabelMap(LabelIndexMap& map) : map_(map) {}
	LabelIndexMap& GetLabelMap() { return map_; }
private:
	LabelIndexMap& map_;
	void operator=(const WithLabelMap&);
};

class WithAttributeMap {
protected:
	WithAttributeMap(AttributeMapBuilder& attribute_map) : attribute_map_(attribute_map) {}
	AttributeMapBuilder& GetAttributeMap() { return attribute_map_; }
private:
	AttributeMapBuilder& attribute_map_;
	void operator=(const WithAttributeMap&);
};

template<>
class RawListToKb<KbLabel> : private WithAllocator, private WithAttributeMap {
public:
	typedef KbLabel output_type;
	RawListToKb(RawAllocator& allocator, AttributeMapBuilder& attribute_map) : WithAllocator(allocator), WithAttributeMap(attribute_map) {}

	KbLabel operator()(iKnow_KB_Label label) {
		return KbLabel(GetAllocator(), label.Name, label.Type, label.Attributes, label.PhaseList, GetAttributeMap());
	}

private:
	void operator=(const RawListToKb&);
};

template<>
class RawListToKb<KbLexrep> : private WithAllocator, private WithLabelMap {
public:
	typedef KbLexrep output_type;
	RawListToKb(RawAllocator& allocator, LabelIndexMap& map, size_t& max_lexrep_size, size_t& max_token_size) : WithAllocator(allocator), WithLabelMap(map), max_lexrep_size_(max_lexrep_size), max_token_size_(max_token_size) {}

#if defined _IRIS
	KbLexrep operator()(CacheList& list) {
		KbLexrep lexrep(GetAllocator(), GetLabelMap(), list[1].AsAString(), list[2].AsAString());
#else
	KbLexrep operator()(iKnow_KB_Lexrep kb_lexrep) {
		KbLexrep lexrep(GetAllocator(), GetLabelMap(), kb_lexrep.Token, kb_lexrep.Labels);
#endif
		size_t size = lexrep.TokenCount();
		if (size > max_lexrep_size_) max_lexrep_size_ = size;
		// for Japanese, we need to calculate the maximum token size, this is an unnecessary step for non-Asian languages, but it's only done while loading, so no runtime performance while indexing...
		size_t max_token = lexrep.maxTokenSize();
		if (max_token > max_token_size_) max_token_size_ = max_token;
		return lexrep;
	}

	size_t MaxLexrepSize() { return max_lexrep_size_; }
	size_t MaxTokenSize() { return max_token_size_; }
private:
	size_t& max_lexrep_size_;
	size_t& max_token_size_;
	void operator=(const RawListToKb&);
	};

template<>
class RawListToKb<KbRule> : private WithAllocator, private WithLabelMap {
public:
	typedef KbRule output_type;
	RawListToKb(RawAllocator& allocator, LabelIndexMap& map) : WithAllocator(allocator), WithLabelMap(map) {}

#if defined _IRIS
	KbRule operator()(CacheList& list) {
		return KbRule(GetAllocator(), GetLabelMap(), list[1].AsAString(), list[2].AsAString(), iknow::core::PhaseFromString(list[4].AsAString()));
	}
#else
	KbRule operator()(iKnow_KB_Rule rule) {
		return KbRule(GetAllocator(), GetLabelMap(), rule.InputPattern, rule.OutputPattern, iknow::core::PhaseFromString(rule.Phase));
	}
#endif

private:
	void operator=(const RawListToKb&);
};

template<>
class RawListToKb<KbAcronym> : private WithAllocator {
public:
	typedef KbAcronym output_type;
	RawListToKb(RawAllocator& allocator) : WithAllocator(allocator) {}

#if defined _IRIS
	KbAcronym operator()(CacheList& list) {
		return KbAcronym(GetAllocator(), list[1].AsAString(), list[2].AsBool() != 0);
	}
#else
	KbAcronym operator()(iKnow_KB_Acronym acronym) {
		return KbAcronym(GetAllocator(), acronym.Token, acronym.IsSentenceEnd);
	}
#endif

private:
	void operator=(const RawListToKb&);
};

template<>
class RawListToKb<KbRegex> : private WithAllocator {
public:
	typedef KbRegex output_type;
	RawListToKb(RawAllocator& allocator) : WithAllocator(allocator) {}

#if defined _IRIS
	KbRegex operator()(CacheList& list) {
		return KbRegex(GetAllocator(), list[1].AsAString(), list[2].AsAString());
	}
#else
	KbRegex operator()(iKnow_KB_Regex regex) {
		return KbRegex(GetAllocator(), regex.name, regex.Pattern);
	}
#endif

private:
	void operator=(const RawListToKb&);
};

template<>
class RawListToKb<KbPreprocessFilter> : private WithAllocator {
public:
	typedef KbPreprocessFilter output_type;
	RawListToKb(RawAllocator& allocator) : WithAllocator(allocator) {}

#if defined _IRIS
	KbPreprocessFilter operator()(CacheList& list) {
		return KbPreprocessFilter(GetAllocator(), list[1].AsAString(), list[2].AsAString());
	}
#else
	KbPreprocessFilter operator()(iKnow_KB_PreprocessFilter filter) {
		return KbPreprocessFilter(GetAllocator(), filter.InputToken, filter.OutputToken);
	}
#endif

private:
	void operator=(const RawListToKb&);
};

template<>
class RawListToKb<KbFilter> : private WithAllocator {
public:
	typedef KbFilter output_type;
	RawListToKb(RawAllocator& allocator) : WithAllocator(allocator) {}

#if defined _IRIS
	KbFilter operator()(CacheList& list) {
		return KbFilter(GetAllocator(), list[1].AsAString(), list[2].AsAString(), list[3].AsBool() != 0, list[4].AsBool() != 0, list[7].AsBool() != 0, list[8].AsBool() != 0);
	}
#else
	KbFilter operator()(iKnow_KB_Filter filter) {
		return KbFilter(GetAllocator(), filter.InputToken, filter.OutputToken, filter.ApplyOnlyAtBeginning, filter.ApplyOnlyAtEnd, filter.IsConceptFilter, filter.IsRelationFilter);
	}
#endif

private:
	void operator=(const RawListToKb&);
};

template<>
class RawListToKb<KbProperty> : private WithAllocator {
public:
	typedef KbProperty output_type;
	RawListToKb(RawAllocator& allocator) : WithAllocator(allocator) {}

#if defined _IRIS
	KbProperty operator()(CacheList& list) {
		return KbProperty(GetAllocator(), static_cast<PropertyId>(list[0].AsLong()), list[1].AsAString());
	}
#else
	KbProperty operator()(const std::pair<int, string>& kb_property) {
		return KbProperty(GetAllocator(), static_cast<PropertyId>(kb_property.first), kb_property.second);
	}
#endif

private:
	void operator=(const RawListToKb&);
};

template<>
class RawListToKb<KbMetadata> : private WithAllocator {
public:
	typedef KbMetadata output_type;
	RawListToKb(RawAllocator& allocator) : WithAllocator(allocator) {}

#if defined _IRIS
	KbMetadata operator()(CacheList& list) {
		return KbMetadata(GetAllocator(), list[1].AsAString(), list[2].AsAString());
	}
#else
	KbMetadata operator()(iKnow_KB_Metadata metadata) {
		return KbMetadata(GetAllocator(), metadata.Name, metadata.Val);
	}
#endif

private:
	void operator=(const RawListToKb&);
};

using namespace iknow::shell::StaticHash;

template<typename VectorT, typename KbT>
void WriteKbBlock(RawAllocator& allocator, const VectorT& v, KbT*& begin, KbT*& end) {
	begin = allocator.InsertRange(v.begin(), v.end());
	end = begin + v.size();
}

template<typename IterT, typename TransformerT>
void LoadKbRange(IterT begin, IterT end, size_t size, TransformerT& transformer, RawAllocator& allocator, const typename TransformerT::output_type*& out_begin, const typename TransformerT::output_type*& out_end) {
	typedef typename TransformerT::output_type KbT;
	vector<KbT> values;
	values.reserve(size);
	transform(begin, end, back_inserter(values), transformer);
	WriteKbBlock(allocator, values, out_begin, out_end);
}

template<typename IterT, typename TransformerT, typename StringT, typename KeyFuncT>
void LoadKbRangeAsTable(IterT begin, IterT end, size_t size, TransformerT& transformer, const Table<StringT, typename TransformerT::output_type>*& table, KeyFuncT key_function, RawAllocator& allocator) {
	typedef typename TransformerT::output_type KbT;
	typedef vector<KbT> Values;
	Values values;
	values.reserve(size);
	transform(begin, end, back_inserter(values), transformer);
	Builder<StringT, KbT> table_builder(values.size());
	for (typename Values::const_iterator i = values.begin(); i != values.end(); ++i) {
		table_builder.Insert(key_function(&*i), allocator.Insert(*i));
	}
	table = allocator.Insert(table_builder.Build(allocator));
}

SharedMemoryKnowledgebase::SharedMemoryKnowledgebase(RawKBData* kb_data) : kb_data_(kb_data) {}
SharedMemoryKnowledgebase::SharedMemoryKnowledgebase(unsigned char* kb_data) : kb_data_(reinterpret_cast<RawKBData*>(kb_data)) {}

SharedMemoryKnowledgebase::SharedMemoryKnowledgebase(RawAllocator& allocator, AbstractKnowledgebase& kb, bool is_compiled) {
	//The RawKBData must be the first thing in the block, we'll use its address for the base of all
	//OffsetPtrs in the structure.
	kb_data_ = allocator.Insert(RawKBData());
	OFFSETPTRGUARD;

	LabelIndexMap label_index_map; // label names mapped to (internal) label indexes.
	AttributeMapBuilder attribute_map_builder;

	size_t labels_count = static_cast<size_t>(kb.LabelCount());
	if (labels_count > iknow::core::kMaxLabelCount) { //Make sure we don't have too many labels
		throw ExceptionFrom<SharedMemoryKnowledgebase>("Number of labels to load exceeds maximum limit.");
	}
	// Transform label data
	{
		RawListToKb<KbLabel> label_transformer(allocator, attribute_map_builder);
		const KbLabel* begin, * end;
		LoadKbRange(kb.kb_labels.begin(), kb.kb_labels.end(), labels_count, label_transformer, allocator, begin, end);
		kb_data_->labels_begin = begin;
		kb_data_->labels_end = end;
	}
	//Store the KbAttributeMap generated via the label loads
	kb_data_->attribute_map = allocator.Insert(attribute_map_builder.ToAttributeMap(allocator));

/*
	//Build the map from label name to offset in labels table
	Builder<String, size_t> labels_table_builder(labels_count);
	for (size_t i = 0; i < labels_count; ++i) {
		const KbLabel* label = kb_data_->labels_begin + i;
		labels_table_builder.Insert(label->PointerToName(), allocator.Insert(i));
		//TODO: Other components should just use the kb_data_->labels table.
		label_index_map.insert(LabelIndexMap::value_type(label->Name(), static_cast<FastLabelSet::Index>(i)));
	}
	kb_data_->labels = allocator.Insert(labels_table_builder.Build(allocator));



	//Special labels
	for (size_t i = BeginLabels; i != EndLabels; i++) {
		kb_data_->special_labels[SpecialLabel(i)] = GetLabelIndexFromList(*kb_data_, kb.GetSpecialLabel(SpecialLabel(i)));
	}

	//Lexreps
	if (!is_compiled) {
		kb_data_->max_lexrep_size = 0;
		kb_data_->max_token_size = 0;
		RawListToKb<KbLexrep> lexrep_transformer(allocator, label_index_map, kb_data_->max_lexrep_size, kb_data_->max_token_size);
		LoadKbStructureToTable(kb,
			&AbstractKnowledgebase::NextLexrep,
			&AbstractKnowledgebase::GetLexrep,
			kb.LexrepCount(),
			lexrep_transformer,
#ifdef AIX
			mem_fun(&KbLexrep::PointerToToken),
#else
			[](const KbLexrep* lexrep) { return lexrep->PointerToToken(); },
#endif
			kb_data_->lexreps,
			allocator);
	}
	//Rules
	RawListToKb<KbRule> rule_transformer(allocator, label_index_map);
	LoadKbStructure(kb,
		&AbstractKnowledgebase::NextRule,
		&AbstractKnowledgebase::GetRule,
		kb.RuleCount(),
		rule_transformer,
		kb_data_->rules_begin,
		kb_data_->rules_end,
		allocator);
	//Acronyms
	RawListToKb<KbAcronym> acronym_transformer(allocator);
	LoadKbStructureToTable(kb,
		&AbstractKnowledgebase::NextAcronym,
		&AbstractKnowledgebase::GetAcronym,
		kb.AcronymCount(),
		acronym_transformer,
#ifdef AIX
		mem_fun(&KbAcronym::PointerToToken),
#else
		[](const KbAcronym* acronym) { return acronym->PointerToToken(); },
#endif
		kb_data_->acronyms,
		allocator);
	//Regexes
	RawListToKb<KbRegex> regex_transformer(allocator);
	LoadKbStructure(kb,
		&AbstractKnowledgebase::NextRegex,
		&AbstractKnowledgebase::GetRegex,
		kb.RegexCount(),
		regex_transformer,
		kb_data_->regexes_begin,
		kb_data_->regexes_end,
		allocator);

	//Preprocess Filters
	RawListToKb<KbPreprocessFilter> preprocess_filter_transformer(allocator);
	LoadKbStructure(kb,
		&AbstractKnowledgebase::NextPreprocessFilter,
		&AbstractKnowledgebase::GetPreprocessFilter,
		kb.PreprocessFilterCount(),
		preprocess_filter_transformer,
		kb_data_->preprocess_filters_begin,
		kb_data_->preprocess_filters_end,
		allocator);

	//Concept Filters
	RawListToKb<KbFilter> concept_filter_transformer(allocator);
	LoadKbStructure(kb,
		&AbstractKnowledgebase::NextConceptFilter,
		&AbstractKnowledgebase::GetConceptFilter,
		kb.ConceptFilterCount(),
		concept_filter_transformer,
		kb_data_->concept_filters_begin,
		kb_data_->concept_filters_end,
		allocator);
	//Input Filters (initially, identical to preprocess filters

	RawListToKb<KbInputFilter> input_filter_transformer(allocator);
	LoadKbStructure(kb,
		&AbstractKnowledgebase::NextInputFilter,
		&AbstractKnowledgebase::GetInputFilter,
		kb.InputFilterCount(),
		input_filter_transformer,
		kb_data_->input_filters_begin,
		kb_data_->input_filters_end,
		allocator);

	//Semantic properties
	RawListToKb<KbProperty> property_transformer(allocator);
	LoadKbStructureToTable(kb,
		&AbstractKnowledgebase::NextProperty,
		&AbstractKnowledgebase::GetProperty,
		kb.PropertyCount(),
		property_transformer,
#ifdef AIX
		mem_fun(&KbProperty::PointerToName),
#else
		[](const KbProperty* property) { return property->PointerToName(); },
#endif
		kb_data_->properties,
		allocator);

	//Language metadata
	RawListToKb<KbMetadata> metadata_transformer(allocator);
	LoadKbStructureToTable(kb,
		&AbstractKnowledgebase::NextMetadata,
		&AbstractKnowledgebase::GetMetadata,
		kb.MetadataCount(),
		metadata_transformer,
#ifdef AIX
		mem_fun(&KbMetadata::PointerToName),
#else
		[](const KbMetadata* metadata) { return metadata->PointerToName(); },
#endif
		kb_data_->metadata,
		allocator);

	//Hash
#if defined(LINUX) || defined(UNIX)
	kb_data_->hash = hash<std::string>()(IkStringEncoding::BaseToUTF8(kb.GetHash()));
#else
	kb_data_->hash = hash<decltype(kb.GetHash())>()(kb.GetHash());
#endif
*/
}

void SharedMemoryKnowledgebase::FilterInput(iknow::base::String& input) const {
  OFFSETPTRGUARD;
  const KbInputFilter* const begin = kb_data_->input_filters_begin;
  const KbInputFilter* const end = kb_data_->input_filters_end;
  for (const KbInputFilter* i=begin; i != end; ++i) {
    i->Apply(input);
  } 
}

bool SharedMemoryKnowledgebase::LabelSingleToken(IkLexrep& lexrep, const String& label_name) const {
	OFFSETPTRGUARD;
	const size_t *idxLabel = kb_data_->labels->Lookup(label_name);
	if (idxLabel) {
	  lexrep.AddLabelIndex(static_cast<FastLabelSet::Index>(*idxLabel));
	  return true;
	} else // label name does not exist.
	  return false;
}

bool SharedMemoryKnowledgebase::LabelSingleToken(IkLexrep& lexrep) const {
  OFFSETPTRGUARD;
  String& token = lexrep.GetNormalizedValue();
  const KbLexrep* kb_lexrep = kb_data_->lexreps->Lookup(token.data(),
							token.data() + token.size());
  if (!kb_lexrep) return false;
  for (const FastLabelSet::Index* i = kb_lexrep->PointerToLabels()->begin();
       i != kb_lexrep->PointerToLabels()->end(); ++i) {
    lexrep.AddLabelIndex(*i);
  }
  return true;
}

//TODO: Ugly and long.
IkLexrep SharedMemoryKnowledgebase::NextLexrep(Lexreps::iterator& current, Lexreps::iterator end) const {
#ifndef WIN32
  using std::min;
#endif //WIN32

  if (!buffered_lexreps_.empty()) {
    IkLexrep lexrep = buffered_lexreps_.front();
    buffered_lexreps_.pop();
    return lexrep;
  }
  OFFSETPTRGUARD;
  size_t max_size = static_cast<size_t>(end - current) > kb_data_->max_lexrep_size ?
    kb_data_->max_lexrep_size :
    static_cast<size_t>(end - current);
  size_t max_token_size=kb_data_->max_token_size; // maximum token size, used for ideographic languages (eg. Japanese, Chinese, Korean, ...)

  String tokens;
  tokens.reserve(16 * max_size);
  Lexreps::iterator lexrep = current;
  typedef vector<size_t> TokenBoundaries;
  TokenBoundaries token_boundaries;
  token_boundaries.reserve(max_size);

  bool is_ideographic = this->GetMetadata<kIsJapanese>();

  size_t match_size_jpn=0;
  if (is_ideographic) { // max_token_size overrules max_size
    size_t ideograph_max = std::min(max_token_size,static_cast<size_t>(end - current)); 
    for (size_t i = 0; i < ideograph_max; ++i) {
      String token = lexrep->GetNormalizedValue();
      tokens += token;
      match_size_jpn++; // length of JPN string
      token_boundaries.push_back(tokens.size());
      if (token.size()>=max_token_size) // this is possible if Katakana is mixed in.
        break;
      ++lexrep;
    }
  } else { // Normal style  
    for (size_t i = 0; i < max_size; ++i) {
      tokens += lexrep->GetNormalizedValue();
      token_boundaries.push_back(tokens.size());
      tokens += ' ';
      ++lexrep;
    }
  }
  const KbLexrep* kb_lexrep = 0;
  size_t match_size = (is_ideographic ? match_size_jpn : max_size);
  for (;match_size > 0; --match_size) {
    kb_lexrep = kb_data_->lexreps->Lookup(tokens.data(), tokens.data() + token_boundaries[match_size - 1]);
    if (kb_lexrep) break;
  }
  if (!kb_lexrep) { // Not found, return original and advance.
    return *current++;
  }

  // If only attribute labels are added, keep the other labels
  bool bAddOnlyAttributes = true; // do we only add attribute labels ?
  std::set<FastLabelSet::Index> new_attribute_labels;
  for (const FastLabelSet::Index* i = kb_lexrep->PointerToLabels()->begin(); i != kb_lexrep->PointerToLabels()->end(); ++i) {
	  if (this->GetLabelTypeAtIndex(*i) != IkLabel::Attribute) {
		  bAddOnlyAttributes = false;
		  break;
	  } else {
		  new_attribute_labels.insert(*i);
	  }
  }

  //TODO: simplify logic and remove duplication
  if (match_size == 1) {  // modify copy of existing lexrep for speed, this is a common case.
    IkLexrep output = *current++;
	
    if (!bAddOnlyAttributes) output.ClearAllLabels(); // don't clear labels if only attributes are added
    for (const FastLabelSet::Index* i = kb_lexrep->PointerToLabels()->begin(); i != kb_lexrep->PointerToLabels()->end(); ++i) {
      output.AddLabelIndex(*i);
    }
	if (!bAddOnlyAttributes) output.SetAnnotated(true); // safe for further lookup actions
    return output;
  }
  //How many "-" separated label segments are there?
#ifdef SOLARIS
  size_t label_segment_count = 0;
  //Have to use the stupid by reference returning "count"
  //because that's all the old RogueWave STL on Sun supports.
  count(kb_lexrep->PointerToLabels()->begin(),
	kb_lexrep->PointerToLabels()->end(),
	IkLabel::BreakIndex(), label_segment_count);
  ++label_segment_count; //one more than the # of breaks
#else //!SOLARIS
  size_t label_segment_count = 1 + count(kb_lexrep->PointerToLabels()->begin(),
					 kb_lexrep->PointerToLabels()->end(),
					 IkLabel::BreakIndex());
#endif
    
  //Just one? Merge the lexrep literals and return a single lexrep.
  if (label_segment_count == 1) {
	  if (!bAddOnlyAttributes) { // normal case
		  IkLexrep output = JoinLexreps(current, current + match_size, is_ideographic ? iknow::base::String() : iknow::base::SpaceString());
		  for (const FastLabelSet::Index* i = kb_lexrep->PointerToLabels()->begin(); i != kb_lexrep->PointerToLabels()->end(); ++i) { // add new labels
			  output.AddLabelIndex(*i);
		  }
		  current = current + match_size; // advance to next
		  output.SetAnnotated(true); // no more lexrep lookups, except when only adding attributes.
		  return output;
	  }
	  else { // only adding attribute labels
		  while (match_size != 0) {
			  IkLexrep output = *current++;
			  for (const FastLabelSet::Index* i = kb_lexrep->PointerToLabels()->begin(); i != kb_lexrep->PointerToLabels()->end(); ++i) { // add attribute labels
				  output.AddLabelIndex(*i);
			  }
			  buffered_lexreps_.push(output); // store in buffer
			  --match_size;
		  }
		  IkLexrep output = buffered_lexreps_.front();
		  buffered_lexreps_.pop();
		  return output;
	  }
  }
  else {
    FastLabelSet::Index break_index = IkLabel::BreakIndex();
    FastLabelSet::Index join_index = this->GetLabelIndex(JoinLabel);
    Lexreps::const_iterator end_match = current + match_size;
    const FastLabelSet::Index* cur_label = kb_lexrep->PointerToLabels()->begin();
    const FastLabelSet::Index* const last_label = kb_lexrep->PointerToLabels()->end();
    Lexreps::const_iterator lexrep_begin = current;
	bool bIsKatakana = iknow::base::IkStringAlg::IsKatakana(lexrep_begin->GetNormalizedValue()[0]);
    Lexreps::const_iterator lexrep_end = current + 1;
	if (bIsKatakana) { while (lexrep_end != end && iknow::base::IkStringAlg::IsKatakana(lexrep_end->GetNormalizedValue()[0])) ++lexrep_end; }  // join Katakana symbols
    std::vector<FastLabelSet::Index> labels;
    labels.reserve(last_label - cur_label);
    for (;cur_label != last_label; ++cur_label) {
      if (*cur_label == break_index) {
        IkLexrep output = JoinLexreps(lexrep_begin, lexrep_end, is_ideographic ? iknow::base::String() : iknow::base::SpaceString());
        output.AddLabelIndices(labels.begin(), labels.end());
		output.SetAnnotated(true); // no more lexrep lookup.
        buffered_lexreps_.push(output);
        lexrep_begin = lexrep_end;
		bIsKatakana = iknow::base::IkStringAlg::IsKatakana(lexrep_begin->GetNormalizedValue()[0]);
        lexrep_end = lexrep_begin + 1;
		if (bIsKatakana) { while (lexrep_end != end && iknow::base::IkStringAlg::IsKatakana(lexrep_end->GetNormalizedValue()[0])) ++lexrep_end; }  // join Katakana symbols
        labels.clear();
      } else if (*cur_label == join_index) {
        if (lexrep_end == end_match)
          throw ExceptionFrom<SharedMemoryKnowledgebase>("Tried to join a non-existent lexrep.");
		bIsKatakana = iknow::base::IkStringAlg::IsKatakana(lexrep_end->GetNormalizedValue()[0]);
        ++lexrep_end;
		if (bIsKatakana) { while (lexrep_end != end && iknow::base::IkStringAlg::IsKatakana(lexrep_end->GetNormalizedValue()[0])) ++lexrep_end; }  // join Katakana symbols
      } else {
        labels.push_back(*cur_label); // collect the label
      }
    }
    //Output remaining labels on rest of lexrep
    if (!labels.empty()) {
      IkLexrep output = JoinLexreps(lexrep_begin, end_match, is_ideographic ? iknow::base::String() : iknow::base::SpaceString());
      output.AddLabelIndices(labels.begin(), labels.end());	  	  
	  output.SetAnnotated(true); // no more lexrep lookup
      buffered_lexreps_.push(output); // last one
    }
    //Could call NextLexrep recursively here, but let's not get cute.
    //(Would need conditional inlining to get good performance).
    IkLexrep output = buffered_lexreps_.front();
    buffered_lexreps_.pop();

    current = current + match_size; // advance to next
    return output;
  }
}

