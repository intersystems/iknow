"""Export definitions from engine.h"""

from libcpp.string cimport string
from libcpp cimport bool as cpp_bool
from libcpp.set cimport set
from libcpp.vector cimport vector
from .IkConceptProximity cimport ProximityPairVector_t


cdef extern from "../../engine/src/engine.h" namespace "iknowdata::Entity" nogil:
	const size_t kNoConcept = <size_t>(-1)
	enum eType:
		NonRelevant = 0,
		Concept
		Relation
		PathRelevant

cdef extern from "../../engine/src/engine.h" namespace "iknowdata::Sent_Attribute" nogil:
	enum aType:
		Negation = 1
		DateTime = 2
		PositiveSentiment = 5
		NegativeSentiment = 6
		Frequency = 8
		Duration = 10
		Measurement = 11
		Certainty = 12

cdef extern from "../../engine/src/engine.h" namespace "iknowdata" nogil:
	ctypedef unsigned short Entity_Ref
	ctypedef unsigned short Attribute_Ref

cdef extern from "../../engine/src/engine.h" namespace "iknowdata::Sentence" nogil:
	ctypedef vector[Entity] Entities
	ctypedef vector[Sent_Attribute] Sent_Attributes
	ctypedef vector[Entity_Ref] Path
	ctypedef vector[Path_Attribute_Span] Path_Attributes

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
		aType type "type_"
		size_t offset_start "offset_start_", offset_stop "offset_stop_"
		string marker "marker_"
		string value "value_", unit "unit_", value2 "value2_", unit2 "unit2_"
		Entity_Ref entity_ref

	struct Path_Attribute_Span:
		Attribute_Ref sent_attribute_ref
		Entity_Ref entity_start_ref, entity_stop_ref

	struct Sentence:
		Entities entities
		Sent_Attributes sent_attributes
		Path path
		Path_Attributes path_attributes

	struct Text_Source:
		Sentences sentences
		Proximity proximity


cdef extern from "../../engine/src/engine.h" nogil:
	cdef cppclass CPPiKnowEngine "iKnowEngine":
		Text_Source m_index
		vector[string] m_traces

		CPPiKnowEngine() except +
		void index(const string& text_source, const string& language, cpp_bool traces) except +
		void addUdctAnnotation(size_t start, size_t stop, const char* UdctLabel) except +

		@staticmethod
		const set[string]& GetLanguagesSet() except +
