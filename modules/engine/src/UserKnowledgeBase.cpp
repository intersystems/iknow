/*
** UserKnowledgeBase.cpp : a flexible KB that can change during runtime, lexreps are mapped, not compiled into a statemachine.
*/
#include <cstring>

#include "UserKnowledgeBase.h"
#include "SharedMemoryKnowledgebase.h"

using namespace iknow::csvdata;
using namespace std;
using iknow::base::IkStringEncoding;
using iknow::base::String;
using namespace iknow::core;

using namespace std;
using namespace iknow::base;
using namespace iknow::core;
using namespace iknow::shell;

typedef std::map<String, FastLabelSet::Index> LabelIndexMap;

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

	KbLexrep operator()(iKnow_KB_Lexrep kb_lexrep) {
		KbLexrep lexrep(GetAllocator(), GetLabelMap(), kb_lexrep.Token, kb_lexrep.Labels);
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

	KbRule operator()(iKnow_KB_Rule rule) {
		return KbRule(GetAllocator(), GetLabelMap(), rule.InputPattern, rule.OutputPattern, iknow::core::PhaseFromString(rule.Phase));
	}

private:
	void operator=(const RawListToKb&);
};

template<>
class RawListToKb<KbAcronym> : private WithAllocator {
public:
	typedef KbAcronym output_type;
	RawListToKb(RawAllocator& allocator) : WithAllocator(allocator) {}

	KbAcronym operator()(iKnow_KB_Acronym acronym) {
		return KbAcronym(GetAllocator(), acronym.Token, acronym.IsSentenceEnd);
	}

private:
	void operator=(const RawListToKb&);
};

template<>
class RawListToKb<KbRegex> : private WithAllocator {
public:
	typedef KbRegex output_type;
	RawListToKb(RawAllocator& allocator) : WithAllocator(allocator) {}

	KbRegex operator()(iKnow_KB_Regex regex) {
		return KbRegex(GetAllocator(), regex.name, regex.Pattern);
	}

private:
	void operator=(const RawListToKb&);
};

template<>
class RawListToKb<KbPreprocessFilter> : private WithAllocator {
public:
	typedef KbPreprocessFilter output_type;
	RawListToKb(RawAllocator& allocator) : WithAllocator(allocator) {}

	KbPreprocessFilter operator()(iKnow_KB_PreprocessFilter filter) {
		return KbPreprocessFilter(GetAllocator(), filter.InputToken, filter.OutputToken);
	}

private:
	void operator=(const RawListToKb&);
};

template<>
class RawListToKb<KbFilter> : private WithAllocator {
public:
	typedef KbFilter output_type;
	RawListToKb(RawAllocator& allocator) : WithAllocator(allocator) {}

	KbFilter operator()(iKnow_KB_Filter filter) {
		return KbFilter(GetAllocator(), filter.InputToken, filter.OutputToken, filter.ApplyOnlyAtBeginning, filter.ApplyOnlyAtEnd, filter.IsConceptFilter, filter.IsRelationFilter);
	}

private:
	void operator=(const RawListToKb&);
};

template<>
class RawListToKb<KbProperty> : private WithAllocator {
public:
	typedef KbProperty output_type;
	RawListToKb(RawAllocator& allocator) : WithAllocator(allocator) {}

	KbProperty operator()(const std::pair<int, std::string>& kb_property) {
		return KbProperty(GetAllocator(), static_cast<PropertyId>(kb_property.first), kb_property.second);
	}

private:
	void operator=(const RawListToKb&);
};

template<>
class RawListToKb<KbMetadata> : private WithAllocator {
public:
	typedef KbMetadata output_type;
	RawListToKb(RawAllocator& allocator) : WithAllocator(allocator) {}

	KbMetadata operator()(iKnow_KB_Metadata metadata) {
		return KbMetadata(GetAllocator(), metadata.Name, metadata.Val);
	}

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

static const size_t kRawSize = 20*1024; // User dictionary is limited in size, 20k should suffice
unsigned char* data_buffer(bool b_init)
{
	static unsigned char udct_memory[kRawSize]; // raw representation of user dictionary data, only allocated when used.

	return (unsigned char*)(b_init ? memset(udct_memory, 0, sizeof udct_memory) : udct_memory);

}
unsigned char* iknow::shell::base_pointer = NULL;

unsigned char* UserKnowledgeBase::generateRAW(bool IsCompiled)
{
	if (!IsDirty()) { // no need to regenerate
		return data_buffer(false);
	}

	iknow::shell::Raw raw(data_buffer(true), kRawSize);
	iknow::shell::RawAllocator allocator(raw);

	RawKBData* kb_data_ = allocator.Insert(RawKBData());
	SetBasePointer(reinterpret_cast<unsigned char*>(kb_data_));

	BasePointerFrame frame_guard(reinterpret_cast<unsigned char*>(kb_data_)); // OFFSETPTRGUARD

	LabelIndexMap label_index_map; // label names mapped to (internal) label indexes.
	AttributeMapBuilder attribute_map_builder;

	size_t labels_count = static_cast<size_t>(LabelCount());
	if (labels_count > iknow::core::kMaxLabelCount) { //Make sure we don't have too many labels
		throw ExceptionFrom<UserKnowledgeBase>("Number of labels to load exceeds maximum limit.");
	}
	// Transform label data
	{
		RawListToKb<KbLabel> label_transformer(allocator, attribute_map_builder);
		const KbLabel* begin, * end;
		LoadKbRange(kb_labels.begin(), kb_labels.end(), labels_count, label_transformer, allocator, begin, end);
		kb_data_->labels_begin = begin;
		kb_data_->labels_end = end;
	}
	//Store the KbAttributeMap generated via the label loads
	kb_data_->attribute_map = allocator.Insert(attribute_map_builder.ToAttributeMap(allocator));

	//Build the map from label name to offset in labels table
	Builder<String, size_t> labels_table_builder(labels_count);
	for (size_t i = 0; i < labels_count; ++i) {
		const KbLabel* label = kb_data_->labels_begin + i;
		labels_table_builder.Insert(label->PointerToName(), allocator.Insert(i));
		//TODO: Other components should just use the kb_data_->labels table.
		label_index_map.insert(LabelIndexMap::value_type(label->Name(), static_cast<FastLabelSet::Index>(i)));
	}
	kb_data_->labels = allocator.Insert(labels_table_builder.Build(allocator));

	//Special labels : map with label index
	/* : no special labels for a user dictionary
	for (size_t i = BeginLabels; i != EndLabels; i++) {
		const size_t* p = kb_data_->labels->Lookup(GetSpecialLabel(SpecialLabel(i)));
		kb_data_->special_labels[SpecialLabel(i)] = *p;
	}
	*/

	//Lexreps : load only if not compiled
	{
		if (!IsCompiled) {
			kb_data_->max_lexrep_size = 0;
			kb_data_->max_token_size = 0;
			RawListToKb<KbLexrep> lexrep_transformer(allocator, label_index_map, kb_data_->max_lexrep_size, kb_data_->max_token_size);
			const iknow::shell::StaticHash::Table<String, KbLexrep>* lexrep_table;
			LoadKbRangeAsTable(kb_lexreps.begin(), kb_lexreps.end(), kb_lexreps.size(), lexrep_transformer, lexrep_table, [](const KbLexrep* lexrep) { return lexrep->PointerToToken(); }, allocator);
			kb_data_->lexreps = lexrep_table;
		}
	}
	//Rules
	{
		RawListToKb<KbRule> rule_transformer(allocator, label_index_map);
		const KbRule* begin, * end;
		LoadKbRange(kb_rules.begin(), kb_rules.end(), kb_rules.size(), rule_transformer, allocator, begin, end);
		kb_data_->rules_begin = begin;
		kb_data_->rules_end = end;

	}
	//Acronyms
	{
		RawListToKb<KbAcronym> acronym_transformer(allocator);
		const iknow::shell::StaticHash::Table<String, KbAcronym>* acronym_table;
		LoadKbRangeAsTable(kb_acronyms.begin(), kb_acronyms.end(), kb_acronyms.size(), acronym_transformer, acronym_table, [](const KbAcronym* acronym) { return acronym->PointerToToken(); }, allocator);
		kb_data_->acronyms = acronym_table;
	}
	//Regexes
	{
		RawListToKb<KbRegex> regex_transformer(allocator);
		const KbRegex* begin, * end;
		LoadKbRange(kb_regex.begin(), kb_regex.end(), kb_regex.size(), regex_transformer, allocator, begin, end);
		kb_data_->regexes_begin = begin;
		kb_data_->regexes_end = end;
	}
	//Preprocess Filters
	{
		RawListToKb<KbPreprocessFilter> preprocess_filter_transformer(allocator);
		const KbPreprocessFilter* begin, * end;
		LoadKbRange(kb_prepro.begin(), kb_prepro.end(), kb_prepro.size(), preprocess_filter_transformer, allocator, begin, end);
		kb_data_->preprocess_filters_begin = begin;
		kb_data_->preprocess_filters_end = end;
	}
	//Concept Filters
	{
		RawListToKb<KbFilter> concept_filter_transformer(allocator);
		const KbFilter* begin, * end;
		LoadKbRange(kb_filter.begin(), kb_filter.end(), kb_filter.size(), concept_filter_transformer, allocator, begin, end);
		kb_data_->concept_filters_begin = begin;
		kb_data_->concept_filters_end = end;
	}
	//Semantic properties
	{
		RawListToKb<KbProperty> property_transformer(allocator);
		const iknow::shell::StaticHash::Table<String, KbProperty>* property_table;
		LoadKbRangeAsTable(kb_properties.begin(), kb_properties.end(), kb_properties.size(), property_transformer, property_table, [](const KbProperty* property) { return property->PointerToName(); }, allocator);
		kb_data_->properties = property_table;
	}
	//Language metadata
	{
		RawListToKb<KbMetadata> metadata_transformer(allocator);
		const iknow::shell::StaticHash::Table<std::string, KbMetadata>* metadata_table;
		LoadKbRangeAsTable(kb_metadata.begin(), kb_metadata.end(), kb_metadata.size(), metadata_transformer, metadata_table, [](const KbMetadata* metadata) { return metadata->PointerToName(); }, allocator);
		kb_data_->metadata = metadata_table;
	}

	kb_data_->hash = std::hash<decltype(GetHash())>()(GetHash());
	m_IsDirty = false; // RAW reflects CSV state...
	return data_buffer(false);
}

iknow::base::String UserKnowledgeBase::GetSpecialLabel(SpecialLabel label) {
	switch (label) {
	case iknow::core::ConceptLabel:
		// return IkStringEncoding::UTF8ToBase(kb_concept_label->Name); // no default concept label in a user dictionary
		return IkStringEncoding::UTF8ToBase("Concept");
		break;
	case RelationLabel:
		return IkStringEncoding::UTF8ToBase("Relation");
		break;
	case UnknownLabel:
		return IkStringEncoding::UTF8ToBase("Unknown");
		break;
	case PunctuationLabel:
		return IkStringEncoding::UTF8ToBase("Punctuation");
		break;
	case SentenceBeginLabel:
		return IkStringEncoding::UTF8ToBase("SBegin");
		break;
	case SentenceEndLabel:
		return IkStringEncoding::UTF8ToBase("SEnd");
		break;
	case JoinLabel:
		return IkStringEncoding::UTF8ToBase("Join");
		break;
	case JoinReverse:
		return IkStringEncoding::UTF8ToBase("JoinReverse");
		break;
	case CapitalInitialLabel:
		return IkStringEncoding::UTF8ToBase("CapitalInitial");
		break;
	case CapitalMixedLabel:
		return IkStringEncoding::UTF8ToBase("CapitalMixed");
		break;
	case CapitalAllLabel:
		return IkStringEncoding::UTF8ToBase("CapitalAll");
		break;
	case SubjectLabel:
		return IkStringEncoding::UTF8ToBase("Subject");
		break;
	case ObjectLabel:
		return IkStringEncoding::UTF8ToBase("Object");
		break;
	case PrimaryRelationLabel:
		return IkStringEncoding::UTF8ToBase("PrimaryRelation");
		break;
	case NonSemanticLabel:
		return IkStringEncoding::UTF8ToBase("NonSemantic");
		break;
	case NumericLabel:
		return IkStringEncoding::UTF8ToBase("Numeric");
		break;
	case AlphaBeticLabel:
		return IkStringEncoding::UTF8ToBase("AlphaBetic");
		break;
	case SpaceLabel:
		return IkStringEncoding::UTF8ToBase("Space");
		break;
	case KatakanaLabel:
		return IkStringEncoding::UTF8ToBase("Katakana");
		break;
	default:
		throw ExceptionFrom<CSV_DataGenerator>("Unknown special label requested.");
	}
}

//////////////////
/// ATTRIBUTES ///
//////////////////

#define IKATTNEGATION	  	1
#define IKATTTIME		  	2
#define IKATTMODIFIER	  	3
#define IKATTNONSEMANTIC  	4
#define IKATTSENPOSITIVE  	5
#define IKATTSENNEGATIVE  	6
#define IKATTENTITYVECTOR 	7
#define IKATTTOPIC        	8
#define IKATTFREQ			9 
#define IKATTDURATION 		10
#define IKATTMEASURE	 	11
#define IKATTCERTAINTY		12

const std::vector<std::pair<int, string>> UserKnowledgeBase::kb_properties = {
	make_pair(IKATTNEGATION, "Negation"),
	make_pair(IKATTTIME, "DateTime"),
	make_pair(IKATTMODIFIER, "Modifier"),
	make_pair(IKATTNONSEMANTIC, "NonSemantic"),
	make_pair(IKATTSENPOSITIVE, "PositiveSentiment"),
	make_pair(IKATTSENNEGATIVE, "NegativeSentiment"),
	make_pair(IKATTENTITYVECTOR, "EntityVector"),
	make_pair(IKATTFREQ, "Frequency"),
	make_pair(IKATTDURATION, "Duration"),
	make_pair(IKATTMEASURE, "Measurement"),
	make_pair(IKATTCERTAINTY, "Certainty")
};

std::vector<std::string> split_row(std::string row_text, char split)
{
	std::vector<std::string> split_data;
	std::istringstream f(row_text);
	std::string s;
	while (std::getline(f, s, split)) {
		split_data.push_back(s);
	}
	return split_data;
}

iKnow_KB_Label LabelFromString(vector<string>& row_label, string& isDefault) // ClassMethod LabelFromString(line As %String, ByRef isDefault = "") As Label
{
	iKnow_KB_Label label; // Set label = ..%New()
	label.Name = row_label[3 - 1]; // Set label.Name = $PIECE(line, ";", 3)
	label.Type = row_label[4 - 1]; // Set label.Type = $PIECE(line, ";", 4)
	isDefault = row_label[6 - 1]; // Set isDefault = $PIECE(line, ";", 6)

	if (row_label.size() > 7) label.Attributes = row_label[8 - 1]; // Set label.Attributes = $PIECE(line, ";", 8)
	label.PhaseList = row_label[2 - 1]; // Set label.PhaseList = $PIECE(line, ";", 2)
	return label;
}

//
// constructor
// every KB needs the special labels loaded
//
UserKnowledgeBase::UserKnowledgeBase() : m_IsDirty(true)
{
	vector<string> special_labels = { // language independent labels
	";1,$;Concept;typeConcept;;0;",
	";1,$;Join;typeOther;;0;",
	";1,$;JoinReverse;typeOther;;0;",
	";1,$;NonRelevant;typeOther;;0;",
	";1,$;Punctuation;typeEndConcept;;0;",
	";1,$;Relation;typeRelation;;0;",
	";1,$;Numeric;typeOther;;0;",
	";1,$;Unknown;typeOther;;0;",
	";1,$;CapitalAll;typeAttribute;;0;",
	";1,$;CapitalInitial;typeAttribute;;0;",
	";1,$;CapitalMixed;typeAttribute;;0;",
	";1,$;NonSemantic;typeAttribute;;0;Entity(NonSemantic)",
	";1,$;User1;typeAttribute;;0;",
	";1,$;User2;typeAttribute;;0;",
	";1,$;User3;typeAttribute;;0;",
	";1,$;AlphaBetic;typeConcept;;0;",
	";1,$;Space;typeOther;;0;",
	";1,$;Katakana;typeConcept;;0;",
	";1,$;UDNegation;typeAttribute;;0;",
	";1,$;UDPosSentiment;typeAttribute;;0;",
	";1,$;UDNegSentiment;typeAttribute;;0;",
	";1,$;UDConcept;typeConcept;;0;",
	";1,$;UDRelation;typeRelation;;0;",
	";1,$;UDNonRelevant;typeNonRelevant;;0;",
	";1,$;UDUnit;typeAttribute;;0;",
	";1,$;UDNumber;typeAttribute;;0;",
	";1,$;UDTime;typeAttribute;;0;"
	};

	string isDefault = "";
	for (vector<string>::iterator it = special_labels.begin(); it != special_labels.end(); ++it) {
		vector<string> row_label = split_row(*it, ';');
		kb_labels.push_back(LabelFromString(row_label, isDefault)); // create and add label object
	}
}

int UserKnowledgeBase::addLexrepLabel(const std::string& token, const std::string& labels) {
	size_t startIndex = 0;
	size_t  endIndex = 0;
	while ((endIndex = labels.find(';', startIndex)) < labels.size())
	{
		string label = labels.substr(startIndex, endIndex - startIndex);
		if (!IsValidLabel(label))
			return -1;
		startIndex = endIndex + 1;
	}
	if (startIndex < labels.size())
	{
		string label = labels.substr(startIndex, endIndex - startIndex);
		if (!IsValidLabel(label))
			return -1;
	}
	kb_lexreps.push_back(iKnow_KB_Lexrep(token, labels));
	m_IsDirty = true; // need recompilation
	return 0;
}

void UserKnowledgeBase::addSEndCondition(const std::string& literal, const bool b_end) {
	kb_acronyms.push_back(iKnow_KB_Acronym(literal, b_end));
	m_IsDirty = true; // need recompilation
}

