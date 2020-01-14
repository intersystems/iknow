#ifndef IKNOW_CORE_KB_H_
#define IKNOW_CORE_KB_H_
#include "IkTypes.h"

namespace iknow {
  namespace core {
    class Kb {
      KbLabel* ConceptLabel() const = 0;
      KbRule* RulesBegin() const = 0;
      KbRule* RulesEnd() const = 0;
      void ApplyPreprocessFilter(iknow::base::String& str) const = 0;
      void ApplyConceptFilter(iknow::base::String& str) const = 0;
    };
  }
}

#endif //IKNOW_CORE_KB_H_
