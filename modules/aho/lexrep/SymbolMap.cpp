#include "StaticModel.h"
namespace iknow {
  namespace model {
    namespace MODELNS {

      static const Symbol symbol_map[] = {
#include "lexrep/SymbolMap.inl"
      };

      const Symbol* SymbolMap() { return symbol_map; }
    }
  }
}
