#ifndef IKNOW_CORE_IKRULEOUTPUTPATTERN_H_
#define IKNOW_CORE_IKRULEOUTPUTPATTERN_H_
#include "IkLabel.h"
#include "IkRuleOption.h"
#include "IkLexrep.h"
#include "IkRuleInputPattern.h"

namespace iknow {
  namespace core {

    //This struct represents the action to be taken for the
    //given label in the pattern.
    class IkRuleOutputAction {
    public:
      enum Action {
        kAddLabel,
        kRemoveLabel
      };
      IkRuleOutputAction() : action_(kAddLabel), label_(FastLabelSet::NPos()) {}
      IkRuleOutputAction(Action action, FastLabelSet::Index label) : 
        action_(action),
        label_(label) {}
      IkRuleOutputAction(const IkRuleOutputAction& other) { // explicit copy constructor
          this->action_ = other.action_;
          this->label_ = other.label_;
      }
      const IkRuleOutputAction& operator=(const IkRuleOutputAction& rhs) { // explicit copy operator
          this->action_ = rhs.action_;
          this->label_ = rhs.label_;
          return *this;
      }

      Action GetAction() const {
        return action_;
      }
      FastLabelSet::Index GetLabel() const {
        return label_;
      }
      void Apply(IkLexrep& lexrep) const {
        if (action_ == kAddLabel) {
          lexrep.AddLabelIndex(label_);
        } else {
          lexrep.RemoveLabelIndex(label_);
        }
      }
    private:
      Action action_;
      FastLabelSet::Index label_;
    };

    class IkRuleOutputPattern {
    public:
      static const size_t kPatternSize = 8;

      //TODO: Remove duplication with IkRuleInputPattern

      template<typename ActionIter>
      IkRuleOutputPattern(ActionIter begin_action, ActionIter end_action, IkRuleOption options) : options_(options) {
		if (begin_action == end_action) throw ExceptionFrom<IkRuleOutputPattern>("Empty rule output pattern.");
        size_t action_size = end_action - begin_action;
		if (action_size > kPatternSize) throw ExceptionFrom<IkRuleOutputPattern>("Rule output pattern too large.");
		if (begin_action->GetLabel() == FastLabelSet::NPos()) throw ExceptionFrom<IkRuleOutputPattern>("Pattern first index cannot be NPos");
        std::fill(std::copy(begin_action, end_action, ActionsBegin()), ActionsEnd(), IkRuleOutputAction());
      }
	  IkRuleOutputPattern() : options_(IkRuleOption::kNOP) { // No Operation output pattern
		  std::fill(ActionsBegin(), ActionsEnd(), IkRuleOutputAction()); // fill with non actions
	  }
      IkRuleOutputPattern(const IkRuleOutputPattern& other) { // explicit copy constructor
          std::copy(other.actions_, other.actions_ + (sizeof other.actions_ / sizeof other.actions_[0]), this->actions_);
          this->options_ = other.options_;
      }
      //Apply this label output pattern to the given lexrep. Some label assignments require consulting
      //the input pattern.
      void Apply(IkLexrep& lexrep, const IkRuleInputPattern& in, Phase phase) const {
		if (GetOptions().HasOption(IkRuleOption::kNOP)) return; // No Operation, don't touch
        if (GetOptions().HasOption(IkRuleOption::kClearAllPhases)) { // Join actions must clear all labels to be backwards compatible
          lexrep.ClearAllLabels();
        }
		bool hasSBegin = lexrep.hasSBeginLabel(phase); // if present, these must be reset.
		bool hasSEnd = lexrep.hasSEndLabel(phase);

        if (GetOptions().HasOption(IkRuleOption::kClearAll)) {
          lexrep.ClearLabels(phase);
        }
        if (GetOptions().HasOption(IkRuleOption::kClearInputLabels)) { //Clear labels from input pattern, regardless of option.
          const FastLabelSet::Index* i_or = in.OrLabelsBegin(); // track the or labels.
		  const IkRuleInputPattern::MatchOption* i_option = in.OptionsBegin(); // track options
          for (const FastLabelSet::Index* i = in.LabelsBegin();
	        i != in.LabelsEnd();
	        ++i, ++i_option) {
            if (*i == FastLabelSet::NPos()) break;
			switch (*i_option) {
			case IkRuleInputPattern::kNormal: // individual labels
				lexrep.RemoveLabelIndex(*i);
				for (size_t OrCnt = 0; OrCnt < IkRuleInputPattern::kOrLabels; ++OrCnt, ++i_or) { // also remove the 'or' label, if they exists.
					if (*i_or != FastLabelSet::NPos()) lexrep.RemoveLabelIndex(*i_or);
				}
				break;
			case IkRuleInputPattern::kTypeNormal: // type labels
				lexrep.RemoveLabelType(*i, phase);
				for (size_t OrCnt = 0; OrCnt < IkRuleInputPattern::kOrLabels; ++OrCnt, ++i_or) { // also remove the 'or' label, if they exists.
					if (*i_or != FastLabelSet::NPos()) lexrep.RemoveLabelType(*i_or, phase);
				}
				break;

			default: // at this moment, the rest are "negated" label selections, no need to remove something that was not present in the first place :-)
				break;
			}
          }
        }
        //Apply actions until we see a null label.
        for (size_t i=0; i < kPatternSize; ++i) {
          if (actions_[i].GetLabel() == FastLabelSet::NPos()) break;
          actions_[i].Apply(lexrep);
        }
		if (hasSEnd) lexrep.setSEndLabel(); // reset SEnd & SBegin labels
		if (hasSBegin) lexrep.setSBeginLabel();
      }
      IkRuleOption GetOptions() const {
        return options_;
      }
      //Needed for Joins, which should only have one label
      FastLabelSet::Index PrimaryLabel() const {
        return actions_[0].GetLabel();
      }

      //TODO:Why do these need to be fixed size? Why not just begin/end pointers in the KB?
      //TODO: Oh yeah, the KB can't have pointers because they need to be offsets in shared memory.
      //All pointers require a call through the KB pointer + vtable etc. Slow.
      const IkRuleOutputAction* ActionsBegin() const { return &(actions_[0]); }
      const IkRuleOutputAction* ActionsEnd() const { return &(actions_[kPatternSize]); }
      IkRuleOutputAction* ActionsBegin() { return &(actions_[0]); }
      IkRuleOutputAction* ActionsEnd() { return &(actions_[kPatternSize]); }

    private:
      IkRuleOutputAction actions_[kPatternSize];
      IkRuleOption options_;
    };
  }
}

#endif //IKNOW_CORE_IKRULEOUTPUTPATTERN_H_
