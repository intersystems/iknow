#ifndef IKNOW_SHELL_KBATTRIBUTE_H_
#define IKNOW_SHELL_KBATTRIBUTE_H_
#include "IkTypes.h"
#include "RawBlock.h"
#include "IkStringEncoding.h"
#include "IkStringAlg.h"
#include "utlExceptionFrom.h"
#include "OffsetPtr.h"
#include "KbAttributeMap.h"
#include <string>
#include <vector>

namespace iknow {
  namespace shell {

    class KbAttribute;

    struct AttributeInserter {
      AttributeInserter(std::vector<AttributeId>& attributes, AttributeMapBuilder& attribute_map) : attributes_(attributes), attribute_map_(attribute_map) {}
      void operator()(const iknow::base::String& attribute) {
        iknow::base::String normalized_attribute(attribute);
        iknow::base::IkStringAlg::Trim(normalized_attribute);
        attributes_.push_back(attribute_map_.Insert(normalized_attribute));
      }
      std::vector<AttributeId>& attributes_;
      AttributeMapBuilder& attribute_map_;
    private:
      void operator=(const AttributeInserter&);
    };

    class KbAttribute {
    public:
      KbAttribute(RawAllocator& allocator, const std::string& value, AttributeMapBuilder& attribute_map) {
        //Values look like: foo(1,2,3,RS,+)
        //foo is the name attribute, the arguments are parameters
        const size_t name_begin = 0;
        const size_t name_end = value.find('(');
        if (name_end + 1 >= value.size()) throw ExceptionFrom<KbAttribute>("Attribute name not found.");
        const size_t params_begin = name_end + 1;
        const size_t params_end = value.find(')', params_begin);
        if (params_end >= value.size()) throw ExceptionFrom<KbAttribute>("Attribute parameters not found.");
        const iknow::base::String name(iknow::base::IkStringEncoding::UTF8ToBase(value.substr(name_begin, name_end - name_begin)));
        const iknow::base::String params(iknow::base::IkStringEncoding::UTF8ToBase(value.substr(params_begin, params_end - params_begin)));
        iknow::base::String normalized_name = name;
        iknow::base::IkStringAlg::Trim(normalized_name);
        type_ = attribute_map.Insert(normalized_name);
        std::vector<AttributeId> parameters;
        parameters.reserve(4);
        AttributeInserter inserter(parameters, attribute_map);
        iknow::base::IkStringAlg::Tokenize(params, ',', inserter);
        begin_parameters_ = allocator.InsertRange(parameters.begin(), parameters.end());
        end_parameters_ = begin_parameters_ + parameters.size();
      }
      KbAttribute(const KbAttribute& other) { // need explicit copy constructor for parameter copying, avoiding random bytes due to type alignment
          this->type_ = other.type_;
          this->begin_parameters_ = other.begin_parameters_;
          this->end_parameters_ = other.end_parameters_;
      }
      const AttributeId* GetType() const {
          return &type_;
      }
      const AttributeId* GetParametersBegin() const {
          return begin_parameters_;
      }
      const AttributeId* GetParametersEnd() const {
          return end_parameters_;
      }
    private:
      AttributeId type_;
      OffsetPtr<const AttributeId> begin_parameters_;
      OffsetPtr<const AttributeId> end_parameters_;
    };
  }
}


#endif //IKNOW_SHELL_KBATTRIBUTE_H_
