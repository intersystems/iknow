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

iknow::base::String AttributeName(AttributeId id, const IkKnowledgebase& kb) {
  AttributeString s = kb.AttributeNameForId(id);
  return iknow::base::String(s.data, s.size);
}

CacheList ToList(FastLabelSet::Index label, size_t attribute_pos, const IkKnowledgebase& kb) {
  CacheList attribute;
  attribute += AttributeName(kb.GetAttributeType(label, attribute_pos), kb);
  for (const AttributeId* i = kb.GetAttributeParamsBegin(label, attribute_pos);
       i != kb.GetAttributeParamsEnd(label, attribute_pos);
       ++i) {
    attribute += AttributeName(*i, kb);
  }
  return attribute;
}

CacheList ToList(const FastLabelSet& labels, const IkKnowledgebase& kb) {
  CacheList out;
  for (size_t i=0; i < static_cast<size_t>(labels.Size()); ++i) {
    FastLabelSet::Index index = labels.At(i);
    out += kb.GetAtIndex(index).GetName();
    size_t attribute_count = kb.GetAttributeCount(index);
    if (attribute_count) {
      for (size_t j=0; j < attribute_count; ++j) {
        out += ToList(index, j, kb);
      }
    }
  }
  return out;
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

/*
CacheList ToList(const IkLexrep& lexrep, const IkKnowledgebase& kb) {
  CacheList out;
  //TODO: Handle colons in values.
  out += (int)lexrep.GetId();
  out += TypeToString(lexrep.GetLexrepType());
  out += lexrep.GetValue();
  out += lexrep.GetNormalizedValue();
  for (Phase p = 0; p < kPhaseCount; ++p) {
    if (!lexrep.GetLabels(p).Empty())
      out += ToList(lexrep.GetLabels(p), kb);
  }
  return out;
}
*/

CacheList ToList(const IkLexrep& lexrep, const IkKnowledgebase& kb) {
  CacheList out;
  //TODO: Handle colons in values.
  out += (int)lexrep.GetId();
  out += TypeToString(lexrep.GetLexrepType());
  out += lexrep.GetValue();
  out += lexrep.GetNormalizedValue();

  typedef std::set<FastLabelSet::Index> LabelSet;
  LabelSet labels;
  for (Phase p = 0; p < kPhaseCount; ++p) { // Collect labels for all phases.
    if (!lexrep.GetLabels(p).Empty()) { // phase has labels
      const FastLabelSet& phase_labels=lexrep.GetLabels(p);
      for (size_t i=0; i < static_cast<size_t>(phase_labels.Size()); ++i) {
        FastLabelSet::Index index = phase_labels.At(i);
        labels.insert(index);
      }
    }
  }
  CacheList labels_out;
  for (LabelSet::iterator it=labels.begin(); it!=labels.end(); it++) {
    FastLabelSet::Index idx=*it;
    labels_out += kb.GetAtIndex(idx).GetName();
    size_t attribute_count = kb.GetAttributeCount(idx);
    if (attribute_count) {
      for (size_t j=0; j < attribute_count; ++j) {
	    labels_out += ToList(idx, j, kb);
      }
    }
  }
  out += labels_out;
  return out;
}

//Without KB data
CacheList ToList(const IkLexrep& lexrep) {
  CacheList out;
  out += (int)lexrep.GetId();
  out += TypeToString(lexrep.GetLexrepType());
  out += lexrep.GetValue();
  out += lexrep.GetNormalizedValue();
  out += lexrep.GetSummaryRelevance();
  //No labels since we can't translate them without the KB.
  return out;
}

//For lexreps
CacheList ToList(const IkMergedLexrep& lexrep, const IkKnowledgebase& kb) {
  CacheList out;
  for (IkMergedLexrep::const_iterator i = lexrep.LexrepsBegin(); i != lexrep.LexrepsEnd(); ++i) {
    out += ToList(*i, kb);
  }
  out += lexrep.GetSummaryRelevance();
  return out;
}

//For lexreps without KB data
CacheList ToList(const IkMergedLexrep& lexrep) {
  CacheList out;
  for (IkMergedLexrep::const_iterator i = lexrep.LexrepsBegin(); i != lexrep.LexrepsEnd(); ++i) {
    out += ToList(*i);
  }
  out += lexrep.GetSummaryRelevance();
  return out;
}

//For rule patterns
CacheList ToList(const FastLabelSet::Index* begin, const FastLabelSet::Index* end, const IkKnowledgebase& kb) {
  CacheList out;
  for (const FastLabelSet::Index* i=begin; i != end; ++i) {
    out += kb.GetAtIndex(*i).GetName();
  }
  return out;
}

String ToString(const IkRuleInputPattern& pattern, const IkKnowledgebase& kb) {
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

//For rule input patterns
CacheList ToList(const IkRuleInputPattern* begin, const IkRuleInputPattern* end, const IkKnowledgebase& kb) {
  CacheList out;
  for (const IkRuleInputPattern* i=begin; i != end; ++i) {
    out += ToString(*i, kb);
  }
  return out;
}

String ToString(const IkRuleOutputPattern& pattern, const IkKnowledgebase& kb) {
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

CacheList ToList(const IkRuleOutputPattern* begin, const IkRuleOutputPattern* end, const IkKnowledgebase& kb) {
  CacheList out;
  for (const IkRuleOutputPattern* i = begin; i != end; ++i) {
    out += ToString(*i, kb);
  }
  return out;
}

//Now, the traces themselves.

void IkIndexDebug::SwitchKnowledgebase(const String& old_language, const String& new_language, double certainty) {
  CacheList out;
  out += old_language;
  out += new_language;
  out += certainty;
  trace_.Add("SwitchKnowledgebase", out);
}

void IkIndexDebug::ApplyRule(size_t rule_id, Lexreps::iterator match, size_t match_length, const IkKnowledgebase& kb)
{
  CacheList out;
  out += (int)rule_id;
  out += (int)match_length;
  for (size_t i=0; i < match_length; ++i) {
    out += ToList(*match++, kb);
  }
  IkRule rule = kb.GetRule(rule_id);
  out += ToList(rule.InputBegin(), rule.InputEnd(), kb);
  out += ToList(rule.OutputBegin(), rule.OutputEnd(), kb);
  out += rule.GetPhase();
  trace_.Add("RuleApplication", out);
}

void IkIndexDebug::RuleApplied(size_t rule_id, Lexreps::iterator match, size_t match_length, const IkKnowledgebase& kb)
{
  CacheList out;
  out += (int)rule_id;
  out += (int)match_length;
  for (size_t i=0; i < match_length; ++i) {
    out += ToList(*match++, kb);
  }
  trace_.Add("RuleApplicationResult", out);
}


void IkIndexDebug::RulesComplete(const IkLexrep& lexrep, const IkKnowledgebase& kb) {
  trace_.Add("RulesComplete", ToList(lexrep, kb));
}

void IkIndexDebug::AmbiguityResolved(const IkLexrep& lexrep, const IkKnowledgebase& kb) {
  trace_.Add("AmbiguityResolved", ToList(lexrep, kb));
}

void IkIndexDebug::PreprocessToken(const String& original, const String& replacement) {
  if (original == replacement) return;
  CacheList out;
  out += original;
  out += replacement;
  trace_.Add("PreprocessToken", out);
}

void IkIndexDebug::NormalizeToken(const String& original, const String& replacement) {
  if (original == replacement) return;
  CacheList out;
  out += original;
  out += replacement;
  trace_.Add("NormalizeToken", out);
}

void IkIndexDebug::LexrepCreated(const IkLexrep& lexrep, const IkKnowledgebase& kb) {
  trace_.Add("LexrepCreated", ToList(lexrep, kb));
}

void IkIndexDebug::LexrepIdentified(const IkLexrep& lexrep, const IkKnowledgebase& kb) {
	CacheList out = ToList(lexrep, kb);
	if (lexrep.GetMetaData()) {
		out += lexrep.GetMetaData();
	}
	trace_.Add("LexrepIdentified", out);
}

void IkIndexDebug::UserDictionaryMatch(const IkLexrep& lexrep, const IkKnowledgebase& kb) {
	trace_.Add("UserDictionaryMatch", ToList(lexrep, kb));
}

void IkIndexDebug::AttributeDetected(const std::string& attribute, const IkLexrep& lexrep, const IkKnowledgebase& kb) {
  CacheList out;
  out += attribute;
  out += ToList(lexrep, kb);
  trace_.Add("AttributeDetected", out);
}

void IkIndexDebug::LexrepTypeAssignment(const IkLexrep& lexrep, const IkKnowledgebase& kb) {
  trace_.Add("LexrepTypeAssignment", ToList(lexrep, kb));
}

void IkIndexDebug::MergingRelations(Lexreps::const_iterator begin, Lexreps::const_iterator end, const IkKnowledgebase& kb) {
  for (Lexreps::const_iterator i = begin; i != end; ++i) {
    MergingRelation(*i, kb);
  }
}

void IkIndexDebug::MergingRelation(const IkLexrep& lexrep, const IkKnowledgebase& kb) {
  trace_.Add("MergingRelation", ToList(lexrep, kb));
}

void IkIndexDebug::MergedRelation(const IkMergedLexrep& lexrep, const IkKnowledgebase& kb) {
  trace_.Add("MergedRelation", ToList(lexrep, kb));
}

void IkIndexDebug::MergedRelationNonrelevant(const IkMergedLexrep& lexrep, const IkKnowledgebase& kb) {
  trace_.Add("MergedRelationNonrelevant", ToList(lexrep, kb));
}

void IkIndexDebug::MergingConcepts(Lexreps::const_iterator begin, Lexreps::const_iterator end, const IkKnowledgebase& kb) {
  for (Lexreps::const_iterator i = begin; i != end; ++i) {
    MergingConcept(*i, kb);
  }
}

void IkIndexDebug::MergingConcept(const IkLexrep& lexrep, const IkKnowledgebase& kb) {
  trace_.Add("MergingConcept", ToList(lexrep, kb));
}

void IkIndexDebug::MergedConcept(const IkMergedLexrep& lexrep, const IkKnowledgebase& kb) {
  trace_.Add("MergedConcept", ToList(lexrep, kb));
}

void IkIndexDebug::ConceptFiltered(const IkLexrep& lexrep, const String& replacement, const IkKnowledgebase& kb) {
  if (replacement == lexrep.GetNormalizedValue()) return;
  CacheList out;
  out += ToList(lexrep, kb);
  out += replacement;
  trace_.Add("ConceptFiltered", out);
}

void IkIndexDebug::RelationFiltered(const IkLexrep& lexrep, const String& replacement, const IkKnowledgebase& kb) {
  if (replacement == lexrep.GetNormalizedValue()) return;
  CacheList out;
  out += ToList(lexrep, kb);
  out += replacement;
  trace_.Add("RelationFiltered", out);
}

void IkIndexDebug::PathRelevantFiltered(const IkLexrep& lexrep, const String& replacement, const IkKnowledgebase& kb) {
	if (replacement == lexrep.GetNormalizedValue()) return;
	CacheList out;
	out += ToList(lexrep, kb);
	out += replacement;
	trace_.Add("PathRelevantFiltered", out);
}

void IkIndexDebug::NonRelevantFiltered(const IkLexrep& lexrep, const String& replacement, const IkKnowledgebase& kb) {
  if (replacement == lexrep.GetNormalizedValue()) return;
  CacheList out;
  out += ToList(lexrep, kb);
  out += replacement;
  trace_.Add("NonRelevantFiltered", out);
}

void IkIndexDebug::SentenceFound(const String& kb_name, double certainty, const String& language_code, const Lexreps& lexreps, const iknow::base::String& separator) {
  CacheList out;
  out += kb_name;
  out += certainty;
  out += language_code;
  String sentence;
  sentence.reserve(lexreps.size() * 16);
  for (Lexreps::const_iterator i = lexreps.begin(); i != lexreps.end(); ++i) {
    String val = i->GetValue();
    if (val.empty()) continue;
    //Merge if it's a split literal (indicated with leading space)
	if (val[0] == ' ' && !sentence.empty() && separator == iknow::base::SpaceString()) {
      sentence.erase(sentence.size() - 1);
      val.erase(0,1);
    }
	if (!sentence.empty()) sentence += separator;
    sentence += val;
  }
  out += sentence;
  trace_.Add("SentenceFound", out);
}

void IkIndexDebug::SentenceComplete(const IkSentence& sentence, const IkKnowledgebase& kb) {
  CacheList out;
  for (MergedLexreps::const_iterator i = sentence.GetLexrepsBegin(); i != sentence.GetLexrepsEnd(); ++i) {
    out += ToList(*i, kb);
  }
  trace_.Add("SentenceComplete", out);
}

void IkIndexDebug::EntityVectors(const IkSentence& sentence) {
  std::vector<iknow::base::String> entities; // first collect the entities
  for (MergedLexreps::const_iterator i = sentence.GetLexrepsBegin(); i != sentence.GetLexrepsEnd(); ++i) {
    entities.push_back(i->GetValue());
  }
  CacheList out;
  for (IkSentence::EntityVector::const_iterator i = sentence.GetEntityVectorBegin(); i != sentence.GetEntityVectorEnd(); ++i) {
      out += entities[*i];
  }
  if (out.size() != static_cast<size_t>(0)) trace_.Add("EntityVector", out); // only emit if we do have entity vectors
}

void IkIndexDebug::InvalidEntityVector(const IkLexrep& lexrep, const IkKnowledgebase& kb) {
	trace_.Add("InvalidEntityVector", ToList(lexrep, kb));
}
void IkIndexDebug::MissingEntityVector(const IkMergedLexrep& lexrep, const IkKnowledgebase& kb) {
	trace_.Add("MissingEntityVector", ToList(lexrep, kb));
}
void IkIndexDebug::SentenceSummarizationComplete(const IkSentence& sentence)
{
  CacheList out;
  for (MergedLexreps::const_iterator i = sentence.GetLexrepsBegin(); i != sentence.GetLexrepsEnd(); ++i) {
    out += ToList(*i);
  }
  trace_.Add("SentenceSummarizationComplete", out);
}

static String OffsetToValue(path::Offset offset, const MergedLexreps& lexreps) {
  if (offset > lexreps.size()) return String();
  return lexreps[offset].GetNormalizedValue();
}

void IkIndexDebug::CRC(const path::CRC& crc, const MergedLexreps& lexreps) {
  CacheList out;
  out += OffsetToValue(crc.master, lexreps);
  out += OffsetToValue(crc.relation, lexreps);
  out += OffsetToValue(crc.slave, lexreps);
  trace_.Add("CRC", out);
}

void IkIndexDebug::StemOccurence(const iknow::base::String& occurence, const iknow::base::String& stem) {
  CacheList out;
  out += occurence;
  out += stem;
  trace_.Add("StemOccurence", out);
}

void IkIndexDebug::JoinResult(const Lexreps::iterator lexrep, const IkKnowledgebase& kb) {
	CacheList out;
	out += ToList(*lexrep, kb);
	trace_.Add("JoinResult", out);
}

void IkIndexDebug::SingleWordFreq(const iknow::base::String& word, size_t frequency) {
	CacheList out;
	out += word;
	out += (int) frequency;
	trace_.Add("SingleWordFreq", out);
}

void IkIndexDebug::TraceKeyDouble(const char* type, const iknow::base::String& name, double value) {
	CacheList out;
	out += name;
	out += value;
	trace_.Add(type, out);
}
void IkIndexDebug::Parameter(const iknow::base::String& name, double value) {
	CacheList out;
	out += name;
	out += value;
	trace_.Add("Parameter", out);
}

void IkIndexDebug::MergedKatakana(const IkLexrep& lexrep, const IkKnowledgebase& kb) {
	trace_.Add("MergedKatakana", ToList(lexrep, kb));
}

void IkIndexDebug::LabelKatakana(const IkLexrep& lexrep, const IkKnowledgebase& kb) {
	trace_.Add("LabelKatakana", ToList(lexrep, kb));
}

#ifdef AIX
void IkIndexDebug::StartTimer(void)
{
}
void IkIndexDebug::TraceTheTime(const int action) {
}
#else //!AIX
// using std::chrono::system_clock;
using std::chrono::steady_clock;
using std::chrono::duration_cast;
using std::chrono::milliseconds;
using std::chrono::microseconds;
static steady_clock::time_point time_point_now;

void IkIndexDebug::StartTimer(void)
{
	time_point_now = steady_clock::now();
}
void IkIndexDebug::TraceTheTime(const int action) {
	auto d = steady_clock::now() - time_point_now;
	size_t milli_seconds = static_cast<size_t>(duration_cast<milliseconds>(d).count());
	size_t micro_seconds = static_cast<size_t>(duration_cast<microseconds>(d).count());

	CacheList out;
	out += action;
	out += milli_seconds;
	out += micro_seconds;
	trace_.Add("TraceTime", out);
}

#endif //!AIX
