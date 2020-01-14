#ifndef _IKNOW_ALI_LANGUAGEBASE_H
#define _IKNOW_ALI_LANGUAGEBASE_H
#include "aliExports.h"
#include "IkTypes.h"

namespace iknow {
  namespace ali {

    // container for language specific info
    class ALI_API LanguageBase {
    public:
      virtual ~LanguageBase();
      //Score the sentence with length n: How similar is it to this language?
      virtual double GetSimilarity(const iknow::base::Char* sentence, size_t n);
    protected:
      //The minimum and maximum cluster sizes for this language base.
      virtual size_t MinClusterSize() const = 0;
      virtual size_t MaxClusterSize() const = 0;
      //Score the cluster at c with length n
      virtual size_t ScoreFor(const iknow::base::Char* c, size_t n) const = 0;
      //Total score for all clusters in this language base.
      virtual size_t TotalScore() const = 0;
    public:
      //Identify the next cluster char for matching
      static iknow::base::Char NextClusterChar(const iknow::base::Char*& current_char,
						const iknow::base::Char* end,
						iknow::base::Char last_cluster_char);
    };
  }
}
#endif //_IKNOW_ALI_LANGUAGEBASE_H
