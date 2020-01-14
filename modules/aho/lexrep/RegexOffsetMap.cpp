#include "StaticModel.h"
namespace iknow {
  namespace model {
    namespace MODELNS {

      static const SmallOffset regex_offset_map[] = {
#include "lexrep/RegexOffsetMap.inl"
      };

      const SmallOffset* RegexOffsetMap() { return regex_offset_map; }
    }
  }
}
