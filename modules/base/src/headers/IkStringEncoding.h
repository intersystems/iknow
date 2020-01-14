#ifndef IKNOW_BASE_IKSTRINGENCODING_
#define IKNOW_BASE_IKSTRINGENCODING_
#include "IkExportBase.h"
#include <string>
#include "IkTypes.h"

/* An IkStringEncoding represents an encoding scheme (e.g. UTF-8) that can be passed to
   the various string normalization functions.
   We don't use a string argument to those functions because building an encoder/decoder object
   for a particular scheme in, e.g., ICU, can be expensive. The construction and destruction of that object
   is encapsulated in this class.
*/
namespace iknow {
  namespace base {
    //forward declare implementation class
    //(we don't want to expose the specific transcoding implementation we're using)
    class IkStringEncodingImpl;

    class BASE_API IkStringEncoding {
    public:
      //Create an encoding given a name. Throws IkException if cannot be created.
      IkStringEncoding(const std::string& encoding_name);

      //Do two encodings represent the same underlying encoding
      bool operator==(const IkStringEncoding& other) const;

      //Returns the minimum and maximum character size for this encoding in bytes. This is useful to
      //distinguish at runtime encodings that should be widened or narrowed on conversion, and for
      //buffer sizing.
      size_t MinCharSize() const;
      size_t MaxCharSize() const;

      //takes a best guess as to the encoding of a given sample string, returns an empty string if
      //can't guess. An optional pointer to a second string will set it to the language value, if detected.
      static std::string GuessEncoding(const std::string& sample, std::string *language_guess = NULL);

      //Converts from this encoding to another. Throws IkException if conversion fails.
      template<typename StringT>
      StringT TranscodeTo(const IkStringEncoding& new_encoding, const StringT& input) const;

      template<typename StringInT, typename StringOutT>
      //convert with widening or narrowing
      StringOutT TranscodeAndConvertTo(const IkStringEncoding& new_encoding, const StringInT& input) const;

      static const IkStringEncoding PlatformWStringEncoding; 
      static const IkStringEncoding CacheWStringEncoding;
      static const IkStringEncoding BaseStringEncoding;
      static const IkStringEncoding UTF8StringEncoding;

      static iknow::base::String UTF8ToBase(const std::string& input);
      static std::string BaseToUTF8(const iknow::base::String& input);

    private:
      IkStringEncodingImpl* impl_;
    };

  }
}

#endif //IKNOW_BASE_IKSTRINGENCODING_
