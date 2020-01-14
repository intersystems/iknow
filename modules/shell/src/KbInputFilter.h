#ifndef IKNOW_SHELL_KBINPUTFILTER_H_
#define IKNOW_SHELL_KBINPUTFILTER_H_

namespace iknow {
  namespace shell {
    //Input filters operate identically to preprocess filters
    class KbInputFilter : public KbPreprocessFilter {
    public:
      KbInputFilter(RawAllocator& allocator, const std::string& input_token, const std::string& output_token) :
	KbPreprocessFilter(allocator, input_token, output_token) {}
    };
  }
}

#endif //IKNOW_SHELL_KBINPUTFILTER_H_
