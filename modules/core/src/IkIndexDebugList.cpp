/*
** IkIndexDebug : module for tracing linguistic processing and performance
*/
#ifdef AIX
#else
#include <chrono>
#endif
#include <sstream>

#include "IkIndexDebug.h"
#include "IkStringEncoding.h"
#include "IkLexrep.h"
#include "IkMergedLexrep.h"
#include "IkRuleInputPattern.h"
#include "IkRuleOutputPattern.h"

using namespace iknow::core;
using namespace iknow::base;

//ToList is overloaded to produce CacheList output values for
//various objects in a trace.

//For label sets

static inline iknow::base::String AttributeName(AttributeId id, const IkKnowledgebase& kb) {
  AttributeString s = kb.AttributeNameForId(id);
  return iknow::base::String(s.data, s.size);
}

static Utf8List ToList(FastLabelSet::Index label, size_t attribute_pos, const IkKnowledgebase& kb) {
    Utf8List attribute;
    attribute.push_back(IkStringEncoding::BaseToUTF8(AttributeName(kb.GetAttributeType(label, attribute_pos), kb)));
    for (const AttributeId* i = kb.GetAttributeParamsBegin(label, attribute_pos);
        i != kb.GetAttributeParamsEnd(label, attribute_pos);
        ++i) {
        attribute.push_back(IkStringEncoding::BaseToUTF8(AttributeName(*i, kb)));
    }
    return attribute;
}

static const char* TypeToString(IkLabel::Type t) {
  switch (t) {
  case IkLabel::Unknown:
    return "Unknown";
  case IkLabel::Nonrelevant:
    return "Nonrelevant";
  case IkLabel::Ambiguous:
    return "Ambiguous";
  case IkLabel::Attribute:
    return "Attribute";
  case IkLabel::Concept:
    return "Concept";
  case IkLabel::Relation:
    return "Relation";
  case IkLabel::BeginConcept:
    return "BeginConcept";
  case IkLabel::EndConcept:
    return "EndConcept";
  case IkLabel::BeginEndConcept:
    return "BeginEndConcept";
  case IkLabel::BeginRelation:
    return "BeginRelation";
  case IkLabel::EndRelation:
    return "EndRelation";
  case IkLabel::BeginEndRelation:
    return "BeginEndRelation";
  case IkLabel::Literal:
	return "Literal";
  case IkLabel::PathRelevant:
    return "PathRelevant";
  default:
    return "???";
  }
}

static Utf8List ToList(const IkLexrep& lexrep, const IkKnowledgebase& kb) {
    Utf8List out;
    std::string trace_data("<lexrep");
    trace_data += " id=" + std::to_string(lexrep.GetId());
    trace_data += std::string(" type=") + TypeToString(lexrep.GetLexrepType());
    trace_data += " value=\""+IkStringEncoding::BaseToUTF8(lexrep.GetValue())+"\" index=\""+IkStringEncoding::BaseToUTF8(lexrep.GetNormalizedValue())+"\"";

    typedef std::set<FastLabelSet::Index> LabelSet;
    LabelSet labels;
    for (Phase p = 0; p < kPhaseCount; ++p) { // Collect labels for all phases.
        if (!lexrep.GetLabels(p).Empty()) { // phase has labels
            const FastLabelSet& phase_labels = lexrep.GetLabels(p);
            for (size_t i = 0; i < static_cast<size_t>(phase_labels.Size()); ++i) {
                FastLabelSet::Index index = phase_labels.At(i);
                labels.insert(index);
            }
        }
    }
    trace_data += " labels=\"";
    for (LabelSet::iterator it = labels.begin(); it != labels.end(); it++) {
        FastLabelSet::Index idx = *it;
        trace_data += IkStringEncoding::BaseToUTF8(kb.GetAtIndex(idx).GetName());
        size_t attribute_count = kb.GetAttributeCount(idx);
        if (attribute_count) {
            std::string attr_data("(a:");
            for (size_t j = 0; j < attribute_count; ++j) {
                Utf8List trace = ToList(idx, j, kb);
                std::for_each(trace.begin(), trace.end(), [&attr_data](std::string& s) mutable { attr_data+=s+","; });
            }
            attr_data += ")";
            trace_data += attr_data;
        }
        trace_data += ";";
    }
    trace_data += "\" />";
    out.push_back(trace_data);
    return out;
}

//Without KB data
static Utf8List ToList(const IkLexrep& lexrep) {
    Utf8List out;
    out.push_back(std::to_string((int)lexrep.GetId()));
    out.push_back(TypeToString(lexrep.GetLexrepType()));
    out.push_back(IkStringEncoding::BaseToUTF8(lexrep.GetValue()));
    out.push_back(IkStringEncoding::BaseToUTF8(lexrep.GetNormalizedValue()));
    out.push_back("sum="+std::to_string(lexrep.GetSummaryRelevance()));
    //No labels since we can't translate them without the KB.
    return out;
}

//For lexreps
static Utf8List ToList(const IkMergedLexrep& lexrep, const IkKnowledgebase& kb) {
    Utf8List out;
    for (IkMergedLexrep::const_iterator i = lexrep.LexrepsBegin(); i != lexrep.LexrepsEnd(); ++i) {
        Utf8List trace = ToList(*i, kb);
        std::for_each(trace.begin(), trace.end(), [&out](std::string& s) mutable { out.push_back(s); });
    }
    out.push_back("sum="+std::to_string(lexrep.GetSummaryRelevance()));
    return out;
}

//For lexreps without KB data
static Utf8List ToList(const IkMergedLexrep& lexrep) {
    Utf8List out;
    for (IkMergedLexrep::const_iterator i = lexrep.LexrepsBegin(); i != lexrep.LexrepsEnd(); ++i) {
        Utf8List trace = ToList(*i);
        std::for_each(trace.begin(), trace.end(), [&out](std::string& s) mutable { out.push_back(s); });
    }
    out.push_back("sum="+std::to_string(lexrep.GetSummaryRelevance()));
    return out;
}

static String ToString(const IkRuleInputPattern& pattern, const IkKnowledgebase& kb) {
  String out;
  if (pattern.IsVariable()) {
	  out += (pattern.IsNullVariable() ? '.' : '*');
	  if (pattern.IsNarrow()) out += '<';
  }
  for (size_t i=0; i < IkRuleInputPattern::kPatternSize; ++i) {
    FastLabelSet::Index index = pattern.GetIndex(i);
    if (index == FastLabelSet::NPos()) break;
	bool bTypeLabel = false;
	switch (pattern.GetOption(i)) {
	case IkRuleInputPattern::kSingleLabel:
		out += '=';
		out += kb.GetAtIndex(index).GetName();
		break;
	case IkRuleInputPattern::kNotSingleLabel:
		out += '^';	// need to separate for multi-character character constant error on lnxrhx64
		out += '=';
		out += kb.GetAtIndex(index).GetName();
		break;
	case IkRuleInputPattern::kSingleLabelInPhase:
		out += '~';
		out += kb.GetAtIndex(index).GetName();
		break;
	case IkRuleInputPattern::kNotSingleLabelInPhase:
		out += '^'; // need to separate for multi-character character constant error on lnxrhx64
		out += '~';
		out += kb.GetAtIndex(index).GetName();
		break;
	case IkRuleInputPattern::kNegated: 
		out += '^';
	case IkRuleInputPattern::kNormal:
		out += kb.GetAtIndex(index).GetName();
		break;
	case IkRuleInputPattern::kTypeNegated:
		out += '^';
	case IkRuleInputPattern::kTypeNormal:
		bTypeLabel = true;
		out += iknow::base::IkStringEncoding::UTF8ToBase(IkLabel::LabelTypeToString(static_cast<iknow::core::IkLabel::Type>(index))); // Index is label type
		break;
	}
	size_t or_offset = i*IkRuleInputPattern::kOrLabels;
	for (size_t j = or_offset; j <= or_offset + IkRuleInputPattern::kOrLabels; j++) {
		FastLabelSet::Index or_index = pattern.GetOrIndex(j);
		if (or_index == FastLabelSet::NPos()) break;
		out += ':'; // or separator 
		if (bTypeLabel) {
			out += iknow::base::IkStringEncoding::UTF8ToBase(IkLabel::LabelTypeToString(static_cast<iknow::core::IkLabel::Type>(index))); // Index is label type
		} else {
			out += kb.GetAtIndex(or_index).GetName();
		}
	}
    out += '+';
  }
  if (!out.empty()) out.erase(out.size() - 1); //Delete last +
  return out;
}

static std::string ToList(const IkRuleInputPattern* begin, const IkRuleInputPattern* end, const IkKnowledgebase& kb) {
    std::string trace_data;
    for (const IkRuleInputPattern* i = begin; i != end; ++i) {
        trace_data+=iknow::base::IkStringEncoding::BaseToUTF8(ToString(*i, kb))+";";
    }
    return trace_data;
}

static String ToString(const IkRuleOutputPattern& pattern, const IkKnowledgebase& kb) {
  String out;
  IkRuleOption option = pattern.GetOptions();
  for (const IkRuleOutputAction* i = pattern.ActionsBegin(); i != pattern.ActionsEnd(); ++i) {
    IkRuleOutputAction::Action action = i->GetAction();
    if (i == pattern.ActionsBegin()) {
      //We don't know how to display the option until we know the first action.
      //We only want to show a + if the next action is an add
      if (option == IkRuleOption::GetAdditionRuleOptions()) {
		if (action == IkRuleOutputAction::kAddLabel) out += option.ToString();
      } else { // Otherwise display it unconditionally.
		out += option.ToString();
      }
    }
    FastLabelSet::Index index = i->GetLabel();
    if (index == FastLabelSet::NPos()) break;
    //Skip the + if we already displayed it above.
    if (action == IkRuleOutputAction::kAddLabel && i != pattern.ActionsBegin()) out += '+';
    if (action == IkRuleOutputAction::kRemoveLabel) out += '-';
    out += kb.GetAtIndex(index).GetName();
  }
  return out;
}

static std::string ToList(const IkRuleOutputPattern* begin, const IkRuleOutputPattern* end, const IkKnowledgebase& kb) {
    std::string trace_data;
    for (const IkRuleOutputPattern* i = begin; i != end; ++i) {
        trace_data+=IkStringEncoding::BaseToUTF8(ToString(*i, kb))+";";
    }
    return trace_data;
}

//Now, the traces themselves.

void IkIndexDebug<Utf8List>::SwitchKnowledgebase(const String& old_language, const String& new_language, double certainty) {
    Utf8List out;
    out.push_back(IkStringEncoding::BaseToUTF8(old_language));
    out.push_back(IkStringEncoding::BaseToUTF8(new_language));
    out.push_back(std::to_string(certainty));
    trace_.Add("SwitchKnowledgebase", out);
}

void IkIndexDebug<Utf8List>::ApplyRule(size_t rule_id, Lexreps::iterator match, size_t match_length, const IkKnowledgebase& kb)
{
    Utf8List out;
    out.push_back("rule_id="+ std::to_string((int)rule_id));
    out.push_back("match_length="+ std::to_string((int)match_length));
    out.push_back("lexrep_match=");
    for (size_t i = 0; i < match_length; ++i) {
        Utf8List trace = ToList(*match++, kb);
        std::for_each(trace.begin(), trace.end(), [&out](std::string& s) mutable { out.push_back(s); });
    }
    IkRule rule = kb.GetRule(rule_id);
    std::string trace_data("rule_input=\""+ ToList(rule.InputBegin(), rule.InputEnd(), kb)+"\"");
    trace_data += " rule_output=\""+ ToList(rule.OutputBegin(), rule.OutputEnd(), kb)+"\"";
    trace_data += " rule_phase="+ std::to_string(rule.GetPhase());
    out.push_back(trace_data);
    trace_.Add("RuleApplication", out);
}

void IkIndexDebug<Utf8List>::RuleApplied(size_t rule_id, Lexreps::iterator match, size_t match_length, const IkKnowledgebase& kb)
{
    Utf8List out;
    out.push_back("rule_id=" + std::to_string((int)rule_id));
    out.push_back("match_length=" + std::to_string((int)match_length));
    for (size_t i = 0; i < match_length; ++i) {
        Utf8List trace = ToList(*match++, kb);
        std::for_each(trace.begin(), trace.end(), [&out](std::string& s) mutable { out.push_back(s); });
    }
    trace_.Add("RuleApplicationResult", out);
}

void IkIndexDebug<Utf8List>::RulesComplete(const IkLexrep& lexrep, const IkKnowledgebase& kb) {
    trace_.Add("RulesComplete", ToList(lexrep, kb));
}

void IkIndexDebug<Utf8List>::AmbiguityResolved(const IkLexrep& lexrep, const IkKnowledgebase& kb) {
    trace_.Add("AmbiguityResolved", ToList(lexrep, kb));
}

void IkIndexDebug<Utf8List>::PreprocessToken(const String& original, const String& replacement) {
    if (original == replacement) return;
    Utf8List out;
    out.push_back(std::string("\"" + IkStringEncoding::BaseToUTF8(original) + "\"=\"" + IkStringEncoding::BaseToUTF8(replacement) + "\""));
    trace_.Add("PreprocessToken", out);
}

void IkIndexDebug<Utf8List>::NormalizeToken(const String& original, const String& replacement) {
    if (original == replacement) return;
    Utf8List out;
    out.push_back(std::string("\"" + IkStringEncoding::BaseToUTF8(original) + "\"=\"" + IkStringEncoding::BaseToUTF8(replacement) + "\""));
    trace_.Add("NormalizeToken", out);
}

void IkIndexDebug<Utf8List>::LexrepCreated(const IkLexrep& lexrep, const IkKnowledgebase& kb) {
    trace_.Add("LexrepCreated", ToList(lexrep, kb));
}

void IkIndexDebug<Utf8List>::LexrepIdentified(const IkLexrep& lexrep, const IkKnowledgebase& kb) {
    Utf8List out = ToList(lexrep, kb);
    if (lexrep.GetMetaData()) {
        out.push_back(lexrep.GetMetaData());
    }
    trace_.Add("LexrepIdentified", out);
}

void IkIndexDebug<Utf8List>::UserDictionaryMatch(const IkLexrep& lexrep, const IkKnowledgebase& kb) {
    trace_.Add("UserDictionaryMatch", ToList(lexrep, kb));
}

void IkIndexDebug<Utf8List>::AttributeDetected(const std::string& attribute, const IkLexrep& lexrep, const IkKnowledgebase& kb) {
    Utf8List out;
    out.push_back(attribute);
    Utf8List trace = ToList(lexrep, kb);
    std::for_each(trace.begin(), trace.end(), [&out](std::string& s) mutable { out.push_back(s); });
    trace_.Add("AttributeDetected", out);
}

void IkIndexDebug<Utf8List>::LexrepTypeAssignment(const IkLexrep& lexrep, const IkKnowledgebase& kb) {
  trace_.Add("LexrepTypeAssignment", ToList(lexrep, kb));
}

void IkIndexDebug<Utf8List>::MergingRelations(Lexreps::const_iterator begin, Lexreps::const_iterator end, const IkKnowledgebase& kb) {
  for (Lexreps::const_iterator i = begin; i != end; ++i) {
    MergingRelation(*i, kb);
  }
}

void IkIndexDebug<Utf8List>::MergingRelation(const IkLexrep& lexrep, const IkKnowledgebase& kb) {
  trace_.Add("MergingRelation", ToList(lexrep, kb));
}

void IkIndexDebug<Utf8List>::MergedRelation(const IkMergedLexrep& lexrep, const IkKnowledgebase& kb) {
  trace_.Add("MergedRelation", ToList(lexrep, kb));
}

void IkIndexDebug<Utf8List>::MergedRelationNonrelevant(const IkMergedLexrep& lexrep, const IkKnowledgebase& kb) {
  trace_.Add("MergedRelationNonrelevant", ToList(lexrep, kb));
}

void IkIndexDebug<Utf8List>::MergingConcepts(Lexreps::const_iterator begin, Lexreps::const_iterator end, const IkKnowledgebase& kb) {
  for (Lexreps::const_iterator i = begin; i != end; ++i) {
    MergingConcept(*i, kb);
  }
}

void IkIndexDebug<Utf8List>::MergingConcept(const IkLexrep& lexrep, const IkKnowledgebase& kb) {
  trace_.Add("MergingConcept", ToList(lexrep, kb));
}

void IkIndexDebug<Utf8List>::MergedConcept(const IkMergedLexrep& lexrep, const IkKnowledgebase& kb) {
  trace_.Add("MergedConcept", ToList(lexrep, kb));
}

void IkIndexDebug<Utf8List>::ConceptFiltered(const IkLexrep& lexrep, const String& replacement, const IkKnowledgebase& kb) {
  if (replacement == lexrep.GetNormalizedValue()) return;
  Utf8List out = ToList(lexrep, kb);
  out.push_back(IkStringEncoding::BaseToUTF8(replacement));
  trace_.Add("ConceptFiltered", out);
}

void IkIndexDebug<Utf8List>::RelationFiltered(const IkLexrep& lexrep, const String& replacement, const IkKnowledgebase& kb) {
  if (replacement == lexrep.GetNormalizedValue()) return;
  Utf8List out = ToList(lexrep, kb);
  out.push_back(IkStringEncoding::BaseToUTF8(replacement));
  trace_.Add("RelationFiltered", out);
}

void IkIndexDebug<Utf8List>::PathRelevantFiltered(const IkLexrep& lexrep, const String& replacement, const IkKnowledgebase& kb) {
	if (replacement == lexrep.GetNormalizedValue()) return;
	Utf8List out = ToList(lexrep, kb);
	out.push_back(IkStringEncoding::BaseToUTF8(replacement));
	trace_.Add("PathRelevantFiltered", out);
}

void IkIndexDebug<Utf8List>::NonRelevantFiltered(const IkLexrep& lexrep, const String& replacement, const IkKnowledgebase& kb) {
  if (replacement == lexrep.GetNormalizedValue()) return;
  Utf8List out = ToList(lexrep, kb);
  out.push_back(IkStringEncoding::BaseToUTF8(replacement));
  trace_.Add("NonRelevantFiltered", out);
}

void IkIndexDebug<Utf8List>::SentenceFound(const String& kb_name, double certainty, const String& language_code, const Lexreps& lexreps, const iknow::base::String& separator) {
  Utf8List out;
  std::string trace_data("<sentence kb=\"" + IkStringEncoding::BaseToUTF8(kb_name) + "\" ali=\"" + std::to_string(certainty) + "\" lang=\"" + IkStringEncoding::BaseToUTF8(language_code) + "\" text=\"");
  String sentence;
  sentence.reserve(lexreps.size() * 16);
  for (Lexreps::const_iterator i = lexreps.begin(); i != lexreps.end(); ++i) {
    String val = i->GetValue();
    if (val.empty()) continue;
    bool bSplitLiteral = (val[0]==' '); //Merge if it's a split literal (indicated with leading space)
    if (bSplitLiteral) val.erase(0, 1); // remove leading ' '
	if (!sentence.empty() && !bSplitLiteral) sentence += separator;
    sentence += val;
  }
  trace_data += IkStringEncoding::BaseToUTF8(sentence) + "\" />";
  out.push_back(trace_data);
  trace_.Add("SentenceFound", out);
}

void IkIndexDebug<Utf8List>::SentenceComplete(const IkSentence& sentence, const IkKnowledgebase& kb) {
  Utf8List out;
  for (MergedLexreps::const_iterator i = sentence.GetLexrepsBegin(); i != sentence.GetLexrepsEnd(); ++i) {
    Utf8List trace = ToList(*i, kb);
    std::for_each(trace.begin(), trace.end(), [&out](std::string& s) mutable { out.push_back(s); });
  }
  trace_.Add("SentenceComplete", out);
}

void IkIndexDebug<Utf8List>::EntityVectors(const IkSentence& sentence) {
  std::vector<iknow::base::String> entities; // first collect the entities
  for (MergedLexreps::const_iterator i = sentence.GetLexrepsBegin(); i != sentence.GetLexrepsEnd(); ++i) {
    entities.push_back(i->GetValue());
  }
  Utf8List out;
  for (IkSentence::EntityVector::const_iterator i = sentence.GetEntityVectorBegin(); i != sentence.GetEntityVectorEnd(); ++i) {
      out.push_back(IkStringEncoding::BaseToUTF8(entities[*i]));
  }
  if (out.size() != static_cast<size_t>(0)) trace_.Add("EntityVector", out); // only emit if we do have entity vectors
}

void IkIndexDebug<Utf8List>::InvalidEntityVector(const IkLexrep& lexrep, const IkKnowledgebase& kb) {
	trace_.Add("InvalidEntityVector", ToList(lexrep, kb));
}

void IkIndexDebug<Utf8List>::MissingEntityVector(const IkMergedLexrep& lexrep, const IkKnowledgebase& kb) {
	trace_.Add("MissingEntityVector", ToList(lexrep, kb));
}

void IkIndexDebug<Utf8List>::SentenceSummarizationComplete(const IkSentence& sentence)
{
  Utf8List out;
  for (MergedLexreps::const_iterator i = sentence.GetLexrepsBegin(); i != sentence.GetLexrepsEnd(); ++i) {
    Utf8List trace = ToList(*i);
    std::for_each(trace.begin(), trace.end(), [&out](std::string& s) mutable { out.push_back(s); });
  }
  trace_.Add("SentenceSummarizationComplete", out);
}

static String OffsetToValue(path::Offset offset, const MergedLexreps& lexreps) {
  if (offset > lexreps.size()) return String();
  return lexreps[offset].GetNormalizedValue();
}

void IkIndexDebug<Utf8List>::CRC(const path::CRC& crc, const MergedLexreps& lexreps) {
  Utf8List out;
  out.push_back(IkStringEncoding::BaseToUTF8(OffsetToValue(crc.master, lexreps)));
  out.push_back(IkStringEncoding::BaseToUTF8(OffsetToValue(crc.relation, lexreps)));
  out.push_back(IkStringEncoding::BaseToUTF8(OffsetToValue(crc.slave, lexreps)));
  trace_.Add("CRC", out);
}

void IkIndexDebug<Utf8List>::StemOccurence(const iknow::base::String& occurence, const iknow::base::String& stem) {
  Utf8List out;
  out.push_back(IkStringEncoding::BaseToUTF8(occurence));
  out.push_back(IkStringEncoding::BaseToUTF8(stem));
  trace_.Add("StemOccurence", out);
}

void IkIndexDebug<Utf8List>::JoinResult(const Lexreps::iterator lexrep, const IkKnowledgebase& kb) {
    Utf8List out = ToList(*lexrep, kb);
	trace_.Add("JoinResult", out);
}

void IkIndexDebug<Utf8List>::SingleWordFreq(const iknow::base::String& word, size_t frequency) {
    Utf8List out;
	out.push_back(IkStringEncoding::BaseToUTF8(word));
	out.push_back(std::to_string((int) frequency));
	trace_.Add("SingleWordFreq", out);
}

void IkIndexDebug<Utf8List>::TraceKeyDouble(const char* type, const iknow::base::String& name, double value) {
    Utf8List out;
	out.push_back(IkStringEncoding::BaseToUTF8(name));
	out.push_back(std::to_string(value));
	trace_.Add(type, out);
}

void IkIndexDebug<Utf8List>::Parameter(const iknow::base::String& name, double value) {
    Utf8List out;
	out.push_back(IkStringEncoding::BaseToUTF8(name));
	out.push_back(std::to_string(value));
	trace_.Add("Parameter", out);
}

void IkIndexDebug<Utf8List>::MergedKatakana(const IkLexrep& lexrep, const IkKnowledgebase& kb) {
	trace_.Add("MergedKatakana", ToList(lexrep, kb));
}

void IkIndexDebug<Utf8List>::LabelKatakana(const IkLexrep& lexrep, const IkKnowledgebase& kb) {
	trace_.Add("LabelKatakana", ToList(lexrep, kb));
}

#ifdef AIX
void IkIndexDebug<Utf8List>::StartTimer(void)
{
}
void IkIndexDebug<Utf8List>::TraceTheTime(const int action) {
}
#else //!AIX
// using std::chrono::system_clock;
using std::chrono::steady_clock;
using std::chrono::duration_cast;
using std::chrono::milliseconds;
using std::chrono::microseconds;
static steady_clock::time_point time_point_now;

void IkIndexDebug<Utf8List>::StartTimer(void)
{
	time_point_now = steady_clock::now();
}

void IkIndexDebug<Utf8List>::TraceTheTime(const int action) {
	auto d = steady_clock::now() - time_point_now;
	size_t milli_seconds = static_cast<size_t>(duration_cast<milliseconds>(d).count());
	size_t micro_seconds = static_cast<size_t>(duration_cast<microseconds>(d).count());

	Utf8List out;
	out.push_back(std::to_string(action));
	out.push_back(std::to_string(milli_seconds));
	out.push_back(std::to_string(micro_seconds));
	trace_.Add("TraceTime", out);
}

#endif //!AIX

const iknow::base::IkTrace<Utf8List>& IkIndexDebug<Utf8List>::GetTrace() const { return trace_; }
