#ifndef IKNOW_SHELL_KBRULE_H_
#define IKNOW_SHELL_KBRULE_H_
#include "IkTypes.h"
#include "IkRuleOption.h"
#include "KbLabel.h"
#include "IkStringAlg.h"
#include <string>
#include <sstream>
#include <vector>
#include "RawBlock.h"
#include "OffsetPtr.h"
#include "utlExceptionFrom.h"
#include "IkRuleInputPattern.h"
#include "IkRuleOutputPattern.h"

namespace iknow {
  namespace shell {

    class KbRule;
    typedef std::vector<iknow::core::IkRuleOption> RuleOptions;
    typedef iknow::core::Phase Phase;
	typedef iknow::core::IkLabel::Type TypeLabel;

    template<typename MapT>
    struct WithRuleLabelMap {
      WithRuleLabelMap(const MapT& label_map) : label_map_(label_map) {}
      const MapT& label_map_;

	  iknow::core::FastLabelSet::Index LookupIndex(const std::string& utf8_label_name) {
		  typename MapT::const_iterator i = label_map_.find(iknow::base::IkStringEncoding::UTF8ToBase(utf8_label_name));
		  if (i == label_map_.end()) 
			throw ExceptionFrom<KbRule>(std::string("Rules loading detected unknown label: \"") + utf8_label_name + std::string("\""));
        return i->second;
      }

    private:
      void operator=(const WithRuleLabelMap& other);
    };

    template<typename MapT>
    struct RuleOutputItemParser  : private WithRuleLabelMap<MapT> {
      RuleOutputItemParser(const MapT& label_map,
			  std::vector<iknow::core::IkRuleOutputAction>& actions) :
	WithRuleLabelMap<MapT>(label_map), actions_(actions) {}
      void operator()(const std::string& item) {
	using iknow::core::IkRuleOutputAction;
	if (item.empty()) throw ExceptionFrom<RuleOutputItemParser>("Illegal empty item in rule output.");
	char action_char = item[0];
	std::string label_name = item.substr(1);
	IkRuleOutputAction::Action action;
	switch (action_char) {
	case '+':
	  action = IkRuleOutputAction::kAddLabel;
	  break;
	case '-':
	  action = IkRuleOutputAction::kRemoveLabel;
	  break;
	default:
	  //Not prefixed, default to +
	  action = IkRuleOutputAction::kAddLabel;
	  label_name.insert(static_cast<size_t>(0),1,action_char);
	}
	actions_.push_back(IkRuleOutputAction(action,
					      WithRuleLabelMap<MapT>::LookupIndex(label_name)));
      }
      std::vector<iknow::core::IkRuleOutputAction>& actions_;
    private:
      void operator=(const RuleOutputItemParser& other);    
    };

    //MapT maps labels names ("base" 2-byte encoding) to indexes 
    template<typename MapT>
    struct RuleOutputStringParser : private WithRuleLabelMap<MapT> {
      RuleOutputStringParser(const MapT& label_map, std::vector<iknow::core::IkRuleOutputPattern>& pattern) :
        WithRuleLabelMap<MapT>(label_map), pattern_(pattern) {}

	  void operator()(const std::string& utf8_label_string) {
		  using iknow::core::IkRuleOption;
		  using iknow::core::FastLabelSet;
		  using iknow::core::IkRuleOutputPattern;
		  using iknow::core::IkRuleOutputAction;

		  if (utf8_label_string == std::string("*")) { // No Operation output pattern : don't touch
			  pattern_.push_back(IkRuleOutputPattern()); // Default is no operation
			  return;
		  }
		  //Parse off any modifier character if present.
		  std::string utf8_label_name = utf8_label_string;
		  IkRuleOption options;

		  if (utf8_label_name == std::string("Join")) { // to be backwards compatible, a Join action needs to remove all labels, even on different phases
            options = IkRuleOption::GetJoinRuleOptions();
		  } else {
            char modifier = utf8_label_name[0];
            switch (modifier) {
            case '+':
			  options = IkRuleOption::GetAdditionRuleOptions();
			  utf8_label_name.erase(0, 1);
			  break;
            case '^':
	          options = IkRuleOption::GetReplacementRuleOptions();
	          utf8_label_name.erase(0, 1);
	          break;
            case '-':
	          //Starting with a - means don't remove labels by default.
	          options = IkRuleOption::GetAdditionRuleOptions();
	          //But we don't parse it out so it will be treated as a removal.
	          break;
            default:
	          options = IkRuleOption::GetDefaultRuleOptions();
            }
	      }

		typedef std::vector<IkRuleOutputAction> Actions; 
		Actions actions;

		RuleOutputItemParser<MapT> item_parser(WithRuleLabelMap<MapT>::label_map_, actions);
		//A simple tokenizer since this doesn't have to be that fast.
		std::string::const_iterator begin = utf8_label_name.begin();
		const std::string::const_iterator end = utf8_label_name.end();
		for (std::string::const_iterator i = begin; i != end; ++i) {
			if (*i == '+' || *i == '-') {
				if (i > begin) item_parser(std::string(begin, i)); //Anything so far?
				begin = i;
			}
		}
		if (begin != end) item_parser(std::string(begin, end)); //Anything left over?

		pattern_.push_back(IkRuleOutputPattern(actions.begin(), actions.end(), options)); // store output pattern
      }
      std::vector<iknow::core::IkRuleOutputPattern>& pattern_;
    private:
      void operator=(const RuleOutputStringParser& other);
    };

	struct RuleInputExtraOptionParser {
		RuleInputExtraOptionParser(short& lexrep_length) : lexrep_length_(lexrep_length) {}
		void operator()(const char* begin_token, const char* end_token) { 
			std::string buf(begin_token, end_token); // std::string buf("len=2");
			size_t equal_sign_offset = buf.find('=');
			if (equal_sign_offset != std::string::npos) {
				std::string key(begin_token, begin_token + equal_sign_offset);
				std::string value(begin_token + equal_sign_offset +1, end_token);
				if (!key.compare("len")) {
					int value_int = static_cast<int> (value[0] - '0'); // range is 0 to 9
					if (value_int > 0 && value_int <= 9) lexrep_length_ = static_cast<short> (value_int);
				}
			}
		}
		short& lexrep_length_;
	private:
		void operator=(const RuleInputExtraOptionParser& other);
	};

    template<typename MapT>
    struct RuleInputItemParser  : private WithRuleLabelMap<MapT> {
      RuleInputItemParser(const MapT& label_map,
			  std::vector<iknow::core::FastLabelSet::Index>& labels,
			  std::vector<iknow::core::FastLabelSet::Index>& or_labels,
			  std::vector<iknow::core::IkRuleInputPattern::MatchOption>& options,
			  bool& uses_label_types) :
      WithRuleLabelMap<MapT>(label_map), labels_(labels), or_labels_(or_labels), options_(options), uses_label_types_(uses_label_types) {}
      void operator()(const char* begin_token, const char* end_token) {
		std::string buf(begin_token, end_token);
		iknow::core::IkRuleInputPattern::MatchOption option = ConsumeOption(buf); // kNormal or kNegated at this stage, or single label
		if (option == iknow::core::IkRuleInputPattern::kSingleLabelInPhase || option == iknow::core::IkRuleInputPattern::kSingleLabel) {
			labels_.push_back(WithRuleLabelMap<MapT>::LookupIndex(buf));
			options_.push_back(option);
			return;
		}
		size_t or_pos = buf.find(':');
		size_t or_cnt = static_cast<size_t>(0);

		if (or_pos == std::string::npos) { // no or-labels
			TypeLabel typLabel = iknow::core::IkLabel::TypeStringToType(buf); // buf might represent a type ?
			if (typLabel != iknow::core::IkLabel::Unknown) { // label is a type
			  uses_label_types_ = true;
			  labels_.push_back(static_cast<iknow::core::FastLabelSet::Index>(typLabel)); // store the type, not the label index
			  options_.push_back(option == iknow::core::IkRuleInputPattern::kNormal ? iknow::core::IkRuleInputPattern::kTypeNormal : iknow::core::IkRuleInputPattern::kTypeNegated);
			} else {
				labels_.push_back(WithRuleLabelMap<MapT>::LookupIndex(buf));
				options_.push_back(option);
			}
		} else {
			std::string or_label(buf, static_cast<size_t>(0), or_pos);
			bool or_is_type = false;
			TypeLabel typLabel = iknow::core::IkLabel::TypeStringToType(or_label); // buf might represent a type ?
			if (typLabel != iknow::core::IkLabel::Unknown) { // label is a type
				uses_label_types_ = or_is_type = true;
				labels_.push_back(static_cast<iknow::core::FastLabelSet::Index>(typLabel)); // store the type, not the label index
				options_.push_back(option == iknow::core::IkRuleInputPattern::kNormal ? iknow::core::IkRuleInputPattern::kTypeNormal : iknow::core::IkRuleInputPattern::kTypeNegated);
			}
			else {
				labels_.push_back(WithRuleLabelMap<MapT>::LookupIndex(or_label));
				options_.push_back(option);
			}
			do {
				size_t or_pos_nxt = buf.find(':', ++or_pos);
				if (or_pos_nxt == std::string::npos) or_pos_nxt = buf.length(); // The last piece
				std::string nxt_label(buf, or_pos, or_pos_nxt-or_pos);
				or_labels_.push_back(or_is_type ? static_cast<iknow::core::FastLabelSet::Index>(iknow::core::IkLabel::TypeStringToType(nxt_label)) : WithRuleLabelMap<MapT>::LookupIndex(nxt_label));
				++or_cnt;
				or_pos = or_pos_nxt;
			} while (or_pos != buf.length());
			if (or_cnt > iknow::core::IkRuleInputPattern::kOrLabels) throw ExceptionFrom<KbRule>("Too many or-labels in rule input pattern.");
		}
		for (; or_cnt < iknow::core::IkRuleInputPattern::kOrLabels; ++or_cnt) or_labels_.push_back(iknow::core::FastLabelSet::NPos()); // neutralize or labels
      }
      //Finds the option value for a label name and returns it, removing any sigil from
      //the label name if present.
      iknow::core::IkRuleInputPattern::MatchOption ConsumeOption(std::string& utf8_label_name) {
        iknow::core::IkRuleInputPattern::MatchOption option = iknow::core::IkRuleInputPattern::kNormal;
        if (utf8_label_name.empty()) return option;
        char modifier = utf8_label_name[0];
		char negation_parameter = utf8_label_name[1];
        switch(modifier) {
        case '^':
			if (negation_parameter == '=') { // "^=" : not single label (on all phases)
				option = iknow::core::IkRuleInputPattern::kNotSingleLabel;
				utf8_label_name.erase(0, 2);
				break;
			}
			if (negation_parameter == '~') { // "~=" : not single label on (rule) phase
				option = iknow::core::IkRuleInputPattern::kNotSingleLabelInPhase;
				utf8_label_name.erase(0, 2);
				break;
			}
          option = iknow::core::IkRuleInputPattern::kNegated;
          utf8_label_name.erase(0,1);
          break;
		case '=': // single label indicator
			option = iknow::core::IkRuleInputPattern::kSingleLabel;
			utf8_label_name.erase(0, 1);
			break;
		case '~': // single phase label indicator
			option = iknow::core::IkRuleInputPattern::kSingleLabelInPhase;
			utf8_label_name.erase(0, 1);
			break;

        default:
          break;
        }
        return option;
      }
      std::vector<iknow::core::FastLabelSet::Index>& labels_;
	  std::vector<iknow::core::FastLabelSet::Index>& or_labels_;
      std::vector<iknow::core::IkRuleInputPattern::MatchOption>& options_;
	  bool& uses_label_types_;
    private:
      void operator=(const RuleInputItemParser& other);
    };

    //MapT maps labels names ("base" 2-byte encoding) to indexes 
    template<typename MapT>
    struct RuleInputStringParser : private WithRuleLabelMap<MapT> {
      RuleInputStringParser(const MapT& label_map, std::vector<iknow::core::IkRuleInputPattern>& pattern) :
        WithRuleLabelMap<MapT>(label_map), pattern_(pattern) {}
      void operator()(const std::string& utf8_label_pattern) {
        if (utf8_label_pattern.empty()) return; //ignore empty labels
        std::vector<iknow::core::FastLabelSet::Index> labels;
		std::vector<iknow::core::FastLabelSet::Index> or_labels;
        std::vector<iknow::core::IkRuleInputPattern::MatchOption> options;
		bool uses_label_types = false; // will be set true if rule input pattern uses type labels
		iknow::core::IkRuleInputPattern::VariableOption rule_var = iknow::core::IkRuleInputPattern::kNot;
		RuleInputItemParser<MapT> item_parser(WithRuleLabelMap<MapT>::label_map_, labels, or_labels, options, uses_label_types);
		const char* begin_pattern = utf8_label_pattern.c_str();
		size_t pattern_extra_options = utf8_label_pattern.find('(');
		const char* end_pattern = begin_pattern + (pattern_extra_options != std::string::npos ? pattern_extra_options : utf8_label_pattern.length());
		short lexrep_length_extra_condition = static_cast<short>(0);
		bool b_narrow = false; // variable narrow flag
		if (pattern_extra_options != std::string::npos) {
			RuleInputExtraOptionParser option_parser(lexrep_length_extra_condition);
			const char* begin_options = begin_pattern + pattern_extra_options + 1; // skip leading '('
			const char* end_options = &(*(utf8_label_pattern.end() - 1)); // skip ending ')'
			iknow::base::IkStringAlg::Tokenize(begin_options, end_options, ',', option_parser);
		}
		iknow::core::IkRuleInputPattern::VariableLength var_length_limits; // possible limitations for variable length rule pattern
		bool hasVariableLimits = false;
		if (*begin_pattern == '*') { // one or more variable
			rule_var = iknow::core::IkRuleInputPattern::kOneOrMore;
			++begin_pattern;
			if (*begin_pattern == '<') { // optional "narrow" select attribute
				b_narrow = true;
				++begin_pattern;
			}
			if (*begin_pattern == '{') { // optional min and max specification
				size_t closing_pattern_position = utf8_label_pattern.find('}');
				if (closing_pattern_position == std::string::npos) throw ExceptionFrom<KbRule>("missing closing '}' found in rule.");// error : no closing symbol '}'

				// char pattern2[] = "{ 5, 10 }";
				std::istringstream iss_pattern(std::string(begin_pattern + 1, utf8_label_pattern.c_str() + closing_pattern_position));
				iss_pattern >> var_length_limits.min_match;
				if ((iss_pattern.rdstate() & std::istringstream::failbit) != 0) throw ExceptionFrom<KbRule>("missing closing '}' found in rule.");// error : no closing symbol '}'
				char ch_separator;
				iss_pattern >> ch_separator >> var_length_limits.max_match;
				if ((iss_pattern.rdstate() & std::istringstream::failbit) != 0) var_length_limits.max_match = var_length_limits.min_match; // equal min & max = fixed length : "{ 12 }"

				begin_pattern = utf8_label_pattern.c_str() + closing_pattern_position +1;
				hasVariableLimits = true;
			}
		}
		if (*begin_pattern == '.') { // zero or more variable
			rule_var = iknow::core::IkRuleInputPattern::kZeroOrMore;
			++begin_pattern;
			if (*begin_pattern == '<') { // optional "narrow" select attribute
				b_narrow = true;
				++begin_pattern;
			}
		}
        iknow::base::IkStringAlg::Tokenize(begin_pattern, end_pattern, '+', item_parser);
		pattern_.push_back(iknow::core::IkRuleInputPattern(labels.begin(), labels.end(), or_labels.begin(), or_labels.end(), options.begin(), options.end(), uses_label_types, rule_var, b_narrow, lexrep_length_extra_condition, begin_pattern, end_pattern));
		if (hasVariableLimits) {
			pattern_.back().SetVariableLimits(var_length_limits);
		}
	  }
      std::vector<iknow::core::IkRuleInputPattern>& pattern_;
    private:
      void operator=(const RuleInputStringParser& other);
    };

    class KbRule {
    public:
      //MapT maps label names to KbLabel*'s
      template<typename MapT>
      KbRule(RawAllocator& allocator, const MapT& label_map, const std::string& input_pattern, const std::string& output_pattern, Phase phase) {
		std::vector<iknow::core::IkRuleInputPattern> input_pattern_vec;
		RuleInputStringParser<MapT> input_adder(label_map, input_pattern_vec);
		iknow::base::IkStringAlg::Tokenize(input_pattern, '|', input_adder);
		std::vector<iknow::core::IkRuleOutputPattern> output_pattern_vec;
		RuleOutputStringParser<MapT> output_adder(label_map, output_pattern_vec);
		iknow::base::IkStringAlg::Tokenize(output_pattern, '|', output_adder);
		input_pattern_begin_ = allocator.InsertRange(input_pattern_vec.begin(), input_pattern_vec.end());
		input_pattern_end_ = input_pattern_begin_ + input_pattern_vec.size();
		output_pattern_begin_ = allocator.InsertRange(output_pattern_vec.begin(), output_pattern_vec.end());
		output_pattern_end_ = output_pattern_begin_ + output_pattern_vec.size();
		if (phase > iknow::core::kMaxPhase)
			throw ExceptionFrom<KbRule>("Illegal phase number found in rule.");
		phase_ = phase;
      }
	  KbRule(const KbRule& other) { // need explicit copy constructor to avoid random binary bytes
		  this->input_pattern_begin_ = other.input_pattern_begin_;
		  this->input_pattern_end_ = other.input_pattern_end_;
		  this->output_pattern_begin_ = other.output_pattern_begin_;
		  this->output_pattern_end_ = other.output_pattern_end_;
		  this->phase_ = other.phase_;
	  }
      const iknow::core::IkRuleInputPattern* InputPatternBegin() const { return input_pattern_begin_;  }
      const iknow::core::IkRuleInputPattern* InputPatternEnd() const { return input_pattern_end_; }
      const iknow::core::IkRuleOutputPattern* OutputPatternBegin() const { return output_pattern_begin_; }
      const iknow::core::IkRuleOutputPattern* OutputPatternEnd() const { return output_pattern_end_; }

      iknow::core::Phase GetPhase() const { return phase_; }

    private:
      OffsetPtr<const iknow::core::IkRuleInputPattern> input_pattern_begin_;
      OffsetPtr<const iknow::core::IkRuleInputPattern> input_pattern_end_;
      OffsetPtr<const iknow::core::IkRuleOutputPattern> output_pattern_begin_;
      OffsetPtr<const iknow::core::IkRuleOutputPattern> output_pattern_end_;
      iknow::core::Phase phase_;
    };
  }
}


#endif //IKNOW_SHELL_KBRULE_H_
