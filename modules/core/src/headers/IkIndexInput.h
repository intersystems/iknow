// IndexInput.h: interface for the CIndexInput class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_INDEXINPUT_H__A4D17E07_25E8_4BF9_B1E5_2DE23457EABA__INCLUDED_)
#define AFX_INDEXINPUT_H__A4D17E07_25E8_4BF9_B1E5_2DE23457EABA__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#ifdef WIN32
// Disable stl warnings
#pragma warning (push)
#pragma warning (disable: 4786)
#endif

#include "IkExportCore.h"
#include "SafeString.h"
#include "IkTypes.h"

#include <list>
#include <map>

namespace iknow
{
  namespace core
  {
	  class CORE_API IkIndexInput
	  {
	  public:
		  struct IknowAnnotation {
			  IknowAnnotation(size_t start, size_t stop, const char* annotation) : start_(start), stop_(stop), annotation_(annotation) {};
			  size_t start_, stop_;
			  std::string annotation_;
		  };
		  typedef struct IknowAnnotation IknowAnnotation_t;
		  typedef std::map<size_t, IknowAnnotation_t> mapInputAnnotations_t;

		  IkIndexInput(iknow::base::String* strText) : m_pText(strText) {
			  size_t teller =  m_mapAnnotations.count((size_t) 0);
			  teller = (size_t) 0;
		  }
		  ~IkIndexInput() {
			  if (m_pText) {
				  m_pText = NULL;
			  }
		  }

		  void SetString(iknow::base::String const * const pText);
		  iknow::base::String const * GetString(void) const {
			return m_pText;
		}
	  std::string const *GetAnnotationLabels(const iknow::base::Char *begin, const iknow::base::Char *end) const;
	  bool IsAnnotated(size_t offset) const {
		  return (m_mapAnnotations.count(offset) > 0);
	  }

	  size_t GetAnnotationStop(size_t start) {
		  mapInputAnnotations_t::const_iterator it = m_mapAnnotations.find(start);
		  if (it == m_mapAnnotations.end()) return (static_cast<size_t>(0)); // non-existing
		  return it->second.stop_;
	  }
	  std::string const *GetAnnotationLabels(size_t offset) const {
		  mapInputAnnotations_t::const_iterator it = m_mapAnnotations.find(offset);
		  if (it == m_mapAnnotations.end()) return (NULL); // non-existing
		  return &(it->second.annotation_);
	  }
	  void AddAnnotation(size_t start, size_t stop, const char* label) {
		  m_mapAnnotations.insert(std::make_pair(start, IknowAnnotation_t(start, stop, label)));
	  }
    protected:
      iknow::base::String const * m_pText; 
	  
	private:
		mapInputAnnotations_t m_mapAnnotations;
		void operator=(const IkIndexInput&);
    };
  }
}

#ifdef WIN32
#pragma warning (pop)
#endif

#endif // !defined(AFX_INDEXINPUT_H__A4D17E07_25E8_4BF9_B1E5_2DE23457EABA__INCLUDED_)

