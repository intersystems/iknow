#ifndef IKNOW_AHO_STATICMODEL_H_
#define IKNOW_AHO_STATICMODEL_H_
#include "AhoTypes.h"
#include "ModelId.h"
namespace iknow {
  namespace model {
    namespace MODELNS {
      const Symbol* OneStateMap();
      const SmallState* NextStateMap();
      const Symbol* SymbolMap();
      const SmallState* StateMap();
      const Regex* RegexMap();
      const SmallOffset* RegexOneStateMap();
      const SmallState* RegexNextStateMap();
      const SmallOffset* RegexOffsetMap();
      const SmallState* RegexStateMap();
      const SmallState* FailureTable();
      const Match* Matches();
      const SmallOffset* MatchSet();
      const SmallCount* MatchesCount();
      const SmallState* ALIFailureTable();
      const Symbol* ALIOneStateMap();
      const SmallState* ALINextStateMap();
      const Symbol* ALISymbolMap();
      const SmallState* ALIStateMap();
      const Score* ALIOutput();
      bool HasRegex();
      size_t MaxWordCount();
      bool IsIdeographic();
    }
  }
}

#endif //IKNOW_AHO_STATICMODEL_H_

