#include "StaticModel.h"
namespace iknow {
  namespace model {
    namespace MODELNS {

      static const SmallState next_state_map[] = {
#include "lexrep/NextStateMap.inl"
      };

      const SmallState* NextStateMap() { return next_state_map; }
    }
  }
}
