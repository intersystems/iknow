#include "StaticModel.h"
namespace iknow {
  namespace model {
    namespace MODELNS {

      static const Symbol ali_one_state_map[] = {
#include "ali/OneStateMap.inl"
      };

      const Symbol* ALIOneStateMap() { return ali_one_state_map; }
    }
  }
}
