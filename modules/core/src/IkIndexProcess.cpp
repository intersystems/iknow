// IndexProcess.cpp: implementation of the CIndexProcess class.
//
//////////////////////////////////////////////////////////////////////

#ifdef WIN32
#pragma warning (disable: 4786)
#pragma warning (disable: 4284)
#endif

#include "IkIndexProcess.h"
#include "IkStringAlg.h"
#include "IkLexrep.h"
#include "IkMergedLexrep.h"
#include "IkSentence.h"
#include "IkStringEncoding.h"
#include "IkPath.h"
#include "Reader.h"
#include <set>
#include "ali.h"
#include "utlExceptionFrom.h"
#include "IkEntityVector.h"
#include <iostream>
#include <algorithm>
#include <functional>
#include <cstdlib>
// #include <fstream>      // std::ofstream

//For recognizing whitespace
#include "unicode/uchar.h"

using namespace std;
using namespace iknow::core;
using namespace iknow::base;
using namespace iknow::ali;

///Tracing macros
#define SEMANTIC_ACTION(x) { if (m_pDebug) (m_pDebug->x); };
#define TIMER_ACTION(x) { if (m_pDebug) (m_pDebug->x); };


const size_t MAX_WORD_SIZE=150; // Maximum allowed wordsize as defined by Michael.

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////
IkIndexProcess::IkIndexProcess(std::map<String, IkKnowledgebase const *> const & languageKbMap)
:m_languageKbMap(languageKbMap), m_pDebug(0)
{
  //An index process used for normalization will have a null map
  if (!m_languageKbMap.empty()) 
    SetCurrentKnowledgebase(m_languageKbMap.begin()->second);
}

IkIndexProcess::~IkIndexProcess() {}

struct IsNotConceptVector : public std::unary_function<const size_t&, bool> {
	IsNotConceptVector(MergedLexreps& merged_lexrep_vector) : merged_lexrep_vector_(merged_lexrep_vector) {}
	bool operator()(const size_t& entity_number) const {
		IkMergedLexrep* lexrep = &(merged_lexrep_vector_[entity_number]);
		return !lexrep->IsConcept();
	}
private:
	MergedLexreps& merged_lexrep_vector_;
	void operator=(const IsNotConceptVector&);
};

void IkIndexProcess::Start(IkIndexInput* pInput, IkIndexOutput* pOut, IkIndexDebug<TraceListType>* pDebug, bool bMergeRelations, bool bBinaryMode, bool delimitedSentences, size_t max_concept_cluster_length, IkKnowledgebase* pUdct)
{
  m_pDebug = pDebug;
  pOut->SetTextPointer((pInput->GetString())->c_str()); // store pointer to input text data
  TIMER_ACTION(StartTimer());

  //We want every debug trace to start with the same lexrep ids.
  //(lexrep ids are just for debugging purposes, anyway)
  IkLexrep::ResetIdCounter();
  static const iknow::base::String SBegin(iknow::base::IkStringEncoding::UTF8ToBase("B"));
  m_begin_lr = IkLexrep(IkLabel::Nonrelevant, m_pKnowledgebase, NULL, NULL, SBegin, m_pKnowledgebase->GetLabelIndex(SentenceBeginLabel)); // SBegin lexrep
  static const iknow::base::String SEnd(iknow::base::IkStringEncoding::UTF8ToBase("E")); 
  m_end_lr = IkLexrep(IkLabel::Nonrelevant, m_pKnowledgebase, NULL, NULL, SEnd, m_pKnowledgebase->GetLabelIndex(SentenceEndLabel)); // SEnd lexrep

  pOut->SetJP(m_pKnowledgebase->GetMetadata<kIsJapanese>());
  pOut->SetRMS(m_pKnowledgebase->GetMetadata<kModifiersOnLeft>()); // language type indicator, modifiers on the left, right is most significant.

  // if no max_concept_cluster_length set, use the KB defined one.
  m_external_max_ccl = (max_concept_cluster_length != 0);
  m_max_concept_cluster_length = (m_external_max_ccl ? max_concept_cluster_length : m_pKnowledgebase->GetMetadata<kConceptsToMergeMax>()); 

  int nPosition = 0;
  int sentence_counter = 0;
  double certainty = 0;
  String kb_name(m_languageKbMap.empty()?String():m_languageKbMap.begin()->first);

  int cntWordLimit = (bBinaryMode ? 0x7FFFFFFF : MAX_SENTENCE_SIZE); // set word count limit
  Lexreps lexrep_vector;
  while ( m_pKnowledgebase->GetMetadata<kIsJapanese>()
		  ?	FindNextSentenceJP(pInput, lexrep_vector, nPosition)
		  : FindNextSentence(pInput, lexrep_vector, nPosition, cntWordLimit, delimitedSentences, kb_name, certainty, pUdct)) // split by sentences and store in a list
    {
	  SEMANTIC_ACTION(SentenceFound(kb_name, certainty, m_pKnowledgebase->GetMetadata<kLanguageCode>(), lexrep_vector, pOut->IsJP() ? String() : SpaceString()));
      if (!bBinaryMode && lexrep_vector.size() >= MAX_SENTENCE_SIZE) continue; // limit sentence length
	  if (lexrep_vector.size()==2) continue; // no text, only SBegin & SEnd

	  Lexreps identified_lexrep_vector;
	  if (pUdct) { // consult the user dictionary
		  Lexreps ud_lexrep_vector;
		  FindKnownLexreps(true, lexrep_vector, ud_lexrep_vector, pUdct); // assign user dictionary labels
		  FindKnownLexreps(false, ud_lexrep_vector, identified_lexrep_vector); // assign lexrep lookup labels
	  } else {
		  FindKnownLexreps(true, lexrep_vector, identified_lexrep_vector);
	  }
	  if (m_pKnowledgebase->GetMetadata<kIsJapanese>()) MergeKatakana(identified_lexrep_vector); // Merge Katakana symbols, tag them single concepts
      AddAttributeLabels(identified_lexrep_vector);
	  SolveAmbiguous(identified_lexrep_vector);
	  pOut->Add(IkSentence(++sentence_counter, m_pKnowledgebase, certainty)); // store the language from the KB, not the KB name
      IkSentence& sentence = pOut->LastSentence();	
      MergedLexreps& merged_lexrep_vector = sentence.GetLexreps();
      merged_lexrep_vector.reserve(identified_lexrep_vector.size());
      MergeAndConvert(identified_lexrep_vector, merged_lexrep_vector, bMergeRelations);
	  if (!m_pKnowledgebase->GetMetadata<kIsJapanese>()) EntityFilter(merged_lexrep_vector); // Japanese does not filter entities

      //Occasionally, what FindNextSentence believes is a "sentence"
      //is actually just something like a single piece of punctuation or a space, and the
      //results of MergeAndConvert/EntityFilter is an empty list. We want to skip this and move
      //on to the next "real" sentence.
      
      if (merged_lexrep_vector.empty()) {
        pOut->RemoveLastSentence();
        --sentence_counter;
        continue;
      }
      if (!m_pKnowledgebase->GetMetadata<kIsJapanese>()) { // Path & CRC generation, but not for Japanese
        Paths& path_vector = sentence.GetPaths();
        path::CRCs& crcs = sentence.GetCRCs(); 
        DetectCRCPatterns(merged_lexrep_vector, crcs, m_pKnowledgebase->GetMetadata<kChainPattern>()); // CRC language or CCR language?
		if (m_pKnowledgebase->GetMetadata<kPathConstruction>() == kCRCSequence) { // old style
			m_pKnowledgebase->GetMetadata<kRuleBasedPaths>() ? BuildPathsFromAttributes(merged_lexrep_vector, path_vector) : BuildPathsFromCRCs(merged_lexrep_vector, crcs, path_vector);
		} else { // path relevant style
			BuildPathsFromPathRelevants(merged_lexrep_vector, path_vector);
		}
      }      
      SEMANTIC_ACTION(SentenceComplete(sentence, *m_pKnowledgebase));
      if (m_pKnowledgebase->GetMetadata<kHasEntityVectors>()) { 
		IkSentence::EntityVector& eVectors = sentence.GetEntityVector();
		BuildEntityVector(merged_lexrep_vector, eVectors);  // Calculate entity vectors
        SEMANTIC_ACTION(EntityVectors(sentence));
      }
    }
	TIMER_ACTION(TraceTheTime(1)); // we'll use codes instead of names, for this is for internal use.
}

void FilterAll(iknow::base::String& word, const IkKnowledgebase* kb) {
  kb->FilterConcept(word, 0, 0);
  kb->FilterRelation(word, 0, 0);
  kb->FilterNonRelevant(word, 0, 0);
}

iknow::base::String IkIndexProcess::NormalizeText(const iknow::base::String& input, const IkKnowledgebase* kb, const IkKnowledgebase* ud, bool bLowerCase, bool bStripPunct) {
  String buffer(input), output;
  output.reserve(buffer.size());

  if (kb->GetMetadata<kIsJapanese>()) { // Follow the Japanese path
	  IkStringAlg::NormalizeDigits(buffer);
	  IkStringAlg::NormalizeWidth(buffer);
	  if (bLowerCase) IkStringAlg::ToLower(buffer);
	  output = buffer;
  }
  else { // Alphanumerical version
	  //TODO: duplicative with CProcess code.
	  if (ud) {
		  ud->FilterInput(buffer);
	  }
	  //TODO: duplicative with ::Preprocess
	  kb->FilterPreprocess(buffer);
	  IkStringAlg::Normalize(buffer, bLowerCase, bStripPunct);
	  //TODO: duplicative with FindNextSentence

	  bool prev_was_space = false;
	  String current_word; current_word.reserve(16);
	  for (String::iterator i = buffer.begin(); i != buffer.end(); ++i) {
		  if (!u_isprint(*i)) continue; // skip non printables.
		  bool cur_is_space = u_isblank(*i) > 0;
		  if (cur_is_space && !current_word.empty()) {
			  FilterAll(current_word, kb);
			  output += current_word;
			  current_word.clear();
		  }
		  if (!cur_is_space || !prev_was_space) {
			  current_word += *i;
		  }
		  prev_was_space = cur_is_space;
	  }
	  if (!current_word.empty()) {
		  FilterAll(current_word, kb);
		  output += current_word;
	  }
	  else {
		  //remove the last space
		  if (!output.empty()) output.erase(output.size() - 1);
	  }
  }
  return output;
}

static inline bool PushLexrep(Lexreps& lexreps, const IkKnowledgebase& kb, const Char* literal_begin, const Char* literal_end, const Char* entity_begin, const Char* entity_end, bool last_lexrep, FastLabelSet::Index concept_label, FastLabelSet::Index punctuation_label) {
  if (entity_end - entity_begin == 1 && IkStringAlg::IsPunctuation(*entity_begin)) { //Punctuation?
    // if literal equals entity, use it !
    size_t lit_length=(literal_end-literal_begin);
    bool bUseLiteral = (lit_length==1 && (*literal_begin == *entity_begin)) || (lit_length==2 && *literal_begin==' ' && *(literal_begin+1)==*entity_begin);

	lexreps.push_back(IkLexrep(IkLabel::Nonrelevant, &kb, literal_begin, (last_lexrep || bUseLiteral) ? literal_end : literal_begin, entity_begin, entity_end, punctuation_label));
    return last_lexrep || bUseLiteral;
  } else {
	lexreps.push_back(IkLexrep(IkLabel::Unknown, &kb, literal_begin, literal_end, entity_begin, entity_end, concept_label));
    //Yes, used the literal, if any
    return literal_begin != literal_end;
  }
}

typedef std::vector<String*, PoolAllocator<String*> > LiteralVector;
typedef std::vector<std::pair<const Char*, const Char*>, PoolAllocator<std::pair<const Char*, const Char*> > > TextRefVector;

struct TokenProcessor {
  Lexreps& result_;
  FastLabelSet::Index concept_label_;
  FastLabelSet::Index punctuation_label_;
  TextRefVector& text_refs_;
  size_t literals_used_;
  size_t max_literal_;
  IkIndexDebug<TraceListType>* m_pDebug; //specially named so SEMANTIC_ACTION macro works
  const IkKnowledgebase& kb_;

  TokenProcessor(Lexreps& result, TextRefVector& text_refs, FastLabelSet::Index concept_label, FastLabelSet::Index punctuation_label, IkIndexDebug<TraceListType>* index_debug, const IkKnowledgebase& kb) :
    result_(result),
    concept_label_(concept_label),
    punctuation_label_(punctuation_label),
    text_refs_(text_refs),
    literals_used_(0), //match literals with the normalized tokens
	max_literal_(text_refs.size()), //until we run out
    m_pDebug(index_debug),
    kb_(kb)
  {}
  void operator()(const Char* begin, const Char* end, bool bLastPiece) {    
	if (begin == end && !bLastPiece) return; // empty tokens in list don't count

    if (bLastPiece && literals_used_ < max_literal_-1) { // not all literals have been used, merge them on the current literal position
	  text_refs_[literals_used_].second = text_refs_[max_literal_-1].second; // cover the merged text
    }
	const Char* literal_begin = 0;
	const Char* literal_end = 0;
    if (literals_used_ < max_literal_) {
	  literal_begin = text_refs_[literals_used_].first;
	  literal_end = text_refs_[literals_used_].second;
    }
	if (begin == end && literals_used_) { // empty token, merge literal with previous lexrep
		result_.back().SetTextPointerEnd(literal_end); // enlarge latest literal
		++literals_used_;
	}
	else { // normal token
		bool literal_used = PushLexrep(result_, kb_, literal_begin, literal_end, begin, end, bLastPiece, concept_label_, punctuation_label_);
		if (literal_used) ++literals_used_;
		SEMANTIC_ACTION(LexrepCreated(result_.back(), kb_));
	}
  }
private:
  void operator=(const TokenProcessor&);
};

//Builds the list of literals from a preprocess filtered string.

struct LiteralVectorBuilder {
  //TODO: Why not make these pairs of Char*'s?
  TextRefVector& text_refs_;
  const Char* original_begin_;
  const Char* original_end_;
  size_t token_count_;
  LiteralVectorBuilder(TextRefVector& text_refs, const Char* original_begin, const Char* original_end) : 
	text_refs_(text_refs),
    original_begin_(original_begin),
    original_end_(original_end),
    token_count_(0)
  {}
  //We need to resplit the original based on the split that came out of the
  //preprocess filter. We can't directly use the output of the preprocess filter
  //because it will have made other changes, like rewriting quotes, that can't
  //be included in the literal.
  void operator()(const Char* token_begin, const Char* token_end, bool bLastPiece) {
    const Char* piece_begin = original_begin_;
    const Char* piece_end;
    if (token_end - token_begin > original_end_ - original_begin_) {
      piece_end = original_end_;
      original_begin_ = original_end_;
    }
    else {
      piece_end = piece_begin + (token_end - token_begin);
      if (bLastPiece) { // make sure to cover the original NBS
        piece_end = original_end_;
      }
      original_begin_ = original_begin_ + (token_end - token_begin);
    }
	text_refs_.push_back(std::make_pair(piece_begin, piece_end));
    token_count_++;
  }
private:
  void operator=(const LiteralVectorBuilder&);
};

// "-" -> "" : if after preprocess the result is empty, should be forced to nonrelevant ?
void IkIndexProcess::Preprocess(const Char* val_begin, const Char* val_end, Lexreps& lexrep_vector, std::string const *user_labels) // Apply preprocessing stuff
{
  if ((size_t)(val_end-val_begin)>MAX_WORD_SIZE) { // refuse words that exceed the maximum limit.
    //Chunk to avoid very long non-semantic strings from overflowing the size limits of the direct output format.
    const static size_t kChunkSize = 4096;
    while (val_begin != val_end) {
      const Char* chunk_end = val_begin + kChunkSize < val_end ? val_begin + kChunkSize : val_end;
      lexrep_vector.push_back(IkLexrep(IkLabel::Nonrelevant, m_pKnowledgebase, val_begin, chunk_end, val_begin, chunk_end, m_pKnowledgebase->GetLabelIndex(NonSemanticLabel)));
      SEMANTIC_ACTION(LexrepCreated(lexrep_vector.back(), *m_pKnowledgebase));
      val_begin = chunk_end;
    }
    return;
  }

  //The rules for preprocess filtering are a bit complicated.
  //Either the preprocess filter split the original token into multiple tokens,
  //or it was left as a single token. In the single token case, we want to use
  //the original string as the literal. The replacement string will be normalized
  //and used as an entity value, later often handled by EntityFilter()
  //
  //But in the multiple token case, as in "They've" -> "They 've", we want two
  //literals to assign to the two corresponding entities that will be found in the
  //normalized value. To signal to later code that might want to rebuild the sentence
  //from the literals, we prefix the second and subsequent tokens with a space, indicating
  //(somewhat ironically) that they should *not* have any space (including the prefix marker)
  //between their preceding literals and themselves.


  //It's a little hairy to convince GNU-like C++ standard library
  //implementations to repeatedly use the same static buffer.
  //"assign" throws away the current buffer and replaces it with
  //a copy (or a ref) of the assigned buffer.
  //"replace" always uses the current buffer if it's big enough.
  static String strIndex(128, '\0');
  strIndex.replace(0, strIndex.size(), val_begin, val_end - val_begin);

  m_pKnowledgebase->FilterPreprocess(strIndex);
  static String strPreprocess(128, '\0');
  strPreprocess.replace(0, strPreprocess.size(), strIndex);

  SEMANTIC_ACTION(PreprocessToken(String(val_begin, val_end), strPreprocess));

  IkStringAlg::Normalize(strIndex);
  SEMANTIC_ACTION(NormalizeToken(strPreprocess, strIndex));

  //Make this a member?
  FastLabelSet::Index conceptLabelIndex = m_pKnowledgebase->GetLabelIndex(ConceptLabel);
  FastLabelSet::Index punctuationLabelIndex = m_pKnowledgebase->GetLabelIndex(PunctuationLabel);

  if (strIndex.empty()) { // nothing left, could be garbage, could be floating punctuation symbol
	  String strLiteral(val_begin, val_end);
	  strLiteral.erase(std::remove_if(strLiteral.begin(), strLiteral.end(), u_iscntrl), strLiteral.end());
	  if (strLiteral.empty()) { // This is garbage, remove
		  SEMANTIC_ACTION(PreprocessToken(String(val_begin, val_end), IkStringEncoding::UTF8ToBase("*** Removed ***")));
	  }
	  else { // This is a floating punctuation
		  lexrep_vector.push_back(IkLexrep(IkLabel::Nonrelevant, m_pKnowledgebase, val_begin, val_end, val_begin, val_end, punctuationLabelIndex));
		  SEMANTIC_ACTION(LexrepCreated(lexrep_vector.back(), *m_pKnowledgebase));
	  }
	  return;
  }
  if (user_labels != NULL) { // keep together and apply user labels
	  lexrep_vector.push_back(IkLexrep(IkLabel::Unknown, m_pKnowledgebase, val_begin, val_end, strIndex.data(), strIndex.data() + strIndex.size(), conceptLabelIndex));
	  String Annotation = IkStringEncoding::UTF8ToBase(*user_labels);
	  m_pKnowledgebase->LabelSingleToken(lexrep_vector.back(), Annotation);
	  SEMANTIC_ACTION(LexrepCreated(lexrep_vector.back(), *m_pKnowledgebase));
	  return;
  }
  //Did preprocessing or normalization split the token?
  if (strPreprocess.find(' ') == String::npos && strIndex.find(' ') == String::npos) {
	//Easy case. No spaces. One token.
	PushLexrep(lexrep_vector,
			  *m_pKnowledgebase,
			  val_begin,
			  val_end,
			  strIndex.data(),
			  strIndex.data() + strIndex.size(),
			  true,
			  conceptLabelIndex,
			  punctuationLabelIndex);
	SEMANTIC_ACTION(LexrepCreated(lexrep_vector.back(), *m_pKnowledgebase));
    return;
  }

  // Multiple tokens, split the literal and distribute amonst entity pieces.
  TextRefVector text_refs;
  text_refs.reserve(4);

  LiteralVectorBuilder literalVectorBuilder(text_refs, val_begin, val_end);
  IkStringAlg::TokenizeWithLPFlag(strPreprocess.data(), strPreprocess.data() + strPreprocess.size(), static_cast<Char>(' '), literalVectorBuilder);  

  TokenProcessor token_processor(lexrep_vector, text_refs, conceptLabelIndex, punctuationLabelIndex, m_pDebug, *m_pKnowledgebase);
  IkStringAlg::TokenizeWithLPFlag(strIndex.data(), strIndex.data() + strIndex.size(), static_cast<Char>(' '), token_processor); // extra flag indication if last piece
}

bool IkIndexProcess::FindNextSentenceJP(IkIndexInput* pInput, Lexreps& lexrep_vector, int& nPosition)
{
    const size_t input_size = pInput->GetString()->size();
    if ((size_t)nPosition >= input_size) return false; // all done

    const Char *pData=(pInput->GetString())->c_str(); // pointer to text data
    int nBeginPos = nPosition; // marks literal start position
	lexrep_vector.clear();
	lexrep_vector.reserve(32); //Japanese needs double...

	LexrepContext::SeenLabels().Clear();
	lexrep_vector.push_back(m_begin_lr); // insert SBegin to mark beginning of sentence
	LexrepContext::SeenLabels().InsertAtIndex(m_pKnowledgebase->GetLabelIndex(SentenceBeginLabel)); // mark SBegin in lexrep context

	// int const nPositionBeginFixed = nPosition; // store start position
	while ((size_t)nPosition < input_size) {
	  Char cCur=pData[nPosition];
      if (lexrep_vector.size()==1 && (IkStringAlg::IsJpnIDSP(cCur) || cCur==' ')) { // prodlog111211: If first char is double space, ignore it, if single space, also ignore it
        nPosition++, nBeginPos++;
        continue;
	  }
	  if (u_isdigit(cCur)) { // isolate numeric text
		const Char* numeric_begin=&pData[nPosition++];
		while ((size_t)nPosition<input_size && u_isdigit(pData[nPosition])) nPosition++; // scan numeric
		const Char* numeric_end=&pData[nPosition];

		static String strIndex(128, '\0');
		strIndex.replace(0, strIndex.size(), numeric_begin, numeric_end - numeric_begin); // copy index symbol
		static String strPreprocess(128, '\0');
		strPreprocess.replace(0, strPreprocess.size(), strIndex);
		IkStringAlg::NormalizeDigits(strIndex);
		SEMANTIC_ACTION(NormalizeToken(strPreprocess, strIndex));
        lexrep_vector.push_back(IkLexrep(IkLabel::Concept, m_pKnowledgebase, pData + nBeginPos, pData + nPosition, strIndex.data(), strIndex.data() + strIndex.size(), m_pKnowledgebase->GetLabelIndex(NumericLabel))); // must be numeric
		SEMANTIC_ACTION(LexrepCreated(lexrep_vector.back(), *m_pKnowledgebase)); // send to trace info
		nBeginPos=nPosition; // advance
		continue; // next
	  }
	  if (u_isblank(cCur)) { // a blank is a separate entity in JPN
		  nPosition++;
		  Char blank[] = { cCur, '\0' };
		  String strIndex(blank); // Should we normalize the space ?
          lexrep_vector.push_back(IkLexrep(IkLabel::Unknown, m_pKnowledgebase, pData + nBeginPos, pData + nPosition, strIndex.data(), strIndex.data() + strIndex.size(), m_pKnowledgebase->GetLabelIndex(SpaceLabel)));
		  SEMANTIC_ACTION(LexrepCreated(lexrep_vector.back(), *m_pKnowledgebase)); // send to trace info
		  nBeginPos = nPosition;
		  continue; // next
	  }
	  if (u_islower(cCur) || u_isupper(cCur)) { // hack for missing isAlphaBetic: alphabetic means having case...
		  nPosition++;
		  Char alphabetic[] = { cCur, '\0' };
		  String strIndex(alphabetic), strPreprocess(alphabetic);
		  IkStringAlg::NormalizeWidth(strIndex);
		  IkStringAlg::ToLower(strIndex);
		  SEMANTIC_ACTION(NormalizeToken(strPreprocess, strIndex));
          lexrep_vector.push_back(IkLexrep(IkLabel::Unknown, m_pKnowledgebase, pData + nBeginPos, pData + nPosition, strIndex.data(), strIndex.data() + strIndex.size(), m_pKnowledgebase->GetLabelIndex(AlphaBeticLabel)));
		  SEMANTIC_ACTION(LexrepCreated(lexrep_vector.back(), *m_pKnowledgebase)); // send to trace info
		  nBeginPos = nPosition;
		  continue; // next
	  }
	  if (IkStringAlg::IsKatakana(cCur)) { // handle Katakana
		  if (IkStringAlg::IsJPnakaguro(cCur)) { // bullet point, make it nonrelevant
			  const Char* dot_begin = &pData[nPosition++];
			  static String strIndex(2, '\0'); // size is 1
			  strIndex.replace(0, strIndex.size(), dot_begin, 1);
			  static String strPreprocess(2, '\0');
			  strPreprocess.replace(0, strPreprocess.size(), strIndex);
			  IkStringAlg::NormalizeWidth(strIndex);
			  SEMANTIC_ACTION(NormalizeToken(strPreprocess, strIndex));
              lexrep_vector.push_back(IkLexrep(IkLabel::Nonrelevant, m_pKnowledgebase, dot_begin, dot_begin+1, strIndex.data(), strIndex.data() + 1, m_pKnowledgebase->GetLabelIndex(UnknownLabel)));
			  SEMANTIC_ACTION(LexrepCreated(lexrep_vector.back(), *m_pKnowledgebase));
			  nBeginPos = nPosition; // advance to current position
			  continue; // next
		  }
		  const Char* kata_begin = &pData[nPosition++];
		  while ((size_t)nPosition < input_size && IkStringAlg::IsKatakana(pData[nPosition])) {
			  nPosition++;
		  }
		  const Char* kata_end = &pData[nPosition];
		  const Char* kata_cur = kata_begin;
		  while (kata_cur != kata_end) {
			  size_t strSize = ((kata_end - kata_cur)>1 ? (size_t)2 : (size_t)1); // size of string to normalize
			  static String strIndex(3, '\0'); // max size is 2
			  strIndex.replace(0, strIndex.size(), kata_cur, strSize);
			  static String strPreprocess(3, '\0'); 
			  strPreprocess.replace(0, strPreprocess.size(), strIndex);
			  IkStringAlg::NormalizeWidth(strIndex); 
			  
			  if (strIndex.size() == 1) { // narrow to single Katakana, or single Katakana
				  SEMANTIC_ACTION(NormalizeToken(strPreprocess, strIndex));
				  lexrep_vector.push_back(IkLexrep(IkLabel::Concept, m_pKnowledgebase, kata_cur, kata_cur+strSize, strIndex.data(), strIndex.data() + 1, m_pKnowledgebase->GetLabelIndex(KatakanaLabel)));   // handle kata_text
				  SEMANTIC_ACTION(LexrepCreated(lexrep_vector.back(), *m_pKnowledgebase)); // send to trace info
			  } else { // single Katakana
				  strIndex.erase(1, 1); // erase second symbol
				  strPreprocess.erase(1, 1); // idem
				  SEMANTIC_ACTION(NormalizeToken(strPreprocess, strIndex));
				  lexrep_vector.push_back(IkLexrep(IkLabel::Concept, m_pKnowledgebase, kata_cur, kata_cur+1, strIndex.data(), strIndex.data() + 1, m_pKnowledgebase->GetLabelIndex(KatakanaLabel)));   // handle kata_text
				  SEMANTIC_ACTION(LexrepCreated(lexrep_vector.back(), *m_pKnowledgebase)); // send to trace info
				  strSize = (size_t)1;
			  }
			  kata_cur += strSize; // next
		  }
		  nBeginPos = nPosition; // advance to current position
		  continue; // next
	  }
	  if (u_isalpha(cCur) || // All "Lx" categories plus "Nd" : add to Literal and Index
		  u_isgraph(cCur) || // Japanese also uses broader graphical symbols
		  u_ispunct(cCur)) { // All "Px" categories = punctuation

        if (IkStringAlg::IsOpenParenthesis(cCur)) { // handle parenthesis. scan for furigana reading aid.
          int tmp_nPosition=nPosition+1; 
		  IkStringAlg::FuriganaClass furigana_class=IkStringAlg::NoFurigana; // to be detected
		  bool bIsFurigana=false;
		  bool bCreatedFurigana = false;
		  while ((size_t)tmp_nPosition<input_size) {
		    const Char current_char=pData[tmp_nPosition++];
            if (IkStringAlg::IsJpnSplit(current_char)) continue; // do not end the sentence before closing parenthesis
		    // double newline ends the sentence
		    if (IkStringAlg::IsCloseParenthesis(current_char)) { // ending furigana parenthesis
              // const Char* closing_parenthesis=&pData[tmp_nPosition]; // iterator style, one past the real closing parenthesis symbol
			  lexrep_vector.push_back(IkLexrep(IkLabel::Nonrelevant, m_pKnowledgebase, pData + nBeginPos, pData + tmp_nPosition, pData + nPosition, pData + tmp_nPosition, m_pKnowledgebase->GetLabelIndex(UnknownLabel)));
			  SEMANTIC_ACTION(LexrepCreated(lexrep_vector.back(), *m_pKnowledgebase));
			  nBeginPos=nPosition=tmp_nPosition;
			  bCreatedFurigana = true;
			  break;
            } else { // check current_char for Furigana
				if (!bIsFurigana) { // to be detected
					furigana_class=IkStringAlg::FindFuriganaClass(current_char);
					if (furigana_class==IkStringAlg::NoFurigana) break; // no Furigana
					else bIsFurigana=true;
				} else { // further scan Furigana
					if (IkStringAlg::FindFuriganaClass(current_char)!=furigana_class) { // no continuation of Furigana
					  if (u_isspace(current_char)) continue; // PL128243
			          if (furigana_class==IkStringAlg::HiraganaFurigana && (IkStringAlg::IsJpnDOT(current_char) || IkStringAlg::IsJpnChoon(current_char))) continue; // exception, a DOT or the Cho-on in Hiragana means, continue
			          else break; // *not* furigana, process normally
					}
				}
			}
          }
		  if (bCreatedFurigana) continue;
        }
        const Char* val_begin = pData + nBeginPos;
        const Char* val_end = pData + nPosition;

		static String strIndex(128, '\0');
		strIndex.replace(0, strIndex.size(), val_end, 1); // copy index symbol
		static String strPreprocess(128, '\0');
		strPreprocess.replace(0, strPreprocess.size(), strIndex);
		IkStringAlg::NormalizeWidth(strIndex);
		IkStringAlg::ToLower(strIndex); // needed for romanic numbers
		SEMANTIC_ACTION(NormalizeToken(strPreprocess, strIndex));

		lexrep_vector.push_back(IkLexrep(IkLabel::Unknown, m_pKnowledgebase, val_begin, val_end + 1, strIndex.data(), strIndex.data() + strIndex.size(), (u_ispunct(cCur) && !IkStringAlg::IsJpnPunctuationException(cCur)) ? m_pKnowledgebase->GetLabelIndex(PunctuationLabel) : m_pKnowledgebase->GetLabelIndex(ConceptLabel)));
		SEMANTIC_ACTION(LexrepCreated(lexrep_vector.back(), *m_pKnowledgebase));
		nBeginPos=++nPosition; // lexrep consumed
		if (IkStringAlg::IsJpnSplit(cCur)) {
			if ((size_t)nPosition < input_size && u_ispunct(pData[nPosition]) && !IkStringAlg::IsOpenPunctuationPair(pData[nPosition]) && !IkStringAlg::IsJpnPunctuationException(pData[nPosition])) { // add consequtive punctuations to the sentence
				const Char* punct_begin = &pData[nPosition++];
				while ((size_t)nPosition < input_size && u_ispunct(pData[nPosition]) && !IkStringAlg::IsOpenPunctuationPair(pData[nPosition]) && !IkStringAlg::IsJpnPunctuationException(pData[nPosition])) nPosition++;
				const Char* punct_end = &pData[nPosition];
				const Char* punct_cur = punct_begin;
				while (punct_cur != punct_end) {
					size_t strSize = ((punct_end - punct_cur)>1 ? (size_t)2 : (size_t)1); // size of string to normalize
					static String strPunctIndex(3, '\0'); // max size is 2
					strPunctIndex.replace(0, strPunctIndex.size(), punct_cur, strSize);
					static String strPunctPreprocess(3, '\0');
					strPunctPreprocess.replace(0, strPunctPreprocess.size(), strPunctIndex);
					IkStringAlg::NormalizeWidth(strPunctIndex);

					if (strPunctIndex.size() == 1) { // narrow to single Katakana, or single Katakana
						SEMANTIC_ACTION(NormalizeToken(strPunctPreprocess, strPunctIndex));
						lexrep_vector.push_back(IkLexrep(IkLabel::Unknown, m_pKnowledgebase, punct_cur, punct_cur+strSize, strPunctIndex.data(), strPunctIndex.data() + 1, m_pKnowledgebase->GetLabelIndex(PunctuationLabel)));   // handle punctuation text
						SEMANTIC_ACTION(LexrepCreated(lexrep_vector.back(), *m_pKnowledgebase)); // send to trace info
					}
					else { // single Katakana
						strPunctIndex.erase(1, 1); // erase second symbol
						strPunctPreprocess.erase(1, 1); // idem
						SEMANTIC_ACTION(NormalizeToken(strPunctPreprocess, strPunctIndex));
						lexrep_vector.push_back(IkLexrep(IkLabel::Unknown, m_pKnowledgebase, punct_cur, punct_cur+1, strPunctIndex.data(), strPunctIndex.data() + 1, m_pKnowledgebase->GetLabelIndex(PunctuationLabel)));   // handle punctuation text
						SEMANTIC_ACTION(LexrepCreated(lexrep_vector.back(), *m_pKnowledgebase)); // send to trace info
						strSize = (size_t)1;
					}
					punct_cur += strSize; // next
				}
				nBeginPos = nPosition; // advance to current position
			}
			break; // End the sentence
		}
		continue;
	  }
	  if (u_iscntrl(cCur)) { // double newline ends the sentence
		Char cPattern[4] = { cCur, 0x00, 0x00, 0x00 };
		static const Char winPattern[] = { '\r', '\n', '\r', '\n' };
		static const Char UnixPattern[] = { '\n', '\n' };
		static const Char MacOSPattern[] = { '\r', '\r' }; 
		for (size_t i=1; ((size_t)nPosition+i<input_size) && (i<4); i++) {  // read control marker, but don't exceed buffer
			cPattern[i]=pData[nPosition+i];
		}
		int c=0;
		for (; c<4; c++) { if (cPattern[c] != winPattern[c]) break; }
		if (c==4) { nPosition+=4; break; } // windows double newline, skip control mark and end the sentence
		for (c=0; c<2; c++) { if (cPattern[c] != UnixPattern[c]) break; }
		if (c==2) { nPosition+=2; break; } // windows double newline, skip control mark and end the sentence
		for (c=0; c<2; c++) { if (cPattern[c] != MacOSPattern[c]) break; }
		if (c==2) { nPosition+=2; break; } // windows double newline, skip control mark and end the sentence
		nBeginPos = ++nPosition; // don't put the control symbol in the lexrep
		continue;
	  }
	  nPosition++; // increment position counters
	}
	lexrep_vector.push_back(m_end_lr); // add SEnd lexrep
	LexrepContext::SeenLabels().InsertAtIndex(m_pKnowledgebase->GetLabelIndex(SentenceEndLabel)); // mark SEnd in lexrep context

	return true; // after last sentence
}

bool IkIndexProcess::FindNextSentence(IkIndexInput* pInput, Lexreps& lexrep_vector, int& nPosition, size_t cntWordLimit, bool delimitedSentences, String& language, double& certainty, IkKnowledgebase *pUdct, double certaintyThresholdForChangingLanguage, int nPositionEndOfPreviousIteration)
{
	lexrep_vector.clear();
	lexrep_vector.reserve(16); //TODO: TRW, better guess?

	//Clear the "SeenLabels" which we use during processing of a single sentence
	//TODO: A more general "context" object that takes callbacks to implement this kind of policy.
	LexrepContext::SeenLabels().Clear();
	m_begin_lr.resetSBeginLabel();
	lexrep_vector.push_back(m_begin_lr); // insert SBegin to mark beginning of sentence
	LexrepContext::SeenLabels().InsertAtIndex(m_pKnowledgebase->GetLabelIndex(SentenceBeginLabel)); // mark SBegin in lexrep context

	int const nPositionBeginFixed = nPosition;
	bool bEndFound = false;
	bool bSeparatorFound = false;
	int nBeginPos = nPosition;
	const int nFixedBeginPos = nBeginPos;
	const Char *pData = (pInput->GetString())->c_str(); // pointer to text data
	const size_t input_size = pInput->GetString()->size();
	token::TokenType prev_type = token::kText;
	bool bIsAlfaNum = false; // tracks if NBS contains alphanumerical symbols

	while ((size_t)nPosition < input_size && !bEndFound) {
		if (pInput->IsAnnotated((size_t) nPosition)) { // separate processing
			if (nPosition>nBeginPos) { // add previous token, if valid
				const Char* val_begin = pData + nBeginPos;
				const Char* val_end = pData + nPosition;
				if (IkStringAlg::IsText(val_begin, val_end)) // add preceding part if it concerns text...
				  Preprocess(val_begin, val_end, lexrep_vector);
			}
			size_t stop_annotation = pInput->GetAnnotationStop(nPosition);
			std::string const *pLabels = pInput->GetAnnotationLabels(nPosition);
			Preprocess(pData + nPosition, pData + stop_annotation, lexrep_vector, pLabels); // store annotated
			nBeginPos = nPosition + static_cast<int>(stop_annotation) + 1;
			nPosition = nBeginPos;
			continue; 
		}
		Char cCur = pData[nPosition];

		/*
		Look for whitespace that signals a sentence or word end.
		Any white space breaks words, but two line endings are
		required to break a sentence.
		*/
		bool bNBSseparator = false;
		bool bSentenceSeparator = false;
		token::TokenType this_type = token::GetType<Char>(cCur);
		switch (this_type) {
		  case token::kSpace: 
			bNBSseparator = true;
		  break;
		  case token::kLine:
			if (lexrep_vector.empty() && nBeginPos == nPosition) { // start of new sentence, ignore
				nPosition++, nBeginPos++;
				continue;
			}
			if (cCur == '\r' && pData[nPosition + 1] == '\n') nPosition++; // treat (binary) Windows end-of-line symbols as one
			bNBSseparator = true;
			if (prev_type == token::kLine) bSentenceSeparator = true; // double line token forces end of sentence
		  break;
		  case token::kPunctuation:
			  if (IkStringAlg::IsPunctuationException(cCur)) bIsAlfaNum = true; // symbol is used frequently in text concatenation
			  break;
		  case token::kText: bIsAlfaNum = true;
		  default:;
		}
		/*
		if (this_type == token::kSpace) bNBSseparator = true;
		if (this_type == token::kLine) {
			bNBSseparator = true;
			if (prev_type == token::kLine) bSentenceSeparator = true; // double line token forces end of sentence
		}
		*/
		if (bNBSseparator) { //NBS 
			if (bSentenceSeparator) { // end of sentence condition
				prev_type = token::kText; // reset
				if (lexrep_vector.empty()) { // no text yet
				  nPosition++, nBeginPos++;
				  continue;
				} else
				  bEndFound = true; // force sentence end.
			}
			const Char* val_begin = pData + nBeginPos;
			const Char* val_end = pData + nPosition;
			if (this_type == token::kLine) { // strip-off line terminator symbols
				while (token::GetType<Char>(*(val_end-1)) == token::kLine && val_end > val_begin) --val_end;
			}

			//We need to recognize abbreviations even if they have quotes or other brackets
			//attached to them.
			//Here we're using the "trick" of using a static string with 
			//"replace" to avoid reallocations.
			static String trimmedValue(128, '\0');
			trimmedValue.replace(0, trimmedValue.size(), val_begin, val_end - val_begin);
			IkStringAlg::RemovePunctuationAndQuotes(trimmedValue);
			if (val_begin != val_end) {
				nBeginPos = nPosition + 1;

				//Guard all calls to things that would detect sentence ends with "delimitedSentence" check.
				//In delimited sentences mode we want only double line breaks to end sentences.
				if (!delimitedSentences && m_pKnowledgebase->IsSentenceSeparator(trimmedValue)) // KB: sentence separator
					bSeparatorFound = true;
				if (!delimitedSentences && m_pKnowledgebase->IsNonSentenceSeparator(trimmedValue)) {
					bSeparatorFound = false; // KB: not sentence separator
				}
				//In delimited sentence mode we ignore the user dictionary, too.
				if (!delimitedSentences && pUdct) { // check user dct (higher priority)
					if (pUdct->IsSentenceSeparator(trimmedValue))  // is it a sentence separator
						bSeparatorFound = true;
					if (pUdct->IsNonSentenceSeparator(trimmedValue)) { // is it NOT a non-separator
						bSeparatorFound = false;
					}
				}
				if (bSeparatorFound) // UDCT has first priority
					bEndFound = true; // break the sentence
				if (bIsAlfaNum) Preprocess(val_begin, val_end, lexrep_vector); // contains text
				else { // not text in token
					lexrep_vector.push_back(IkLexrep(IkLabel::Nonrelevant, m_pKnowledgebase, val_begin, val_end, val_begin, val_end, m_pKnowledgebase->GetLabelIndex(PunctuationLabel)));
					SEMANTIC_ACTION(LexrepCreated(lexrep_vector.back(), *m_pKnowledgebase));
				}
				bIsAlfaNum = false; // reset for next token
				if (lexrep_vector.size() >= cntWordLimit) // force sentence separator if limit has been reached
					break;
			}
			else
				nBeginPos = nPosition + 1;
		}
		else { // not NBS separator
			if (!delimitedSentences && m_pKnowledgebase->FastIsSentenceSeparator(cCur)) {
				bSeparatorFound = true;
			}
			else {
				if (bSeparatorFound) { // previous is sentence separator, check quotes
					if (!IkStringAlg::IsQuote(pData[nPosition]))
						bSeparatorFound = false; // call off sentence separator condition
				}
			}
		}
		prev_type = this_type; // remember the previous Char
		nPosition++;
	}
	//Now we should have a complete word.

	if (!bEndFound && nBeginPos < nPosition) { // copy the remaining
		const Char* val_begin = pData + nBeginPos;
		const Char* val_end = pData + nPosition;
		if (bIsAlfaNum) Preprocess(val_begin, val_end, lexrep_vector); // contains text
		else { // not text in token
			lexrep_vector.push_back(IkLexrep(IkLabel::Nonrelevant, m_pKnowledgebase, val_begin, val_end, val_begin, val_end, m_pKnowledgebase->GetLabelIndex(PunctuationLabel)));
			SEMANTIC_ACTION(LexrepCreated(lexrep_vector.back(), *m_pKnowledgebase));
		}
	}
	else {
		bEndFound = (lexrep_vector.size()==1); // Only SBegin is present
	}
	//TODO, TRW: Is there a way we could figure out we need to switch languages earlier?
	if ((m_languageKbMap.size() > 1) && nPosition != nPositionEndOfPreviousIteration)
	{
		//TODO:TRW, Restore ALI sentence length limit
		String new_language = identify(pInput->GetString()->begin() + nPositionBeginFixed,
			pInput->GetString()->begin() + nPosition, certainty);
		if (certainty > certaintyThresholdForChangingLanguage)
		{
			SEMANTIC_ACTION(SwitchKnowledgebase(language, new_language, certainty));
			language = new_language;
			KbMap::const_iterator lookup = m_languageKbMap.find(language);
			if (lookup == m_languageKbMap.end()) {
				//TODO: Status code
				throw MessageExceptionFrom<IkIndexProcess>("IKNoKBLoaded");
			}
			IkKnowledgebase const * const & kbThatShouldBeActive = lookup->second;
			if (m_pKnowledgebase != kbThatShouldBeActive)
			{
				SetCurrentKnowledgebase(kbThatShouldBeActive); // change KB

				// restart sentence detection with this kb set.
				int nPositionEnd = nPosition; // set the endposition for the next recursionstep, if the sentence detected with the new kb is the same sentence, languageidentification isn't needed a second time for the same sentence
				nPosition = nPositionBeginFixed; // reset beginposition
				m_begin_lr.SetKnowledgebase(kbThatShouldBeActive); // adjust SBegin & SEnd to new kb.
				m_begin_lr.setSBeginLabel();
				m_end_lr.SetKnowledgebase(kbThatShouldBeActive);
				m_end_lr.setSEndLabel();

				return FindNextSentence(pInput, lexrep_vector, nPosition, cntWordLimit, delimitedSentences, language, certainty, pUdct, certainty, nPositionEnd); // identify next sentence again with the new kb // TODO, infinite loop prevention
			}

		} // else language is not known, keep language of previous sentence
	}
	//Clean up sentence end punctuation.
	if (!lexrep_vector.empty()) {
		//TODO: Too much string creation/destruction here.
		Lexreps::reverse_iterator i = lexrep_vector.rbegin();
		while (i != lexrep_vector.rend() && i->GetValue().empty()) ++i;
          if (i != lexrep_vector.rend() && !i->GetNormalizedValue().empty()) {
			String& lastIndex = i->GetNormalizedValue();
			bool endsWithDot = false;
			if (lastIndex[lastIndex.size() - 1] == '.') {
				endsWithDot = true;
				lastIndex.erase(lastIndex.size() - 1);
			}

			//IkStringAlg::RemoveAllMarkers(lastIndex);
			//Special case: If this looks like an abbreviation at the end of the sentence, put
			//its "." back
			if (endsWithDot) {
				static String lastValue(64, static_cast<Char>(0));
				lastValue.replace(0, lastValue.size(), 0, static_cast<Char>(0));
				lastValue.append(i->GetValue());
				IkStringAlg::RemoveAllMarkers(lastValue);
				lastValue.append(1, '.');
				if (m_pKnowledgebase->IsNonSentenceSeparator(lastValue) || (pUdct && pUdct->IsNonSentenceSeparator(lastValue))) {
					lastIndex.append(1, '.');
				}
			}
			SEMANTIC_ACTION(NormalizeToken(i->GetValue(), lastIndex));
			bool merged_literal = (i->GetValue()[0] == static_cast<Char>(' ')); // literal is part of a merged collection, don't use literal for lexrep reconstruction
			//TODO: Remove duplication with Preprocess()
			if (lastIndex.empty()) { // nothing left, make it nonrelevant punctuation
				if (merged_literal) lastIndex.append(1, '.'); 
				*i = IkLexrep(IkLabel::Nonrelevant, m_pKnowledgebase, i->GetTextPointerBegin(), i->GetTextPointerEnd(), (merged_literal ? i->GetNormalizedValue() : i->GetValue()), m_pKnowledgebase->GetLabelIndex(PunctuationLabel));
				SEMANTIC_ACTION(LexrepCreated(*i, *m_pKnowledgebase));
			}
		}
	}
	m_end_lr.resetSEndLabel(); // reset the SEnd labels
	lexrep_vector.push_back(m_end_lr); // add SEnd lexrep
	LexrepContext::SeenLabels().InsertAtIndex(m_pKnowledgebase->GetLabelIndex(SentenceEndLabel)); // mark SEnd in lexrep context

	return nPosition != nFixedBeginPos; //Did we make any progress? If not, we're done.

}

IkLexrep IkIndexProcess::NextLexrep(Lexreps::iterator& current, Lexreps::iterator end, IkKnowledgebase* pkb)
{
  return pkb ? pkb->NextLexrep(current, end) : m_pKnowledgebase->NextLexrep(current, end);
}

//TODO: TRW, pervasive constness in lexrep vectors
void IkIndexProcess::FindKnownLexreps(bool first_run, Lexreps& in_lexrep_vector, Lexreps& out_lexrep_vector, IkKnowledgebase* pkb)
{
	out_lexrep_vector.clear();
	out_lexrep_vector.reserve(in_lexrep_vector.size());
	pkb ? pkb->ResetLexrepsBuffer() : m_pKnowledgebase->ResetLexrepsBuffer(); // need an explicit reset for reentrancy problem after error throwning

	Lexreps::iterator span_begin = in_lexrep_vector.begin();
	while (span_begin != in_lexrep_vector.end()) {
		while (span_begin != in_lexrep_vector.end() && span_begin->IsAnnotated()) { // do not lookup in lexrep database
			out_lexrep_vector.push_back(*span_begin); // no lexrep lookup, directly copy to output
			++span_begin;
		}
		Lexreps::iterator span_end = (first_run ? in_lexrep_vector.end() : span_begin);
		while (span_end != in_lexrep_vector.end() && !span_end->IsAnnotated())  // find last to lookup
			++span_end;
		while (span_begin != span_end ||
			(pkb ? pkb->MoreLexrepsBuffered() : m_pKnowledgebase->MoreLexrepsBuffered())) {
			out_lexrep_vector.push_back(NextLexrep(span_begin, span_end, pkb));
			if (pkb) { // User dictionary
				// if (out_lexrep_vector.back().IsAnnotated()) // only if annotated by the user dictionary
				SEMANTIC_ACTION(UserDictionaryMatch(out_lexrep_vector.back(), *m_pKnowledgebase));
			}
			else { // language model
				SEMANTIC_ACTION(LexrepIdentified(out_lexrep_vector.back(), *m_pKnowledgebase));
			}
		}
	}
}

struct HasLabel : public unary_function<const IkLexrep, bool> {
	HasLabel(FastLabelSet::Index label_index) : index_(label_index) {}
	FastLabelSet::Index index_;
	bool operator()(const IkLexrep& lexrep) const {
		return lexrep.Contains(index_);
	}
};

void IkIndexProcess::MergeKatakana(Lexreps& lexrep_vector) 
{
	const FastLabelSet::Index katLabelIndex = m_pKnowledgebase->GetLabelIndex(KatakanaLabel);
	const FastLabelSet::Index conLabelIndex = m_pKnowledgebase->GetLabelIndex(ConceptLabel);
	const FastLabelSet::Index nonLabelIndex = m_pKnowledgebase->GetLabelIndex(UnknownLabel);
	bool bPreviousIsKat = false;
	for (Lexreps::iterator i = lexrep_vector.begin(); i != lexrep_vector.end(); i++) { // restore Katakana labels
		// if (i == lexrep_vector.begin() && IkStringAlg::IsJPnakaguro((i->GetNormalizedValue())[0])) continue; // skip 'dot' if at beginning.
		bool bIsKatakana = IkStringAlg::IsKatakana(i->GetNormalizedValue());
		if (bIsKatakana && !i->Contains(katLabelIndex)) { // single symbol overwritten after lexrep lookup
		  if (bPreviousIsKat) i->AddLabelIndex(katLabelIndex); // restore Katakana label if previous lexrep is Katakana.
		}
		bPreviousIsKat = bIsKatakana; // i->Contains(katLabelIndex); // note if previous is Kat
	}
	bPreviousIsKat = false;
	for (Lexreps::reverse_iterator ri = lexrep_vector.rbegin(); ri != lexrep_vector.rend(); ri++) { // restore Katakana labels
		// if (ri == (lexrep_vector.rend()-1) && IkStringAlg::IsJPnakaguro((ri->GetNormalizedValue())[0])) continue; // skip 'dot' if at beginning.
		bool bIsKatakana = IkStringAlg::IsKatakana(ri->GetNormalizedValue());
		if (bIsKatakana && !ri->Contains(katLabelIndex)) { // single symbol overwritten after lexrep lookup
			if (bPreviousIsKat) {
				ri->AddLabelIndex(katLabelIndex); // restore Katakana label if previous lexrep is Katakana.
			}
		}
		bPreviousIsKat = ri->Contains(katLabelIndex); // note if previous is Kat
	}
	Lexreps::iterator i = lexrep_vector.begin();
	while (i != lexrep_vector.end()) {
	  if (i->Contains(katLabelIndex)) { // i->SetLexrepType(IkLabel::Concept);
		String newEntity = i->GetNormalizedValue();
		const Char* newLiteralp = i->GetTextPointerEnd();
		i->RemoveLabelIndex(katLabelIndex); // remove special label
		Lexreps::iterator i_kat = i + 1;  
		if (IkStringAlg::IsJPnakaguro(newEntity[0])) {
			int cntNakaguro = 1;
			while (i_kat != lexrep_vector.end() && (IkStringAlg::IsJPnakaguro(i_kat->GetNormalizedValue()[0]))) { // scan JPnakaguro
				newEntity += i_kat->GetNormalizedValue();
				newLiteralp = i_kat->GetTextPointerEnd();
				++i_kat, ++cntNakaguro;
			}
			if (cntNakaguro > 1 || i == lexrep_vector.begin()) { // merge Nakaguro symbols, make single symbol non-relevant if first.
				i->SetNormalizedValue(newEntity); // write merged Katakana Index
				i->SetTextPointerEnd(newLiteralp); // write merged Katakana Literal value
				i->ClearAllLabels(); 
				i->AddLabelIndex(nonLabelIndex); // and tag it non-relevant
			} else { // single Nakaguro symbol
				++i; continue;
			}
		}
		else {
			while (i_kat != lexrep_vector.end() && i_kat->Contains(katLabelIndex) && !IkStringAlg::IsJPnakaguro(i_kat->GetNormalizedValue()[0])) { // scan Katakana
				newEntity += i_kat->GetNormalizedValue();
				newLiteralp = i_kat->GetTextPointerEnd();
				++i_kat;
			}
			if ((i_kat - i) == 1) { // single piece, possible due to Nakaguro interference, don't merge
				++i; continue;
			}
			i->SetNormalizedValue(newEntity); // write merged Katakana Index
			i->SetTextPointerEnd(newLiteralp);  // write merged Katakana Literal value
			i->ClearAllLabels();
			i->AddLabelIndex(conLabelIndex); // and tag it concept
		}
		SEMANTIC_ACTION(MergedKatakana(*i, *m_pKnowledgebase)); // Trace merge operation
		i = i_kat;
	  }
	  else ++i;
	}
	// remove remaining Katakana labeled lexreps
	const Lexreps::iterator new_end = remove_if(lexrep_vector.begin(), lexrep_vector.end(), HasLabel(katLabelIndex));
	lexrep_vector.erase(new_end, lexrep_vector.end());
	// mark Katakana words
	i = lexrep_vector.begin();
	for (; i != lexrep_vector.end(); i++) { // restore Katakana labels
		if (IkStringAlg::IsKatakana(i->GetNormalizedValue()) && !IkStringAlg::IsJPnakaguro((i->GetNormalizedValue())[0])) {
			i->AddLabelIndex(katLabelIndex);
			SEMANTIC_ACTION(LabelKatakana(*i, *m_pKnowledgebase)); // Trace merge operation
		}
	}
}

void IkIndexProcess::AddAttributeLabels(Lexreps& lexrep_vector) {
  //TODO: TRW, Replace with a lambda expression when we get C++11 everywhere (i.e., 2020 or so).
  for (Lexreps::iterator i = (lexrep_vector.begin()+1); i != (lexrep_vector.end()-1); ++i) { // skip SBegin & SEnd
    AddAttributeLabels(*i);
  }
}

void IkIndexProcess::AddAttributeLabels(IkLexrep& lexrep) {
  DetectCapitalization(lexrep); //Capitalization
}

void IkIndexProcess::AddUserLabels(Lexreps& lexrep_vector, const IkKnowledgebase& ud) {
  for (Lexreps::iterator i = lexrep_vector.begin(); i != lexrep_vector.end(); ++i) {
    AddUserLabels(*i, ud);
  }
}

void IkIndexProcess::AddUserLabels(IkLexrep& lexrep, const IkKnowledgebase& ud) {
	if (ud.LabelSingleToken(lexrep))
		SEMANTIC_ACTION(UserDictionaryMatch(lexrep, *m_pKnowledgebase));
}

void IkIndexProcess::DetectCapitalization(IkLexrep& lexrep) {
  FastLabelSet::Index capital_label;
  switch (IkStringAlg::FindCapitalizationClass(lexrep.GetValue())) {
  case IkStringAlg::NoCapitals:
    return;
  case IkStringAlg::InitialCapital:
    capital_label = m_pKnowledgebase->GetLabelIndex(CapitalInitialLabel);
    break;
  case IkStringAlg::MixedCapitals:
    capital_label = m_pKnowledgebase->GetLabelIndex(CapitalMixedLabel);
    break;
  case IkStringAlg::AllCapitals:
    capital_label = m_pKnowledgebase->GetLabelIndex(CapitalAllLabel);
    break;
  default:
    throw ExceptionFrom<IkIndexProcess>("Unknown capitalization class.");
  }
  lexrep.AddLabelIndex(capital_label);
  SEMANTIC_ACTION(AttributeDetected("Capitalization", lexrep, *m_pKnowledgebase));
}

struct MatchesPattern : public binary_function<const IkLexrep, const IkRuleInputPattern&, bool>  {
  MatchesPattern(Phase phase) : phase_(phase) {}
  bool operator()(const IkLexrep& lexrep, const IkRuleInputPattern& pattern) const {
	  if (const short lexrep_length_option = pattern.GetLexrepLengthOption()) {
		  if (lexrep.GetNormalizedValue().length() != static_cast<size_t>(lexrep_length_option)) return false;
	  }
	if (!pattern.HasLabelTypes()) {
		return pattern.IsMatch(pattern.MatchesGlobalLabelsSet() ? lexrep.GetLabels() : lexrep.GetLabels(phase_));
	} else // send type information only if necessary
		return pattern.IsMatch(lexrep.GetLabels(phase_), lexrep.GetTypeLabels(phase_)); // TODO: send type information only if necessary
  }
  Phase phase_;
};

static void ReplaceLabels(Lexreps::iterator lexrep,
	const IkRuleInputPattern* in,
	Phase phase,
	const IkRuleOutputPattern* out_begin,
	const IkRuleOutputPattern* out_end) {

	for (const IkRuleOutputPattern* out = out_begin; out != out_end; ++in, ++out, ++lexrep) {
		out->Apply(*lexrep, *in, phase);
	}
}

static bool CouldRuleApply(const IkRuleInputPattern* rule_begin, const IkRuleInputPattern* rule_end, const FastLabelBitSet& labels_seen) {
  //TODO: Algorithm
  for (const IkRuleInputPattern* i = rule_begin; i != rule_end; ++i) {
    if (!i->MightMatch(labels_seen)) return false;
  }
  return true;
}

//Doesn't filter anything.
struct NullLexrepFilter {
  bool FilterEntity(const IkLexrep&) const { return false; }
  bool FilterLiteral(const IkLexrep&) const { return false; }
};


//Merges lexrep literal and entity values from begin to end and sets out to those values.
template<typename Filter>
static inline void JoinLexreps(IkLexrep& out, const Lexreps::iterator& begin, const Lexreps::iterator& end, const Filter& filter, const Char ChSep=' ') {
  static String newEntity; // Use static buffers for performance
  const size_t desired_capacity = (end - begin) * 8;
  if (newEntity.capacity() < desired_capacity) newEntity.reserve(desired_capacity);
  newEntity.clear();

  for (Lexreps::iterator i = begin; i != end; ++i) {
    if (!i->GetNormalizedValue().empty() && !filter.FilterEntity(*i)) {
      newEntity += i->GetNormalizedValue();
      if (i+1!=end) {
        if (ChSep!='\0') newEntity += ChSep; // SpaceString();
      }
    }
  }
  //Use .data/size() to force new string creation and not use GNU libstdc++ COW semantics, which foul up the static String buffers
  //Note we drop labels on the floor, as we're past ambiguity resolution
  out.SetNormalizedValue(String(newEntity.data(), newEntity.size()));
  out.SetTextPointerBegin(begin->GetTextPointerBegin());
  int last=1; // deviation to last lexrep
  while ((end-last)->GetTextPointerEnd() == NULL) // last lexrep can have NULL literal pointer if punctuation has been isolated: "in other words ,"
	  ++last;
  out.SetTextPointerEnd((end-last)->GetTextPointerEnd());
  out.RemoveLabelType(iknow::core::IkLabel::Literal); // Remove literal labels after joining the lexreps.
}

struct IsPrimaryLabel : public std::unary_function<const IkRuleOutputPattern, bool> {
  IsPrimaryLabel(FastLabelSet::Index label_index) : index_(label_index) {}
  FastLabelSet::Index index_;
  bool operator()(const IkRuleOutputPattern& pattern) const {
    return pattern.PrimaryLabel() == index_;
  }
};

void IkIndexProcess::SolveAmbiguous(Lexreps& lexreps)
{	
  //TRW: The loop below is *very* hot (# rules X # lexreps) so carefully test performance implications of anything done here.
  size_t ruleCount = m_pKnowledgebase->RuleCount();

  //We have been keeping track of a bitset of every label we see in the sentence, and add to it whenever
  //we see a new label go in (never remove). Then, if a rule contains a label for an unset bit, we can skip it.
  FastLabelBitSet& labels_seen = LexrepContext::SeenLabels();
  const FastLabelSet::Index joinLabelIndex = m_pKnowledgebase->GetLabelIndex(JoinLabel);
  const FastLabelSet::Index joinReverseIndex = m_pKnowledgebase->GetLabelIndex(JoinReverse);
  // labels_seen.InsertAtIndex(joinLabelIndex);  //TODO : is this necessary, JOIN is never an input rule selector ?
  
  static size_t last_kb_hash = 0;
  typedef std::vector<IkRule> RuleVector;
  static RuleVector rule_cache;
  size_t cur_kb_hash = m_pKnowledgebase->GetHash();
  if (last_kb_hash != cur_kb_hash) {
    rule_cache.resize(ruleCount);
    for (size_t i=0; i < ruleCount; ++i) {
      rule_cache[i] = m_pKnowledgebase->GetRule(i);
    }
    last_kb_hash = cur_kb_hash;
  }
  for (size_t i=0; i < ruleCount; ++i) {
    const IkRuleInputPattern* orig_begin = rule_cache[i].InputBegin();
    const IkRuleInputPattern* orig_end = rule_cache[i].InputEnd();

    if (!CouldRuleApply(orig_begin, orig_end, labels_seen)) {
      continue;
    }
    const IkRuleOutputPattern* repl_begin = rule_cache[i].OutputBegin();
    const IkRuleOutputPattern* repl_end = rule_cache[i].OutputEnd();
    const Phase phase = rule_cache[i].GetPhase();
    size_t pattern_size = orig_end - orig_begin;

    Lexreps::iterator search_begin = lexreps.begin();
    Lexreps::iterator search_end = lexreps.end();
    if (lexreps.size() < pattern_size) continue;
	bool join_required = false, join_reverse_required = false;
	bool is_variable_rule = (find_if(orig_begin, orig_end, [](const IkRuleInputPattern& rip) { return rip.IsVariable(); }) != orig_end); // true;
	if (is_variable_rule) {
		for (Lexreps::iterator match = search_begin; match != search_end; ++match) { // iterate lexreps
			bool isMatch = false;
			Lexreps::iterator rule_lexrep_iterator = match; // temporary iterator for rules scanning
			const IkRuleInputPattern* inRule = orig_begin;  // rule iterator
			static std::vector<int> rule_input_matches;
			rule_input_matches.clear();
			rule_input_matches.resize(pattern_size);
			int rule_offset = 0, real_pattern_size = 0;
			for (; inRule != orig_end && rule_lexrep_iterator != search_end; ++inRule) { // iterate rules
				isMatch = MatchesPattern(phase)(*rule_lexrep_iterator, *inRule);
				if (!isMatch) {
					if (inRule->IsNullVariable()) { // can match zero times
						rule_input_matches[rule_offset] = 0; // no match for this rule
						++rule_offset; // next rule
						continue; // investigate next input rule
					}
					break;
				} else {
					if (inRule->IsNullVariable() && inRule->IsNarrow()) { // check next selector, if match, skip "null" selector
						const IkRuleInputPattern* nxtRule = inRule + 1; // next rule
						if (nxtRule != orig_end) { // current rule may not be the last
							if (MatchesPattern(phase)(*rule_lexrep_iterator, *nxtRule)) { // next rule does also match, skip this (null) rule
								rule_input_matches[rule_offset] = 0; // no match for this rule
								++rule_offset; // next rule
								continue;
							}
						}
					}
					rule_input_matches[rule_offset] = 1;
					++real_pattern_size;
					if (inRule->IsVariable()) { // try to match consecutive lexreps
						Lexreps::iterator rule_variable_lexrep_iterator = rule_lexrep_iterator + 1;
						while (rule_variable_lexrep_iterator != search_end && MatchesPattern(phase)(*rule_variable_lexrep_iterator, *inRule)) {
							if (inRule->IsNarrow()) { // break current rule if next rule matches
								const IkRuleInputPattern* nxtRule = inRule + 1; // next rule
								if (nxtRule != orig_end) { // current rule may not be the last
									if (MatchesPattern(phase)(*rule_variable_lexrep_iterator, *nxtRule)) { // next rule does match, stop this rule
										break;
									}
								}
							}
							++rule_variable_lexrep_iterator;
							++rule_input_matches[rule_offset];
							++real_pattern_size;
						}
						// does rule_input_matches[rule_offset] exceeds min or max ?
						int& lexrep_match_count = rule_input_matches[rule_offset];
						if (lexrep_match_count < inRule->MinVariable() || lexrep_match_count > inRule->MaxVariable()) { isMatch=false; break; }
						if (rule_variable_lexrep_iterator == search_end) { ++inRule; break; } // no more lexreps, iterate rule to examine succesfull match
						else rule_lexrep_iterator = rule_variable_lexrep_iterator - 1; // next lexrep to examine
					}
					++rule_lexrep_iterator;
					++rule_offset;
				}
			}
			if (isMatch && inRule == orig_end) { // rule fully fires...
				SEMANTIC_ACTION(ApplyRule(i, match, real_pattern_size, *m_pKnowledgebase));
				{
					Lexreps::iterator lexrep = match;
					const IkRuleInputPattern* in = orig_begin;
					rule_offset = 0;

					for (const IkRuleOutputPattern* out = repl_begin; out != repl_end; ++in, ++out) {
						if (rule_input_matches[rule_offset]) {
							out->Apply(*lexrep, *in, phase);
							for (int j = 1; j < rule_input_matches[rule_offset]; j++) {
								out->Apply(*++lexrep, *in, phase);
							}
							++lexrep;
						}
						++rule_offset;	
					}
				}
				SEMANTIC_ACTION(RuleApplied(i, match, real_pattern_size, *m_pKnowledgebase));
				if (!join_required) join_required = find_if(repl_begin, repl_end, IsPrimaryLabel(joinLabelIndex)) != repl_end; //Check for the join label in the replacement labels.
				if (!join_reverse_required) join_reverse_required = find_if(repl_begin, repl_end, IsPrimaryLabel(joinReverseIndex)) != repl_end; // same for join reverse
			}
		}
	}
	else {
		Lexreps::iterator match = search(search_begin, search_end, orig_begin, orig_end, MatchesPattern(phase));
		while (match != search_end) {
			SEMANTIC_ACTION(ApplyRule(i, match, pattern_size, *m_pKnowledgebase));
			ReplaceLabels(match, orig_begin, phase, repl_begin, repl_end);
			SEMANTIC_ACTION(RuleApplied(i, match, pattern_size, *m_pKnowledgebase));
			if (!join_required) join_required = find_if(repl_begin, repl_end, IsPrimaryLabel(joinLabelIndex)) != repl_end; //Check for the join label in the replacement labels.
			if (!join_reverse_required) join_reverse_required = find_if(repl_begin, repl_end, IsPrimaryLabel(joinReverseIndex)) != repl_end; // same for join reverse
			match = search(match + 1, search_end, orig_begin, orig_end, MatchesPattern(phase));
		}
	}
    if (join_required) {
      Lexreps::iterator join_begin = find_if(search_begin, search_end, HasLabel(joinLabelIndex));
	  if (join_begin == search_begin) { // first is "Join", skip
		  join_begin->RemoveLabelIndex(joinLabelIndex);
		  SEMANTIC_ACTION(JoinResult(join_begin, *m_pKnowledgebase)); // Make error traceable
		  join_begin = find_if(search_begin, search_end, HasLabel(joinLabelIndex));
	  }
	  Lexreps::iterator join_end;
	  while (join_begin != search_end) {
		  join_end = find_if(join_begin + 1, search_end, not1(HasLabel(joinLabelIndex))); // find the next lexrep without a join label
		  // Merge all into the previous
		  JoinLexreps(*(join_begin - 1), join_begin - 1, join_end, NullLexrepFilter(), (m_pKnowledgebase->GetMetadata<kIsJapanese>() ? '\0' : ' '));
		  for (Lexreps::iterator itLex = join_begin; itLex != join_end; ++itLex) { // Collect the attribute-type labels
			  for (size_t j = 0; j < itLex->NumberOfLabels(phase); ++j) { // rule phase for attributes not on last phase ('$')
				  FastLabelSet::Index lblIndex = itLex->GetLabelIndexAt(j, phase);
				  if (m_pKnowledgebase->GetLabelTypeAtIndex(lblIndex) == IkLabel::Attribute) (join_begin - 1)->AddLabelIndex(lblIndex);
			  }
			  for (size_t j = 0; j < itLex->NumberOfLabels(); ++j) { // rule phase for attributes on last phase ('$')
				  FastLabelSet::Index lblIndex = itLex->GetLabelIndexAt(j);
				  if (m_pKnowledgebase->GetLabelTypeAtIndex(lblIndex) == IkLabel::Attribute) (join_begin - 1)->AddLabelIndex(lblIndex);
			  }
		  }
		  SEMANTIC_ACTION(JoinResult(join_begin - 1, *m_pKnowledgebase));
		  join_begin = find_if(join_end, search_end, HasLabel(joinLabelIndex));
	  }
	  //remove all join-labeled lexreps
	  const Lexreps::iterator new_end = remove_if(lexreps.begin(), lexreps.end(), HasLabel(joinLabelIndex));
	  lexreps.erase(new_end, lexreps.end());
    }
	if (join_reverse_required) {
		Lexreps::reverse_iterator join_reverse_begin = find_if(lexreps.rbegin(), lexreps.rend(), HasLabel(joinReverseIndex));
		if (join_reverse_begin == lexreps.rbegin()) { // first "in reverse" is "JoinReverse", skip
			join_reverse_begin->RemoveLabelIndex(joinReverseIndex);
			SEMANTIC_ACTION(JoinResult(join_reverse_begin.base(), *m_pKnowledgebase)); // Make error traceable
			join_reverse_begin = find_if(lexreps.rbegin(), lexreps.rend(), HasLabel(joinReverseIndex));
		}
		Lexreps::reverse_iterator join_reverse_end;
		while (join_reverse_begin != lexreps.rend()) {
			join_reverse_end = find_if(join_reverse_begin + 1, lexreps.rend(), not1(HasLabel(joinReverseIndex))); // find the next reverse lexrep without a joinReverse label
			JoinLexreps(*(join_reverse_begin - 1), join_reverse_end.base(), (join_reverse_begin - 1).base(), NullLexrepFilter(), (m_pKnowledgebase->GetMetadata<kIsJapanese>() ? '\0' : ' '));
			for (Lexreps::reverse_iterator itLex = join_reverse_begin; itLex != join_reverse_end; ++itLex) { // Collect the attribute-type labels
				for (size_t j = 0; j < itLex->NumberOfLabels(phase); ++j) { // rule phase for attributes not on last phase ('$')
					FastLabelSet::Index lblIndex = itLex->GetLabelIndexAt(j, phase);
					if (m_pKnowledgebase->GetLabelTypeAtIndex(lblIndex) == IkLabel::Attribute) (join_reverse_begin - 1)->AddLabelIndex(lblIndex);
				}
				for (size_t j = 0; j < itLex->NumberOfLabels(); ++j) { // rule phase for attributes on last phase ('$')
					FastLabelSet::Index lblIndex = itLex->GetLabelIndexAt(j);
					if (m_pKnowledgebase->GetLabelTypeAtIndex(lblIndex) == IkLabel::Attribute) (join_reverse_begin - 1)->AddLabelIndex(lblIndex);
				}
			}
			SEMANTIC_ACTION(JoinResult(join_reverse_begin.base(), *m_pKnowledgebase));
			join_reverse_begin = find_if(join_reverse_end, lexreps.rend(), HasLabel(joinReverseIndex));
		}
		// remove all join-reverse labeled lexreps
		const Lexreps::iterator new_end = remove_if(lexreps.begin(), lexreps.end(), HasLabel(joinReverseIndex));
		lexreps.erase(new_end, lexreps.end());
	}
  }
  // resolve multilabel lexreps: if same type, set the type, if different types, set as UNKNOWN (nonrelevant)
  // extra : (new) literal types are transparent.
  for (Lexreps::iterator i=lexreps.begin(); i != lexreps.end(); ++i) {
    IkLexrep& lexrep = *i;
    SEMANTIC_ACTION(RulesComplete(lexrep, *m_pKnowledgebase));

    size_t nbrLabels = lexrep.NumberOfLabels();
    bool bEqualType=true;
    //Find the first non-Attribute label.
    IkLabel::Type firstType = IkLabel::Unknown;
    FastLabelSet::Index firstIndex = m_pKnowledgebase->GetLabelIndex(UnknownLabel);
    size_t j = 0;
    for (; j < nbrLabels; ++j) {
      firstIndex = lexrep.GetLabelIndexAt(j);
      firstType = m_pKnowledgebase->GetLabelTypeAtIndex(firstIndex);
	  if (!(firstType == IkLabel::Attribute || firstType == IkLabel::Literal)) break;
    }
    for (; j < nbrLabels; ++j) {
      IkLabel::Type type = m_pKnowledgebase->GetLabelTypeAtIndex(lexrep.GetLabelIndexAt(j));
	  if (type == IkLabel::Attribute || type == IkLabel::Literal) continue; 
      if (type != firstType) {
        bEqualType=false;
        break;
      }
    }
    //Lexrep types and label types are the same type.
    //(slowly being merged...)
    IkLexrep::Type lexrep_type;
    if (!bEqualType) {
      lexrep_type = IkLabel::Unknown;
    } else {   // Generalize the first type.
      switch (firstType) {
	  case IkLabel::PathRelevant: {
        if (m_pKnowledgebase->GetMetadata<kPathConstruction>() == kCRCSequence) lexrep_type = IkLabel::Nonrelevant; // old style
        else lexrep_type = IkLabel::PathRelevant; // new style : kPathRelevant
	  } break;
      case IkLabel::Unknown:
	  case IkLabel::Nonrelevant:
	  case IkLabel::Ambiguous:
	  case IkLabel::Other:
        lexrep_type = IkLabel::Nonrelevant;
        break;
	  case IkLabel::Attribute: // if all we have is attribute or literal labels, define the lexrep concept, the default assignment.
	  case IkLabel::Literal: // a single literal is treated as a (default) concept
        lexrep_type = IkLabel::Concept;
        break;
      case IkLabel::Concept:
      case IkLabel::BeginConcept:
      case IkLabel::EndConcept:
      case IkLabel::BeginEndConcept:
      case IkLabel::Relation:
      case IkLabel::BeginRelation:
      case IkLabel::EndRelation:
      case IkLabel::BeginEndRelation:
        lexrep_type = firstType;
        break;
      default:
        throw ExceptionFrom<IkIndexProcess>("Unknown label type encountered in IkIndexProcess::SolveAmbiguous().");
      } 
    }
    lexrep.SetLexrepType(lexrep_type);
    SEMANTIC_ACTION(AmbiguityResolved(lexrep, *m_pKnowledgebase));
  }
}

struct IsRelation {
  bool operator()(const IkLexrep& lexrep) {
    switch (lexrep.GetLexrepType()) {
    case IkLabel::Relation:
    case IkLabel::BeginRelation:
    case IkLabel::EndRelation:
    case IkLabel::BeginEndRelation:
      return true;
    default:
      return false;
    }
  }
};

struct RelationLexrepFilter : public NullLexrepFilter
{
  bool FilterEntity(const IkLexrep& lexrep) const {
    IsRelation is_relation;
    return !(is_relation(lexrep));
  }
};

void IkIndexProcess::MergeRelation(const Lexreps::iterator& begin, const Lexreps::iterator& end, size_t& cnt_nonrels_in_relation, MergedLexreps& out_lexrep_vector) {
  if (begin == end) return; //nothing to merge.
  bool bMerge = (cnt_nonrels_in_relation <= m_max_nonrels_in_relation); // do not merge if non-relevants in relation exceed maximum allowed
  cnt_nonrels_in_relation = 0; // reset for next relation merge
  if (!bMerge) { // do not merge, too many nonrelevants in relation
	  IsRelation is_relation;
	  for (Lexreps::iterator i = begin; i != end; ++i) {
		  out_lexrep_vector.push_back(IkMergedLexrep(*i));
		  out_lexrep_vector.back().SetLexrepType(is_relation(*i) ? IkLabel::Relation : IkLabel::Nonrelevant);
	  }
	  return;
  }
  SEMANTIC_ACTION(MergingRelations(begin, end, *m_pKnowledgebase));
  out_lexrep_vector.push_back(IkMergedLexrep(begin, end));
  //Setting the type to a generic relation now that we're done with merging makes future logic simpler.
  out_lexrep_vector.back().SetLexrepType(IkLabel::Relation);
  SEMANTIC_ACTION(MergedRelation(out_lexrep_vector.back(), *m_pKnowledgebase));			     
}

void IkIndexProcess::MergeRelationNonrelevant(IkLexrep& lexrep, MergedLexreps& out_lexrep_vector) {
  lexrep.SetLexrepType(IkLabel::Unknown);
  out_lexrep_vector.push_back(IkMergedLexrep(lexrep));
  SEMANTIC_ACTION(MergedRelationNonrelevant(out_lexrep_vector.back(), *m_pKnowledgebase));
}

void IkIndexProcess::MergeRelations(Lexreps& in_lexrep_vector, MergedLexreps& out_lexrep_vector)
{
  //TRW: The way relation merging works is that internal nonrelevants are included, while any non-relevants
  //after the last relation are emitted separately.
  //Find the last relation.
  const Lexreps::iterator begin_suffix(std::find_if(in_lexrep_vector.rbegin(), in_lexrep_vector.rend(), IsRelation()).base());
  Lexreps::iterator next_begin = in_lexrep_vector.begin();
  bool in_relation = false;
  size_t cnt_sequential_nonrels_in_relation = 0;
  for (Lexreps::iterator i = next_begin; i != begin_suffix; ++i) {
    switch (i->GetLexrepType()) {
    case IkLabel::BeginEndRelation:
      //Merge the last one, if any.
		MergeRelation(next_begin, i, cnt_sequential_nonrels_in_relation, out_lexrep_vector);
      //Merge this one.
		MergeRelation(i, i + 1, cnt_sequential_nonrels_in_relation, out_lexrep_vector);
      //Set start of next after this one.
      next_begin = i+1;
      in_relation = false;
      break;
    case IkLabel::EndRelation:
      //Merge the last run, including this one.
		MergeRelation(next_begin, i + 1, cnt_sequential_nonrels_in_relation, out_lexrep_vector);
      //Set start of next after this one.
      next_begin = i+1;
      in_relation = false;
      break;
    case IkLabel::BeginRelation:
      //Merge the last run, if any, not including this one.
		MergeRelation(next_begin, i, cnt_sequential_nonrels_in_relation, out_lexrep_vector);
      //Set the start of next to be this one.
      next_begin = i;
      in_relation = true;
      break;
    case IkLabel::Relation:
      in_relation = true;
	  cnt_sequential_nonrels_in_relation = 0; // reset sequential nonrelevants 
      break;
    default:
      //All merged relations must begin and end with a relation.
      if (!in_relation) {
        //Emit as nonrelevant
        MergeRelationNonrelevant(*i, out_lexrep_vector);
        //Start a potential new relation on the next lexrep.
        next_begin = i + 1;
	  } else {
		  cnt_sequential_nonrels_in_relation++; // count nonrels in relation
	  }
      break;
    }
  }
  //Merge the remainder
  MergeRelation(next_begin, begin_suffix, cnt_sequential_nonrels_in_relation, out_lexrep_vector);
  //now stuff after the last relation, non-relevant
  for (Lexreps::iterator i = begin_suffix; i != in_lexrep_vector.end(); ++i) {
    MergeRelationNonrelevant(*i, out_lexrep_vector);
  }
}

void IkIndexProcess::EntityFilter(MergedLexreps& lexrep_vector)
{
  for (MergedLexreps::iterator i = lexrep_vector.begin(); i != lexrep_vector.end();) {
    bool first = 1;
    bool last = 0;
    for (IkMergedLexrep::iterator lexrep = i->LexrepsBegin(); lexrep != i->LexrepsEnd();/* need to control iterator explicitly */) {
	  if (lexrep->bHasNoLiteral()) {
        lexrep = i->LexrepErase(lexrep);
        continue; //Don't include lexreps with no literal
      }
      last = lexrep + 1 == i->LexrepsEnd();
      const String& strNormalized = lexrep->GetNormalizedValue();
      static String strIndex(64, static_cast<Char>('\0'));
      strIndex.replace(0, strIndex.size(), 0, static_cast<Char>('\0'));
      strIndex.append(strNormalized.begin(), strNormalized.end());
      switch(lexrep->GetLexrepType()) {
      case IkLabel::Concept: // concept case: apply KB filter rules
        m_pKnowledgebase->FilterConcept(strIndex, first, last); // filter
        SEMANTIC_ACTION(ConceptFiltered(*lexrep, strIndex, *m_pKnowledgebase));
        break;
      case IkLabel::Relation: // apply KB relation filter
        m_pKnowledgebase->FilterRelation(strIndex, first, last); // filter
        SEMANTIC_ACTION(RelationFiltered(*lexrep, strIndex, *m_pKnowledgebase));
        break;
	  case IkLabel::PathRelevant: // apply KB pathrelevant filter
        m_pKnowledgebase->FilterPathRelevant(strIndex, first, last); // filter PathRelevant
        SEMANTIC_ACTION(PathRelevantFiltered(*lexrep, strIndex, *m_pKnowledgebase));
        break;
      case IkLabel::Nonrelevant: // apply KB nonrelevant filter
      case IkLabel::Unknown:
        m_pKnowledgebase->FilterNonRelevant(strIndex, first, last); // filter
        SEMANTIC_ACTION(NonRelevantFiltered(*lexrep, strIndex, *m_pKnowledgebase));
        break;
      default:
	;
      }
      first = 0;
      if (strIndex != strNormalized) { // Did we change anything?
		lexrep->SetNormalizedValue(strIndex);
      }
      ++lexrep;
    }
    if (i->LexrepsCount() && i->IsNormalizedValueEmpty()) { // Set lexreps with completely filtered away normalized values to type unknown
      i->SetLexrepType(IkLabel::Unknown);
	}
    if (!i->LexrepsCount()) {
      i = lexrep_vector.erase(i);
	} else {
      ++i;
    }
  }
}

#define merge_concepts_if(n_concept_counter, buffer_lexrep_vector, merged_concepts) \
{ \
	if (n_concept_counter) { \
		MergeConcepts(buffer_lexrep_vector, merged_concepts);	\
		buffer_lexrep_vector.clear(); \
		n_concept_counter = 0; \
	} \
}

#define merge_relations_if(mergeRelations, bPreviousRelation, buffer_lexrep_vector, out_lexrep_vector) \
if (mergeRelations) { \
	if (bPreviousRelation) { \
		MergeRelations(buffer_lexrep_vector, out_lexrep_vector); \
		buffer_lexrep_vector.clear(); \
	} \
	bPreviousRelation = false; \
}

void IkIndexProcess::MergeAndConvert(Lexreps& in_lexrep_vector, MergedLexreps& out_lexrep_vector, bool mergeRelations)
{
  out_lexrep_vector.clear();
  out_lexrep_vector.reserve(in_lexrep_vector.size());
  Lexreps buffer_lexrep_vector;
  buffer_lexrep_vector.reserve(in_lexrep_vector.size());

  int nConceptCounter = 0; // the concept counter 

  bool bPreviousRelation = false; // only relevant for the mergerelations option
  int nRelationCounter = 0;		// only relevant for the mergerelations option
  int nRelNorCounter = 0;			// only relevant for the mergerelations option

  for (Lexreps::iterator i = (in_lexrep_vector.begin()+1); i != (in_lexrep_vector.end()-1); ++i) { // skip SBegin & SEnd
    IkLexrep::Type type = i->GetLexrepType();
    if (i->Contains(m_pKnowledgebase->GetLabelIndex(PunctuationLabel))) { // JDN: force punctuations to nonrelevant, should be arranged for in KB
      type=IkLabel::Unknown; 
      i->SetLexrepType(type);
	  merge_relations_if(mergeRelations, bPreviousRelation, buffer_lexrep_vector, out_lexrep_vector);
    }
    switch( type )
    {
      case IkLabel::Concept:
      case IkLabel::BeginConcept:
      case IkLabel::EndConcept:
      case IkLabel::BeginEndConcept:
        nConceptCounter++;
		merge_relations_if(mergeRelations, bPreviousRelation, buffer_lexrep_vector, out_lexrep_vector);
        buffer_lexrep_vector.push_back(*i);
        break;

      case IkLabel::Relation:
      case IkLabel::BeginRelation:
      case IkLabel::EndRelation:
      case IkLabel::BeginEndRelation:
        if (mergeRelations)
        {
          bPreviousRelation = true;
          nRelationCounter++;
          nRelNorCounter++;
        }
        merge_concepts_if(nConceptCounter, buffer_lexrep_vector, out_lexrep_vector);
        buffer_lexrep_vector.push_back(*i);
        break;

	  case IkLabel::PathRelevant: // do not merge...
        merge_relations_if(mergeRelations, bPreviousRelation, buffer_lexrep_vector, out_lexrep_vector);
        merge_concepts_if(nConceptCounter, buffer_lexrep_vector, out_lexrep_vector);
        out_lexrep_vector.push_back(IkMergedLexrep(*i)); // store individually
        break;

      default: // non_relevant
        merge_concepts_if(nConceptCounter, buffer_lexrep_vector, out_lexrep_vector);
        if (mergeRelations && bPreviousRelation) { // in the middle of a relation?
          buffer_lexrep_vector.push_back(*i); // we'll process it with the relation
        } else { //otherwise just put it in the output as a non-relevant
          out_lexrep_vector.push_back(IkMergedLexrep(*i));
        }
      }	
  }
  if (nConceptCounter)
    MergeConcepts(buffer_lexrep_vector, out_lexrep_vector);	//  merge with the previous concepts
  else if (mergeRelations && nRelationCounter)
    MergeRelations(buffer_lexrep_vector, out_lexrep_vector);
}

void IkIndexProcess::MergeConceptLexreps(const Lexreps::iterator& begin, const Lexreps::iterator& end, MergedLexreps& out_lexrep_vector) {
  //Generify type
  for (Lexreps::iterator i = begin; i != end; ++i) {
    i->SetLexrepType(IkLabel::Concept);
  }
  if (static_cast<size_t>(end - begin) > m_max_concept_cluster_length) { // Too big? Spec currently says we have to return all concept lexreps individually, no merging
    for (Lexreps::iterator i = begin; i != end; ++i) {
      out_lexrep_vector.push_back(IkMergedLexrep(*i));
    }
    return;
  }
  //add to output
  SEMANTIC_ACTION(MergingConcepts(begin, end, *m_pKnowledgebase));
  out_lexrep_vector.push_back(IkMergedLexrep(begin, end));
  SEMANTIC_ACTION(MergedConcept(out_lexrep_vector.back(), *m_pKnowledgebase));
}
  
//returns the next concept lexrep after applying concept merge rules. begin is adjusted to reflect next
//concept start position
//TODO: TRW: Use output iterators instead of output vectors
void IkIndexProcess::MergeNextConcepts(Lexreps::iterator& begin, const Lexreps::iterator& end, MergedLexreps& out_lexrep_vector) {
  Lexreps::iterator first = begin; //keep the first one around in case we hit the max limit
  Lexreps::iterator i = first;
  while (i != end) {
    switch (i->GetLexrepType()) {
    case IkLabel::BeginEndConcept:
      //BeginEndConcepts start and end a concept in one lexrep.
      if (i == first) {
	//This solitary concept is the next one.
	MergeConceptLexreps(first, first + 1, out_lexrep_vector);
	begin = i + 1;
      }
      else {
	//Otherwise return the current concept
	//Next one starts here.
	begin = i;
	//Merge everything else, excluding this one.
	MergeConceptLexreps(first, i, out_lexrep_vector);
	//Note that I could do a more complicated bit of case fall-through to merge
	//this code with the duplicate block below, but I don't believe it's worth it.
	//
	//There's probably a simpler implementation dying to get out of this switch/case.
      }
      //always done when we see a BeginEnd.
      return;
    case IkLabel::BeginConcept:
      if (i == first) {
	//we just started, keep going
	++i;
	continue;
      }
      else {
	//the next one starts here
	begin = i;
	MergeConceptLexreps(first, i, out_lexrep_vector); //merge everything before, excluding this one
	return;
      }
    case IkLabel::EndConcept: //Always ends a concept
      begin = i + 1; //start next with the next lexrep
      MergeConceptLexreps(first, begin, out_lexrep_vector); //merge including this one
      return;
    default:
      ++i;
    }
  }
  begin = i;
  MergeConceptLexreps(first, i, out_lexrep_vector);
  return;
} 

void IkIndexProcess::MergeConcepts(Lexreps& in_lexrep_vector, MergedLexreps& out_lexrep_vector)
{
  Lexreps::iterator begin = in_lexrep_vector.begin();
  Lexreps::iterator end = in_lexrep_vector.end();
  while  (begin != end) {
    MergeNextConcepts(begin, end, out_lexrep_vector);
  }
}

void IkIndexProcess::AddPath(path::CRCs::const_iterator start, path::CRCs::const_iterator end, Paths& out_path_vector) {
  if (end - start < 1) return; //paths must have length
  const path::Offsets no_offsets; //no non-relevant support yet
  out_path_vector.push_back(IkPath(start, end, no_offsets.begin(), no_offsets.end()));
}

void IkIndexProcess::DetectCRCPatterns(const MergedLexreps& in_lexrep_vector, path::CRCs& out_crc_vector, const ChainPattern pattern)
{
  path::CRCs& crcs = out_crc_vector;
  crcs.reserve(in_lexrep_vector.size());
  path::DetectCRCs(in_lexrep_vector.begin(), in_lexrep_vector.end(), std::back_inserter(crcs), pattern);
}

void IkIndexProcess::BuildPathsFromPathRelevants(const MergedLexreps& in_lexrep_vector, Paths& out_path_vector) 
{
  path::Offsets path_offsets; // fully constructed from concepts/relations & path_relevants
  for (MergedLexreps::const_iterator i = in_lexrep_vector.begin(); i != in_lexrep_vector.end(); ++i) {
	  if (i->IsConcept() || i->IsRelation() || i->IsPathRelevant()) { // these are the path building parts
		  const size_t offset = i - in_lexrep_vector.begin();
		  path_offsets.push_back(offset);
	  }
  }
  out_path_vector.clear();
  if (path_offsets.size()>static_cast<size_t>(1)) // a path must be at least 2 entities
    out_path_vector.push_back(IkPath(path_offsets.begin(), path_offsets.end()));
}

void IkIndexProcess::BuildPathsFromCRCs(const MergedLexreps& in_lexrep_vector, const path::CRCs& crc_vector, Paths& out_path_vector) {
  out_path_vector.clear();
  out_path_vector.reserve(crc_vector.size()); //TODO: probably should be smaller

  if (crc_vector.empty()) return;

  path::CRCs::const_iterator path_begin = crc_vector.begin();
  SEMANTIC_ACTION(CRC(*path_begin, in_lexrep_vector));
  for (path::CRCs::const_iterator i = path_begin + 1; i != crc_vector.end(); ++i) {
    SEMANTIC_ACTION(CRC(*i, in_lexrep_vector));
    if (!(i-1)->DoesPathContinue(*i)) {
      AddPath(path_begin, i, out_path_vector);
      path_begin = i;
    }
  }
  AddPath(path_begin, crc_vector.end(), out_path_vector);
}

void IkIndexProcess::BuildPathsFromAttributes(const MergedLexreps& in_lexrep_vector, Paths& out_path_vector) {
  static String begin_path_str = IkStringEncoding::UTF8ToBase("PathBegin");
  static String end_path_str = IkStringEncoding::UTF8ToBase("PathEnd");
  AttributeId begin_attr = m_pKnowledgebase->AttributeIdForName(begin_path_str);
  AttributeId end_attr = m_pKnowledgebase->AttributeIdForName(end_path_str);
  
  MergedLexreps::const_iterator path_begin = in_lexrep_vector.end();
  for (MergedLexreps::const_iterator i = in_lexrep_vector.begin(); i != in_lexrep_vector.end(); ++i) {
    if (i->HasAttributeWithType(begin_attr) && path_begin == in_lexrep_vector.end()) path_begin = i;
    if (i->HasAttributeWithType(end_attr) && path_begin != in_lexrep_vector.end()) {
      path::Offset begin_offset = path_begin - in_lexrep_vector.begin();
      path::Offset end_offset = i - in_lexrep_vector.begin();
      out_path_vector.push_back(IkPath(begin_offset, end_offset));
      path_begin = in_lexrep_vector.end();
    }
  }
  //Leftover
  if (path_begin != in_lexrep_vector.end()) { 
    path::Offset begin_offset = path_begin - in_lexrep_vector.begin();
    path::Offset end_offset = in_lexrep_vector.size();
    out_path_vector.push_back(IkPath(begin_offset, end_offset));
  }
}
#ifndef _RWSTD_NO_MEMBER_TEMPLATES
typedef IkEntityVectorEvaluator<AttributeId, size_t, PoolAllocator<AttributeId> > EntityVectorEvaluator;
#else
typedef IkEntityVectorEvaluator<AttributeId, size_t> EntityVectorEvaluator;
#endif

//We will transform the EV* attributes of a sentence
//into these EV expressions, which can be evaluated in order
//a sequence set at a time when applied to an IkEntityVectorEvaulator.
class EVExpr {
public:
  //Vtable-like value for how to evaluate
  enum Operation {
    kSlot,
    kValue
  };
  EVExpr(Operation operation, size_t offset, size_t sequence,
	 size_t priority, AttributeId type, EV::Direction direction,
	 EV::Order order) :
    operation_(operation), offset_(offset), sequence_(sequence),
    priority_(priority), type_(type), direction_(direction), order_(order) {}
  void Evaluate(EntityVectorEvaluator& eve) const {
    switch (operation_) {
    case kValue:
      eve.AddValue(type_, offset_, direction_, order_);
      return;
    case kSlot:
      eve.AddSlot(type_, direction_, order_);
      return;
    }
  }
  //for (stable!) sorting by sequence
  bool operator<(const EVExpr& other) const {
    return sequence_ < other.sequence_; 
  }
  size_t GetSequence() const {
    return sequence_;
  }
  size_t GetOffset() const {
    return offset_;
  }
  size_t GetPriority() const {
    return priority_;
  }
  Operation GetOperation() const {
    return operation_;
  }
private:
  Operation operation_;
  //The offset of the associated lexrep
  size_t offset_;
  //The sequence set of the operation.
  //All 0-expr's are evaluated first, followed
  //by all 1-expr's etc.
  size_t sequence_;
  //The priority in which to apply multiple expressions
  //with the same value.
  size_t priority_;
  //The type of the slot or value
  AttributeId type_;
  //Which queue to put the final value. (L or R)
  EV::Direction direction_;
  //Which order to insert the final value into
  //that queue (F or B)
  EV::Order order_;
};

//Compare EVExpr's by priority

bool PriorityCompare(const EVExpr& a, const EVExpr& b) {
  return a.GetPriority() < b.GetPriority();
}

//Sorts each contiguous offset group of EVExpr's by priority.
template<typename IterT>
void Prioritize(IterT begin, IterT end) {
  while (begin != end) {
    IterT range_begin = begin;
    IterT range_end = range_begin;
    //Advance the end of the range until it's on a different offset
    while (range_end != end &&
	   range_end->GetOffset() == range_begin->GetOffset()) ++range_end;
    std::stable_sort(range_begin, range_end, PriorityCompare);
    begin = range_end;
  }
}


static int AttribToInt(AttributeId id, const IkKnowledgebase& kb) {
  AttributeString as = kb.AttributeNameForId(id);
  char buf[16];
  if (as.size > sizeof(buf) - 1)
    throw ExceptionFrom<IkIndexProcess>("AttribToInt tried to translate a number that was too big.");
  for(size_t i = 0; i < as.size; ++i) {
    buf[i] = static_cast<char>(as.data[i]);
  }
  buf[as.size] = 0;
  return std::atoi(buf);
}

static char AttribToChar(AttributeId id, const IkKnowledgebase& kb) {
  AttributeString as = kb.AttributeNameForId(id);
  if (as.size != 1)
    throw ExceptionFrom<IkIndexProcess>("AttribToChar found a non-char-length attribute.");
  return static_cast<char>(*(as.data));
}

static EV::Direction CharToDirection(char c) {
  switch (c) {
  case 'R':
    return EV::kRight;
  case 'L':
    return EV::kLeft;
  default:
    throw ExceptionFrom<IkIndexProcess>("Illegal direction argument to entity vector attribute.");
  }
}

static EV::Order CharToOrder(char c) {
  switch (c) {
  case 'F':
    return EV::kFront;
  case 'B':
    return EV::kBack;
  default:
    throw ExceptionFrom<IkIndexProcess>("Illegal order argument to entity vector attribute.");
  }
}

//Take advantage of the fact that EVValue() and EVSlot() have isometric syntax
template<typename OutIterT>
OutIterT BuildEVExpr(EVExpr::Operation operation, LabelAttribute attrib, size_t offset, OutIterT out, const IkKnowledgebase& kb) {
  const AttributeId* argv = kb.GetAttributeParamsBegin(attrib.GetIndex(), attrib.GetPosition());
  size_t argc = kb.GetAttributeParamsEnd(attrib.GetIndex(), attrib.GetPosition()) - argv;
  if (argc != 5) throw ExceptionFrom<IkIndexProcess>("Unexpected arg count for EV attribute.");
  size_t sequence = static_cast<size_t>(AttribToInt(argv[0], kb)+1); // 0-level reserved for missing entity vectors
  size_t priority = static_cast<size_t>(AttribToInt(argv[1], kb));
  AttributeId type = argv[2];
  EV::Direction dir = CharToDirection(AttribToChar(argv[3], kb));
  EV::Order order = CharToOrder(AttribToChar(argv[4], kb));
  *out = EVExpr(operation, offset, sequence, priority, type, dir, order);
  //A workaround for Solaris compiler, which thinks
  //back_inserter is creating a ref-to-temp when we
  //use *out++
  ++out;
  return out;
}
template<typename OutIterT>
OutIterT BuildEVSlotExpr(LabelAttribute attrib, size_t offset, OutIterT out, const IkKnowledgebase& kb) {
  return BuildEVExpr(EVExpr::kSlot, attrib, offset, out, kb);
}

template<typename OutIterT>
OutIterT BuildEVValueExpr(LabelAttribute attrib, size_t offset, OutIterT out, const IkKnowledgebase& kb) {
  return BuildEVExpr(EVExpr::kValue, attrib, offset, out, kb);
}

template<typename OutIterT>
OutIterT BuildEVConceptExtr(size_t offset, OutIterT out) { // add on the right side
	*out = EVExpr(EVExpr::kValue, offset, (size_t)0, (size_t)0, (AttributeId)0, EV::kRight, EV::kBack);
	++out;
	return out;
}
//Reads merged lexrep iterator from begin to end, scanning their lexreps for labels and
//scanning the labels for attributes corresponding to EV expressions. The value for an
//EV slot is determined by calling the value function val_func with the merged lexrep iterator
//associated with the EVValue attribute. In the initial implementation, this is an offset from the
//start of the sentence (always read L-R)
template<typename InIterT, typename OutIterT, typename ValFunc>
OutIterT BuildEVExprs(InIterT begin, InIterT end, OutIterT out, ValFunc val_func, const IkKnowledgebase& kb, IkIndexDebug<TraceListType>* pDebug) {
  //Get KB-specific attribute types for EVSlot() and EVValue();
  static const String kEVSlotTypeName = IkStringEncoding::UTF8ToBase("EVSlot");
  static const String kEVValueTypeName = IkStringEncoding::UTF8ToBase("EVValue");

  AttributeId evslot_attrib = kb.AttributeIdForName(kEVSlotTypeName);
  AttributeId evvalue_attrib = kb.AttributeIdForName(kEVValueTypeName);
  //We at least need one value or this language model will not produce any entity vectors.
  if (evvalue_attrib == kNoSuchAttribute) return out;

  //For each merged lexrep
  for (InIterT merged_lexrep = begin; merged_lexrep != end; ++merged_lexrep) {
    bool bIsConcept = merged_lexrep->IsConcept();
	bool bHasEV = false;
    Lexreps::const_iterator lexreps_end = merged_lexrep->LexrepsEnd();
    for (Lexreps::const_iterator lexrep = merged_lexrep->LexrepsBegin(); lexrep != lexreps_end; ++lexrep) { //For each lexrep of the merged lexrep
      //For each (final phase) label of the lexrep.
      FastLabelSet::const_iter labels_end = lexrep->LabelsEnd();
      for (FastLabelSet::const_iter label = lexrep->LabelsBegin(); label != labels_end; ++label) { //For each attribute in the label
        LabelAttributeIterator attribs_end = kb.LabelAttributesEnd(*label);
        for (LabelAttributeIterator attrib = kb.LabelAttributesBegin(*label); attrib != attribs_end; ++attrib) {
          AttributeId attrib_type = kb.GetAttributeType(attrib->GetIndex(), attrib->GetPosition());
          if (attrib_type == evslot_attrib) {
            out = BuildEVSlotExpr(*attrib, val_func(merged_lexrep), out, kb); 
          }
          else if (attrib_type == evvalue_attrib) {
			  if (!bIsConcept) { // non-concepts cannot have entity vector value information, trace & continue
				  if (pDebug) pDebug->InvalidEntityVector(*lexrep, kb);
				  continue;
			  }
            out = BuildEVValueExpr(*attrib, val_func(merged_lexrep), out, kb), bHasEV=true;
          }
        }
      }
    }
	if (bIsConcept && bHasEV == false) { // concept without Entity Vectors, add with lowest priority
		if (pDebug) pDebug->MissingEntityVector(*merged_lexrep, kb);
		out = BuildEVConceptExtr(val_func(merged_lexrep), out);
	}
  }
  return out;
}

//Turns an iterator into an offset from a begin() iterator.
//For use as value function in building the entity vector
//from offsets when scanning L-R
class OffsetFunc {
public:
  OffsetFunc(MergedLexreps::const_iterator begin) : begin_(begin) {}
  size_t operator()(MergedLexreps::const_iterator iter) {
    return iter - begin_;
  }
private:
  MergedLexreps::const_iterator begin_;
};

//Turns a reverse iterator into an offset from the rend()
//reverse iterator. For use as a value function in building
//the entity vector based on a merged lexrep offset when
//scanning R-L.

class ReverseOffsetFunc {
public:
  ReverseOffsetFunc(MergedLexreps::const_reverse_iterator rend) : rend_(rend) {}
  size_t operator()(MergedLexreps::const_reverse_iterator riter) {
    //Remember that reverse_iterators end one before the begin() element.
    return rend_ - riter - 1;
  }
private:
  MergedLexreps::const_reverse_iterator rend_;
};

void IkIndexProcess::BuildEntityVector(const MergedLexreps& in_lexreps, IkSentence::EntityVector& out_entity_vector) {
  typedef std::vector<EVExpr, PoolAllocator<EVExpr> > EVExprs;
  EVExprs ev_exprs;
  ev_exprs.reserve(in_lexreps.size());

  if (m_pKnowledgebase->GetMetadata<kEntityVectorScanDirection>() == kRightToLeft) {
    ReverseOffsetFunc rofunc(in_lexreps.rend());
    BuildEVExprs(in_lexreps.rbegin(), in_lexreps.rend(), std::back_inserter(ev_exprs), rofunc, *m_pKnowledgebase, this->m_pDebug);
  }
  else {
    OffsetFunc ofunc(in_lexreps.begin());
    BuildEVExprs(in_lexreps.begin(), in_lexreps.end(), std::back_inserter(ev_exprs), ofunc, *m_pKnowledgebase, this->m_pDebug);
  }
  
  if (ev_exprs.empty()) return; //nothing to do

  //Sort the ev_exprs by phase but otherwise preserving their order.
  std::stable_sort(ev_exprs.begin(), ev_exprs.end());
  
  //Within each offset (i.e. a single merged lexrep) stable sort
  //EVExprs by priority.
  Prioritize(ev_exprs.begin(), ev_exprs.end());
  
  //The evaluator evalutes the expressions in order, but we have to notice
  //when the sequence changes.
  EntityVectorEvaluator eve;
  eve.VectorBegin();
  eve.PhaseBegin();
  //There must be at least one member because we quit above if there was not.
  size_t cur_sequence = ev_exprs[0].GetSequence();
  size_t cur_offset = ev_exprs[0].GetOffset();
  bool offset_inserted = false;
  for (EVExprs::const_iterator i = ev_exprs.begin(); i != ev_exprs.end(); ++i) {
    //Along with all the sorting above, this logic is too complicated.
    //TODO: Encapsulate this logic in the structure of the
    //expressions.

    //New sequence?
    if (i->GetSequence() != cur_sequence) {
      eve.PhaseEnd();
      eve.PhaseBegin();
      cur_sequence = i->GetSequence();
    }
    //New offset?
    if (i->GetOffset() != cur_offset) {
      cur_offset = i->GetOffset();
      offset_inserted = false;
    }
    //Is it a value? If so, check if the offset
    //has already been inserted
    if (i->GetOperation() == EVExpr::kValue) {
      if (offset_inserted) {
	//Skip evaluation
	continue;
      }
      //Note the insertion for future expressions
      //with this offset.
      offset_inserted = true;
    }
    i->Evaluate(eve);
  }
  eve.PhaseEnd();
  eve.VectorEnd();
  eve.ReadValues(std::back_inserter(out_entity_vector));
}
