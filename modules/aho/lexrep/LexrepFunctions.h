#ifndef IKNOW_AHO_LEXREP_LEXREPFUNCTIONS_H_
#define IKNOW_AHO_LEXREP_LEXREPFUNCTIONS_H_
#include "AhoTypes.h"
#include "LexrepData.h"
#include "AhoExport.h"
#include <vector>
#include <utility>

namespace iknow {
  namespace model {

    class AHO_API LexrepFunctions {
    public:
      typedef SmallOffset RegexIndex;
      LexrepFunctions(const LexrepData& data) : data_(data) {}
      State Goto(State s, const Char* begin, const Char* end) const;
      State Goto(State s, Char c) const;
      State Failure(State s) const;
      const Match* OutputBegin(State s) const;
      const Match* OutputEnd(State s) const;
      //The count of regexes, indexed from 0 - count-1
      size_t RegexCount(State s) const;
      //Use only when count = 1
      const Regex& GetSoleRegex(State s) const;
      State GetSoleRegexNextState(State s) const;
      //Use only when count > 1
      const Regex& GetRegex(RegexIndex idx) const;
      const RegexIndex* RegexBegin(State s) const;
      const SmallState* RegexGotoBegin(State s) const;
      size_t MaxWordCount() const { return data_.MaxWordCount(); }
      bool IsIdeographic() const { return data_.IsIdeographic(); }
    private:
      LexrepData data_;
    };


    inline State LexrepFunctions::Failure(State s) const {
      return data_.FailureTable()[s];
    }

    inline const Match* LexrepFunctions::OutputBegin(State s) const {
      SmallCount count = data_.MatchCount()[s];
      if (!count) return 0;
      SmallOffset idx = data_.MatchSet()[s];
      return &(data_.Matches()[idx]);
    }
    
    inline const Match* LexrepFunctions::OutputEnd(State s) const {
      SmallCount count = data_.MatchCount()[s];
      if (!count) return 0;
      SmallOffset idx = data_.MatchSet()[s];
      return &(data_.Matches()[idx+count]); 
    }

    /// Goto implementation
    //GScores represent the desirability of a match.
    //We rate by number of character transitions used,
    //then by number of regex transitions used.
    //TRW: Linguists have requested static score (priority)
    //values for regexes. TODO by adding a priorty value
    //to the output Match object
    class GScore {
    public:

      GScore() : char_transitions_(0), regex_transitions_(0) {}

      GScore& AddChar() {
	++char_transitions_;
	return *this;
      }
      GScore& AddRegex() {
	++regex_transitions_;
	return *this;
      }
      bool operator<(const GScore& other) const {
	//char transitions are prioritized.
	if (char_transitions_ != other.char_transitions_) {
	  return char_transitions_ < other.char_transitions_;
	}
	return regex_transitions_ < other.regex_transitions_;
      }
      bool operator>(const GScore& other) const {
	return other < *this;
      }
    private:
      size_t char_transitions_;
      size_t regex_transitions_;
    };

    //The state of an exploration of the state machine.
    class GState {
    public:
      GState(State state, const Char* begin) : state_(state), begin_(begin) {}
      State GetState() const { return state_; }
      void SetState(State state) { state_ = state; }
      const Char* GetBegin() const { return begin_; }
      void IncBegin() { ++begin_; }
      void SetBegin(const Char* begin) { begin_ = begin; }
      GScore GetScore() const { return score_; }
      void AddRegexScore() {
	score_.AddRegex();
      }
      void AddCharScore() {
	score_.AddChar();
      }
    private:
      State state_;
      const Char* begin_;
      GScore score_;
    };

    //The stack of work to do
    class GStack {
    public:
      void PushState(const GState& state) {
	stack_.push_back(state);
      }
      GState PopState() {
	GState state = stack_.back();
	stack_.pop_back();
	return state;
      }
      bool empty() const {
	return stack_.empty();
      }
      void clear() {
	stack_.clear();
      }
    private:
      typedef std::vector<GState> StateVector;
      StateVector stack_;
    };

    static inline State MatchRegex(const Regex& regex, const Char*& begin, const Char* end, State success, State failure) {
      const Char* match = regex.Match(begin, end);
      if (match == begin) {
	//Didn't match
	return failure;
      }
      //Adjust begin to cover what it "ate"
      begin = match;
      //Return the success state
      return success;
    }

    static inline void ProcessCharacter(const LexrepFunctions& f, GStack& stack, GState state) {
      State s = state.GetState();
      State t;
      Char c = *state.GetBegin();
      while ((t = f.Goto(s, c)) == kFailState) {
        if (!s) return; //if we're failing from state 0 there's nothing to be done.
        s = f.Failure(s);
        if (!s) return; //if failstate is startstate 0 then word does not exist (can only happen in Ideographical languages)
      }
      s = t;
      //If we go back to 0, ignore.
      if (s) {
        state.SetState(s);
        state.IncBegin();
        state.AddCharScore();
        stack.PushState(state);
      } 
    }

    static inline void ProcessRegexMatch(GStack& stack, GState state, State t, const Char* begin) {
      state.SetState(t);
      state.SetBegin(begin);
      state.AddRegexScore();
      stack.PushState(state);
    }

    inline size_t LexrepFunctions::RegexCount(State s) const {
      if (!data_.HasRegex()) return 0;
      SmallOffset offset = data_.RegexOneStateMap()[s];
      //No regex transitions
      if (offset == 0) return 0;
      //One regex transition
      if (offset != kNullOffset) return 1;
      //Multiple regex transitions
      size_t range = data_.RegexNextStateMap()[s];
      size_t count = range >> 20;
      return count;
    }

    inline const Regex& LexrepFunctions::GetSoleRegex(State s) const {
      return data_.RegexMap()[data_.RegexOneStateMap()[s] - 1];
    }

    inline State LexrepFunctions::GetSoleRegexNextState(State s) const {
      return data_.RegexNextStateMap()[s];
    }

    inline const Regex& LexrepFunctions::GetRegex(LexrepFunctions::RegexIndex idx) const {
      return data_.RegexMap()[idx];
    }

    inline const LexrepFunctions::RegexIndex* LexrepFunctions::RegexBegin(State s) const {
      size_t range = data_.RegexNextStateMap()[s];
      size_t offset = range & 03777777;
      return &(data_.RegexOffsetMap()[offset]);
    }

    inline const SmallState* LexrepFunctions::RegexGotoBegin(State s) const {
      size_t range = data_.RegexNextStateMap()[s];
      size_t offset = range & 03777777;
      return &(data_.RegexStateMap()[offset]);
    }

    static inline void ProcessRegexes(const LexrepFunctions& f, GStack& stack, GState state, const Char* end) {
      State s = state.GetState();
      size_t count = f.RegexCount(s);

      //No possible regexes to match
      if (!count) return;

      //One possible regex to match
      if (count == 1) {
	const Char* begin = state.GetBegin();
	State t = MatchRegex(f.GetSoleRegex(s), begin, end, f.GetSoleRegexNextState(s), kFailState);
	if (t == kFailState) return;
	ProcessRegexMatch(stack, state, t, begin);
	return;
      }

      //Multiple possible regexes to match
      const LexrepFunctions::RegexIndex* r = f.RegexBegin(s);
      const SmallState* ns = f.RegexGotoBegin(s);
      for (size_t i = 0; i < count; ++i, ++r, ++ns) {
	const Char* begin = state.GetBegin();
	State t = MatchRegex(f.GetRegex(*r), begin, end, *ns, kFailState);
	if (t != kFailState) {
	  ProcessRegexMatch(stack, state, t, begin);
	}
      }
    }

    static inline GState ProcessStack(const LexrepFunctions& f, GStack& stack, GState output, const Char* end) {
      while (!stack.empty()) {
	GState state = stack.PopState();
	if (state.GetBegin() == end) {
	  //A "final" state. Compare score to current output.
	  if (state.GetScore() > output.GetScore()) {
	    output = state;
	  }
	  continue; //Don't extend output states.
	}
	ProcessCharacter(f, stack, state);
	ProcessRegexes(f, stack, state, end);
      }
      return output;
    }

    //Goto either eats the word and returns the best scoring 
    //subsequent state or returns state 0 (if in 0 to begin with) 
    //or kFailState to indicate failure.
    inline State LexrepFunctions::Goto(State s, const Char* begin, const Char* end) const {
      static GStack stack;
      stack.clear();
      stack.PushState(GState(s, begin));
      State failState = s ? kFailState : 0;
      //GState(failState, begin) is the output if there are no better results:
      //Don't consider anything processed, and let the caller use Failure() to backtrack
      GState final = ProcessStack(*this, stack, GState(failState, begin), end);
      return final.GetState();
    }

    inline State LexrepFunctions::Goto(State s, Char c) const {
      //Failing from state 0 should take us back to state 0.
      const State failState = s == 0 ? 0 : kFailState;
      const Symbol* OneStateMapP = data_.OneStateMap();
      const SmallState* NextStateMapP = data_.NextStateMap();
      const Symbol* SymbolMapP = data_.SymbolMap();
      const SmallState* StateMapP = data_.StateMap();

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
      Symbol sym = OneStateMapP[s];
      //If the symbol is 0, there are no exit transitions
      if (sym == 0) return failState;
      //If the symbol is not -1, check that the exit transition matches the character
      //we have. If so, return the corresponding next state.
      if (sym != (Symbol)-1) return sym == c ? NextStateMapP[s] : failState;
      //If the symbol is -1, unpack the value in NextStateMap into a count of
      //transitions and their offset in the transition list.
      size_t range = NextStateMapP[s];
      size_t count = range >> 20; //2^12 = 4096 maximum transitions for a single state
      size_t offset = range & 03777777; //2^20 = A million possible transitions
      //The location is an offset into the SymbolMap and StateMap tables.
      //We'll binary search the (sorted) range in the SymbolMap table to find
      //the offset for a matching transition, if any.
      const Symbol* begin(&SymbolMapP[offset]);
      const Symbol* end = begin + count;
      const Symbol* lower_bound = std::lower_bound(begin, end, c);
      if (lower_bound == end) return failState;
      if (*lower_bound == c) return StateMapP[offset + (lower_bound - begin)];
      return failState;
    }

  }
}

#endif //IKNOW_AHO_LEXREP_LEXREPFUNCTIONS_H_
