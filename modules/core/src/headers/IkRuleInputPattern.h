#ifndef IKNOW_CORE_IKRULEINPUTPATTERN_H_
#define IKNOW_CORE_IKRULEINPUTPATTERN_H_
#include "IkLabel.h"
#include "utlExceptionFrom.h"
#include <algorithm>
#include <limits.h>

namespace iknow {
  namespace core {
    class IkRuleInputPattern { // Provides semantics for matching label sets.
    public:
      enum MatchOption {
        kNormal,
        kNegated,
		kTypeNormal,
		kTypeNegated,
		kSingleLabel, // single label in lexrep
		kSingleLabelInPhase, // single label in rule phase
		kNotSingleLabel,	// not single label in lexrep
		kNotSingleLabelInPhase	// not single label in rule phase
	  };
	  enum VariableOption {
		  kNot, // default, fixed, one rule pattern per lexrep
		  kOneOrMore, // one or more consecutive lexreps matching the rule
		  kZeroOrMore // not matching (zero), or more lexreps matching the rule 
	  };
	  struct VariableLength { 
		  VariableLength() : min_match(1), max_match(INT_MAX) {} // constructor
		  int min_match;
		  int max_match;
	  };
      static const size_t kPatternSize = 8;
	  static const size_t kOrLabels = 7; // 7 extra or-labels per rule input pattern label
      template<typename IndexIter, typename OptionIter>
	  IkRuleInputPattern(
		  IndexIter begin_index, IndexIter end_index,
		  IndexIter begin_or_index, IndexIter end_or_index,
		  OptionIter begin_option, OptionIter end_option,
		  bool usesTypeLabels, 
		  VariableOption variable_pattern, bool b_narrow, 
		  short lexrep_length_option,
		  const char *begin_pattern = NULL, const char *end_pattern = NULL) : /* two more parameters to pass the input rule pattern in case of trouble, at lot, i know, but this code is not critical.*/
		  usesTypeLabels_(usesTypeLabels),
		  variable_pattern_(variable_pattern),
		  lexrep_length_(lexrep_length_option),
		  var_narrow_(b_narrow) {
			if (begin_index == end_index) throw ExceptionFrom<IkRuleInputPattern>("Empty rule input pattern.");
			size_t pattern_size = end_index - begin_index;
			size_t option_size = end_option - begin_option;
			std::string rule_input_pattern("unknow rule input pattern");
			if (begin_pattern && end_pattern) rule_input_pattern = std::string(begin_pattern, end_pattern);

			if (pattern_size > kPatternSize) {
			  std::string error_msg = std::string("Rule input pattern too large: \"") + rule_input_pattern + std::string("\"");
			  throw ExceptionFrom<IkRuleInputPattern>(error_msg.c_str());
			}
			if (option_size > kPatternSize) {
			  std::string error_msg = std::string("Rule option pattern too large: \"") + rule_input_pattern + std::string("\"");
			  throw ExceptionFrom<IkRuleInputPattern>(error_msg.c_str());
			}
			if (*begin_index == FastLabelSet::NPos()) {
			  std::string error_msg = std::string("Pattern first index cannot be NPos: \"") + rule_input_pattern + std::string("\"");
			  throw ExceptionFrom<IkRuleInputPattern>(error_msg.c_str());
			}
			std::fill(std::copy(begin_index, end_index, LabelsBegin()), LabelsEnd(), FastLabelSet::NPos());
			std::fill(std::copy(begin_or_index, end_or_index, OrLabelsBegin()), OrLabelsEnd(), FastLabelSet::NPos());
			std::fill(std::copy(begin_option, end_option, OptionsBegin()), OptionsEnd(), kNormal);
			size_t required_count = 0;
			if (variable_pattern_ != kZeroOrMore) { // if 'zero' match, no labels can be on the required list.
				for (size_t i = 0; i < kPatternSize; ++i) {
					if (options_[i] == kNormal && or_labels_[i*kOrLabels] == FastLabelSet::NPos() && labels_[i] != FastLabelSet::NPos()) // OR labels cannot be put on the required list
						required_labels_[required_count++] = labels_[i];
				}
			}
			for (size_t i=required_count; i < kPatternSize; ++i) {
				required_labels_[i] = FastLabelSet::NPos();
			}
      }
	  IkRuleInputPattern(const IkRuleInputPattern& other) { // explicit copy constructor to avoid random binary data
		std::copy(other.labels_, other.labels_ + (sizeof other.labels_ / sizeof other.labels_[0]), this->labels_);
		std::copy(other.or_labels_, other.or_labels_ + (sizeof other.or_labels_ / sizeof other.or_labels_[0]), this->or_labels_);
		std::copy(other.required_labels_, other.required_labels_ + (sizeof other.required_labels_ / sizeof other.required_labels_[0]), this->required_labels_);
		std::copy(other.options_, other.options_ + (sizeof other.options_ / sizeof other.options_[0]), this->options_);
		this->usesTypeLabels_ = other.usesTypeLabels_;
		this->variable_pattern_ = other.variable_pattern_;
		this->variable_length_ = other.variable_length_;
		this->lexrep_length_ = other.lexrep_length_; 
		this->var_narrow_ = other.var_narrow_;
	  }
	  bool IsMatch(const FastLabelSet& labels) const {
		  for (size_t i = 0; i < kPatternSize; ++i) {
			  if (labels_[i] == FastLabelSet::NPos()) break; // reached end of rule input pattern
			  bool is_present = labels.Contains(labels_[i]); // does lexrep phase(p) labels contain rule input pattern label ?
			  if (options_[i] == kSingleLabel || options_[i] == kSingleLabelInPhase) { // shortcut on single label
				  return is_present && (labels.Size() == 1); // the only label for the lexrep AND present
			  }
			  if (options_[i] == kNotSingleLabel || options_[i] == kNotSingleLabelInPhase) {
				  return !(is_present && (labels.Size() == 1)); // the only label for the lexrep AND *not* present
			  }
			  bool is_negated = (options_[i] == kNegated); // and does it have be ?
			  if (!is_present) { // scan the or-list
				  for (size_t or_index = i*kOrLabels; or_index < (i*kOrLabels) + kOrLabels && !is_present; or_index++) {
					  if (or_labels_[or_index] == FastLabelSet::NPos()) break; // no more or labels
					  is_present = labels.Contains(or_labels_[or_index]);
				  }
			  }
			  if (!(is_present ^ is_negated)) return false; // if both are equal, return false, if not, continue
		  }
		  return true;
	  }
      bool IsMatch(const FastLabelSet& labels, const FastLabelTypeSet& type_labels) const {
		for (size_t i = 0; i < kPatternSize; ++i) {
			if (labels_[i] == FastLabelSet::NPos()) break;
			bool is_type = (options_[i] == kTypeNormal || options_[i] == kTypeNegated);
			if (!is_type) {
				bool is_present = labels.Contains(labels_[i]);
				bool is_negated = (options_[i] == kNegated);
				if (!is_present) { // scan the or-list
					for (size_t or_index = i*kOrLabels; or_index < (i*kOrLabels) + kOrLabels && !is_present; or_index++) {
						if (or_labels_[or_index] == FastLabelSet::NPos()) break; // no more or labels
						is_present = labels.Contains(or_labels_[or_index]);
					}
				}
				if (!(is_present ^ is_negated)) return false;
			} else { // label type search
				bool is_present = type_labels.Contains(labels_[i]);
				bool is_negated = options_[i] == kTypeNegated;
				if (!is_present) { // scan the or-list
					for (size_t or_index = i*kOrLabels; or_index < (i*kOrLabels) + kOrLabels && !is_present; or_index++) {
						if (or_labels_[or_index] == FastLabelSet::NPos()) break; // no more or labels
						is_present = type_labels.Contains(or_labels_[or_index]);
					}
				}
				if (!(is_present ^ is_negated)) return false;
			}
		}
        return true;
      }
      //No semantics, just check for required labels against a set of all seen labels.
      bool MightMatch(const FastLabelBitSet& labels) const {
        for (size_t i = 0; i < kPatternSize; ++i) {
          if (required_labels_[i] == FastLabelSet::NPos()) break;
          if (!labels.Contains(required_labels_[i])) return false;
        }
        return true;
      }
	  bool HasLabelTypes(void) const { return usesTypeLabels_; }
	  bool MatchesGlobalLabelsSet(void) const { return (GetOption(0) == kSingleLabel || GetOption(0) == kNotSingleLabel); } // do we match per phase labels or per lexrep labels
	  bool IsVariable(void) const { return variable_pattern_ != kNot; }
	  bool IsNullVariable(void) const { return (variable_pattern_ == kZeroOrMore /*|| variable_length_.min_match==0*/); }
	  int MinVariable(void) const { return variable_length_.min_match;  }
	  int MaxVariable(void) const { return variable_length_.max_match;  }
	  void SetVariableLimits(VariableLength& var_length_limits) { variable_length_ = var_length_limits;  }

      FastLabelSet::Index PrimaryLabel() const {
        return GetIndex(0);
      }
	  MatchOption PrimaryLabelOption() const {
        return GetOption(0);
	  }
      FastLabelSet::Index GetIndex(size_t i) const {
        return labels_[i];
      }
	  FastLabelSet::Index GetOrIndex(size_t i) const {
		  return or_labels_[i];
	  }
      MatchOption GetOption(size_t i) const {
        return options_[i];
      }
      FastLabelSet::Index* LabelsBegin() { return &(labels_[0]); }
      FastLabelSet::Index* LabelsEnd() { return &(labels_[kPatternSize]); }
	  FastLabelSet::Index* OrLabelsBegin() { return &(or_labels_[0]); }
	  FastLabelSet::Index* OrLabelsEnd() { return &(or_labels_[kPatternSize*kOrLabels]); }

      const FastLabelSet::Index* LabelsBegin() const { return &(labels_[0]); }
      const FastLabelSet::Index* LabelsEnd() const { return &(labels_[kPatternSize]); }
	  const FastLabelSet::Index* OrLabelsBegin() const { return &(or_labels_[0]); }
	  const FastLabelSet::Index* OrLabelsEnd() const { return &(or_labels_[kPatternSize*kOrLabels]); }

      MatchOption* OptionsBegin() { return &(options_[0]); }
      MatchOption* OptionsEnd() { return &(options_[kPatternSize]); }
	  const MatchOption* OptionsBegin() const { return &(options_[0]); }
	  const MatchOption* OptionsEnd() const { return &(options_[kPatternSize]); }

	  short GetLexrepLengthOption() const { return lexrep_length_; }
	  bool IsNarrow(void) const { return var_narrow_; }

    private:
      FastLabelSet::Index labels_[kPatternSize];
	  FastLabelSet::Index or_labels_[kPatternSize*kOrLabels]; // extra optional or-labels...
      FastLabelSet::Index required_labels_[kPatternSize];
      MatchOption options_[kPatternSize];
	  bool usesTypeLabels_;
	  VariableOption variable_pattern_;
	  VariableLength variable_length_;
	  short lexrep_length_; // if specified, this is an extra criterion that specifies the exact length of the lexrep
	  bool var_narrow_; // if true, variable lexrep selections match narrow : if next rule matches next lexrep, finish current rule selector
    };
  }
}

#endif //IKNOW_CORE_IKRULEINPUTPATTERN_H_
