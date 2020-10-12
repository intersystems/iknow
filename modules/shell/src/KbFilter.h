#ifndef IKNOW_SHELL_KBFILTER_H_
#define IKNOW_SHELL_KBFILTER_H_
#include "IkTypes.h"
#include "RawBlock.h"
#include "OffsetPtr.h"

namespace iknow {
  namespace shell {
    class KbFilter {
    public:
      typedef enum {
        _concept, _relation, _nonrelevant, _pathrelevant
      } Flt_Type ;

      KbFilter(RawAllocator& allocator, const std::string& input_token, const std::string& output_token, bool is_applied_only_at_beginning, bool is_applied_only_at_end, bool is_concept_f, bool is_relation_f) :
        input_token_(allocator.InsertString(iknow::base::IkStringEncoding::UTF8ToBase(input_token))),
        output_token_(allocator.InsertString(iknow::base::IkStringEncoding::UTF8ToBase(output_token))),
        is_applied_only_at_beginning_(is_applied_only_at_beginning),
        is_applied_only_at_end_(is_applied_only_at_end),
		// we use a hack for pathrelevant, if both concept and relation are true, it flags pathrelevant !
        type_(is_concept_f && is_relation_f ? _pathrelevant : (is_concept_f ? _concept : (is_relation_f ? _relation : _nonrelevant))) {}

      KbFilter(const KbFilter& other) { // explicit copy constructor
         this->input_token_ = other.input_token_;
         this->output_token_ = other.output_token_;
         this->is_applied_only_at_beginning_ = other.is_applied_only_at_beginning_;
         this->is_applied_only_at_end_ = other.is_applied_only_at_end_;
         this->type_ = other.type_;
      }
      iknow::base::String InputToken() const { return iknow::base::String(*input_token_); }
      const CountedBaseString* PointerToInputToken() const { return input_token_; }
      iknow::base::String OutputToken() const { return iknow::base::String(*output_token_); }
      const CountedBaseString* PointerToOutputToken() const { return output_token_; }
      bool IsAppliedOnlyAtBeginning() const { return is_applied_only_at_beginning_; }
      bool IsAppliedOnlyAtEnd() const { return is_applied_only_at_end_; }
      bool Apply(iknow::base::String& str, int filt_type, bool first, bool last) const {
        if (filt_type != type_) // not the right kind of filter
          return false;
        bool was_applied = false;
        switch (PositionCode()) {
        case 0:
          was_applied = Filter(str);
          break;
        case 1:
          was_applied = first && Filter(str);
          break;
        case 2:
          was_applied = last && Filter(str);
          break;
        case 3:
          was_applied = (first && last) && Filter(str); // "3" means being first and last simultaneously.
          break;
        }
        if (was_applied) iknow::base::IkStringAlg::Trim(str);
        return was_applied;
      }
    private:
      int PositionCode() const {
        int code = 0;
        if (IsAppliedOnlyAtBeginning()) code += 1;
        if (IsAppliedOnlyAtEnd()) code += 2;
        return code;
      }
      bool Filter(iknow::base::String& str) const {
	    size_t nPos = is_applied_only_at_end_ ? str.rfind(PointerToInputToken()->data(), str.length()-1, PointerToInputToken()->size) : str.find(PointerToInputToken()->data(), 0, PointerToInputToken()->size);
        bool bReplace=false;
        if (nPos != iknow::base::String::npos) { // detected
          if (is_applied_only_at_beginning_) bReplace=(nPos==0); // must be at beginning
          else {
            if (is_applied_only_at_end_) bReplace=(nPos+PointerToInputToken()->size==str.length()); // must be at end
            else bReplace=true; // no conditions
          }
        }
        if (bReplace) {
          size_t src_size=PointerToInputToken()->size;
          size_t dest_size=PointerToOutputToken()->size;
          if (dest_size==0) { // check for double space problem
            if (nPos>0 && (nPos+src_size)<str.length()) {
              if (str[nPos-1]==' ' && str[nPos+src_size]==' ') src_size++; // double space at word boundary, remove
            }
          }
          str.replace(nPos, src_size, PointerToOutputToken()->data(), dest_size);
        }
        return bReplace;
      }
      OffsetPtr<const CountedBaseString> input_token_;
      OffsetPtr<const CountedBaseString> output_token_;
      bool is_applied_only_at_beginning_;
      bool is_applied_only_at_end_;
      Flt_Type type_;
    };
  }
}


#endif //IKNOW_SHELL_KBFILTER_H_
