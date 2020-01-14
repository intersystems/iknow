#ifndef IKNOW_AHO_LEXREP_LEXREPDATA_H_
#define IKNOW_AHO_LEXREP_LEXREPDATA_H_
#include "AhoTypes.h"
#include "AhoExport.h"

namespace iknow {
  namespace model {
    class AHO_API LexrepData {
    public:
      //TODO: Such an ugly constructor
      LexrepData(const Symbol* one_state_map,
		 const SmallState* next_state_map,
		 const Symbol* symbol_map,
		 const SmallState* state_map,
		 const Regex* regex_map,
		 const SmallOffset* regex_one_state_map,
		 const SmallState* regex_next_state_map,
		 const SmallOffset* regex_offset_map,
		 const SmallState* regex_state_map,
		 const SmallState* failure_table,
		 const Match* matches,
		 const SmallOffset* match_set,
		 const SmallCount* match_count,
		 size_t max_word_count,
		 bool has_regex,
		 bool is_ideographic) :
	one_state_map_(one_state_map),
	next_state_map_(next_state_map),
	symbol_map_(symbol_map),
	state_map_(state_map),
	regex_map_(regex_map),
	regex_one_state_map_(regex_one_state_map),
	regex_next_state_map_(regex_next_state_map),
	regex_offset_map_(regex_offset_map),
	regex_state_map_(regex_state_map),
	failure_table_(failure_table),
	matches_(matches),
	match_set_(match_set),
	match_count_(match_count),
	max_word_count_(max_word_count),
	has_regex_(has_regex),
	is_ideographic_(is_ideographic)
      {} 
      //Character-based transitions

      //Most states have 0/1 valid transitions.
      //If there is 1, the symbol is here.
      const Symbol* OneStateMap() const { return one_state_map_; }

      //For states with 1 transition, the next state
      //for that transition is here. For more than one,
      //we pack the offset and length of the ranges
      //of symbols and corresponding next states into
      //the SmallState value.
      const SmallState* NextStateMap() const { return next_state_map_; }

      //Those offsets and lengths are ranges on these
      //two tables. Each symbol map range is sorted and the offset
      //where the transition is found corresponds to the
      //offset to find the next state in the state map.
      const Symbol* SymbolMap() const { return symbol_map_; }
      const SmallState* StateMap() const { return state_map_; }

      //Regex-based transitions

      //The regular expressions themselves.
      const Regex* RegexMap() const { return regex_map_; }

      //Similar to the character based map: if there's
      //only one valid regex its offset is here.
      const SmallOffset* RegexOneStateMap() const { return regex_one_state_map_; }

      //For states with 1 regex transition, the next state for
      //that transition is here. For more than one, we pack the
      //offset and length of the ranges of regexes and
      //corresponding next states into the SmallState value.
      const SmallState* RegexNextStateMap() const { return regex_next_state_map_; }

      //The offsets and lengths are ranges into these
      //tables. Every regex in the range must be tested to find
      //the set of valid transition states.
      const SmallOffset* RegexOffsetMap() const { return regex_offset_map_; }
      const SmallState* RegexStateMap() const { return regex_state_map_; }

      //Does this state machine support regular expressions at all?
      bool HasRegex() const { return has_regex_; }

      //Failure

      //The state to fail to when no valid transitions are
      //possible.
      const SmallState* FailureTable() const { return failure_table_; }
      

      //Output

      //Amalgamated ranges of per-state matches (output)
      const Match* Matches() const { return matches_; }
      
      //Offset of the range for each state
      const SmallOffset* MatchSet() const { return match_set_; }

      //Length of the range for each state
      const SmallCount* MatchCount() const { return match_count_; }

      //Metadata

      size_t MaxWordCount() const { return max_word_count_; }
      bool IsIdeographic() const { return is_ideographic_; }
    private:
      const Symbol* one_state_map_;
      const SmallState* next_state_map_;
      const Symbol* symbol_map_;
      const SmallState* state_map_;
      const Regex* regex_map_;
      const SmallOffset* regex_one_state_map_;
      const SmallState* regex_next_state_map_;
      const SmallOffset* regex_offset_map_;
      const SmallState* regex_state_map_;
      const SmallState* failure_table_;
      const Match* matches_;
      const SmallOffset* match_set_;
      const SmallCount* match_count_;
      size_t max_word_count_;
      bool has_regex_;
      bool is_ideographic_;
    };
  }
}

#endif //IKNOW_AHO_LEXREP_LEXREPDATA_H_
