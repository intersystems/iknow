//ICU and our sysCommon.h both want to define a "UChar" type.
//So I hacked sysCommon.h to not define that type if this symbol is set
//TODO, TRW: Convince Jamie to call this type something else.
#define NO_UCHAR_TYPE
#include "IkStringEncoding.h"
#include "unicode/ucnv.h"
#include "unicode/ucsdet.h"
#include "utlExceptionFrom.h"
#include <map>
#include <list>
#include "IkTypes.h"

#ifdef __GNUC__
#ifndef MACOSX 
//For Linux,
//see http://gcc.gnu.org/bugzilla/show_bug.cgi?id=31368
//BUT: Since Mac uses --enable-fully-dynamic-string, this definition
//will cause cross-library deallocation issues there.
//See http://gcc.gnu.org/bugzilla/show_bug.cgi?id=24196
template class std::basic_string<unsigned short>;
#endif //!MACOSX
#endif //__GNUC__

using namespace iknow::base;

class iknow::base::IkStringEncodingImpl {
public:
  static IkStringEncodingImpl* GetEncodingFor(const std::string& encoding_name);
  template<typename inputT, typename outputT>
  outputT ConvertTo(const IkStringEncodingImpl& new_encoding, const inputT& input) const;
  static std::string GuessEncoding(const std::string& sample, std::string *language = NULL);
  size_t MinCharSize() const;
  size_t MaxCharSize() const;
private:
  IkStringEncodingImpl(const std::string& encoding_name);
  ~IkStringEncodingImpl();

  typedef std::map<std::string, IkStringEncodingImpl*> EncodingMap;
  static EncodingMap encodings_;

  UConverter *converter_;
};


IkStringEncodingImpl::EncodingMap IkStringEncodingImpl::encodings_;

size_t IkStringEncodingImpl::MinCharSize() const {
  return ucnv_getMinCharSize(converter_);
}

size_t IkStringEncodingImpl::MaxCharSize() const {
  return ucnv_getMaxCharSize(converter_);
}

IkStringEncodingImpl* IkStringEncodingImpl::GetEncodingFor(const std::string& encoding_name) {
  //check the static cache first
  if (encodings_.count(encoding_name) > 0) return encodings_[encoding_name];
  
  //add it to the cache if it doesn't exist, return
  //note that once created, these encoding implementations are never destroyed.
  IkStringEncodingImpl *encoding = new IkStringEncodingImpl(encoding_name);
  encodings_.insert(EncodingMap::value_type(encoding_name, encoding));
  return encoding;
}

IkStringEncodingImpl::IkStringEncodingImpl(const std::string& encoding_name) {
  UErrorCode converr =  U_ZERO_ERROR;
  converter_ = ucnv_open(encoding_name.c_str(), &converr);
  if (U_FAILURE(converr)) {
    throw MessageExceptionFrom<IkStringEncodingImpl>("IKNoSuchEncoding", encoding_name);
  }  
}

IkStringEncodingImpl::~IkStringEncodingImpl() {
  ucnv_close(converter_);
}

template<typename inputT, typename outputT>
outputT IkStringEncodingImpl::ConvertTo(const IkStringEncodingImpl& new_encoding, const inputT& input) const {
  //For empty strings, return empty strings
  if (input.empty()) return outputT();
  //we need to allocate a buffer the size of our input + 1 for the null...
  size_t input_size = sizeof(typename inputT::value_type) * (input.size());
  //...times an expansion factor. we'll assume maximum character expansion, from the minimum codepoint size
  //of the input to the maximum codepoint size of the output, rounding up. 3 / 2 -> factor 2, for example.
  size_t min_input_size = MinCharSize();
  size_t max_output_size = new_encoding.MaxCharSize();
  size_t expansion_factor = (max_output_size / min_input_size) + (max_output_size % min_input_size ? 1 : 0);
  size_t buf_size = input_size * expansion_factor;
  //TRW: Obviously not thread-safe!
  //TODO: TRW, lower buffer high water mark after a while?
  static typename outputT::value_type *buf = 0;
  static size_t buf_max = 0;
  if (buf_size > buf_max) {
    if (buf) delete[] buf;
    buf = new typename outputT::value_type[buf_size];
    buf_max = buf_size;
  }
  typename outputT::value_type *buf_cursor = buf;
  const typename inputT::value_type *in = input.data();
  UErrorCode converr = U_ZERO_ERROR;
  ucnv_convertEx(new_encoding.converter_, converter_,
		 (char**)&buf_cursor, ((char*)buf)+(sizeof(typename outputT::value_type) * buf_size),
		 (const char**)&in, ((const char*)in) + input_size,
		 NULL, NULL, NULL, NULL, true, true, &converr);
  if (U_FAILURE(converr)) {
    throw MessageExceptionFrom<IkStringEncodingImpl>("IKFailedTranscode");
  }

  //we use the counted constructor here because we may be converting into
  //a narrower character type than the encoding format, e.g., UTF-16 in a
  //char array, and want to include the embedded nulls. The chief use
  //of this feature is currently ikHelper.cpp's BuildListVarArgs, which is told
  //the type of its argument and casts.
  //
  //the output is null terminated, we don't want to include it, so -1
  outputT output(buf, buf_cursor - buf);
  return output;
}


std::string IkStringEncodingImpl::GuessEncoding(const std::string& sample, std::string *language_guess) {
  //For RAII
  struct DetectorWrapper {
    DetectorWrapper() {
      UErrorCode error = U_ZERO_ERROR;
      detector = ucsdet_open(&error);
      if (U_FAILURE(error)) {
	throw ExceptionFrom<IkStringEncodingImpl>("Failed to create charset detector.");
      }
    }
    //implicit conversion back to the pointer
    operator UCharsetDetector* () { return detector; }
    ~DetectorWrapper() { ucsdet_close(detector); }
  UCharsetDetector *detector;
  };
  
  UErrorCode icu_err = U_ZERO_ERROR;
  DetectorWrapper detector;
  //better not try to guess anything bigger than 2GB!
  ucsdet_setText(detector, sample.c_str(), static_cast<int32_t>(sample.size()), &icu_err);
  if (U_FAILURE(icu_err)) {
    throw ExceptionFrom<IkStringEncodingImpl>("Failed to set charset detector text.");
  }
  const UCharsetMatch* charset_match = ucsdet_detect(detector, &icu_err);
  std::string encoding_guess;
  if (U_FAILURE(icu_err)) {
    //no text or no result
    encoding_guess = "";
  }
  else {
    encoding_guess = ucsdet_getName(charset_match, &icu_err);
    if (language_guess) {
      *language_guess = ucsdet_getLanguage(charset_match, &icu_err);
    } 
    if (U_FAILURE(icu_err)) encoding_guess = "";
  }
  return encoding_guess;
}

IkStringEncoding::IkStringEncoding(const std::string& encoding_name) : impl_(IkStringEncodingImpl::GetEncodingFor(encoding_name)) {
}

bool IkStringEncoding::operator==(const IkStringEncoding& other) const {
  //Since we always use the same implementation for encodings of the same name, we can
  //just compare the implementation pointer.
  //Of course, if you create two encodings using ICU "equivalent" names, they will return != here, but
  //that's nothing we can easily do anything about.
  return impl_ == other.impl_;
}

size_t IkStringEncoding::MinCharSize() const {
  return impl_->MinCharSize();
}

size_t IkStringEncoding::MaxCharSize() const {
  return impl_->MaxCharSize();
}

std::string IkStringEncoding::GuessEncoding(const std::string& sample, std::string *language) {
  return IkStringEncodingImpl::GuessEncoding(sample, language);
}

template<typename StringT>
StringT IkStringEncoding::TranscodeTo(const IkStringEncoding& new_encoding, const StringT& input) const {
  return impl_->ConvertTo<StringT, StringT>(*(new_encoding.impl_), input);
}

template<typename StringInT, typename StringOutT>
StringOutT IkStringEncoding::TranscodeAndConvertTo(const IkStringEncoding& new_encoding, const StringInT& input) const {
  return impl_->ConvertTo<StringInT, StringOutT>(*(new_encoding.impl_), input);
}

//returns an appropriate encoding for std::wstring's on this platform
static IkStringEncoding GetPlatformWStringEncoding() {
  size_t wchar_size = sizeof(wchar_t);
  std::string output_encoding;
  output_encoding.reserve(32); //plenty big
  
  if (wchar_size == 2) { 
    output_encoding = "UTF16";
  }
  else if (wchar_size == 4) {
    output_encoding = "UTF32";
  }
  else {
    throw ExceptionFrom<IkStringEncoding>("Unexpected wchar_t size.");
  }

  output_encoding += "_PlatformEndian";

  return IkStringEncoding(output_encoding);

}

//returns an appropriate encoding for Cache WStrings (two byte, platform endian)
static IkStringEncoding GetCacheWStringEncoding() {
  return IkStringEncoding("UTF16_PlatformEndian");
}

//Get these once.
const IkStringEncoding IkStringEncoding::PlatformWStringEncoding(GetPlatformWStringEncoding()); 
const IkStringEncoding IkStringEncoding::CacheWStringEncoding(GetCacheWStringEncoding());
const IkStringEncoding IkStringEncoding::BaseStringEncoding(CacheWStringEncoding);
const IkStringEncoding IkStringEncoding::UTF8StringEncoding("UTF-8");

template std::string BASE_API IkStringEncoding::TranscodeTo(const IkStringEncoding& new_encoding, const std::string& input) const;
template iknow::base::String BASE_API IkStringEncoding::TranscodeTo(const IkStringEncoding& new_encoding, const iknow::base::String& input) const;

template iknow::base::String BASE_API IkStringEncoding::TranscodeAndConvertTo<std::string, iknow::base::String>(const IkStringEncoding& new_encoding, const std::string& input) const;
template std::string BASE_API IkStringEncoding::TranscodeAndConvertTo<iknow::base::String, std::string>(const IkStringEncoding& new_encoding, const iknow::base::String& input) const;
template iknow::base::String BASE_API IkStringEncoding::TranscodeAndConvertTo<iknow::base::String, iknow::base::String>(const IkStringEncoding& new_encoding, const iknow::base::String& input) const;

#ifdef UNIX
//On Unix platforms, wstring != String, as it does on Windows
template std::wstring BASE_API IkStringEncoding::TranscodeAndConvertTo<std::string, std::wstring>(const IkStringEncoding& new_encoding, const std::string& input) const;
template std::wstring BASE_API IkStringEncoding::TranscodeAndConvertTo<iknow::base::String, std::wstring>(const IkStringEncoding& new_encoding, const iknow::base::String& input) const;
template std::string BASE_API IkStringEncoding::TranscodeAndConvertTo<std::wstring, std::string>(const IkStringEncoding& new_encoding, const std::wstring& input) const;
#endif //!UNIX

iknow::base::String IkStringEncoding::UTF8ToBase(const std::string& input) {
  return UTF8StringEncoding.TranscodeAndConvertTo<std::string, iknow::base::String>(BaseStringEncoding, input);
}

std::string IkStringEncoding::BaseToUTF8(const iknow::base::String& input) {
  return BaseStringEncoding.TranscodeAndConvertTo<iknow::base::String, std::string>(UTF8StringEncoding, input);
}
