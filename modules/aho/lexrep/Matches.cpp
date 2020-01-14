#include "StaticModel.h"
namespace iknow {
  namespace model {
    namespace MODELNS {

#include "lexrep/MatchObjs.inl"
      static const Match matches[] = {
#include "lexrep/Matches.inl"
      };

      const Match* Matches() { return matches; }
    }
  }
}
