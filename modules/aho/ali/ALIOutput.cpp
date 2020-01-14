#include "StaticModel.h"
namespace iknow {
  namespace model {
    namespace MODELNS {

      static const Score ali_output[] = {
#include "ali/Output.inl"
      };

      const Score* ALIOutput() { return ali_output; }
    }
  }
}
