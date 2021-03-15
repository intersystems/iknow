MAKEPATH := $(realpath $(dir $(abspath $(lastword $(MAKEFILE_LIST)))).)
ROOT_DIR = $(realpath $(MAKEPATH)/.)

LANGLIST = cs de en es fr ja nl pt ru sv uk

#TODO: TRW - generalize!
PLATFORM = $(IKNOWPLAT)
MODE ?= release

include $(ROOT_DIR)/build/make/platforms/$(PLATFORM).mak

all : engine

test : enginetest
	$(RUNTIMELOADPATHVAR)=$(ROOT_DIR)/kit/$(PLATFORM)/$(MODE)/bin:$(ICUDIR)/lib $(ROOT_DIR)/kit/$(PLATFORM)/$(MODE)/bin/iknowenginetest

languagecompiler : base core
	$(MAKE) -f $(ROOT_DIR)/modules/compiler/iKnowLanguageCompiler/languagecompiler.mak
enginetest : engine base
	$(MAKE) -f $(ROOT_DIR)/modules/enginetest/enginetest.mak
engine : base shell core icu
	$(MAKE) -f $(ROOT_DIR)/modules/engine/engine.mak
shell : base ali core modelcommon models
	$(MAKE) -f $(ROOT_DIR)/modules/shell/shell.mak
models : base ali core modelcommon
	cd $(ROOT_DIR)/modules/aho && (ls -d $(LANGLIST) | xargs -I lang sh -c "export IKNOWMODELLANG=lang ; $(MAKE) -f model0.mak ; $(MAKE) -f model1.mak")

modelcommon : base ali core
	$(MAKE) -f $(ROOT_DIR)/modules/aho/model_common.mak
core : base ali icu
	$(MAKE) -f $(ROOT_DIR)/modules/core/core.mak
ali : base icu
	$(MAKE) -f $(ROOT_DIR)/modules/ali/ali.mak
base : icu
	$(MAKE) -f $(ROOT_DIR)/modules/base/base.mak
icu :
	test -d $(ICUDIR)/include/unicode && (ls $(ICUDIR)/lib/libicu* > /dev/null)

clean :
	$(MAKE) -f $(ROOT_DIR)/modules/compiler/iKnowLanguageCompiler/languagecompiler.mak clean
	$(MAKE) -f $(ROOT_DIR)/modules/enginetest/enginetest.mak clean
	$(MAKE) -f $(ROOT_DIR)/modules/engine/engine.mak clean
	$(MAKE) -f $(ROOT_DIR)/modules/shell/shell.mak clean
	cd $(ROOT_DIR)/modules/aho && (ls -d $(LANGLIST) | xargs -I lang sh -c "export IKNOWMODELLANG=lang ; $(MAKE) -f model0.mak clean ; $(MAKE) -f model1.mak clean")
	$(MAKE) -f $(ROOT_DIR)/modules/aho/model_common.mak clean
	$(MAKE) -f $(ROOT_DIR)/modules/core/core.mak clean
	$(MAKE) -f $(ROOT_DIR)/modules/ali/ali.mak clean
	$(MAKE) -f $(ROOT_DIR)/modules/base/base.mak clean

.PHONY : all clean test enginetest languagecompiler engine shell models modelcommon core ali base icu
