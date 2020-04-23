#ifndef IKNOW_MODEL_EN_SCANNER_H_
#define IKNOW_MODEL_EN_SCANNER_H_
#include "IkTypes.h"
#include "IkLabel.h"
#include "IkLexrep.h"
#include "IkKnowledgebase.h"
#include <algorithm>
#include <vector>
#include "AhoCorasick.h"
#include "PoolAllocator.h"

//FOR DEBUGGING - REMOVE
#include <iostream>
#include <fstream>

namespace iknow {
  namespace shell {

    template<typename IterT>
    iknow::core::IkLexrep JoinLexreps(IterT begin, IterT end, const iknow::base::String& separator) {
      using iknow::base::String;
      using iknow::base::Char;
      using iknow::core::IkLexrep;
      static String merged_entity(64, static_cast<Char>(0));
      merged_entity.replace(0, merged_entity.size(), 0, static_cast<Char>(0));
      
      for(IterT i = begin; i < end; ++i) {
        if (!(i->GetNormalizedValue()).empty()) {
          merged_entity += i->GetNormalizedValue();
          merged_entity += separator;
        }
      }
      if (!merged_entity.empty()) merged_entity.erase(merged_entity.size() - separator.size()); //delete last space.
	  IterT first = begin; // first lexrep for joining
	  const Char* literal_begin = first->GetTextPointerBegin(); // literal_first can be NULL if it starts with a punctuation: ", he said ,"
	  while (literal_begin == NULL && first != (end-1)) {
		  ++first;
		  literal_begin = first->GetTextPointerBegin();
	  }
	  IterT last = end - 1; // last lexrep for joining
	  const Char* literal_end = last->GetTextPointerEnd();
	  while (literal_end == NULL && last != begin) { // literal_end can be NULL if the last symbol has been isolated : "in other words ,"
		  --last;
		  literal_end = last->GetTextPointerEnd();
	  }
	  return IkLexrep((end-1)->GetLexrepType(), (end-1)->GetKnowledgebase(), literal_begin, literal_end, merged_entity);
    }

    class MatchAccepter {};
    template<typename InputIter, typename OutIter>
    void AcceptMatch(const iknow::model::Match& match, const iknow::core::IkKnowledgebase* kb, InputIter begin, InputIter end, OutIter& out, const iknow::base::String& separator) {
      using iknow::base::String;
      using iknow::base::Char;
      using iknow::core::IkLexrep;
      using iknow::core::IkLabel;
      using iknow::core::IkKnowledgebase;
      using iknow::core::JoinLabel;
      using iknow::core::FastLabelSet;
      using iknow::core::PooledLabelVector;
      using std::count;
	  
	  // collect existing attribute labels
	  std::set<FastLabelSet::Index> existing_attribute_labels;
	  for (InputIter itLexrep = begin; itLexrep != end; ++itLexrep) {
		  for (size_t j = 0; j < itLexrep->NumberOfLabels(); ++j) {
			  if (kb->GetLabelTypeAtIndex(itLexrep->GetLabelIndexAt(j)) == IkLabel::Attribute) existing_attribute_labels.insert(itLexrep->GetLabelIndexAt(j)); // add attribute label
		  }
	  }
      size_t word_count = end - begin;
      if (word_count == 1) {
		IkLexrep output(*(begin));
		output.SetMetaData(match.meta_text); // !new, set pointer to metatext
		output.ClearAllLabels();
		for (size_t i=0; i < match.label_count; ++i) {
			output.AddLabelIndex(match.label_id[i]);
		}
		output.AddLabelIndices(existing_attribute_labels.begin(), existing_attribute_labels.end()); // add existing attribute labels
		*out = output;
		++out;
      }
      else {
	//How many "-" separated label segments are there?
#ifdef SOLARIS
	size_t label_segment_count = 0;
	//Have to use the stupid by reference returning "count"
	//because that's all the old RogueWave STL on Sun supports.
	count(match.label_id,
	      match.label_id + match.label_count,
	      IkLabel::BreakIndex(), label_segment_count);
	++label_segment_count; //one more than the # of breaks
#else //!SOLARIS
	size_t label_segment_count = 1 + count(match.label_id,
					       match.label_id + match.label_count,
					       IkLabel::BreakIndex());
#endif
	//Just one? Merge the lexrep literals and return a single lexrep.
	if (label_segment_count == 1) {
	  IkLexrep output = JoinLexreps(begin, end, separator);
	  output.SetMetaData(match.meta_text); // !new, set pointer to metatext
	  for (size_t i=0; i < match.label_count; ++i) {
	    output.AddLabelIndex(match.label_id[i]);
	  }
	  output.AddLabelIndices(existing_attribute_labels.begin(), existing_attribute_labels.end()); // add existing attribute labels
	  *out = output;
	  ++out;
	}
	else {
	  FastLabelSet::Index break_index = IkLabel::BreakIndex();
	  FastLabelSet::Index join_index = kb->GetLabelIndex(JoinLabel);
	  const FastLabelSet::Index* cur_label = match.label_id;
	  const FastLabelSet::Index* const last_label = cur_label + match.label_count;
	  InputIter lexrep_begin = begin;
	  bool bIsKatakana = iknow::base::IkStringAlg::IsKatakana(lexrep_begin->GetNormalizedValue()[0]);
	  InputIter lexrep_end = begin + 1;
	  if (bIsKatakana) { while (lexrep_end != end && iknow::base::IkStringAlg::IsKatakana(lexrep_end->GetNormalizedValue()[0])) ++lexrep_end; }  // join Katakana symbols
	  PooledLabelVector labels;
	  labels.reserve(match.label_count);
	  for (;cur_label != last_label; ++cur_label) {
	    if (*cur_label == break_index) {
	      IkLexrep output = JoinLexreps(lexrep_begin, lexrep_end, separator);
		  output.SetMetaData(match.meta_text); // !new, set pointer to metatext

		  // collect existing attribute labels in segment
		  std::set<FastLabelSet::Index> segment_attribute_labels;
		  for (InputIter itSegmentLexrep = lexrep_begin; itSegmentLexrep != lexrep_end; ++itSegmentLexrep) {
			  for (size_t j = 0; j < itSegmentLexrep->NumberOfLabels(); ++j) {
				  if (kb->GetLabelTypeAtIndex(itSegmentLexrep->GetLabelIndexAt(j)) == IkLabel::Attribute) segment_attribute_labels.insert(itSegmentLexrep->GetLabelIndexAt(j)); // add attribute label
			  }
		  }
	      output.AddLabelIndices(labels.begin(), labels.end());
		  output.AddLabelIndices(segment_attribute_labels.begin(), segment_attribute_labels.end()); // add existing attribute labels

	      *out = output;
	      ++out;
	      lexrep_begin = lexrep_end;
		  bIsKatakana = iknow::base::IkStringAlg::IsKatakana(lexrep_begin->GetNormalizedValue()[0]);
	      lexrep_end = lexrep_begin + 1;
		  if (bIsKatakana) { while (lexrep_end != end && iknow::base::IkStringAlg::IsKatakana(lexrep_end->GetNormalizedValue()[0])) ++lexrep_end; }
	      labels.clear();
	    }
	    else if (*cur_label == join_index) {
		  if (lexrep_end == end) {
			IkLexrep scanner_match = JoinLexreps(begin, end, separator);
			std::string scanner_match_string = iknow::base::IkStringEncoding::BaseToUTF8(scanner_match.GetNormalizedValue());
			std::string error_message = std::string("Tried to join a non-existent lexrep in (") + scanner_match_string + std::string(") lexrep match.");
			throw ExceptionFrom<MatchAccepter>(error_message.c_str());
		  }
		  bIsKatakana = iknow::base::IkStringAlg::IsKatakana(lexrep_end->GetNormalizedValue()[0]);
	      ++lexrep_end;
		  if (bIsKatakana) { while (lexrep_end != end && iknow::base::IkStringAlg::IsKatakana(lexrep_end->GetNormalizedValue()[0])) ++lexrep_end; }  // join Katakana symbols
	    }
	    else {
	      labels.push_back(*cur_label);
	    }
	  }
		if (!labels.empty()) { // Output remaining labels on rest of lexrep
			IkLexrep output = JoinLexreps(lexrep_begin, end, separator);
			output.SetMetaData(match.meta_text);  // !new, set pointer to metatext
			output.AddLabelIndices(labels.begin(), labels.end());

			std::set<FastLabelSet::Index> last_segment_attribute_labels; // collect existing attribute labels in last segment
			for (InputIter itLastSegmentLexrep = lexrep_begin; itLastSegmentLexrep != end; ++itLastSegmentLexrep) {
				for (size_t jj = 0; jj < itLastSegmentLexrep->NumberOfLabels(); ++jj) {
					if (kb->GetLabelTypeAtIndex(itLastSegmentLexrep->GetLabelIndexAt(jj)) == IkLabel::Attribute) last_segment_attribute_labels.insert(itLastSegmentLexrep->GetLabelIndexAt(jj)); // add attribute label
				}
			}
			output.AddLabelIndices(last_segment_attribute_labels.begin(), last_segment_attribute_labels.end()); // add existing attribute labels

			*out = output;
			++out;
		}
	}
      }
    }

    class Scanner {
    public:
      template<typename LexrepIterT, typename MatchIterT, typename ModelT>
		void MatchInPhase(const ModelT* model, LexrepIterT lexrep_begin, LexrepIterT lexrep_end, MatchIterT matches_begin) {
			using namespace iknow::model;
			using iknow::base::String;
			using iknow::model::Match;

			State s = 0;
			bool is_ideographic = model->IsIdeographic();
			if (!is_ideographic) s = model->GotoChar(s, ' ');

			// The reset state is the state we're in when ready to match actual content
			// from a cold start. For space-separated languages it's the state from 0 with a ' ' transition.
			const State reset_state = s;
			for (LexrepIterT in_lexrep = lexrep_begin; in_lexrep != lexrep_end; ++in_lexrep) {
				const String& token = in_lexrep->GetNormalizedValue();
				// Iterate through all the characters, plus a space before looking for a match.
				State orig_s = s;
				s = model->GotoWord(s, token.data(), token.data() + token.size());
				// If we fail to match a word, that means we're not continuing an existing match, and need to
				// check as if this is a new word from the "reset state".
				if ((s == 0 || s == kFailState) && orig_s != reset_state) s = model->GotoWord(reset_state, token.data(), token.data() + token.size());

				if (s == 0 || s == kFailState) { // Still failing? Reset and go to the next word
					s = reset_state;
					continue;
				}
				if (!is_ideographic) { // Once more for the space, TODO: Eliminate duplication
					State t;
					while ((t = model->GotoChar(s, ' ')) == kFailState) {
						s = model->Failure(s);
					}
					s = t;
				}
				if (s) {
					size_t match_count = model->OutputCount(s);
					if (!match_count) continue;
					const Match* match = model->OutputBegin(s);
					for (size_t i = 0; i < match_count; ++i,++match) {
						if (in_lexrep - lexrep_begin < match->word_count - 1) {
							throw ExceptionFrom<Scanner>("Match word count exceeds number of words read.");
						}
						const size_t match_begin = (in_lexrep - lexrep_begin) - (match->word_count - 1);
						const size_t match_end = match_begin + match->word_count;
						// Can we include this match? Must be no match ongoing (symbolized by the null Match). 
						// That means either there's no match beginning at the target index, or there's a previous match that begins there.
						const Match** matches = &(*matches_begin);
						if (!matches[match_begin] || !matches[match_begin]->IsNull()) {
							matches[match_begin] = match; //Note the match
							for (size_t j = match_begin + 1; j < match_end; ++j) { //Extend it with nulls
								matches[j] = Match::NullMatch();
							}
						}
					}
				}
			}
      } 

      //Scans the matches corresponding to the lexreps looking for null, non-matches. Contiguous sequences of these are then
      //checked for matches in the given phase. (Ignoring phases for now!)
      template<typename LexrepIterT, typename MatchIterT, typename ModelT>
      void FindMatchesForPhase(const ModelT& model, LexrepIterT lexrep_begin, LexrepIterT lexrep_end, MatchIterT match_begin) {
		while (lexrep_begin != lexrep_end) {
			while (lexrep_begin != lexrep_end && *match_begin) { // Advance to next null match
				lexrep_begin++, match_begin++;
			}
			MatchIterT match_range_begin = match_begin;
			LexrepIterT lexrep_range_begin = lexrep_begin;
			// Advance past end of null match range (conveniently, the "begin"  outer loop iterators will
			// be the range "end" iterators and all set for the next loop)
			while (lexrep_begin != lexrep_end && !*match_begin) {
				lexrep_begin++, match_begin++;
			}
			MatchInPhase(model, lexrep_range_begin, lexrep_begin, match_range_begin);  
		}
      }

      //Identifies known lexrep tokens in this language model.
      //A new implementation that uses lexrep input and output iterators to identify all lexreps at once.
      //TODO: too long, nested loops, refactor!
      template<typename InputIter, typename OutputIter, typename ModelIterT>
      void Identify(InputIter begin, const InputIter end, OutputIter out, ModelIterT models_begin, ModelIterT models_end, const iknow::core::IkKnowledgebase* kb) {
		using namespace iknow::model;
		using iknow::model::Match;
		using iknow::base::String;

		//TODO: Hack. Ideographic could be per model
		bool is_ideographic = (*models_begin)->IsIdeographic(); 

		typedef std::vector<const Match*, iknow::base::PoolAllocator<const Match*> > Matches;
		Matches matches;
		matches.resize(end - begin,0); //at most one match per input value
		for (ModelIterT model = models_begin; model != models_end; ++model) {
			FindMatchesForPhase(*model, begin, end, matches.begin());
		}
		// Output all matches
		InputIter in_lexrep = begin;
		for (Matches::const_iterator match = matches.begin(); match != matches.end(); /* iterate in loop */) {
			if (!*match) {
				*out = *in_lexrep; // copy the original
				++out;
				++in_lexrep;
				++match;
			} else {
				AcceptMatch(**match, kb, in_lexrep, in_lexrep + (*match)->word_count, out, is_ideographic ? iknow::base::String() : iknow::base::SpaceString() );
				in_lexrep += (*match)->word_count;
				match += (*match)->word_count;
			}
		}
      }
    };
  }
}



#endif //IKNOW_MODEL_EN_SCANNER_H_
