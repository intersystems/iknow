"""Export definitions from engine.h"""

from libcpp.string cimport string
from libcpp cimport bool as cpp_bool
from libcpp.set cimport set
from libcpp.vector cimport vector
from libcpp.utility cimport pair

from .IkConceptProximity cimport ProximityPairVector_t

cdef extern from "../../engine/src/engine.h" namespace "iknowdata" nogil:
	enum Attribute:
		Negation = 1
		DateTime = 2
		PositiveSentiment = 5
		NegativeSentiment = 6
		EntityVector = 7
		Frequency = 9
		Duration = 10
		Measurement = 11
		Certainty = 12
		Generic1 = 13
		Generic2 = 14
		Generic3 = 15

cdef extern from "../../engine/src/engine.h" namespace "iknowdata::Entity" nogil:
	enum eType:
		NonRelevant = 0,
		Concept
		Relation
		PathRelevant

cdef extern from "../../engine/src/engine.h" namespace "iknowdata" nogil:
	ctypedef unsigned short Entity_Ref
	ctypedef unsigned short Attribute_Ref

cdef extern from "../../engine/src/engine.h" namespace "iknowdata::Sent_Attribute" nogil:
	ctypedef vector[pair[string, string]] Sent_Attribute_Parameters

cdef extern from "../../engine/src/engine.h" namespace "iknowdata::Sentence" nogil:
	ctypedef vector[Entity] Entities
	ctypedef vector[Sent_Attribute] Sent_Attributes
	ctypedef vector[Entity_Ref] Path
	ctypedef vector[Path_Attribute] Path_Attributes
	ctypedef vector[CRC] CRCs

cdef extern from "../../engine/src/engine.h" namespace "iknowdata::Text_Source" nogil:
	ctypedef ProximityPairVector_t Proximity
	ctypedef vector[Sentence] Sentences

cdef extern from "../../engine/src/engine.h" namespace "iknowdata" nogil:
	struct Entity:
		eType type "type_"
		size_t offset_start "offset_start_", offset_stop "offset_stop_"
		string index "index_"
		double dominance_value "dominance_value_"
		size_t entity_id "entity_id_"

	struct Sent_Attribute:
		Attribute type "type_"
		size_t offset_start "offset_start_", offset_stop "offset_stop_"
		string marker "marker_"
		Sent_Attribute_Parameters parameters "parameters_"
		Entity_Ref entity_ref
		Path entity_vector

	struct Path_Attribute:
		Attribute type
		unsigned short pos
		unsigned short span

	struct CRC:
		string head "head_token"
		string relation "relation_token"
		string tail "tail_token"

	struct Sentence:
		Entities entities
		Sent_Attributes sent_attributes
		Path path
		Path_Attributes path_attributes
		CRCs crc_chains

	struct Text_Source:
		Sentences sentences
		Proximity proximity


cdef extern from "../../engine/src/engine.h" nogil:
	cdef cppclass CPPUserDictionary "UserDictionary":
		CPPUserDictionary() except +
		void clear() except +

		int addLabel(const string& literal, const char* UdctLabel) except +
		void addSEndCondition(const string& literal, cpp_bool b_end) except +

		void addConceptTerm(const string& literal) except +
		void addRelationTerm(const string& literal) except +
		void addNonrelevantTerm(const string& literal) except +

		void addNegationTerm(const string& literal) except +
		void addPositiveSentimentTerm(const string& literal) except +
		void addNegativeSentimentTerm(const string& literal) except +
		void addUnitTerm(const string& literal) except +
		void addNumberTerm(const string& literal) except +
		void addTimeTerm(const string& literal) except +

		int addCertaintyLevel(const string& literal, int level) except +

		void addGeneric1(const string& literal) except +
		void addGeneric2(const string& literal) except +
		void addGeneric3(const string& literal) except +


cdef extern from "../../engine/src/engine.h" nogil:
	cdef cppclass CPPiKnowEngine "iKnowEngine":
		Text_Source m_index
		vector[string] m_traces

		CPPiKnowEngine() except +
		void index(const string& text_source, const string& language, cpp_bool traces) except +

		void loadUserDictionary(CPPUserDictionary& udct) except +
		void unloadUserDictionary() except +
		void setALIonSourceLevel() except +
		void setALIonSentenceLevel() except +

		@staticmethod
		const set[string]& GetLanguagesSet() except +

		@staticmethod
		string NormalizeText(const string& text_source, const string& language, cpp_bool bUserDct, cpp_bool bLowerCase, cpp_bool bStripPunct) except +

		#static std::string IdentifyLanguage(const std::string& text_source, double& certainty);
		@staticmethod
		string IdentifyLanguage(const string& text_source, double& certainty) except +

