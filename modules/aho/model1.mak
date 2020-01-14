ROOT_DIR = ../..
#Set the boost version we are going to build against.

ifeq ($(IKNOWMODELLANG),)
$error("IKNOWMODELLANG must be defined.")
endif 

#TODO: TRW - generalize!
PLATFORM = $(IKNOWPLAT)

STRICT = 1

CPP_LANGUAGE = 14

INCLUDEDIRS = ./inl/$(IKNOWMODELLANG)_regex ./lexrep ./ali . ../base/src/headers ../ali ../core/src/headers \
	 $(ROOT_DIR)/shared/System/unix $(ROOT_DIR)/shared/System $(ROOT_DIR)/shared/Utility $(ICUDIR)/include $(ROOT_DIR)/kernel/common/h

include module_list.mak

LOCATION = bin
CREATELIBRARY = 1
PROJECT = libiknowmodel$(IKNOWMODELLANG)x
OBJECTFLAGS = -D_DOUBLEBYTE -DCACHE_COM_DISABLE -DMODELINDEX=1 -DMODELID=$(IKNOWMODELLANG)
IS_DYLIB = 1

#There's a bug in gcc 4.4+ that
#causes an out of memory error if the dead subexpression elimination
#pass is run. As to why it shows up in Japanese,
#my guess is the number of Match values in the Aho-Corasick
#output function for lexrep matching. But that's just a guess.
#Reported to the gcc maintainers:
# https://gcc.gnu.org/bugzilla/show_bug.cgi?id=63191
ifeq ($(findstring $(PLATFORM), lnxrhx86 lnxrhppc64), $(PLATFORM))
OBJECTFLAGS += -fno-dse
endif

#There's also an apparent gcc bug in instruction scheduling on PowerPC
#that results in similar massive memory consumption.
ifeq ($(findstring $(PLATFORM), lnxrhppc64), $(PLATFORM))
OBJECTFLAGS += -fno-schedule-insns
endif

LIBRARIES = iknowbase iknowali iknowcore iknowmodelcom
ifeq ($(findstring $(PLATFORM), ppc ppc64), $(PLATFORM))
LIBRARIES += icuuc40 icui18n40
else
LIBRARIES += icuuc icui18n
endif
LIBRARYDIRS = $(ROOT_DIR)/kit/$(PLATFORM)/$(MODE)/bin $(ICUDIR)/lib

include $(ROOT_DIR)/build/make/master.mak
