#include "IkMetadataCache.h"
#include "IkKnowledgebase.h"
#include "IkTypes.h"
#include <string>

using iknow::base::String;
using iknow::base::Char;
using iknow::core::IkMetadataCache;
using iknow::core::IkKnowledgebase;
using iknow::core::ChainPattern;
using iknow::core::ScanDirection;
using iknow::core::kCRCPattern;
using iknow::core::kCCRPattern;
using iknow::core::MetadataValue;
using iknow::core::kConceptsToMergeMax;
using iknow::core::kChainPattern;
using iknow::core::kModifiersOnLeft;
using iknow::core::kIsJapanese;
using iknow::core::kHasEntityVectors;
using iknow::core::kEntityVectorScanDirection;
using iknow::core::PathConstruction;
using iknow::core::kCRCSequence;
using iknow::core::kPathRelevant;
using iknow::core::FuriganaHandling;

template<>
size_t IkMetadataCache::ConvertValue<size_t>(const String& val) {
  if (val.empty()) return 0;
  String::const_iterator i = val.begin();
  int sign = 1;
  if (*i == '-') {
    ++i;
    sign = -1;
  }
  size_t result = 0;
  for (; i != val.end(); ++i) {
    //must be a digit
    if (*i < '0' || *i > '9') return 0;
    result *= 10;
    result += *i - '0';
  }
  return result * sign;
}

template<>
ChainPattern IkMetadataCache::ConvertValue<ChainPattern>(const String& val) {
  static const Char ccr_array[] = { 'C', 'C', 'R', '\0' };
  static const String ccr_string(ccr_array);
  if (val == ccr_string) {
    return kCCRPattern;
  }
  return IkMetadataCache::DefaultValue<kChainPattern>();
}

template<>
ScanDirection IkMetadataCache::ConvertValue<ScanDirection>(const String& val) {
  static const Char rl_array[] = { 'R', 'L', '\0' };
  static const String rl_string(rl_array);
  if (val == rl_string) {
    return kRightToLeft;
  }
  return kLeftToRight;
}

template<>
PathConstruction IkMetadataCache::ConvertValue<PathConstruction>(const String& val) {
	static const Char pr_array[] = { 'P', 'R', '\0' };
	static const String pr_string(pr_array);
	if (val == pr_string) {
		return kPathRelevant;
	}
	return kCRCSequence; // Default value
}

template<>
FuriganaHandling IkMetadataCache::ConvertValue<FuriganaHandling>(const String& val) {
    static const Char off_array[] = { 'o', 'f', 'f', '\0' };
    static const String off_string(off_array);
    if (val == off_string) {
        return kFuriOff;
    }
    return kFuriDefault; // Default value
}
template<>
bool IkMetadataCache::ConvertValue<bool>(const String& val) {
  if (val.empty()) return true; // default is modifiers on left
  static const Char CFalse='0'; // zero is false indicator
  return (val[0]!=CFalse); // non-zero means true
}

template<>
String IkMetadataCache::ConvertValue<String>(const String& val) {
  return String(val);
}

//This needs to be here in the implementation to avoid a circular dependency on IkKnowledgebase.
String IkMetadataCache::ReadStringValue(const IkKnowledgebase& kb, const std::string& key) {
  return kb.GetMetadataString(key);
}

IkMetadataCache::IkMetadataCache(const IkKnowledgebase& kb) {
  LoadValue<kConceptsToMergeMax>(kb);
  LoadValue<kNonRelevantsToMergeMax>(kb);
  LoadValue<kChainPattern>(kb);
  LoadValue<kModifiersOnLeft>(kb);
  LoadValue<kIsJapanese>(kb);
  LoadValue<kRuleBasedPaths>(kb);
  LoadValue<kHasEntityVectors>(kb);
  LoadValue<kEntityVectorScanDirection>(kb);
  LoadValue<kLanguageCode>(kb);
  LoadValue<kP1>(kb);
  LoadValue<kP2>(kb);
  LoadValue<kP3>(kb);
  LoadValue<kP4>(kb);
  LoadValue<kPScale>(kb);
  LoadValue<kPathConstruction>(kb);
  LoadValue<kRegexSplitter>(kb);
  LoadValue<kFuriganaHandling>(kb);
}

