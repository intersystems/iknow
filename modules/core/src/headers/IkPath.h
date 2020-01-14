#ifndef IKNOW_CORE_IKPATH_H_
#define IKNOW_CORE_IKPATH_H_
#include <vector>
#include <algorithm>
#include <iterator>
#include "utlExceptionFrom.h"
#include "PoolAllocator.h"

namespace iknow
{
  namespace core
  {
    class IkMergedLexrep;

    namespace path {


      //Identify CRC and path patterns as offsets from the
      //input range (a sentence).
      typedef size_t Offset;
      inline Offset NullOffset() { return Offset(-1); }
      struct CRC {
	CRC() : master(NullOffset()),
		relation(NullOffset()),
		slave(NullOffset()) {}

	void Clear() {
	  (*this) = CRC();
	}

	void Rotate() {
	  master = slave;
	  relation = NullOffset();
	  slave = NullOffset();
	}

	Offset Begin() const {
	  using std::min;
	  using std::max; //Windows has macros, alas
	  return min(master, min(relation, slave)); }
	Offset End() const {
	  using std::max;
	  if (HasMaster() && HasSlave()) {
	    return max(master, slave);
	  }
	  else if (HasMaster() && !HasSlave()) {
	    return max(master, relation);
	  }
	  else {
	    return max(slave, relation);
	  }
	}

	bool DoesPathContinue(const CRC& next) const {
	  return HasSlave() && slave == next.master;
	}

	bool IsComplete() const { return HasMaster() && HasRelation() && HasSlave(); }

	bool CanAddConcept() const { return NeedsMaster() || NeedsSlave(); }
	bool CanAddRelation() const { return NeedsRelation(); }

	bool CanPropagateForward(const CRC& next) const {
	  return HasSlave() && next.NeedsMaster();
	}

	bool CanPropagateBackward(const CRC& prev) const {
	  return HasMaster() && prev.NeedsSlave();
	}

	void PropagateForward(CRC& next) const {
	  next.AddMaster(slave);
	}

	void PropagateBackward(CRC& prev) const {
	  prev.AddSlave(master);
	}

	void AddConcept(Offset n) {
	  if (NeedsMaster()) {
	    master = n;
	  }
	  else if (NeedsSlave()) {
	    slave = n;
	  }
	  else {
	    throw ExceptionFrom<CRC>("Tried to stuff a concept into a full CRC.");
	  }
	}

	void AddMaster(Offset n) {
	  if (NeedsMaster()) {
	    master = n;
	  }
	  else {
	    throw ExceptionFrom<CRC>("Tried to add a master to a CRC that already has one.");
	  }
	}

	void AddSlave(Offset n) {
	  if (NeedsSlave()) {
	    slave = n;
	  }
	  else {
	    throw ExceptionFrom<CRC>("Tried to add a slave to a CRC that already has one.");
	  }
	}

	void AddRelation(Offset n) {
	  if (NeedsRelation()) {
	    relation = n;
	  }
	  else {
	    throw ExceptionFrom<CRC>("Tried to stuff a relation into a full CRC.");
	  }
	}

	bool NeedsMaster() const { return !HasMaster(); }
	bool NeedsRelation() const { return !HasRelation(); }
	bool NeedsSlave() const { return !HasSlave(); }

	bool HasMaster() const { return master != NullOffset(); };
	bool HasRelation() const { return relation != NullOffset(); }
	bool HasSlave() const { return slave != NullOffset(); }

	template<typename TLexrepIter>
	static IkMergedLexrep* OffsetToLexrep(Offset offset, const TLexrepIter& sentence_begin) {
	  if (offset == NullOffset()) return 0;
	  return &(*(sentence_begin + offset));
	}

	template<typename TLexrepIter>
	IkMergedLexrep* MasterLexrep(const TLexrepIter& sentence_begin) const {
	  return OffsetToLexrep(master, sentence_begin);
	}

	template<typename TLexrepIter>
	IkMergedLexrep* RelationLexrep(const TLexrepIter& sentence_begin) const {
	  return OffsetToLexrep(relation, sentence_begin);
	}

	template<typename TLexrepIter>
	IkMergedLexrep* SlaveLexrep(const TLexrepIter& sentence_begin) const {
	  return OffsetToLexrep(slave, sentence_begin);
	}

	Offset master;
	Offset relation;
	Offset slave;
      };

      typedef std::vector<CRC, iknow::base::PoolAllocator<CRC> > CRCs;
      typedef std::vector<Offset, iknow::base::PoolAllocator<Offset> > Offsets;
      
      //Convenience functions for "propagating" masters and slaves across the CRC chain. That is,
      //in a single path with two CRCs
      //C1 R1 C2, C3 R2 C4
      //C2 must equal C3
      inline bool TryPropagateForward(const CRCs::iterator it_crc, const CRCs::iterator end) {
	const CRCs::iterator next = it_crc + 1;
	if (next == end) return true; //"successfully" propagates off the deep end.
	if (!it_crc->CanPropagateForward(*next)) return false; //no slot to propagate into.
	it_crc->PropagateForward(*next);
	return true;
      }
      
      inline bool TryPropagateBackward(const CRCs::iterator it_crc, const CRCs::iterator begin) {
	if (it_crc == begin) return true; //"successfully" propagates off the deep end.
	const CRCs::iterator prev = it_crc - 1;
	if (!it_crc->CanPropagateBackward(*prev)) return false; //no slot to propagate into.
	it_crc->PropagateBackward(*prev);
	return true;
      }
      
      //Convenience functions for locating and adding potential concepts in relative positions
      //TODO: The proliferation of arguments here suggests wrapping this up in a class...
      template<typename TLexIter, typename TNeedsPred, typename TAddFunc>
      bool TryAddX(const CRCs::iterator it_crc, const TLexIter it_rel, ptrdiff_t offset, const TLexIter begin, const TLexIter end, TNeedsPred needs, TAddFunc add) {
	//In order to eliminate code duplication purely for different search directions,
	//we need to check for some edge cases and set up some parameters for the search.
	if (!needs(*it_crc) || offset == 0 || begin == end) return false;
	ptrdiff_t step = offset > 0 ? 1 : -1;
	TLexIter stop = offset > 0 ? end - 1 : begin;
	TLexIter lexrep = it_rel;
	//Now we scan we've seen the right number number of regular concept slots right (positive) or
	//left (negative). Or we hit a stop condition.
	while (offset != 0) {
	  if (lexrep == stop) return false;
	  lexrep += step;
	  //Never cross a relation to find a concept.
	  //TODO: May have to adjust this restriction for CCR languages.
	  if (lexrep->IsRelation()) return false;
	  if (lexrep->IsConcept() && !(lexrep->IsObject() || lexrep->IsSubject())) {
	    offset -= step;
	  }
	}
	//Once the offset hits 0, we're at the right lexrep.
	add(*it_crc, lexrep - begin);
	return true;
      }

      template<typename TLexIter>
      bool TryAddSlave(const CRCs::iterator it_crc, TLexIter it_rel, ptrdiff_t offset, TLexIter begin, TLexIter end) {
	return TryAddX(it_crc, it_rel, offset, begin, end, [](CRC& crc) { return crc.NeedsSlave(); }, [](CRC& crc, Offset offset) { crc.AddSlave(offset); });
      }

      template<typename TLexIter>
      bool TryAddMaster(const CRCs::iterator it_crc, TLexIter it_rel, ptrdiff_t offset, TLexIter begin, TLexIter end) {
	return TryAddX(it_crc, it_rel, offset, begin, end, [](CRC& crc) { return crc.NeedsMaster(); }, [](CRC& crc, Offset offset) { crc.AddMaster(offset); });
      }


      template<typename TLexIter,    //The input type, lexreps in the sentence
	       typename TCRCIter    //CRCs to output are written here.
	       >
      TCRCIter DetectCRCs(const TLexIter begin, const TLexIter end, TCRCIter crc_out, const ChainPattern pattern) {
	//Create a CRC for each relation to later "hang" concepts on to.
	CRCs crcs;
	crcs.reserve(end - begin);

	//"primary" relations go to the front of the line
	for (TLexIter i = begin; i < end; ++i) {
	  if (i->IsPrimaryRelation()) {
	    const size_t offset = i - begin;
	    CRC crc;
	    crc.AddRelation(offset);
	    crcs.push_back(crc);
	  }
	}

	//Remaining relations follow
	for (TLexIter i = begin; i < end; ++i) {
	  if (i->IsRelation() && !i->IsPrimaryRelation()) {
	    const size_t offset = i - begin;
	    CRC crc;
	    crc.AddRelation(offset);
	    crcs.push_back(crc);
	  }
	}

	//The next CRC with an empty master
	CRCs::iterator it_xrc = crcs.begin();
	//The next CRC with an empty slave
	CRCs::iterator it_crx = crcs.begin();

	//Subject and object concepts are similarly filled
	//in first.
	for (TLexIter i = begin; i < end; ++i) {
	  const size_t offset = i - begin;
	  if (i->IsSubject() && it_xrc != crcs.end()) {
	    (it_xrc++)->AddMaster(offset);
	  }
	  if (i->IsObject() && it_crx != crcs.end()) {
	    (it_crx++)->AddSlave(offset);
	  }
	}

	//Fill in the remainder, using the linguistic pattern of the relation
	//to find the appropriate concept.
	for (CRCs::iterator it_crc = crcs.begin(); it_crc < crcs.end(); ++it_crc) {
	  const size_t relation_offset = it_crc->relation;
	  TLexIter it_rel = begin + relation_offset;
	  switch (pattern) {
	  case kCRCPattern:
	    TryAddMaster(it_crc, it_rel, -1, begin, end);
	    TryAddSlave(it_crc, it_rel, +1, begin, end);
	    break;
	  case kCCRPattern:
	    TryAddMaster(it_crc, it_rel, -2, begin, end);
	    TryAddSlave(it_crc, it_rel, -1, begin, end);
	    break;
	  }
	}

	//Emit CRCs
	return std::copy(crcs.begin(), crcs.end(), crc_out);
      }
    }
    
    
    
    class CORE_API IkPath
    {
    public:
      //Efficient construction. 
      IkPath(path::CRCs::const_iterator crcs_begin, path::CRCs::const_iterator crcs_end,
	     path::Offsets::const_iterator offsets_begin, path::Offsets::const_iterator offsets_end) {
          //Now add "relevant" offsets and CRC offsets to offsets, sort and deduplicate.
          offsets_.reserve((crcs_end - crcs_begin) * 3 + (offsets_end - offsets_begin));
          offsets_.insert(offsets_.begin(), offsets_begin, offsets_end);
          for (;crcs_begin != crcs_end; ++crcs_begin) {
            if (crcs_begin->HasMaster()) offsets_.push_back(crcs_begin->master);
            if (crcs_begin->HasRelation()) offsets_.push_back(crcs_begin->relation);
            if (crcs_begin->HasSlave()) offsets_.push_back(crcs_begin->slave);
          }
          std::sort(offsets_.begin(), offsets_.end());
          offsets_.erase(std::unique(offsets_.begin(), offsets_.end()), offsets_.end());
      }
      //Contiguous paths
      IkPath(path::Offset begin, path::Offset end) {
        offsets_.reserve(end - begin);
        for (; begin != end; ++begin) {
          offsets_.push_back(begin);
        }
      }
	  IkPath(path::Offsets::const_iterator offsets_begin, path::Offsets::const_iterator offsets_end) {
		  offsets_.reserve(offsets_end - offsets_begin);
		  offsets_.insert(offsets_.begin(), offsets_begin, offsets_end);
	  }
      path::Offsets::const_iterator OffsetsBegin() const { return offsets_.begin(); }
      path::Offsets::const_iterator OffsetsEnd() const { return offsets_.end(); }
      size_t Size() const { return offsets_.size(); }

    private:
      //The offset list can contain "path relevant non-relevants".
      path::Offsets offsets_;
    };
  }
}

#endif //IKNOW_CORE_IKPATH_H_
