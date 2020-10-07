from .version import __version__
from .engine import iKnowEngine

from collections import namedtuple

class UserDictionary(object):

    # constants
    # role labels
    CONCEPT = "UDConcept"
    RELATION = "UDRelation"

    # attribute labels
    NEGATION = "UDNegation"
    MEASUREMENT = "UDMeasurement"


    # constructor
    def __init__(self, entries=[]):
        self.entries = entries

    # generic method (make private?)
    def add_label(self, string, label):
        # declare Entry named tuple for convenience
        Entry = namedtuple("Entry", ["string", "label"])
        self.entries.append(Entry(string, label))

    # example of specific (public) method
    def add_concept(self, string):
        self.add_label(string, UserDictionary.CONCEPT)