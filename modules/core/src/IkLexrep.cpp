#include "IkLexrep.h"
#include "IkKnowledgebase.h"

using namespace iknow::core;

FastLabelBitSet* LexrepContext::SeenLabelsPointer() {
  static FastLabelBitSet seen_labels;
  return &seen_labels;
}

iknow::base::StringPool* IkLexrep::string_pool_ = 0;
const iknow::base::Char* IkLexrep::text_buffer_ = 0;

LexrepStore** IkLexrep::GetLexrepStorePointer() {
  static LexrepStore* store = 0;
  return &store;
}

const Phase* IkLexrep::GetPhasesBegin(FastLabelSet::Index index) const {
  return m_kb->GetPhasesBegin(index);
}

const Phase* IkLexrep::GetPhasesEnd(FastLabelSet::Index index) const {
  return m_kb->GetPhasesEnd(index);
}

FastLabelTypeSet IkLexrep::GetTypeLabels(Phase p) const {
	FastLabelTypeSet label_type_set;
	const FastLabelSet& label_set = LabelSet(p);
	for (FastLabelSet::const_iter it = label_set.begin(); it != label_set.end(); ++it) {
		FastLabelSet::Index idx_label = *it;
		IkLabel::Type typ_label = m_kb->GetLabelTypeAtIndex(idx_label);
		label_type_set.InsertAtIndex(static_cast<FastLabelTypeSet::Index>(typ_label));
	}
	return label_type_set;
}

void IkLexrep::RemoveLabelType(const FastLabelTypeSet::Index label_type)
{
	for (Phase p = 0; p < kPhaseCount; ++p) {
		if (NumberOfLabels(p) > static_cast<size_t>(0)) { // Phase has labels
			RemoveLabelType(label_type, p);
		}
	}

}

void IkLexrep::RemoveLabelType(const FastLabelTypeSet::Index label_type, Phase p)
{
	const FastLabelSet& label_set = LabelSet(p); // collect all labels from Phase p
	std::vector<FastLabelSet::Index> label_type_collection; // need to collect first, since remove invalidates the iterator
	for (FastLabelSet::const_iter it = label_set.begin(); it != label_set.end(); ++it) {
		if (label_type == m_kb->GetLabelTypeAtIndex(*it)) label_type_collection.push_back(*it);
	}
	for (std::vector<FastLabelSet::Index>::const_iterator itLabel = label_type_collection.begin(); itLabel != label_type_collection.end(); ++itLabel) {
		RemoveLabelIndex(*itLabel);
	}
}


//Clear labels present in a particular phase, including clearing their representation in other phases to which they belong.
void IkLexrep::ClearLabels(Phase p) {
	const FastLabelSet& labels = LabelSet(p);
	if (p == kMaxPhase) { //Optimize for the final phase
		ClearAllLabels(p);
		return;
	}
	//TODO: More efficient implementation of this iteration
	//Remove labels in this phase from all phases (including this one) until this phase is empty.
	while (!labels.Empty()) {
		if (m_kb->GetLabelTypeAtIndex(labels.At(0)) == IkLabel::Literal) { // do not delete a literal label
			if (labels.Size() > 1) RemoveLabelIndex(labels.At(1)); // lexrep can only have one literal label
			else break; // break the loop, no more labels to delete.
		} else
		  RemoveLabelIndex(labels.At(0));
	}
}

bool IkLexrep::hasSBeginLabel(Phase p) const {
	const FastLabelSet::Index SBeginLabelIndex = m_kb->GetLabelIndex(SentenceBeginLabel);
	return this->GetLabels(p).Contains(SBeginLabelIndex);
}
bool IkLexrep::hasSEndLabel(Phase p) const {
	const FastLabelSet::Index SEndLabelIndex = m_kb->GetLabelIndex(SentenceEndLabel);
	return this->GetLabels(p).Contains(SEndLabelIndex);
}
void IkLexrep::setSBeginLabel() {
	this->AddLabelIndex(m_kb->GetLabelIndex(SentenceBeginLabel));
}
void IkLexrep::setSEndLabel() {
	this->AddLabelIndex(m_kb->GetLabelIndex(SentenceEndLabel));
}

FastLabelSet::Index IkLexrep::FindAttributeWithType(AttributeId type, Phase p) const {
  size_t label_count = NumberOfLabels(p);
  for (size_t i=0; i < label_count; ++i) {
    FastLabelSet::Index label = GetLabelIndexAt(i, p);
    size_t attr_count = m_kb->GetAttributeCount(label);
    for (size_t j=0; j < attr_count; ++j) {
      if (m_kb->GetAttributeType(label, j) == type) return label;
    }
  }
  return FastLabelSet::NPos();
}


LabelAttribute IkLexrep::GetFirstLabelAttributeWithType(AttributeId type, Phase p) const {
  size_t label_count = NumberOfLabels(p);
  for (size_t i=0; i < label_count; ++i) {
    FastLabelSet::Index label = GetLabelIndexAt(i, p);
    size_t attr_count = m_kb->GetAttributeCount(label);
    for (size_t j = 0; j < attr_count; ++j) {
      if (m_kb->GetAttributeType(label, j) == type) {
	LabelAttribute attr(label, j);
	return attr;
      }
    }
  }
  return LabelAttribute();
}

size_t IkLexrep::GetLiteralTokenCount(bool first_in_merge) const {
	size_t n = 0;
	iknow::base::String strValue = GetValue(); // make a local copy, TODO :  work on text buffers
	if (m_kb->GetMetadata<kIsJapanese>()) { // every symbol is a token in ideographic languages
		n = strValue.length();
		if (n>1 && strValue[0] == ' ') --n; // don't count the starting space 
		return n;
	}
#ifdef SOLARIS
	std::count(strValue.begin(), strValue.end(), ' ', n);
	n = n + 1;
#else
	n = std::count(strValue.begin(), strValue.end(), ' ') + 1;
#endif
	if (strValue[0] == ' ') { // no separate token, except if first in merge
		n = n - (first_in_merge ? 1 : 2); // reduce number of "real" tokens.
	}
	return n;
}

iknow::base::String IkLexrep::GetValue(void) const
{
	if (text_begin_ == NULL || text_end_ == NULL) return iknow::base::String(); // no literal representation
	bool b_not_space_prepended = false;
	if (text_begin_ > text_buffer_ && !m_kb->GetMetadata<kIsJapanese>()) { // not first, and not for Japanese
		iknow::base::Char previous_char = *(text_begin_ - 1);
		b_not_space_prepended = !u_isspace(previous_char);
	}
	iknow::base::String raw_literal = iknow::base::String(text_begin_, text_end_);
	iknow::base::String clean_literal;
	bool b_last_is_space = false; // set true if space has been added
	for (iknow::base::String::iterator scan = raw_literal.begin(); scan != raw_literal.end(); ++scan) { // clean raw_literal
		iknow::core::token::TokenType tt = iknow::core::token::GetType<iknow::base::Char>(*scan);
		switch (tt) {
		case iknow::core::token::kText:
		case iknow::core::token::kPunctuation:
		case iknow::core::token::kControl: // treat control chars as part of the literal, to better reflect the text source
			clean_literal += *scan;
			b_last_is_space = false;
			break;
		case iknow::core::token::kSpace:
			if (m_kb->GetMetadata<kIsJapanese>()) clean_literal += *scan; // no space exception in Japanese.
			else {
				if (!clean_literal.empty() && !b_last_is_space) {
					clean_literal += *scan; // use the same space...
					b_last_is_space = true;
				}	
			}
			break;
		case iknow::core::token::kLine:
		// case iknow::core::token::kControl:
		//	if (static_cast<int>(*scan) == 173) clean_literal += *scan; //Strange little hack to guarantee same literal output as before, should be removed once we stop expressing literals as strings...
		//	else {
			if (m_kb->GetMetadata<kIsJapanese>()) break; // ignore in Japanese
			if (!clean_literal.empty() && !b_last_is_space) {
					clean_literal += ' ';
					b_last_is_space = true;
			}
		//	}
			break;
		default: // rather strange symbol, ignore it !
			;
		}
	}
	if (b_last_is_space) { // if we end with a space, remove it.
		clean_literal.erase(clean_literal.size() - 1, 1);	//remove the last (space) element
	}
	if (b_not_space_prepended)
		return iknow::base::SpaceString() + clean_literal; // Space prepend means no space (SCJ)
	else
		return clean_literal;
}

void IkLexrep::SetMetaData(const char* meta_data) { // parsing of metadata should be done here
	if (meta_data == NULL) return;

	std::string certainty_value = iknow::base::IkStringAlg::GetMetaValue(meta_data, "c=");
	if (!certainty_value.empty())
		certainty_value_ = certainty_value[0];
}

std::string IkLexrep::GetMetaData() const { // return of metadata should be done here
	std::string meta_data;
	if (certainty_value_ != '\0') {
		meta_data = std::string("c=0");
		meta_data[2] = certainty_value_;
	}
	return meta_data;
}

