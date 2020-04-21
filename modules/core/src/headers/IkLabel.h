// IkLabel.h: interface for the IkLabel class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_IKLABEL_H__E07804E6_F5A6_45A2_AE35_0BC3D45483AE__INCLUDED_)
#define AFX_IKLABEL_H__E07804E6_F5A6_45A2_AE35_0BC3D45483AE__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#ifdef WIN32
// Disable stl warnings
#pragma warning (push)
#pragma warning (disable: 4786)
#endif

#include "IkExportCore.h"
#include "IkObject.h"
#include "FastSet.h"
#include "IkTypes.h"
#include "SafeString.h"
#include "PoolAllocator.h"
#include "utlExceptionFrom.h"
#include <vector>
#include <cstdlib>

namespace iknow
{
  namespace core
  {
    //! The label class
    //Labels as objects are used very rarely, really only for tracing.
    //This file/class now mostly holds type information for the index-based
    //labels used prominently throughout the engine for performance.
    static const size_t kMaxLabelCount = 1024;
    //Use a byte for label indexes
    typedef iknow::base::SmallSet<2, short> FastLabelSet, FastLabelTypeSet;
    typedef iknow::base::FastSet<kMaxLabelCount, short> FastLabelBitSet;

    typedef std::vector<FastLabelSet::Index, iknow::base::PoolAllocator<FastLabelSet::Index> > PooledLabelVector;

    //Phases
    typedef unsigned char Phase;
    static const size_t kPhaseCount = 100;
    static const Phase kMaxPhase = kPhaseCount - 1;
    static const char * const kDefaultPhases = "$";

    inline Phase PhaseFromString(const std::string& phase_str) {
      if (phase_str == "$" || phase_str.empty()) return kMaxPhase;
      int phase_int = std::atoi(phase_str.c_str());
      if (phase_int < 0 || phase_int > kMaxPhase) throw ExceptionFrom<Phase>("Illegal phase value.");
      return static_cast<Phase>(phase_int);
    }

    //Attributes (structured properties of labels)
    typedef unsigned short AttributeId;
    const AttributeId kNoSuchAttribute = static_cast<AttributeId>(-1);

    class IkKnowledgebase;

    class CORE_API IkLabel  : public iknow::base::IkObject
    {
    public:
      enum Type { Unknown, Nonrelevant, Ambiguous, Attribute, Concept, Relation, BeginConcept, EndConcept, BeginEndConcept, BeginRelation, EndRelation, BeginEndRelation, Literal, Other, PathRelevant };
      //The index used to break label lists into chunks
      //There's a little bit of a hack here since
      //FastLabelSet::npos is also -1. But that's OK, since
      //the use of npos (in rules) and this feature (in lexreps)
      //don't overlap. We actually have 127 negative indexes we
      //could use.
      static FastLabelSet::Index BreakIndex() {	return static_cast<FastLabelSet::Index>(-1); } 
      IkLabel();/*!< Constructor. */
      
      /*!
       *	Constructor
       *	\param strName: name of the label
       *	\param nType: the type of the label
       */
      IkLabel(const iknow::base::String& strName, Type nType);
      virtual ~IkLabel();/*!< Destructor. */

      iknow::base::String GetName(void) const { return m_strName; }
      //We can save a string copy in the very hot MatchLabel/Contains loop if we return a reference
      const iknow::base::String& GetNameRef(void) { return m_strName; }
      Type GetLabelType(void) const { return m_nType; }

      bool operator< (const IkLabel& other) const { // For putting IkLabels in ordered containers
        if (m_nType < other.m_nType) return true;
        if (m_nType > other.m_nType) return false;
        return m_strName < other.m_strName; 
      }
      static iknow::core::IkLabel::Type TypeStringToType(const std::string& typeString);
      static std::string LabelTypeToString(iknow::core::IkLabel::Type label_type);

    private:
      iknow::base::String m_strName;
      Type m_nType;
    };
    
    const IkLabel& BeginLabel();
    const IkLabel& EndLabel();

    
    //LabelAttribute (a label and an attribute position)
    class LabelAttribute {
    public:
      LabelAttribute(FastLabelSet::Index index, size_t position) : index_(index), position_(position) {}
      LabelAttribute(const LabelAttribute& other) : index_(other.index_), position_(other.position_) {}
      LabelAttribute() : index_(FastLabelSet::NPos()), position_(0) {}

      bool is_valid() const {
	return index_ != FastLabelSet::NPos();
      }
      LabelAttribute& operator=(const LabelAttribute& other) {
	index_ = other.index_;
	position_ = other.position_;
	return *this;
      }
      bool operator==(const LabelAttribute& other) const {
	return index_ == other.index_ && position_ == other.position_;
      }
      bool operator!=(const LabelAttribute& other) const {
	return !(*this == other);
      }
      size_t GetPosition() const {
	return position_;
      }
      size_t& GetPosition() {
	return position_;
      }
      FastLabelSet::Index GetIndex() const {
	return index_;
      }
    private:
      FastLabelSet::Index index_;
      size_t position_;
    };

    class LabelAttributeIterator : public std::iterator<std::input_iterator_tag, LabelAttribute> {
    public:
      explicit LabelAttributeIterator(const LabelAttribute& attribute) : attribute_(attribute) {}
      LabelAttributeIterator(const LabelAttributeIterator& other) : attribute_(other.attribute_){}
      LabelAttributeIterator& operator=(const LabelAttributeIterator& other) {
	attribute_ = other.attribute_;
	return *this;
      }
      bool operator==(const LabelAttributeIterator& other) const {
	return attribute_ == other.attribute_;
      }
      bool operator!=(const LabelAttributeIterator& other) const {
	return !(*this == other);
      }
      LabelAttribute operator*() const {
	return attribute_;
      }
      const LabelAttribute* operator->() const {
	return &attribute_;
      }
      LabelAttributeIterator& operator++() {
	++(attribute_.GetPosition());
	return *this;
      }
      LabelAttributeIterator operator++(int) {
	LabelAttributeIterator orig(*this);
	++(*this);
	return orig;
      }
    private:
      LabelAttribute attribute_;
    };


  }
}



#ifdef WIN32
#pragma warning (pop)
#endif

#endif // !defined(AFX_IKLABEL_H__E07804E6_F5A6_45A2_AE35_0BC3D45483AE__INCLUDED_)

