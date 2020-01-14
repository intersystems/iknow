#ifndef IKNOW_AHO_AHOTYPES_H_
#define IKNOW_AHO_AHOTYPES_H_
#include <cstring>
#include "IkTypes.h"
#include "IkLabel.h"
#include "ModelRegex.h"

namespace iknow {
  namespace model {
    typedef size_t State;
    static const State kFailState = static_cast<State>(-1);
    typedef unsigned short TokenLength;
    typedef iknow::base::Char Char;
    typedef Char Symbol;
    static const Symbol kNullSymbol = static_cast<Symbol>(-1);
    typedef iknow::base::String String;
    typedef unsigned short WordCount;
    static const WordCount kNullWordCount = static_cast<WordCount>(-1);
    typedef uint8_t SmallCount;
    typedef SmallCount SmallWordCount;
    typedef unsigned short MatchCount;
    typedef unsigned short SmallOffset;
    static const SmallOffset kNullOffset = static_cast<SmallOffset>(-1);
    typedef uint32_t SmallState;
    typedef unsigned short LabelCount; /* TODO: Derive from FastLabelSet */
    typedef iknow::core::FastLabelSet::Index LabelId;
    typedef uint32_t Score;
    typedef int DataValue;

    typedef size_t PhaseNum;

    static const PhaseNum kPhaseBase = 0;
    static const PhaseNum kPhaseRegex = 1;
    static const PhaseNum kPhaseBegin = 0;
    static const PhaseNum kPhaseEnd = 2;

    struct Match {
      Match() : label_id(NULL), label_count(0), word_count(0), meta_text(NULL) {}
      Match(const LabelId* id, LabelCount labels, WordCount words, const char* meta=NULL) : label_id(id), label_count(labels), word_count(words), meta_text(meta) {}

      const LabelId* label_id;
      LabelCount label_count;
      WordCount word_count;
	  const char* meta_text;
      
      static const Match* NullMatch() { // Null matches are used to demarcate already-matched lexreps
		static const Match Null(NULL, 0, kNullWordCount);
		return &Null;
      }
      bool IsNull() const { return word_count == kNullWordCount; }
    };
  }
}
#endif //IKNOW_AHO_AHOTYPES_H_
