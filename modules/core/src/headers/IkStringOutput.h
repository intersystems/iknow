#ifndef IKNOW_CORE_IKSTRINGOUTPUT_H_
#define IKNOW_CORE_IKSTRINGOUTPUT_H_
#include <string>
#include "IkTypes.h"
#include "IkExportCore.h"


namespace iknow {
  namespace core {
    class CORE_API EndOutput {};
    static const EndOutput End = EndOutput();
    //A client-provided way to output iknow::base::Strings
    class CORE_API IkStringOutput {
    public:
      virtual IkStringOutput& operator<<(const iknow::base::String&) = 0;
      //for automatic conversion
      virtual IkStringOutput& operator<<(const std::string&);
      //End of record
      virtual IkStringOutput& operator<<(const EndOutput&) = 0;
    };
  }
}

#endif //IKNOW_CORE_IKSTRINGOUTPUT_H_
