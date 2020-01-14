#ifndef IKNOW_CORE_IKMERGEDLEXREP_H_
#define IKNOW_CORE_IKMERGEDLEXREP_H_
#include <vector>
#include "IkLexrep.h"
#include "IkKnowledgebase.h"
#include "IkMetadataCache.h"
#include "PoolAllocator.h"

namespace iknow {
  namespace core {

    class IkMergedLexrep {
    public:
      typedef Lexreps::const_iterator const_iterator;
      typedef Lexreps::iterator iterator;
      explicit IkMergedLexrep(const IkLexrep& lexrep) : normalized_cache_(0), value_cache_(0) {
	lexreps_.push_back(lexrep);
      }

      explicit IkMergedLexrep(size_t size_guess) : normalized_cache_(0), value_cache_(0) {
	lexreps_.reserve(size_guess);
      }

      template<typename IterT>
      IkMergedLexrep(const IterT& lexreps_begin, const IterT& lexreps_end) : normalized_cache_(0), value_cache_(0) {
	lexreps_.insert(lexreps_.begin(), lexreps_begin, lexreps_end);
      }

      template<typename IterT>
      void Append(const IterT& lexreps_begin, const IterT& lexreps_end) {
	lexreps_.insert(lexreps_.end(), lexreps_begin, lexreps_end);
	normalized_cache_ = 0;
	value_cache_ = 0;
     }

      void Append(const IkLexrep& lexrep) {
	lexreps_.push_back(lexrep);
	normalized_cache_ = 0;
	value_cache_ = 0;
      }
      
      size_t LexrepsCount() const {
	return lexreps_.size();
      }
      
      iterator LexrepsBegin() {
	return lexreps_.begin();
      }
      
      iterator LexrepsEnd() {
	return lexreps_.end();
      }

      iterator LexrepErase(iterator i) {
	return lexreps_.erase(i);
      }

      const_iterator LexrepsBegin() const { return lexreps_.begin(); }
      const_iterator LexrepsEnd() const { return lexreps_.end(); }

	  const iknow::base::Char* GetTextPointerBegin() const { return lexreps_.begin()->GetTextPointerBegin(); }
	  const iknow::base::Char* GetTextPointerEnd() const { return (lexreps_.end() - 1)->GetTextPointerEnd(); }

      size_t GetTokenCount() const {
        size_t token_count = 0;
        for (Lexreps::const_iterator i = LexrepsBegin(); i != LexrepsEnd(); ++i) {
          token_count += i->GetTokenCount();
        }
       return token_count;
      }

      bool HasAttributeWithType(AttributeId type) const {
	for (Lexreps::const_iterator i = LexrepsBegin(); i != LexrepsEnd(); ++i) {
	  if (i->FindAttributeWithType(type) != FastLabelSet::NPos()) return true;
	}
	return false;
      }

      LabelAttribute GetFirstLabelAttributeWithType(AttributeId type) const {
	for (Lexreps::const_iterator i = LexrepsBegin(); i != LexrepsEnd(); ++i) {
	  LabelAttribute attr = i->GetFirstLabelAttributeWithType(type);
	  if (attr.is_valid()) return attr;
	}
	return LabelAttribute();
      }

      bool IsNormalizedValueEmpty() const {
	for (Lexreps::const_iterator i = LexrepsBegin(); i != LexrepsEnd(); ++i) {
	  if (!i->GetNormalizedValue().empty()) return false;
	}
	return true;
      }

      double GetSummaryRelevance() const {
	return lexreps_.empty() ? 0 : lexreps_[0].GetSummaryRelevance();
      }
      void increaseSummaryRelevance(size_t amount) {
	if (!lexreps_.empty()) lexreps_[0].increaseSummaryRelevance(amount);
      }
      iknow::base::String GetText() const {
	return GetValue();
      }
      iknow::base::String GetNormalizedText() const {
	return GetNormalizedValue();
      }
      IkLexrep::Type GetLexrepType(void) const {
	return lexreps_.empty() ?
	  IkLabel::Unknown :
	  lexreps_[0].GetLexrepType();
      }

      void SetLexrepType(IkLexrep::Type type) {
	//ignore if empty
	if (lexreps_.empty()) return;
	lexreps_[0].SetLexrepType(type);
      }

      const iknow::base::String& GetNormalizedValue(const iknow::base::String& separator=iknow::base::SpaceString()) const {
	if (LexrepsCount() == 1) return lexreps_[0].GetNormalizedValue();
        if (normalized_cache_) return *normalized_cache_;
	static iknow::base::String output(64,static_cast<iknow::base::Char>(0));
	//Clear it without resizing the buffer
	output.replace(0,output.size(), 0, static_cast<iknow::base::Char>(0));
        const IkLexrep::Type type = GetLexrepType();
        for (const_iterator i = LexrepsBegin(); i != LexrepsEnd(); ++i) {
          if (i->GetNormalizedValue().empty()) continue; //Skip empty entity values
          IkLabel::Type type_lexrep = i->GetLexrepType();
	  bool IsRelation = (type_lexrep == IkLabel::Relation || type_lexrep == IkLabel::BeginRelation || type_lexrep == IkLabel::EndRelation || type_lexrep == IkLabel::BeginEndRelation); // last one might be unnecessary ?
          if (type == IkLabel::Relation && !IsRelation) {
            continue; //Skip non-relations in normalized relation merges
          }
          output += i->GetNormalizedValue();
          output += separator; // Add lexrep separator
        }
        if (!(output.size() < separator.size())) output.erase(output.size() - separator.size());
	normalized_cache_ = IkLexrep::PoolString(output);
	return *normalized_cache_;
      }

      void SetNormalizedValue(const iknow::base::String& value) { // merge lexreps
		if (!lexreps_.empty()) {
			lexreps_[0].SetNormalizedValue(value);
			lexreps_[0].SetTextPointerEnd(lexreps_[lexreps_.size() - 1].GetTextPointerEnd());
		}
		lexreps_.resize(1, lexreps_[0]);
		value_cache_ = 0;
		normalized_cache_ = 0;
      }

      const iknow::base::String& GetValue(const iknow::base::String& separator=iknow::base::SpaceString()) const {
		if (value_cache_) return *value_cache_;
		if (LexrepsCount() == 1) {
			if (separator == iknow::base::String() && iknow::base::IkStringAlg::IsJpnChar(lexreps_[0].GetValue()[0])) { // prepend with space to signal no-space (SCJ logic)
				value_cache_ = IkLexrep::PoolString(iknow::base::SpaceString() + lexreps_[0].GetValue());
			} else
				value_cache_ = IkLexrep::PoolString(lexreps_[0].GetValue());
			return *value_cache_;
		}
		static iknow::base::String output(64,static_cast<iknow::base::Char>(0));
		output.replace(0,output.size(), 0, static_cast<iknow::base::Char>(0)); // Clear it without resizing the buffer
		for (const_iterator i = LexrepsBegin(); i != LexrepsEnd(); ++i) {
			if (i->GetValue().empty()) continue;
			if (i == LexrepsBegin() && separator == iknow::base::String() && iknow::base::IkStringAlg::IsJpnChar(i->GetValue()[0])) output += iknow::base::SpaceString(); // for Japanese, start the first lexrep with a space to signal no-space
			//Space prepended literals need to be attached to the previous
			//lexrep if present, which means removing its trailing space as
			//well as the space prepended marker.
			if (i->GetValue()[0] == ' ' && !output.empty() && separator==iknow::base::SpaceString()) { // only if space is word separator, not for Japanese !
				output.erase(output.size() - separator.size()); // remove the separator
				output += i->GetValue().substr(1);
			} else {
				output += i->GetValue();
			}
			output += separator; // Add lexrep separator except for last item. // iknow::base::SpaceString();
		}
		if (!(output.size() < separator.size())) output.erase(output.size() - separator.size());
		value_cache_ = IkLexrep::PoolString(output);
		return *value_cache_;
      }

      //For CRC detection
      bool IsConcept() const { return GetLexrepType() == IkLabel::Concept; }
      bool IsRelation() const { return GetLexrepType() == IkLabel::Relation; }
	  bool IsPathRelevant() const { return GetLexrepType() == IkLabel::PathRelevant; }
	  // bool IsNonRelevant() const { return GetLexrepType() == IkLabel::Nonrelevant; }

      bool HasSpecialLabel(SpecialLabel label) const {
        for (const_iterator i = LexrepsBegin(); i != LexrepsEnd(); ++i) {
          if (i->Contains(i->GetKnowledgebase()->GetLabelIndex(label))) return true;
        }
        return false;
      }
      
      bool IsSubject() const { return IsConcept() && HasSpecialLabel(SubjectLabel); }
      bool IsObject() const { return IsConcept() && HasSpecialLabel(ObjectLabel); }
      bool IsPrimaryRelation() const { return IsRelation() && HasSpecialLabel(PrimaryRelationLabel); }

    private:
      // size_t offsetLiteralBegin_, offsetLiteralEnd_; // refers to literal representation in the original text
      Lexreps lexreps_;
      mutable iknow::base::String* normalized_cache_;
      mutable iknow::base::String* value_cache_;
    };
    typedef std::vector<IkMergedLexrep, iknow::base::PoolAllocator<IkMergedLexrep> > MergedLexreps;

  }
}

#endif //IKNOW_CORE_IKMERGEDLEXREP_H_
