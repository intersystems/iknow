#include "StaticModel.h"
namespace iknow {
  namespace model {
    namespace MODELNS {

      static const SmallState ali_next_state_map[] = {
#include "ali/NextStateMap.inl"
      };

      const SmallState* ALINextStateMap() { return ali_next_state_map; }
    }
  }
}
