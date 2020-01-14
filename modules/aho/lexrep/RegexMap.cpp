#include "StaticModel.h"
namespace iknow {
  namespace model {
    namespace MODELNS {

#include "lexrep/RegexObjs.inl"

      static const Regex regex_map[] = {
#include "lexrep/RegexMap.inl"
      };

      const Regex* RegexMap() { return regex_map; }
    }
  }
}
