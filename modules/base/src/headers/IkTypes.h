#ifndef IKNOW_BASE_IKTYPES_H_
#define IKNOW_BASE_IKTYPES_H_
#include "unicode/utypes.h"
#include "SafeString.h"
#include "IkExportBase.h"

namespace iknow {
  namespace base {
    typedef UChar Char;
    typedef std::basic_string<Char> String;
    inline const String& SpaceString() {
      static const Char space[] = {' ','\0'};
      static const String Space(space);
      return Space;
    }

    //An "intrusive" string type that does not
    //manage its own storage.
    class IString {
    public:
      IString(const Char* begin, const Char* end) : begin_(begin), end_(end) {}
      IString(const Char* begin, size_t size) : begin_(begin), end_(begin + size) {}
      explicit IString(const String& string) : begin_(string.data()), end_(string.data() + string.size()) {}
      size_t size() const { return end_ - begin_; }
      const Char* begin() const {
	return begin_;
      }
      const Char* end() const {
	return end_;
      }
    private:
      const Char* begin_;
      const Char* end_;
    };
  }
}
#endif //IKNOW_BASE_IKTYPES_H_

