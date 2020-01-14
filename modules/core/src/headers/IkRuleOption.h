#ifndef IKNOW_CORE_IKRULEOPTION_H_
#define IKNOW_CORE_IKRULEOPTION_H_
#include "IkTypes.h"

namespace iknow {
  namespace core {
    //Defines a set of options associated with a rule element (intitially only one
    //of the output values).
    //N.B.: In a "clean" implementation, this would be a pure virtual class
    //implemented by the KB implementation. But rather than force all those vcalls,
    //as long as it remains a "POD" it can be stored directly in the shared memory KB.
    class IkRuleOption {
    public:
      enum Option {
        kClearAll = 0x1,
        kClearInputLabels = 0x2,
        kClearAllPhases = 0x4, // when using "Join", we need to clear all labels, even on different phases
		kNOP = 0x8 // No OPeration, do not touch
      };
      //The set of options
      typedef char Options;
      explicit IkRuleOption(Options options) : options_(options) {}
      IkRuleOption() : options_(0) {}
      //By default, a rule match clears all labels in the match and applies
      //the output label.
      static IkRuleOption GetDefaultRuleOptions() {
        return IkRuleOption(static_cast<Options>(kClearAll));
      }
      static IkRuleOption GetAdditionRuleOptions() {
        return IkRuleOption(static_cast<Options>(0));
      }
      static IkRuleOption GetReplacementRuleOptions() {
        return IkRuleOption(static_cast<Options>(kClearInputLabels));
      }
	  static IkRuleOption GetJoinRuleOptions() {
		  return IkRuleOption(static_cast<Options>(kClearAllPhases));
	  }
	  static IkRuleOption GetNOPOptions() {
		  return IkRuleOption(static_cast<Options>(kNOP));
	  }
      bool HasOption(Option option) const {
        return (options_ & option) != 0;
      }
      void SetOption(Option option) {
        options_ |= option;
      }
      bool operator==(const IkRuleOption& other) const {
        return other.options_ == options_;
      }
      iknow::base::String ToString() const {
        iknow::base::String out;
		if (*this == GetNOPOptions()) { out += '*'; return out; }
        if (*this == GetDefaultRuleOptions()) return out;
		if (*this == GetJoinRuleOptions()) return out;
        if (*this == GetAdditionRuleOptions()) { out += '+'; return out; }
        if (*this == GetReplacementRuleOptions()) { out += '^'; return out; }
        out += '?'; return out;
      }

    private:
      Options options_;
    };
  }
}

#endif //IKNOW_CORE_IKRULEOPTION_H_
