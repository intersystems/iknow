#ifndef IKNOW_SHELL_KBREGEX_H_
#define IKNOW_SHELL_KBREGEX_H_
#include "RawBlock.h"
#include "IkTypes.h"
#include "IkStringEncoding.h"

namespace iknow {
  namespace shell {
    
    class KbRegex {
    public:
      KbRegex(RawAllocator& allocator, const std::string& name, const std::string& pattern) :
	name_(allocator.InsertString(iknow::base::IkStringEncoding::UTF8ToBase(name))),
	pattern_(allocator.InsertString(iknow::base::IkStringEncoding::UTF8ToBase(pattern))) {
      }

      iknow::base::String Name() const {
	return iknow::base::String(*name_);
      }
      
      const CountedBaseString* PointerToName() const {
	return name_;
      }
      
      iknow::base::String Pattern() const {
	return iknow::base::String(*pattern_);
      }
      
      const CountedBaseString* PointerToPattern() const {
	return pattern_;
      }

    private:
      OffsetPtr<const CountedBaseString> name_;
      OffsetPtr<const CountedBaseString> pattern_;
    };
  }
}
#endif //IKNOW_SHELL_KBREGEX_H_
