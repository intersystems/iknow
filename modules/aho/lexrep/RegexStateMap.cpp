#include "StaticModel.h"
namespace iknow {
  namespace model {
    namespace MODELNS {

      static const SmallState regex_state_map[] = {
#include "lexrep/RegexStateMap.inl"
      };

      const SmallState* RegexStateMap() { return regex_state_map; }
    }
  }
}
