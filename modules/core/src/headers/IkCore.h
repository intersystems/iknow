// Core.h: interface for the CCore class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_CORE_H__69068F13_B0F3_4B0A_BC06_C7B6A5F27BD0__INCLUDED_)
#define AFX_CORE_H__69068F13_B0F3_4B0A_BC06_C7B6A5F27BD0__INCLUDED_

#include "IkExportCore.h"
#include "IkObject.h"
#include "IkTypes.h"

//shared objects
#include "IkKnowledgebase.h" 
#include "IkOntology.h"

//processes
//#include "IkIndexProcess.h"

//input-output
#include "IkIndexInput.h"
#include "IkIndexOutput.h"
#include "IkIndexDebug.h"

#ifdef WIN32
// Disables stl warnings
#pragma warning(push)
#pragma warning(disable: 4786)
#pragma warning(disable:4251)
#endif

namespace iknow
{
  namespace core
  {

    class IkIndexProcess;
		
    //! Core class, the heart and soul of the engine
    /*!
     *	Class handles most (all) communication with the outside world.
     *	Exceptions: IkException 
     */
    class CORE_API IkCore : iknow::base::IkObject
    {
    public:
      /*!
       *	Constructor
       *	\param languageKbMap: collection of multiple kb's with their language
       */
      IkCore();

      virtual ~IkCore();/*!< Destructor. */

      // workaround for not being able to pass pointers inside std maps in vc6
      void FinishConstruction();
      /*{
	m_pIndexProcess = new IkIndexProcess(m_languageKbMap, m_pOntology);
	}*/

      /*!
       *	Start the indexing process
       *	\param pInput: pointer to a IkIndexInput, so the input
       *	\param pOutput: pointer to a IkIndexOutput, so the output
       *	\param nMaxConceptSize: the maximum concept-size
       *	\exception IkException: UNKNOWN ERROR
       */
      void Index(IkIndexInput* pInput, IkIndexOutput* pOutput, IkIndexDebug* pDebug, bool mergeRelations, bool binaryMode, bool delimitedSentences, size_t max_concept_cluster_length=0, IkKnowledgebase* pUdct=NULL);

      // workaround for not being able to pass pointers inside std maps in vc6
      void AddToLanguageKbMap(iknow::base::String const & key, IkKnowledgebase const * const & value)
      {
        m_languageKbMap[key] = value;
      }

    private:
      IkIndexProcess*	m_pIndexProcess; /*!< Indexing process */
      std::map<iknow::base::String, IkKnowledgebase const *> m_languageKbMap;// workaround for not being able to pass pointers in std maps in vc6

    };
  }
}

#ifdef WIN32
#pragma warning (pop)
#endif

#endif // !defined(AFX_CORE_H__69068F13_B0F3_4B0A_BC06_C7B6A5F27BD0__INCLUDED_)

