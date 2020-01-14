#include "StaticModel.h"
namespace iknow {
  namespace model {
    namespace MODELNS {

      static const Symbol one_state_map[] = {
#include "lexrep/OneStateMap.inl"
      };

      const Symbol* OneStateMap() { return one_state_map; }
    }
  }
}
