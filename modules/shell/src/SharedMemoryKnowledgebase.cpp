#include "SharedMemoryKnowledgebase.h"
#include "Scanner.h"
#include "IkStringEncoding.h"
#include "utlExceptionFrom.h"
#include "MethodIterator.h"
#include "IkMetadataCache.h"
#include <algorithm>
#include <iterator>
#include <functional>
#include <string>

#include <vector>
#include <map>
#include <utility>

using namespace iknow::base;
using namespace iknow::core;
using namespace iknow::shell;
using namespace iknow::shell::StaticHash;
using std::string;
using std::for_each;
using std::count;
using std::vector;
using std::pair;
using std::map;
using std::back_inserter;
using std::transform;

SharedMemoryKnowledgebase::SharedMemoryKnowledgebase(RawKBData* kb_data) : kb_data_(kb_data) {}
SharedMemoryKnowledgebase::SharedMemoryKnowledgebase(unsigned char* kb_data) : kb_data_(reinterpret_cast<RawKBData*>(kb_data)) {}

void SharedMemoryKnowledgebase::FilterInput(iknow::base::String& input) const {
    OFFSETPTRGUARD;
    const KbInputFilter* const begin = kb_data_->input_filters_begin;
    const KbInputFilter* const end = kb_data_->input_filters_end;
    for (const KbInputFilter* i = begin; i != end; ++i) {
        i->Apply(input);
    }
}

bool SharedMemoryKnowledgebase::LabelSingleToken(IkLexrep& lexrep, const String& label_name) const {
    OFFSETPTRGUARD;
    const size_t* idxLabel = kb_data_->labels->Lookup(label_name);
    if (idxLabel) {
        lexrep.AddLabelIndex(static_cast<FastLabelSet::Index>(*idxLabel));
        return true;
    }
    else // label name does not exist.
        return false;
}

bool SharedMemoryKnowledgebase::LabelSingleToken(IkLexrep& lexrep) const {
    OFFSETPTRGUARD;
    String& token = lexrep.GetNormalizedValue();
    const KbLexrep* kb_lexrep = kb_data_->lexreps->Lookup(token.data(),
        token.data() + token.size());
    if (!kb_lexrep) return false;
    for (const FastLabelSet::Index* i = kb_lexrep->PointerToLabels()->begin();
        i != kb_lexrep->PointerToLabels()->end(); ++i) {
        lexrep.AddLabelIndex(*i);
    }
    return true;
}

//TODO: Ugly and long.
IkLexrep SharedMemoryKnowledgebase::NextLexrep(Lexreps::iterator& current, Lexreps::iterator end) const {
#ifndef WIN32
    using std::min;
#endif //WIN32

    if (!buffered_lexreps_.empty()) {
        IkLexrep lexrep = buffered_lexreps_.front();
        buffered_lexreps_.pop();
        return lexrep;
    }
    OFFSETPTRGUARD;
    size_t max_size = static_cast<size_t>(end - current) > kb_data_->max_lexrep_size ?
        kb_data_->max_lexrep_size :
        static_cast<size_t>(end - current);
    size_t max_token_size = kb_data_->max_token_size; // maximum token size, used for ideographic languages (eg. Japanese, Chinese, Korean, ...)

    String tokens;
    tokens.reserve(16 * max_size);
    Lexreps::iterator lexrep = current;
    typedef vector<size_t> TokenBoundaries;
    TokenBoundaries token_boundaries;
    token_boundaries.reserve(max_size);

    bool is_ideographic = this->GetMetadata<kIsJapanese>();

    size_t match_size_jpn = 0;
    if (is_ideographic) { // max_token_size overrules max_size
        size_t ideograph_max = std::min(max_token_size, static_cast<size_t>(end - current));
        for (size_t i = 0; i < ideograph_max; ++i) {
            String token = lexrep->GetNormalizedValue();
            tokens += token;
            match_size_jpn++; // length of JPN string
            token_boundaries.push_back(tokens.size());
            if (token.size() >= max_token_size) // this is possible if Katakana is mixed in.
                break;
            ++lexrep;
        }
    }
    else { // Normal style  
        for (size_t i = 0; i < max_size; ++i) {
            tokens += lexrep->GetNormalizedValue();
            token_boundaries.push_back(tokens.size());
            tokens += ' ';
            ++lexrep;
        }
    }
    const KbLexrep* kb_lexrep = 0;
    size_t match_size = (is_ideographic ? match_size_jpn : max_size);
    for (; match_size > 0; --match_size) {
        kb_lexrep = kb_data_->lexreps->Lookup(tokens.data(), tokens.data() + token_boundaries[match_size - 1]);
        if (kb_lexrep) break;
    }
    if (!kb_lexrep) { // Not found, return original and advance.
        return *current++;
    }

    // If only attribute labels are added, keep the other labels
    bool bAddOnlyAttributes = true; // do we only add attribute labels ?
    std::set<FastLabelSet::Index> new_attribute_labels;
    for (const FastLabelSet::Index* i = kb_lexrep->PointerToLabels()->begin(); i != kb_lexrep->PointerToLabels()->end(); ++i) {
        if (this->GetLabelTypeAtIndex(*i) != IkLabel::Attribute) {
            bAddOnlyAttributes = false;
            break;
        }
        else {
            new_attribute_labels.insert(*i);
        }
    }

    //TODO: simplify logic and remove duplication
    if (match_size == 1) {  // modify copy of existing lexrep for speed, this is a common case.
        IkLexrep output = *current++;

        if (!bAddOnlyAttributes) output.ClearAllLabels(); // don't clear labels if only attributes are added
        for (const FastLabelSet::Index* i = kb_lexrep->PointerToLabels()->begin(); i != kb_lexrep->PointerToLabels()->end(); ++i) {
            output.AddLabelIndex(*i);
        }
        if (!bAddOnlyAttributes) output.SetAnnotated(true); // safe for further lookup actions
        return output;
    }
    //How many "-" separated label segments are there?
#ifdef SOLARIS
    size_t label_segment_count = 0;
    //Have to use the stupid by reference returning "count"
    //because that's all the old RogueWave STL on Sun supports.
    count(kb_lexrep->PointerToLabels()->begin(),
        kb_lexrep->PointerToLabels()->end(),
        IkLabel::BreakIndex(), label_segment_count);
    ++label_segment_count; //one more than the # of breaks
#else //!SOLARIS
    size_t label_segment_count = 1 + count(kb_lexrep->PointerToLabels()->begin(),
        kb_lexrep->PointerToLabels()->end(),
        IkLabel::BreakIndex());
#endif

    //Just one? Merge the lexrep literals and return a single lexrep.
    if (label_segment_count == 1) {
        if (!bAddOnlyAttributes) { // normal case
            IkLexrep output = JoinLexreps(current, current + match_size, is_ideographic ? iknow::base::String() : iknow::base::SpaceString());
            for (const FastLabelSet::Index* i = kb_lexrep->PointerToLabels()->begin(); i != kb_lexrep->PointerToLabels()->end(); ++i) { // add new labels
                output.AddLabelIndex(*i);
            }
            current = current + match_size; // advance to next
            output.SetAnnotated(true); // no more lexrep lookups, except when only adding attributes.
            return output;
        }
        else { // only adding attribute labels
            while (match_size != 0) {
                IkLexrep output = *current++;
                for (const FastLabelSet::Index* i = kb_lexrep->PointerToLabels()->begin(); i != kb_lexrep->PointerToLabels()->end(); ++i) { // add attribute labels
                    output.AddLabelIndex(*i);
                }
                buffered_lexreps_.push(output); // store in buffer
                --match_size;
            }
            IkLexrep output = buffered_lexreps_.front();
            buffered_lexreps_.pop();
            return output;
        }
    }
    else {
        FastLabelSet::Index break_index = IkLabel::BreakIndex();
        FastLabelSet::Index join_index = this->GetLabelIndex(JoinLabel);
        Lexreps::const_iterator end_match = current + match_size;
        const FastLabelSet::Index* cur_label = kb_lexrep->PointerToLabels()->begin();
        const FastLabelSet::Index* const last_label = kb_lexrep->PointerToLabels()->end();
        Lexreps::const_iterator lexrep_begin = current;
        bool bIsKatakana = iknow::base::IkStringAlg::IsKatakana(lexrep_begin->GetNormalizedValue()[0]);
        Lexreps::const_iterator lexrep_end = current + 1;
        if (bIsKatakana) { while (lexrep_end != end && iknow::base::IkStringAlg::IsKatakana(lexrep_end->GetNormalizedValue()[0])) ++lexrep_end; }  // join Katakana symbols
        std::vector<FastLabelSet::Index> labels;
        labels.reserve(last_label - cur_label);
        for (; cur_label != last_label; ++cur_label) {
            if (*cur_label == break_index) {
                IkLexrep output = JoinLexreps(lexrep_begin, lexrep_end, is_ideographic ? iknow::base::String() : iknow::base::SpaceString());
                output.AddLabelIndices(labels.begin(), labels.end());
                output.SetAnnotated(true); // no more lexrep lookup.
                buffered_lexreps_.push(output);
                lexrep_begin = lexrep_end;
                bIsKatakana = iknow::base::IkStringAlg::IsKatakana(lexrep_begin->GetNormalizedValue()[0]);
                lexrep_end = lexrep_begin + 1;
                if (bIsKatakana) { while (lexrep_end != end && iknow::base::IkStringAlg::IsKatakana(lexrep_end->GetNormalizedValue()[0])) ++lexrep_end; }  // join Katakana symbols
                labels.clear();
            }
            else if (*cur_label == join_index) {
                if (lexrep_end == end_match)
                    throw ExceptionFrom<SharedMemoryKnowledgebase>("Tried to join a non-existent lexrep.");
                bIsKatakana = iknow::base::IkStringAlg::IsKatakana(lexrep_end->GetNormalizedValue()[0]);
                ++lexrep_end;
                if (bIsKatakana) { while (lexrep_end != end && iknow::base::IkStringAlg::IsKatakana(lexrep_end->GetNormalizedValue()[0])) ++lexrep_end; }  // join Katakana symbols
            }
            else {
                labels.push_back(*cur_label); // collect the label
            }
        }
        //Output remaining labels on rest of lexrep
        if (!labels.empty()) {
            IkLexrep output = JoinLexreps(lexrep_begin, end_match, is_ideographic ? iknow::base::String() : iknow::base::SpaceString());
            output.AddLabelIndices(labels.begin(), labels.end());
            output.SetAnnotated(true); // no more lexrep lookup
            buffered_lexreps_.push(output); // last one
        }
        //Could call NextLexrep recursively here, but let's not get cute.
        //(Would need conditional inlining to get good performance).
        IkLexrep output = buffered_lexreps_.front();
        buffered_lexreps_.pop();

        current = current + match_size; // advance to next
        return output;
    }
}

