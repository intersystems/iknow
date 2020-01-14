#ifndef IKNOW_CORE_IKRULE_H_
#define IKNOW_CORE_IKRULE_H_

#include "IkExportCore.h"
#include "IkRuleInputPattern.h"
#include "IkRuleOutputPattern.h"
#include "IkLabel.h"

namespace iknow
{
  namespace core
  {

    class CORE_API IkRule
    {
    public:
      IkRule(const IkRuleInputPattern* input_begin, const IkRuleInputPattern* input_end,
	     const IkRuleOutputPattern* output_begin, const IkRuleOutputPattern* output_end, Phase phase) :
	input_begin_(input_begin), input_end_(input_end),
	output_begin_(output_begin), output_end_(output_end), phase_(phase) {}
     
      //Empty constructor for rule cache in IkIndexProcess
      IkRule() {}
 
      const IkRuleInputPattern* InputBegin() const { return input_begin_; }
      const IkRuleInputPattern* InputEnd() const { return input_end_; }
      const IkRuleOutputPattern* OutputBegin() const { return output_begin_; }
      const IkRuleOutputPattern* OutputEnd() const { return output_end_; }

      Phase GetPhase() const { return phase_; }

    private:
      const IkRuleInputPattern* input_begin_;
      const IkRuleInputPattern* input_end_;
      const IkRuleOutputPattern* output_begin_;
      const IkRuleOutputPattern* output_end_;
      Phase phase_;
    };
  }
}

#endif // !defined(AFX_IKRULE_H__83E0CC55_8B86_4372_8428_A46C31B391AF__INCLUDED_)

