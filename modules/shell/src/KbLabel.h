#ifndef IKNOW_SHELL_KBLABEL_H_
#define IKNOW_SHELL_KBLABEL_H_
#include "IkTypes.h"
#include "IkLabel.h"
#include "RawBlock.h"
#include "IkStringEncoding.h"
#include "IkStringAlg.h"
#include "utlExceptionFrom.h"
#include "OffsetPtr.h"
#include "KbAttribute.h"
#include <string>
#include <vector>

namespace iknow {
  namespace shell {

    class KbLabel;

    typedef std::basic_string<iknow::core::FastLabelSet::Index> IndexPattern;
    typedef CountedString<iknow::core::FastLabelSet::Index> CountedIndexString;
    typedef iknow::core::Phase Phase;

    //MapT maps labels names ("base" 2-byte encoding) to indexes 
    template<typename MapT>
    struct AddIndexForLabelNameToString {
      AddIndexForLabelNameToString(const MapT& label_map, IndexPattern& string) : label_map_(label_map), string_(string) {}
      void operator()(const std::string& utf8_label_name) {
		if (utf8_label_name.empty()) return; //ignore empty labels
		if (utf8_label_name == "-") {
			string_ += iknow::core::IkLabel::BreakIndex(); // "-" is a special break
			//TODO: Confirm that number of tokens matches number of label segments.
		} else {
			typename MapT::const_iterator i = label_map_.find(iknow::base::IkStringEncoding::UTF8ToBase(utf8_label_name));
			if (i == label_map_.end())
				throw ExceptionFrom<AddIndexForLabelNameToString>(std::string("Unknown label: \"") + utf8_label_name + std::string("\" while loading lexreps."));
			string_ += i->second;
		}
      }
      const MapT& label_map_;
      IndexPattern& string_;
    private:
    void operator=(const AddIndexForLabelNameToString&);
    };

    struct FullAttributeInserter {
      FullAttributeInserter(RawAllocator& allocator, std::vector<KbAttribute>& attributes, AttributeMapBuilder& attribute_map) :
	allocator_(allocator), attributes_(attributes), attribute_map_(attribute_map) {}
      void operator()(const std::string& attribute) {
	attributes_.push_back(KbAttribute(allocator_, attribute, attribute_map_));
      }
      RawAllocator& allocator_;
      std::vector<KbAttribute>& attributes_;
      AttributeMapBuilder& attribute_map_;
    private:
      void operator=(const FullAttributeInserter&);
    };

    struct PhaseInserter {
      PhaseInserter(std::vector<Phase>& phases) : phases_(phases) {}
      void operator()(const std::string& phase) {
	Phase phase_value = iknow::core::PhaseFromString(phase);
	if (phase_value > iknow::core::kMaxPhase)
	  throw ExceptionFrom<KbLabel>("Illegal phase number encountered in label.");
	phases_.push_back(phase_value);
      }
      std::vector<Phase>& phases_;
    private:
      void operator=(const PhaseInserter&);
    };

    class KbLabel {
    public:
      KbLabel(RawAllocator& allocator,
	      const std::string& name,
	      const std::string& type_string,
	      const std::string& attributes,
	      const std::string& phases,
	      AttributeMapBuilder& attribute_map) :
      name_(allocator.InsertString(iknow::base::IkStringEncoding::UTF8ToBase(name))),
      type_(iknow::core::IkLabel::TypeStringToType(type_string)) {
        std::vector<KbAttribute> attribute_vector;
        FullAttributeInserter attribute_inserter(allocator, attribute_vector, attribute_map);
        iknow::base::IkStringAlg::Tokenize(attributes, '|', attribute_inserter);
        attributes_begin_ = allocator.InsertRange(attribute_vector.begin(), attribute_vector.end());
        attributes_end_ = attributes_begin_ + attribute_vector.size();
        std::vector<Phase> phase_vector;
        PhaseInserter phase_inserter(phase_vector);
        iknow::base::IkStringAlg::Tokenize(phases != "" ? phases : iknow::core::kDefaultPhases,
					   ',', phase_inserter);
        phases_begin_ = allocator.InsertRange(phase_vector.begin(), phase_vector.end());
        phases_end_ = phases_begin_ + phase_vector.size();	
      }
      KbLabel(const KbLabel& other) { // explicit copy constructor to avoid random bytes (due to type alignment).
          this->name_ = other.name_;
          this->type_ = other.type_;
          this->attributes_begin_ = other.attributes_begin_;
          this->attributes_end_ = other.attributes_end_;
          this->phases_begin_ = other.phases_begin_;
          this->phases_end_ = other.phases_end_;
      }
      iknow::base::String Name() const { return *name_; }
      const CountedBaseString* PointerToName() const { return name_; }
      iknow::core::IkLabel::Type Type() const { return type_; }
      size_t AttributeCount() const { return attributes_end_ - attributes_begin_; }
      const KbAttribute* GetAttribute(size_t position) const {
        if (position > AttributeCount()) throw ExceptionFrom<KbLabel>("Illegal attribute position.");
        return attributes_begin_ + position;
      }
      size_t PhaseCount() const { return phases_end_ - phases_begin_; }
      const Phase* GetPhasesBegin() const { return phases_begin_; }
      const Phase* GetPhasesEnd() const { return phases_end_; }

    private:
      OffsetPtr<const CountedBaseString> name_;
      iknow::core::IkLabel::Type type_;
      OffsetPtr<const KbAttribute> attributes_begin_;
      OffsetPtr<const KbAttribute> attributes_end_;
      OffsetPtr<const Phase> phases_begin_;
      OffsetPtr<const Phase> phases_end_;
    };
  }
}


#endif //IKNOW_SHELL_KBLABEL_H_
