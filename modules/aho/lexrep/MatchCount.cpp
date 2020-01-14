#include "StaticModel.h"
namespace iknow {
  namespace model {
    namespace MODELNS {

      static const SmallCount match_count[] = {
#include "lexrep/MatchCount.inl"
      };

      const SmallCount* MatchesCount() { return match_count; }
    }
  }
}
