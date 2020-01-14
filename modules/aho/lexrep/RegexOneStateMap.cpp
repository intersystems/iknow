#include "StaticModel.h"
namespace iknow {
  namespace model {
    namespace MODELNS {

      static const SmallOffset regex_one_state_map[] = {
#include "lexrep/RegexOneStateMap.inl"
      };

      const SmallOffset* RegexOneStateMap() { return regex_one_state_map; }
    }
  }
}
