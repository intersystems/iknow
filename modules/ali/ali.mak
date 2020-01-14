MAKEPATH := $(realpath $(dir $(abspath $(lastword $(MAKEFILE_LIST)))).)
ROOT_DIR = $(realpath $(MAKEPATH)/../..)

#TODO: TRW - generalize!
PLATFORM = $(IKNOWPLAT)

STRICT = 1

CPP_LANGUAGE = 14

INCLUDEDIRS = $(ROOT_DIR)/modules/ali $(ROOT_DIR)/shared/System $(ROOT_DIR)/shared/System/unix $(ROOT_DIR)/shared/Utility $(ROOT_DIR)/modules/base/src/headers $(ICUDIR)/include
SOURCES = $(ROOT_DIR)/modules/ali/*.cpp
LOCATION = bin
CREATELIBRARY = 1
PROJECT = libiknowali
IS_DYLIB = 1

LIBRARYDIRS = $(ROOT_DIR)/kit/$(PLATFORM)/$(MODE)/bin $(ICUDIR)/lib
LIBRARIES = iknowbase icuuc
ifeq ($(findstring $(PLATFORM), ppc ppc64), $(PLATFORM))
LIBRARIES = iknowbase icuuc40
endif
include $(ROOT_DIR)/build/make/master.mak

