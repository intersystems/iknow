MAKEPATH := $(realpath $(dir $(abspath $(lastword $(MAKEFILE_LIST)))).)
ROOT_DIR = $(realpath $(MAKEPATH)/../..)

#TODO: TRW - generalize!
PLATFORM = $(IKNOWPLAT)

CPP_LANGUAGE = 14

INCLUDEDIRS = $(ROOT_DIR)/modules/engine/src $(ROOT_DIR)/modules/shell/src/SDK/headers $(ROOT_DIR)/modules/shell/src $(ROOT_DIR)/modules/ali $(ROOT_DIR)/modules/core/src/headers $(ROOT_DIR)/modules/base/src/headers $(ROOT_DIR)/modules/compiler/iKnowLanguageCompiler \
	 $(ROOT_DIR)/kernel/$(PLATFORM)/h $(ROOT_DIR)/kernel/ux/h $(ROOT_DIR)/kernel/common/h $(ROOT_DIR)/shared/System/unix $(ROOT_DIR)/shared/System \
	$(ROOT_DIR)/shared/Utility $(ICUDIR)/include

OBJECTFLAGS += -DMACHINETYP=58

SOURCES =  $(ROOT_DIR)/modules/engine/src/*.cpp $(ROOT_DIR)/modules/engine/language_data/*.c
LOCATION = bin
CREATELIBRARY = 1
PROJECT = libiknowengine
IS_DYLIB = 1

LIBRARIES = iknowbase iknowshell iknowcore icui18n icuuc icuio icudata
LIBRARYDIRS = $(ROOT_DIR)/kit/$(PLATFORM)/$(MODE)/bin $(ICUDIR)/lib

include $(ROOT_DIR)/build/make/master.mak
