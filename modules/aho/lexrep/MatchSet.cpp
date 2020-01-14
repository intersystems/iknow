#include "StaticModel.h"
namespace iknow {
  namespace model {
    namespace MODELNS {

      static const SmallOffset match_set[] = {
#include "lexrep/MatchSet.inl"
      };

      const SmallOffset* MatchSet() { return match_set; }
    }
  }
}
