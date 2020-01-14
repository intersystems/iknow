#include "StaticModel.h"
namespace iknow {
  namespace model {
    namespace MODELNS {

      static const SmallState state_map[] = {
#include "lexrep/StateMap.inl"
      };

      const SmallState* StateMap() { return state_map; }
    }
  }
}
