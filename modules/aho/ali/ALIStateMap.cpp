#include "StaticModel.h"
namespace iknow {
  namespace model {
    namespace MODELNS {

      static const SmallState ali_state_map[] = {
#include "ali/StateMap.inl"
      };

      const SmallState* ALIStateMap() { return ali_state_map; }
    }
  }
}
