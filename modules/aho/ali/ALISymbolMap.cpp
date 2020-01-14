#include "StaticModel.h"
namespace iknow {
  namespace model {
    namespace MODELNS {

      static const Symbol ali_symbol_map[] = {
#include "ali/SymbolMap.inl"
      };

      const Symbol* ALISymbolMap() { return ali_symbol_map; }
    }
  }
}
