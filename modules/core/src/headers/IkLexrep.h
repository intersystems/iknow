// iknLexrep.h: interface for the iknLexrep class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_IKNLEXREP_H__67214598_FD04_4D6E_AB7E_C8F12D46B866__INCLUDED_)
#define AFX_IKNLEXREP_H__67214598_FD04_4D6E_AB7E_C8F12D46B866__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "IkExportCore.h"
#include "IkObject.h"
#include "IkLabel.h"
#include "IkOntologyElement.h"
#include "IkDocumentPart.h"
#include "SafeString.h"
#include <set>
#include <vector>
#include <algorithm>
#include <bitset>
#include "FastSet.h"
#include "IkTypes.h"
#include "StringPool.h"
#include "IndexMap.h"
#include "IkStringAlg.h"
#include "PoolAllocator.h"
#include <iterator>
#include "Reader.h"
#include "unicode/uchar.h"

namespace iknow
{
  namespace core 
  {
    class IkKnowledgebase; //Forward declaration

    //It's useful to be able to track the labels in play over a certain period of time. Thankfully, we're single threaded,
    //so static context is fine.
    class LexrepContext {
    public:
      //We need the object to live in single location, but we don't want to have
      //to call across a shared library boundary every time we need it, so we
      //look it up a single time once the symbols have all been resolved.
      static FastLabelBitSet& SeenLabels() {
	static FastLabelBitSet* seen_labels_ptr = 0;
	if (!seen_labels_ptr) seen_labels_ptr = SeenLabelsPointer();
	return *seen_labels_ptr;
      }
    private:
      static FastLabelBitSet* SeenLabelsPointer();
    };

    //Column-oriented storage class
    class LexrepStore {
    public:
      typedef size_t Offset;
      LexrepStore(size_t size) : next_offset_(0) {
        Allocate(kMaxPhase); //Ensure we always have last phase allocated
        Resize(size);
      }
      Offset Allocate() {
        if (next_offset_ >= Capacity()) {
          Resize(Capacity() * 2);
        }
        return next_offset_++;
      }
      void SetLabel(Offset n, FastLabelSet::Index i, Phase p = kMaxPhase) {
        EnsureIsAllocated(p);
        label_sets_[p][n].InsertAtIndex(i);
      }
      void SetLabels(Offset n, FastLabelSet labels, Phase p = kMaxPhase) {
        EnsureIsAllocated(p);
        label_sets_[p][n] = labels;
      }

      void ClearLabel(Offset n, FastLabelSet::Index i, Phase p = kMaxPhase) {
        if (IsAllocated(p)) label_sets_[p][n].RemoveAtIndex(i); //If not allocated, by definition not set and nothing to do.
      }
      void ClearAllLabels(Offset n, Phase p = kMaxPhase) {
        if (IsAllocated(p)) label_sets_[p][n].Clear();
      }

      const FastLabelSet& GetLabelSet(Offset n, Phase p = kMaxPhase) const {
        static const FastLabelSet empty_set;
        if (!IsAllocated(p)) return empty_set;
        return label_sets_[p][n];
      }
      iknow::base::String*& GetNormalizedValue(Offset n) { return normalized_values_[n]; }
      iknow::base::String* const& GetNormalizedValue(Offset n) const { return normalized_values_[n]; }
    private:
      //We allocate phases lazily, since many won't end up being used.
      bool IsAllocated(Phase p) const {
#ifdef SOLARIS
	return phase_bitmap_.test(p);
#else
	return phase_bitmap_[p];
#endif
      }
      void Allocate(Phase p) {
		phase_bitmap_.set(p);
      }
      //When we allocate a phase, we size up its vector to the current capacity.
      void EnsureIsAllocated(Phase p) {
		if (!IsAllocated(p)) {
			label_sets_[p].resize(Capacity());
			Allocate(p);
		}
      }
      //The last phase is always allocated, so we can use its size as a proxy for the capacity
      size_t Capacity() {
		return label_sets_[kMaxPhase].size();
      }
      //Resizing involves scanning for allocated phases and only resizing those. Unallocated phases will resize
      //lazily once they're allocated.
      void Resize(Offset new_max) {
		for (Phase i=0; i < kPhaseCount; ++i) {
			if (IsAllocated(i)) label_sets_[i].resize(new_max);
		}
		normalized_values_.resize(new_max);
      }
      Offset next_offset_;
      typedef std::vector<FastLabelSet, iknow::base::PoolAllocator<FastLabelSet> > FastLabelSets;
      typedef std::bitset<kPhaseCount> PhaseBitmap; // The phase bitmap tracks the allocated phases.
      PhaseBitmap phase_bitmap_;
      FastLabelSets label_sets_[kPhaseCount];
      typedef std::vector<iknow::base::String*> Strings;
      Strings normalized_values_;
    };

    //! The lexrep class
    /*!
     *	This class represent objects thet are lexreps
     *	Exceptions: IkException 
     */
    class CORE_API IkLexrep
    {
    public:

      typedef IkLabel::Type Type;

      IkLexrep(Type type, const IkKnowledgebase* kb,
        const iknow::base::Char* val_begin,
        const iknow::base::Char* val_end,
	    const iknow::base::String& strNormalizedValue
	    ) :
        offset_(GetLexrepStore().Allocate()),
        m_type(type), m_kb(kb),
		m_summaryRelevance(0), m_id(NextId()), is_annotated_(false),
		text_begin_(val_begin), text_end_(val_end),
        certainty_value_('\0') {
            NormalizedValue() = PoolString(strNormalizedValue);
        }
      IkLexrep(Type type, const IkKnowledgebase* kb, //Build a lexrep with one label index
        const iknow::base::Char* val_begin,
        const iknow::base::Char* val_end,
	    const iknow::base::String& strNormalizedValue,
	    FastLabelSet::Index label_index
		) :
        offset_(GetLexrepStore().Allocate()),
        m_type(type), m_kb(kb),
		m_summaryRelevance(0), m_id(NextId()), is_annotated_(false),
		text_begin_(val_begin), text_end_(val_end),
        certainty_value_('\0') {
          AddLabelIndex(label_index);
          NormalizedValue() = PoolString(strNormalizedValue);
        }
      IkLexrep(Type type, const IkKnowledgebase* kb, //Build a lexrep with one label index, using Char pointers rather than string objects
	    const iknow::base::Char* val_begin,
	    const iknow::base::Char* val_end,
	    const iknow::base::Char* norm_val_begin,
	    const iknow::base::Char* norm_val_end,
	    FastLabelSet::Index label_index
		) :
        offset_(GetLexrepStore().Allocate()),
        m_type(type), m_kb(kb),
		m_summaryRelevance(0), m_id(NextId()), is_annotated_(false),
		text_begin_(val_begin), text_end_(val_end),
        certainty_value_('\0') {
          AddLabelIndex(label_index);
          NormalizedValue() = PoolString(norm_val_begin, norm_val_end);
        }
      IkLexrep() {} //Null constructor for reserving vectors

      Type GetLexrepType(void) const { return m_type; }
      void SetLexrepType(Type type) { m_type = type; }

      const IkKnowledgebase* GetKnowledgebase() const { return m_kb; }
	  void SetKnowledgebase(const IkKnowledgebase* kb_switch) { m_kb = kb_switch; }

      const iknow::base::String& GetNormalizedValue() const { return *NormalizedValue(); }
      iknow::base::String& GetNormalizedValue() { return *NormalizedValue(); }
      void SetNormalizedValue(const iknow::base::String& value) { NormalizedValue() = PoolString(value); }
      const FastLabelSet& GetLabels(Phase p = kMaxPhase) const { return LabelSet(p); }
	  FastLabelTypeSet GetTypeLabels(Phase p = kMaxPhase) const;

      void SetLabels(FastLabelSet& labels, Phase p = kMaxPhase) { return GetLexrepStore().SetLabels(offset_, labels, p); }
      bool Contains(FastLabelSet::Index label_index, Phase p = kMaxPhase) const { return LabelSet(p).Contains(label_index); }

	  bool hasSBeginLabel(Phase p) const;
	  bool hasSEndLabel(Phase p) const;
	  void setSBeginLabel();
	  void setSEndLabel();
	  void resetSBeginLabel() {
		  ClearAllLabels();
		  setSBeginLabel();
	  }
	  void resetSEndLabel() {
		  ClearAllLabels();
		  setSEndLabel();
	  }

      //Returns the index of a label on this lexrep which contains the attribute
      FastLabelSet::Index FindAttributeWithType(AttributeId attribute, Phase p = kMaxPhase) const;
      LabelAttribute GetFirstLabelAttributeWithType(AttributeId type, Phase p = kMaxPhase) const;

      FastLabelSet::const_iter LabelsBegin(Phase p = kMaxPhase) const { return LabelSet(p).begin(); }
      FastLabelSet::const_iter LabelsEnd(Phase p = kMaxPhase) const { return LabelSet(p).end(); }

      //TODO: TRW, May want to optimize for the use, which is a check for a single label.
      size_t NumberOfLabels(Phase p = kMaxPhase) const { return LabelSet(p).Size(); }
      FastLabelSet::Index GetLabelIndexAt(size_t position, Phase p = kMaxPhase) const { return LabelSet(p).At(position); } 

      //TODO: Don't bother adjusting phases past the current one?
      void AddLabelIndex(const FastLabelSet::Index label_index) { //Add to all relevant phases
        const Phase* phase = GetPhasesBegin(label_index);
        const Phase* end = GetPhasesEnd(label_index);
        for (; phase != end; ++phase) {
          SetLabel(label_index, *phase);
        }
        LexrepContext::SeenLabels().InsertAtIndex(label_index); //Note that we saw it.
      }
      template<typename IterT>
      void AddLabelIndices(IterT begin, IterT end) {
        for(;begin != end; ++begin) {
          AddLabelIndex(*begin);
        }
      }
      void RemoveLabelIndex(const FastLabelSet::Index label_index) { //Remove from all relevant phases
        const Phase* phase = GetPhasesBegin(label_index);
        const Phase* end = GetPhasesEnd(label_index);
        for (; phase != end; ++phase) {
          ClearLabel(label_index, *phase);
        }
      }
	  void RemoveLabelType(const FastLabelTypeSet::Index label_type, Phase p); // remove all labels from the specified type that exist on the p Phase
	  void RemoveLabelType(const FastLabelTypeSet::Index label_type); // remove all labels from the specified type

      void ClearAllLabels() { //Clear all labels from every phase.
        for (Phase p = 0; p < kPhaseCount; ++p) {
          ClearAllLabels(p);
        }
      }
	  void ClearLabels(Phase p); //Clear labels present in a particular phase, including clearing their representation in other phases to which they belong. 

      size_t GetTokenCount() const {
#ifdef SOLARIS
	size_t n = 0;
	std::count(GetNormalizedValue().begin(), GetNormalizedValue().end(), ' ', n);
	return n + 1;
#else
	return std::count(GetNormalizedValue().begin(), GetNormalizedValue().end(), ' ') + 1;
#endif
      }
	  size_t GetLiteralTokenCount(bool first_in_merge) const;

      iknow::base::String GetText() const { return GetValue(); } // inherited from IkDocumentPart
	  bool bHasNoLiteral(void) { return (text_begin_ == NULL || text_end_ == NULL || text_begin_ == text_end_); }
	  iknow::base::String GetValue(void) const;
      iknow::base::String GetNormalizedText() const { return GetNormalizedValue(); } // inherited from IkDocumentPart

      double GetSummaryRelevance() const { return m_summaryRelevance; }

      void increaseSummaryRelevance(size_t value) { m_summaryRelevance += value; }
      size_t GetId() const { return m_id; }
      static void ResetIdCounter() { GetIdCounter() = 1; }

      static void SetStringPool(iknow::base::StringPool* string_pool) { string_pool_ = string_pool; }
	  static void SetTextBuffer(const iknow::base::Char* text_buffer) { text_buffer_ = text_buffer; }
      static iknow::base::String* PoolString(const iknow::base::Char* begin, const iknow::base::Char* end) { return GetStringPool()->Allocate(begin, end); }
      static iknow::base::String* PoolString(const iknow::base::String& s) { return GetStringPool()->Allocate(s); }
	  static void SetLexrepStore(LexrepStore& store) { *GetLexrepStorePointer() = &store; }

	  const iknow::base::Char* GetTextPointerBegin() const { return text_begin_; }
	  const iknow::base::Char* GetTextPointerEnd() const { return text_end_; }
	  void SetTextPointerBegin(const iknow::base::Char* text_begin) { text_begin_ = text_begin; }
	  void SetTextPointerEnd(const iknow::base::Char* text_end) { text_end_ = text_end; }
	  
	  void SetAnnotated(bool mark_annotation) { is_annotated_ = mark_annotation; }
	  bool IsAnnotated(void) { return is_annotated_; }

	  // const char* GetMetaData() const { return meta_data_; }
	  // void SetMetaData(const char* meta_data) { meta_data_ = meta_data; }

      std::string GetMetaData() const;
      void SetMetaData(const char* meta_data);
      char GetCertainty() const {
          return certainty_value_;
      }

    protected:
      const FastLabelSet& LabelSet(Phase p = kMaxPhase) const { return GetLexrepStore().GetLabelSet(offset_, p); }
      void SetLabel(FastLabelSet::Index i, Phase p = kMaxPhase)  { return GetLexrepStore().SetLabel(offset_, i, p); }
      void ClearLabel(FastLabelSet::Index i, Phase p = kMaxPhase) { return GetLexrepStore().ClearLabel(offset_, i, p); }
	  void ClearAllLabels(Phase p) { return GetLexrepStore().ClearAllLabels(offset_, p); }

      iknow::base::String*& NormalizedValue() const { return GetLexrepStore().GetNormalizedValue(offset_); }
      
      const Phase* GetPhasesBegin(FastLabelSet::Index index) const;
      const Phase* GetPhasesEnd(FastLabelSet::Index index) const;

      //Because this offset will be copied by value in the default copy constructor,
      //copies will share the content in the store. Typically this is fine, since a copied lexrep
      //is almost invariably being copied into a later phase and the original is no longer needed.
      LexrepStore::Offset offset_;

      static size_t& GetIdCounter() {
        static size_t id = 1;
        return id;
      }
      static size_t NextId() { return GetIdCounter()++; }

      double computeSummaryRelevanceCore() const // inherited from IkDocumentPart
      {	return 0; }
      Type m_type;
      const IkKnowledgebase* m_kb;
      //A fast label set for Contains()
      double m_summaryRelevance;
      size_t m_id;

    private:
      bool is_annotated_; // if true, lexrep is annotated
      const iknow::base::Char *text_begin_, *text_end_; // text input pointers

      // metadata values
      char certainty_value_; // certainty range '0' to '9', '\0' = undefined

	  static const iknow::base::Char* text_buffer_;
      static iknow::base::StringPool* string_pool_;
      static iknow::base::StringPool* GetStringPool() {
        if (!string_pool_) throw ExceptionFrom<IkLexrep>("No string pool specified for IkLexrep.");
          return string_pool_;
      }
      //LexrepStore
      static LexrepStore** GetLexrepStorePointer();
      static LexrepStore& GetLexrepStore() {
        //We want the store pointer in a static location in the core library, so that
        //all libraries which use it are referencing the same one, but we also want a header
        //(and potentially "inlined") location for performance.
        static LexrepStore** local_pointer = 0;
        if (!local_pointer) local_pointer = GetLexrepStorePointer();
        return **local_pointer;
      }
      friend class IkMergedLexrep; //for access to the string pool.
    };
    typedef std::vector<IkLexrep, iknow::base::PoolAllocator<IkLexrep> > Lexreps;
  }
}

#endif // !defined(AFX_IKNLEXREP_H__67214598_FD04_4D6E_AB7E_C8F12D46B866__INCLUDED_)

