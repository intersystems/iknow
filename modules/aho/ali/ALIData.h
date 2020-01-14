#ifndef IKNOW_AHO_ALI_ALIDATA_H_
#define IKNOW_AHO_ALI_ALIDATA_H_
#include "AhoTypes.h"
#include "AhoExport.h"

namespace iknow {
  namespace model {
    class AHO_API ALIData {
    public:
      //TODO: Such an ugly constructor
      ALIData(const Symbol* one_state_map,
	      const SmallState* next_state_map,
	      const Symbol* symbol_map,
	      const SmallState* state_map,
	      const SmallState* failure_table,
	      const Score* output) :
	one_state_map_(one_state_map),
	next_state_map_(next_state_map),
	symbol_map_(symbol_map),
	state_map_(state_map),
	failure_table_(failure_table),
	output_(output) {}

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

      //Failure

      //The state to fail to when no valid transitions are
      //possible.
      const SmallState* FailureTable() const { return failure_table_; }
      

      //Output

      const Score* Output() const { return output_; }

    private:
      const Symbol* one_state_map_;
      const SmallState* next_state_map_;
      const Symbol* symbol_map_;
      const SmallState* state_map_;
      const SmallState* failure_table_;
      const Score* output_;
    };
  }
}

#endif //IKNOW_AHO_LEXREP_LEXREPDATA_H_
