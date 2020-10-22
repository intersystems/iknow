/*
** CSV_DataGenerator.cpp
*/

#ifdef WIN32
#pragma warning (disable: 4251)
#endif

#include "CSV_DataGenerator.h"
#include "Util.h"
#include "IkStringEncoding.h"

#include <string>
#include <iomanip>
#include <fstream>
#include <sstream>
#include <algorithm>

using namespace iknow::csvdata;
using namespace std;
using iknow::base::IkStringEncoding;
using iknow::base::String;
using namespace iknow::core;

vector<iKnow_KB_Metadata> CSV_DataGenerator::kb_metadata;
vector<iKnow_KB_Acronym> CSV_DataGenerator::kb_acronyms;
vector<iKnow_KB_Regex> CSV_DataGenerator::kb_regex;
vector<iKnow_KB_Filter> CSV_DataGenerator::kb_filter;
vector<iKnow_KB_Label> CSV_DataGenerator::kb_labels;
CSV_DataGenerator::lexreps_Type CSV_DataGenerator::kb_lexreps;
std::unordered_map<std::string, int> CSV_DataGenerator::lexrep_index;
vector<iKnow_KB_PreprocessFilter> CSV_DataGenerator::kb_prepro;
vector<iKnow_KB_Rule> CSV_DataGenerator::kb_rules;

CSV_DataGenerator::CSV_DataGenerator(string csv_path, string aho_path, string ldata_path) : 
	csv_path_(csv_path), aho_path_(aho_path), language_data_path_(ldata_path)
{
	// default data reservation, should do for most languages...

	kb_metadata.reserve(16);
	kb_acronyms.reserve(1024);
	kb_regex.reserve(1024);
	kb_filter.reserve(256);
	kb_labels.reserve(1024);
	kb_lexreps.reserve(144000);
	lexrep_index.reserve(144000);
	kb_prepro.reserve(256);
	kb_rules.reserve(2560);
}

CSV_DataGenerator::~CSV_DataGenerator()
{
}


iknow::base::String CSV_DataGenerator::GetSpecialLabel(SpecialLabel label) {
	switch (label) {
	case iknow::core::ConceptLabel:
		return IkStringEncoding::UTF8ToBase(kb_concept_label->Name);
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

/*
Method GetProperty(key As %Integer) As %List
{
Quit $case(key, 1:$lb($$$IKATTNEGATION, "Negation"), 2 : $lb($$$IKATTTIME, "DateTime"), 3 : $lb($$$IKATTMODIFIER, "Modifier"), 4 : $lb($$$IKATTNONSEMANTIC, "NonSemantic"), 5 : $lb($$$IKATTSENPOSITIVE, "PositiveSentiment"), 6 : $lb($$$IKATTSENNEGATIVE, "NegativeSentiment"), 7 : $LB($$$IKATTENTITYVECTOR, "EntityVector"), 8 : $LB($$$IKATTFREQ, "Frequency"), 9 : $LB($$$IKATTDURATION, "Duration"), 10 : $LB($$$IKATTMEASURE, "Measurement"), 11 : $LB($$$IKATTCERTAINTY, "Certainty"), : "")
}
*/
const std::vector<std::pair<int, string>> CSV_DataGenerator::kb_properties = {
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

void CSV_DataGenerator::loadCSVdata(std::string language, bool IsCompiled, std::ostream& os)
{
	/*
	Do ..LoadTableFromCSV(prefix _ "metadata.csv", "%iKnow.KB.Metadata", kb)
	Do ..LoadTableFromCSV(prefix _ "acro.csv", "%iKnow.KB.Acronym", kb)
	Do ..LoadTableFromCSV(prefix _ "regex.csv", "%iKnow.KB.Regex", kb)
	Do ..LoadTableFromCSV(prefix _ "filter.csv", "%iKnow.KB.Filter", kb)
	Do ..LoadTableFromCSV(prefix _ "labels.csv", "%iKnow.KB.Label", kb)
	// Quit:'##class(%File).Exists(fileName) ##class(%Exception.StatusException).%New("LoadTableFromCSV","9999",,$Listbuild("File " _ fileName _ " does *not* exist."))
	Do:complete ..LoadTableFromCSV(prefix _ "lexreps.csv", "%iKnow.KB.Lexrep", kb)
	Do ..LoadTableFromCSV(prefix _ "prepro.csv", "%iKnow.KB.PreprocessFilter", kb)
	Do ..LoadTableFromCSV(prefix _ "rules.csv", "%iKnow.KB.Rule", kb)
	*/
	kb_language = language;
	Hash = language; // just a unique string per KB
	stringstream message;
	message << "Loading CSV data for language \"" << language << "\"" << endl;
	cout << message.str(); os << message.str();

	kb_metadata.clear();
	size_t cap = kb_metadata.capacity();
	cout << "Reading metadata..." << endl;
	iKnow_KB_Metadata::ImportFromCSV(csv_path_ + language + "/" + "metadata.csv", *this);
	cout << kb_metadata.size() << " metadata items (reserved=" << cap << ")" << endl;

	kb_acronyms.clear();
	cap = kb_acronyms.capacity();
	cout << "Reading acronym data..." << endl;
	iKnow_KB_Acronym::ImportFromCSV(csv_path_ + language + "/" + "acro.csv", *this);
	cout << kb_acronyms.size() << " acronym items (reserved=" << cap << ")" << endl;

	kb_regex.clear();
	cap = kb_regex.capacity();
	cout << "Reading regex data..." << endl;
	iKnow_KB_Regex::ImportFromCSV(csv_path_ + language + "/" + "regex.csv", *this);
	cout << kb_regex.size() << " regex items (reserved=" << cap << ")" << endl;

	kb_filter.clear();
	cap = kb_filter.capacity();
	cout << "Reading filter data..." << endl;
	iKnow_KB_Filter::ImportFromCSV(csv_path_ + language + "/" + "filter.csv", *this);
	cout << kb_filter.size() << " filter items (reserved=" << cap << ")" << endl;

	kb_labels.clear();
	cap = kb_labels.capacity();
	cout << "Reading label data..." << endl;
	if (!iKnow_KB_Label::ImportFromCSV(csv_path_ + language + "/" + "labels.csv", *this))
		throw ExceptionFrom<CSV_DataGenerator>("Cannot build a language model without external labels !!!");
	cout << kb_labels.size() << " label items (reserved=" << cap << ")" << endl;

	if (!IsCompiled) {
		cap = kb_lexreps.capacity();
		iKnow_KB_Lexrep::ImportFromCSV(csv_path_ + language + "/" + "lexreps.csv", *this);
		cout << endl << kb_lexreps.size() << " lexrep items (reserved=" << cap << ")" << endl;
	}
	kb_prepro.clear();
	cap = kb_prepro.capacity();
	cout << "Reading prepro data..." << endl;
	iKnow_KB_PreprocessFilter::ImportFromCSV(csv_path_ + language + "/" + "prepro.csv", *this);
	cout << kb_prepro.size() << " prepro items (reserved=" << cap << ")" << endl;

	kb_rules.clear();
	cap = kb_rules.capacity();
	cout << "Reading rules data..." << endl;
	iKnow_KB_Rule::ImportFromCSV(csv_path_ + language + "/" + "rules.csv", *this);
	cout << kb_rules.size() << " rule items (reserved=" << cap << ")" << endl;
	if (!os.fail()) {
		os << left << "Rule CSV mapping:" << endl;
		for (int idRule = 0; idRule < kb_rules.size(); idRule++) {
			os << setw(12) << string(to_string(idRule) + ":" + kb_rules[idRule].csv_id) << "\t" << kb_rules[idRule].InputPattern << "\t->\t" << kb_rules[idRule].OutputPattern << "\t(" << kb_rules[idRule].Phase << ")" << endl;
		}
	}
	labelIndexTable_type &label_index_table = labelIndexTable;
	int label_idx = 0;
	if (!os.fail())
		os << endl << "Internal label numbers:" << endl << endl;
	for_each(kb_labels.begin(), kb_labels.end(), [&label_index_table, &label_idx, &os](iKnow_KB_Label label) { if (!os.fail()) os << label_idx << "\t" << label.Name << endl; label_index_table[label.Name] = label_idx++; }); // Do ..BuildLabelIndexTable(kb, .labelIndexTable)
	//Override value for "-"
	labelIndexTable["-"] = -1; //Set table("-") = -1
}

/// Returns the pattern of a given regex index
String CSV_DataGenerator::GetRegexPattern(String regexName) // Method GetRegexPattern(regexName As %String) As %String
{
	// Set pattern = ""
	//	Set kbId = ..%Id()
	//	&sql(select Pattern into : pattern from Regex where Knowledgebase = : kbId and Name = : regexName)
	for (vector<iKnow_KB_Regex>::iterator it_regex = kb_regex.begin(); it_regex != kb_regex.end(); ++it_regex) {
		if (it_regex->name == IkStringEncoding::BaseToUTF8(regexName)) {
			return IkStringEncoding::UTF8ToBase(it_regex->Pattern); // Quit $ZCONVERT(pattern, "I", "UTF8")
		}
	}
	return String();
}

static const size_t kRawSize = 48000000;
#define INCLUDE_GENERATE_IMAGE_CODE
#include "RawBlock.h"
#include "OffsetPtr.h"
#include "KbLabel.h"
#include "IkKnowledgebase.h"
#include "KbLexrep.h"
#include "KbPreprocessFilter.h"
#include "KbProperty.h"
#include "KbAcronym.h"
#include "KbRegex.h"
#include "KbRule.h"
#include "KbFilter.h"
#include "KbInputFilter.h"
#include "KbMetadata.h"

// static definition:
// const IkLabel::LabelTypeMap IkLabel::label_type_map_;

using namespace iknow::shell;

struct RawKBData {
	OffsetPtr<const KbLabel> labels_begin;
	OffsetPtr<const KbLabel> labels_end;
	//Table from label name to offset from labels_begin
	typedef StaticHash::Table<iknow::base::String, size_t> LabelsTable;
	OffsetPtr<const LabelsTable> labels;
	//Array of "special" label indexes
	size_t special_labels[iknow::core::EndLabels];
	typedef StaticHash::Table<iknow::base::String, KbLexrep> LexrepsTable;
	OffsetPtr<const LexrepsTable> lexreps;
	typedef StaticHash::Table<iknow::base::String, KbProperty> PropertyTable;
	OffsetPtr<const PropertyTable> properties;
	OffsetPtr<const KbRegex> regexes_begin;
	OffsetPtr<const KbRegex> regexes_end;
	OffsetPtr<const KbRule> rules_begin;
	OffsetPtr<const KbRule> rules_end;
	typedef StaticHash::Table<iknow::base::String, KbAcronym> AcronymsTable;
	OffsetPtr<const AcronymsTable> acronyms;
	OffsetPtr<const KbPreprocessFilter> preprocess_filters_begin;
	OffsetPtr<const KbPreprocessFilter> preprocess_filters_end;
	OffsetPtr<const KbFilter> concept_filters_begin;
	OffsetPtr<const KbFilter> concept_filters_end;
	OffsetPtr<const KbInputFilter> input_filters_begin;
	OffsetPtr<const KbInputFilter> input_filters_end;
	OffsetPtr<const KbAttributeMap> attribute_map;
	typedef StaticHash::Table<std::string, KbMetadata> MetadataTable;
	OffsetPtr<const MetadataTable> metadata;
	size_t max_lexrep_size;
	size_t max_token_size; // JDN: for Asian languages (not whitespace separated) we need to know the maximum length of tokens
	bool IsAsian; // JDN: indicates Asian language.
	size_t hash;
};

using namespace iknow::base;
using namespace iknow::core;

typedef map<String, FastLabelSet::Index> LabelIndexMap;

class CacheList;

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
class RawListToKb<KbLabel> : private WithAllocator, private WithAttributeMap{
public:
	typedef KbLabel output_type;
	RawListToKb(RawAllocator& allocator, AttributeMapBuilder& attribute_map) : WithAllocator(allocator), WithAttributeMap(attribute_map) {}

#if defined _IRIS
	KbLabel operator()(CacheList& list) {
		return KbLabel(GetAllocator(), list[1].AsAString(), list[2].AsAString(), list[3].AsAString(), list[4].AsAString(), GetAttributeMap());
	}
#else
	KbLabel operator()(iKnow_KB_Label label) {
		return KbLabel(GetAllocator(), label.Name, label.Type, label.Attributes, label.PhaseList, GetAttributeMap());
	}
#endif

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
class RawListToKb<KbAcronym> : private WithAllocator{
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
class RawListToKb<KbMetadata> : private WithAllocator{
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

unsigned char* iknow::shell::base_pointer = NULL;

void CSV_DataGenerator::generateRAW(bool IsCompiled)
{
	unsigned char* buf_ = new unsigned char[kRawSize];
	iknow::shell::Raw raw(buf_, kRawSize);
	iknow::shell::RawAllocator allocator(raw);

	RawKBData* kb_data_ = allocator.Insert(RawKBData());
	SetBasePointer(reinterpret_cast<unsigned char*>(kb_data_));

	BasePointerFrame frame_guard(reinterpret_cast<unsigned char*>(kb_data_)); // OFFSETPTRGUARD

	LabelIndexMap label_index_map; // label names mapped to (internal) label indexes.
	AttributeMapBuilder attribute_map_builder;

	size_t labels_count = static_cast<size_t>(LabelCount());
	if (labels_count > iknow::core::kMaxLabelCount) { //Make sure we don't have too many labels
		throw ExceptionFrom<CSV_DataGenerator>("Number of labels to load exceeds maximum limit.");
	}
	// Transform label data
	{
		RawListToKb<KbLabel> label_transformer(allocator, attribute_map_builder);
		const KbLabel *begin, *end;
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
	for (size_t i = BeginLabels; i != EndLabels; i++) {
		const size_t* p = kb_data_->labels->Lookup(GetSpecialLabel(SpecialLabel(i)));
		kb_data_->special_labels[SpecialLabel(i)] = *p;
	}

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
		const KbRule *begin, *end;
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
		const KbRegex *begin, *end;
		LoadKbRange(kb_regex.begin(), kb_regex.end(), kb_regex.size(), regex_transformer, allocator, begin, end);
		kb_data_->regexes_begin = begin;
		kb_data_->regexes_end = end;
	}
	//Preprocess Filters
	{
		RawListToKb<KbPreprocessFilter> preprocess_filter_transformer(allocator);
		const KbPreprocessFilter *begin, *end;
		LoadKbRange(kb_prepro.begin(), kb_prepro.end(), kb_prepro.size(), preprocess_filter_transformer, allocator, begin, end);
		kb_data_->preprocess_filters_begin = begin;
		kb_data_->preprocess_filters_end = end;
	}
	//Concept Filters
	{
		RawListToKb<KbFilter> concept_filter_transformer(allocator);
		const KbFilter *begin, *end;
		LoadKbRange(kb_filter.begin(), kb_filter.end(), kb_filter.size(), concept_filter_transformer, allocator, begin, end);
		kb_data_->concept_filters_begin = begin;
		kb_data_->concept_filters_end = end;
	}
	//Semantic properties
	{
		RawListToKb<KbProperty> property_transformer(allocator);
		const iknow::shell::StaticHash::Table<String, KbProperty>* property_table;
		LoadKbRangeAsTable(kb_properties.begin(), kb_properties.end(), kb_properties.size(), property_transformer, property_table, [](const KbProperty* property){ return property->PointerToName(); }, allocator);
		kb_data_->properties = property_table;
	}
	//Language metadata
	{
		RawListToKb<KbMetadata> metadata_transformer(allocator);
		const iknow::shell::StaticHash::Table<string, KbMetadata>* metadata_table;
		LoadKbRangeAsTable(kb_metadata.begin(), kb_metadata.end(), kb_metadata.size(), metadata_transformer, metadata_table, [](const KbMetadata* metadata) { return metadata->PointerToName(); }, allocator);
		kb_data_->metadata = metadata_table;
	}

	kb_data_->hash = std::hash<decltype(GetHash())>()(GetHash());

#ifdef INCLUDE_GENERATE_IMAGE_CODE
	allocator.generate_image(language_data_path_, GetName());
#endif

	delete[] buf_;
}


class RegexPredicate : public Predicate // Class %iKnow.Compiler.RegexPredicate Extends %iKnow.Compiler.Predicate[Hidden, System = 4]
{
public:
	static bool MatchRegex; // Property MatchRegex As %Boolean; /// If true, matches regex lexreps. If false, matches non-regex lexreps

  /*static*/ bool Check(iKnow_KB_Lexrep& obj) { // Method Check(obj As %iKnow.KB.Lexrep) As %Boolean
		const string &token = obj.Token; // Set token = obj.Token
		// bool foundOutput = MatchRegex; // Set foundOutput = ..MatchRegex
		bool escaping = false; // Set escaping = 0
		// Set len = $L(token)
		for (int i = 1; i <= token.length(); ++i) { // For i = 1:1 : len{
			char char_ = token[i - 1]; // Set char = $E(token, i)
			if (char_ == '\\' && !escaping) { escaping = true; continue; } // If(char = "\") && 'escaping Set escaping = 1 Continue
			if (char_ == '{' && !escaping) return MatchRegex; // If(char = "{") && 'escaping Return ..MatchRegex
			escaping = false; // Set escaping = 0
		}
		return !MatchRegex; // Quit '..MatchRegex
	}
	void operator()(iKnow_KB_Lexrep& lexrep) {
		lexrep.isRegex = Check(lexrep);
	}
};
bool RegexPredicate::MatchRegex = true; // scan for regular expressions.

void CSV_DataGenerator::generateAHO(void) 
{
	cout << "Compiling " << GetName() << " outputDir=\"" << aho_path_ << "\"" << std::endl;
	string outputDir = aho_path_ + "/inl/" + GetName() + "/lexrep";

	for_each(kb_lexreps.begin(), kb_lexreps.end(), RegexPredicate());
	
	//RegexPredicate *predicate = new RegexPredicate; // // Set predicate = ##class(RegexPredicate).%New()
	//First the non-regexes
	bool predicate = false; //predicate->MatchRegex = false;
	CompileLexrepDictionaryPhase(/*kb,*/ "", predicate, outputDir); // Do ..CompileLexrepDictionaryPhase(kb, "", predicate, outputDir)
	//Now the regexes
	predicate = true; //predicate->MatchRegex = true; // Set predicate.MatchRegex = 1
	CompileLexrepDictionaryPhase(/*kb,*/ "_regex", predicate, outputDir); // Do ..CompileLexrepDictionaryPhase(kb, "_regex", predicate, outputDir)
	// delete predicate;
}

#include "GotoFunction.h"
#include "KnowledgebaseRegexDictionary.h"
#include "LexrepOutputFunction.h"
#include "FailureFunction.h"
#include "LexrepStateOutputFunc.h"
#include "MetadataTable.h"

vector<int> CSV_DataGenerator::CreateLabelsIndexVector(iKnow_KB_Lexrep& lexrep, std::unordered_map<string, int>& table) // ClassMethod CreateLabelsIndexVector(lexrep As %iKnow.KB.Lexrep, ByRef table As %String) As %List[Private]
{
	vector<string> labelList = split_row(lexrep.Labels, ';'); // Set labelList = lexrep.GetLabels()
	int labelListLen = (int)labelList.size(); // Set labelListLen = $Length(labelList, ";")
	vector<int> indexList; // Set indexList = ""
	for (int j = 1; j <= labelListLen; ++j) {
		string labelName = labelList[j - 1];
		if (labelName == "") continue;
		labelIndexTable_type::iterator it = table.find(labelName);
		if (it == table.end()) { // If '$Data(table(labelName)) {
			cout << endl << "Missing label (" << labelName << ") from lexrep """ << lexrep.Token << """(" << lexrep.Labels << ") in label table."; // Write !, "Missing label (" _ labelName _ ") from lexrep """ _ lexrep.Token _ """(" _ labelList _ ") in label table."
			break; // Break
		}
		indexList.push_back(it->second);
	}
	return indexList;
}

void CSV_DataGenerator::CompileLexrepDictionaryPhase(/*kb As %iKnow.KB.Knowledgebase,*/ string phase, bool phase_switch /*Predicate *phasePredicate*/, string& outputDir_)
{
	cout << "CompileLexrepDictionaryPhase " << phase << endl;

	string language = GetName();
	vector<string> pieces = split_row(outputDir_, '/');
	for_each(pieces.begin(), pieces.end(), [phase,language](string& piece){ if (piece == language) piece = language + phase;  });
	std::string outputDir = merge_row(pieces, '/');

	bool isIdeographic = (MetadataLookup("IsJapanese") == "1"); // (kb.MetadataLookup("IsJapanese") = 1)
	bool hasRegex = (phase == "_regex"); // Set hasRegex = (phase = "_regex")

	iknow::AHO::GotoFunction *gotoFunc = new iknow::AHO::GotoFunction; // Set gotoFunc = ##class(GotoFunction).%New()
	gotoFunc->RegexEnabled = phase_switch; // regex_predicate->MatchRegex;
	gotoFunc->RegexDictionary = new iknow::AHO::KnowledgebaseRegexDictionary; // Set gotoFunc.RegexDictionary = ##class(KnowledgebaseRegexDictionary).%New()
	gotoFunc->RegexDictionary->Knowledgebase = this; // Set gotoFunc.RegexDictionary.Knowledgebase = kb

	iknow::AHO::LexrepOutputFunction *outputFunc = new iknow::AHO::LexrepOutputFunction; // Set outputFunc = ##class(LexrepOutputFunction).%New()
	outputFunc->StatesGlobal_Output.reserve(400000); // to be on the safe side :-)
	iknow::AHO::LexrepStateOutputFunc *outputAdapter = (isIdeographic ? new iknow::AHO::LexrepIdeographicStateOutputFunction : new iknow::AHO::LexrepStateOutputFunc); // Set outputAdaptor = ##class(LexrepIdeographicStateOutputFunction).%New()
	outputAdapter->pOutputFunc = outputFunc;

	cout << "Building goto and first output table..." << endl;
	lexreps_Type::iterator key = kb_lexreps.begin(); // Set key = kb.NextLexrep("")
	while (key != kb_lexreps.end()) { // While key '= ""
		iKnow_KB_Lexrep &lexrep = *key; // Set lexrep = kb.GetLexrep(key)
		// if (!regex_predicate->Check(lexrep)) { ++key; continue; } // If 'phasePredicate.Check(lexrep) Goto SkipLexrep
		if (!(lexrep.isRegex == phase_switch)) { ++key; continue; } // If 'phasePredicate.Check(lexrep) Goto SkipLexrep
		vector<int> labels = CreateLabelsIndexVector(lexrep, labelIndexTable); // Set labels = ..CreateLabelsIndexVector(lexrep, .labelIndexTable)

		iknow::base::String token = IkStringEncoding::UTF8ToBase(lexrep.Token); // Set token = $ZCONVERT(lexrep.Token, "I", "UTF8")

		outputAdapter->Labels = labels; // Set outputAdaptor.Labels = labels
		// Set labelString = lexrep.Labels
		// If $E(labelString, *) = ";" Set $E(labelString, $L(labelString)) = ""  // remove ending ';' for conformity		
		outputAdapter->LabelString = lexrep.Labels; // // Set outputAdaptor.LabelString = labelString
		outputAdapter->MetaString = lexrep.Meta;

		String word = (isIdeographic ? token : SpaceString() + token + SpaceString());

		gotoFunc->Addword(word, outputAdapter);

// SkipLexrep:
		++key;  // Set key = kb.NextLexrep(key)
	}
	iknow::AHO::MetadataTable *metadataTable = new iknow::AHO::MetadataTable; // Set metadataTable = ##class(MetadataTable).%New()
	if (iknow::AHO::LexrepStateOutputFunc *p_outputAdapter = dynamic_cast<iknow::AHO::LexrepStateOutputFunc*>(outputAdapter)) { // We have a LexrepStateOutputFunc here
		metadataTable->AddValue(p_outputAdapter->MaxWordCount); // Do metadataTable.AddValue(outputAdaptor.MaxWordCount)
	}
	metadataTable->AddValue(isIdeographic); // do metadataTable.AddValue(isIdeographic)
	metadataTable->AddValue(hasRegex); // do metadataTable.AddValue(hasRegex)

	cout << "Building failure and second output table..." << endl;
	iknow::AHO::FailureFunction *failureFunc = iknow::AHO::FailureFunction::Create(gotoFunc, outputFunc); // Set failureFunc = ##class(FailureFunction).Create(gotoFunc, outputFunc)
	// Set sc = ##class(%File).CreateDirectoryChain(outputDir)
	// If 'sc Throw ##class(%Exception.StatusException).CreateFromStatus(sc)

	cout << "Writing goto table data..." << endl;
	gotoFunc->ToC(outputDir); // Do gotoFunc.ToC(outputDir)
	cout << "writing output table data..." << endl;
	outputFunc->ToC(outputDir); // Do outputFunc.ToC(outputDir)
	cout << "writing failure table data..." << endl;
	failureFunc->ToC(outputDir); // Do failureFunc.ToC(outputDir)
	cout << "writing metadata table..." << endl;
	metadataTable->ToC(outputDir); // Do metadataTable.ToC(outputDir)

	cout << "Cleanup..." << endl << endl;
	delete metadataTable;
	delete outputAdapter;
	delete outputFunc;
	delete gotoFunc->RegexDictionary;
	delete gotoFunc;
}

//
// Utility functions...
//
std::vector<std::string> CSV_DataGenerator::split_row(std::string row_text, char split)
{
	std::vector<std::string> split_data;
	std::istringstream f(row_text);
	std::string s;
	while (std::getline(f, s, split)) {
		split_data.push_back(s);
	}
	return split_data;
}
std::string CSV_DataGenerator::merge_row(std::vector<std::string>& row_vector, char split)
{
	std::string merge;
	static const char split_string[] = { split, '\0' };
	static const std::string Split(split_string);
	for_each(row_vector.begin(), row_vector.end(), [&merge](std::string& piece) mutable { merge += (piece + Split);  });
	return merge;
}
void CSV_DataGenerator::handle_UTF8_BOM(std::ifstream& ifs)
{
	char utf8BOM[3];
	ifs.read(utf8BOM, 3);
	if (utf8BOM[0] == char('\xEF') && utf8BOM[1] == char('\xBB') && utf8BOM[2] == char('\xBF'));  // file is utf8 with BOM
	else { // file should be utf8 encoded, but no BOM present
		ifs.putback(utf8BOM[2]);
		ifs.putback(utf8BOM[1]);
		ifs.putback(utf8BOM[0]);
	}
}
