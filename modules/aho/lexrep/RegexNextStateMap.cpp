#include "StaticModel.h"
namespace iknow {
  namespace model {
    namespace MODELNS {

      static const SmallState regex_next_state_map[] = {
#include "lexrep/RegexNextStateMap.inl"
      };

      const SmallState* RegexNextStateMap() { return regex_next_state_map; }
    }
  }
}
