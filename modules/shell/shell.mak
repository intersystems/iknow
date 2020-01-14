MAKEPATH := $(realpath $(dir $(abspath $(lastword $(MAKEFILE_LIST)))).)
ROOT_DIR = $(realpath $(MAKEPATH)/../..)

#TODO: TRW - generalize!
PLATFORM = $(IKNOWPLAT)

STRICT = 1

CPP_LANGUAGE = 14

INCLUDEDIRS = $(ROOT_DIR)/modules/shell/src $(ROOT_DIR)/modules/shell/src/SDK/headers $(ROOT_DIR)/modules/base/src/headers $(ROOT_DIR)/modules/ali $(ROOT_DIR)/modules/core/src/headers $(ROOT_DIR)/modules/aho $(ROOT_DIR)/shared/System/unix $(ROOT_DIR)/shared/System $(ROOT_DIR)/shared/Utility $(ROOT_DIR)/kernel/common/h $(ICUDIR)/include

SOURCES = $(ROOT_DIR)/modules/shell/src/*.cpp $(ROOT_DIR)/modules/shell/src/SDK/*.cpp  \
	$(ROOT_DIR)/shared/System/sysCom.cpp \
	$(ROOT_DIR)/shared/System/sysException.cpp \
	$(ROOT_DIR)/shared/Utility/utlCacheList.cpp \
	$(ROOT_DIR)/shared/Utility/utlCountedByteArray.cpp

LOCATION = bin
CREATELIBRARY = 1
PROJECT = libiknowshell
OBJECTFLAGS = -D_DOUBLEBYTE -DCACHE_COM_DISABLE
IS_DYLIB = 1

LIBRARIES = iknowbase iknowali iknowcore iknowmodelcom \
	iknowmodelde  iknowmodeldex \
	iknowmodelen iknowmodelenx \
	iknowmodeles iknowmodelesx \
	iknowmodelfr iknowmodelfrx \
	iknowmodelja iknowmodeljax \
	iknowmodelnl iknowmodelnlx \
	iknowmodelpt iknowmodelptx \
	iknowmodelru iknowmodelrux \
	iknowmodeluk iknowmodelukx \
	iknowmodelsv iknowmodelsvx \
	iknowmodelcs iknowmodelcsx

## ISC SOH280+
ifeq ($(findstring $(PLATFORM), ppc ppc64), $(PLATFORM))
LIBRARIES += icuuc40 icui18n40
else
LIBRARIES += icuuc icui18n
endif
## ISC SOH280-

LIBRARYDIRS = $(ROOT_DIR)/kit/$(PLATFORM)/$(MODE)/bin $(ICUDIR)/lib

include $(ROOT_DIR)/build/make/master.mak

