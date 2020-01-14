#ifndef IKNOW_AHO_ALI_ALIFUNCTIONS_H_
#define IKNOW_AHO_ALI_ALIFUNCTIONS_H_
#include "AhoTypes.h"
#include "ALIData.h"
#include <algorithm>
#include <numeric>
#include "LanguageBase.h"
#include "unicode/uchar.h"
#include "AhoExport.h"
#include <vector>
#include <utility>

namespace iknow {
  namespace model {

    class AHO_API ALIFunctions {
    public:
      typedef iknow::base::Char Char;
      explicit ALIFunctions(const ALIData& data) : data_(data) {}
      double ScoreText(const Char* begin, const Char* end, size_t language_total_score) const;
    private:
      State Goto(State s, Char c) const;
      State Failure(State s) const;
      Score Output(State s) const;
      ALIData data_;
      friend struct OutputAccumulator;
    };
    

    struct AHO_API OutputAccumulator {
      explicit OutputAccumulator(const ALIFunctions& funcs) : f(funcs), state(0), total(0), matches(0) {}
      const ALIFunctions& f;
      State state;
      Score total;
      size_t matches;
      OutputAccumulator operator+(Char next_char) const {
	OutputAccumulator next = *this;
	next += next_char;
	return next;
      }

      OutputAccumulator& operator+=(Char next_char) {
	State s = state;
	State t;
	while ((t = f.Goto(s, next_char)) == kFailState) {
	  s = f.Failure(s);
	}
	state = t;
	const Score score = f.Output(state);
	total += score;
	++matches;
	return *this;
      }
    private:
      void operator=(const OutputAccumulator&);
    };

    inline double ALIFunctions::ScoreText(const Char* begin, const Char* end, size_t language_total_score) const {
      const size_t cluster_size = 4;
      if (static_cast<size_t>(end - begin) + 1 < cluster_size) return 0; //no clusters
      const Char* pos = begin;
      Char last_char = 0;
      size_t last_alpha_char = 0;
      OutputAccumulator out(*this);
      out += ' ';
      while (pos != end) {
	Char cur_char = iknow::ali::LanguageBase::NextClusterChar(pos, end, last_char);
	if (!cur_char) continue;
	//If the character is alphanumeric, it will be around for cluster_size cycles
	if ((cur_char >= L'A') && (cur_char <= L'z')) {
	  last_alpha_char = cluster_size;
	}
	//For non-ASCII chars, check ICU
	else if (cur_char > 127 && u_isalpha(cur_char)) {
	  last_alpha_char = cluster_size;
	}
	//If we have an alpha char in the current cluster, accumulate
	if (last_alpha_char) {
	  out += cur_char;
	  --last_alpha_char;
	}
	last_char = cur_char;
      }
      if (out.matches < cluster_size) return 0;
      //Don't consider the first few chars, impossible to match there.
      const size_t str_total_score = out.matches - (cluster_size - 1);
      const size_t match_score = out.total;
      return str_total_score?1000000.0*(double)match_score/(double)language_total_score/(double)str_total_score:0;
    }

    inline State ALIFunctions::Goto(State s, Char c) const {
      //0 accepts any value and loops back to state 0
      const State failState = s == 0 ? 0 : kFailState;
      //We use a heterogeneous structure to store the DFA in order
      //to minimize storage requirements and maximize cache-ability.
      //

      //Most states (65% in the English model) have one exit transition.

      //Here, we use a pair of arrays each with
      //a value for each state. A pair of arrays is used instead of a single array
      //for alignment reasons: the padding of storing structs of pairs would
      //increase space requirements significantly.
      //
      //The symbol for a state in the one state map is either the symbol
      //for the sole exit transition of that state, or 0, indicating no
      //exit transitions, or -1, indicating many.
      Symbol sym = data_.OneStateMap()[s];
      //If the symbol is 0, there are no exit transitions
      if (sym == 0) return failState;
      //If the symbol is not -1, check that the exit transition matches the character
      //we have. If so, return the corresponding next state.
      if (sym != (Symbol)-1) return sym == c ? data_.NextStateMap()[s] : failState;
      //If the symbol is -1, unpack the value in NextStateMap into a count of
      //transitions and their offset in the transition list.
      size_t range = data_.NextStateMap()[s];
      size_t count = range >> 20; //2^12 = 4096 maximum transitions for a single state
      size_t offset = range & 03777777; //2^20 = A million possible transitions
      //The location is an offset into the SymbolMap and StateMap tables.
      //We'll binary search the (sorted) range in the SymbolMap table to find
      //the offset for a matching transition, if any.
      const Symbol* begin(&(data_.SymbolMap()[offset]));
      const Symbol* end = begin + count;
      const Symbol* lower_bound = std::lower_bound(begin, end, c);
      if (lower_bound == end) return failState;
      if (*lower_bound == c) return data_.StateMap()[offset + (lower_bound - begin)];
      return failState;
    } 

    inline State ALIFunctions::Failure(State s) const {
      return data_.FailureTable()[s];
    }

    inline Score ALIFunctions::Output(State s) const {
      return data_.Output()[s];
    }

  }
}

#endif //IKNOW_AHO_ALI_ALIFUNCTIONS_H_
