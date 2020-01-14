# -*- Makefile -*-
#includes
-include $(ROOT_DIR)/build/make/platforms/superplatforms/unix.mak

###Universal tools
MKDIR = mkdir -p $(dir $@)
DELETE = rm -f

MACSDK = /Developer/Platforms/iPhoneOS.platform/Developer/SDKs/iPhoneOS2.1.sdk/
PLATARCH = -arch arm

###C vs. C++
GNUTOOL = $(if $(CMODE),$(C_COMPILER),$(CPP_COMPILER))

###Stage 1: Source->Objects
C_COMPILER = /Developer/Platforms/iPhoneOS.platform/Developer/usr/bin/gcc
CPP_COMPILER = /Developer/Platforms/iPhoneOS.platform/Developer/usr/bin/g++
COMPILER = $(GNUTOOL)

HEADERINCLUDEFLAG = -I
OBJECTOUTPUTFLAG = -o
OBJECTSUFFIX = .o
OBJECTFLAGS += -c $(PLATARCH) -isysroot $(MACSDK) -DMY_BIG_ENDIAN=__BIG_ENDIAN__ -D_ISC_BIGENDIAN=__BIG_ENDIAN__ -DBIT64PLAT=__LP64__ -DMACOSX $(if $(CREATELIBRARY), -fPIC) -DUNIX

###Stage 2a: Objects->Library
LIBRARIAN = $(GNUTOOL)
LIBRARYDIRFLAG = -L
LIBRARYSEARCHFLAG = -l
# Note that dylib's need a -install_name to be set
LIBRARYFLAGS += $(PLATARCH) -Wl,-syslibroot,$(MACSDK) $(if $(IS_DYLIB), -dynamiclib -install_name @executable_path/$(@F), -dynamic -bundle) -Wl,-multiply_defined,suppress
LIBRARYOUTPUTFLAG = -o

#Note that for OS X we allow libraries to override
#their extension. This hack exists only to support
#libirisodbc, which needs a .dylib extension despite
#being a bundle, not a dynamic library. This "need"
#is due to installation backward compatibility.
ifeq "$(IS_DYLIB)" "1"
LIBRARYSUFFIX ?= .dylib
else
LIBRARYSUFFIX ?= .so
endif
ifeq ($(MULTITHREADED),1)
LIBRARYFLAGS += -lpthread
endif


###Stage 2b: Objects->Executable
LINKER = $(GNUTOOL)
EXECUTABLEDIRFLAG = -L
EXECUTABLESEARCHFLAG = -l
EXECUTABLEFLAGS += $(PLATARCH)
ifeq ($(MULTITHREADED),1)
EXECUTABLEFLAGS += -lpthread
endif
EXECUTABLEOUTPUTFLAG = -o
EXECUTABLESUFFIX = 



ifeq ($(HIDE_SYMBOLS),1)
OBJECTFLAGS += -fvisibility=hidden
endif


#Thirdparty definitions for building ICU/Xerces/Xalan
BUILD_THIRDPARTY = 1
ifeq ($(UNIVERSAL),1)
APACHE_XML_FLAGS = $(foreach f,$(PLATARCH),-z $f) $(foreach f,$(PLATARCH),-l $f)
RUN_CONFIGURE_ICU_PARAMS = MacOSX/Universal --enable-threads=no 
else
RUN_CONFIGURE_ICU_PARAMS = MacOSX --enable-threads=no 
endif
RUN_CONFIGURE_XERCES_PARAMS = -p macosx -c gcc -x g++ -m inmem -t icu -n socket -r none -b 32 $(APACHE_XML_FLAGS)
RUN_CONFIGURE_XALAN_PARAMS = -p macosx -c gcc -x g++ -m inmem -t icu -b 32 $(APACHE_XML_FLAGS)
SHARED_LIB_VAR = DYLD_LIBRARY_PATH
CONFIG_PLAT_CFLAGS=-arch ppc -arch i386
CONFIG_PLAT_LDFLAGS=-arch ppc -arch i386
PLAT_MOD_CSP22_INC = freebsd
PLAT_MOD_CSP22_OBJECTFLAGS = 
PLAT_MOD_CSP22_LIBRARYFLAGS = -undefined suppress -flat_namespace

#Debugging flags
ifeq ($(MODE),debug)
OBJECTFLAGS += -g -O0 -DDEBUG
LIBRARYFLAGS += -g -DDEBUG
EXECUTABLEFLAGS += -g -DDEBUG
RUN_CONFIGURE_ICU_PARAMS += --enable-debug
RUN_CONFIGURE_XERCES_PARAMS += -d
RUN_CONFIGURE_XALAN_PARAMS += -d
CONFIG_PLAT_CFLAGS+=-g -O0 -DDEBUG
CONFIG_PLAT_LDFLAGS+=-g -DDEBUG
else
OBJECTFLAGS += -g1 -O
LIBRARYFLAGS += -g1 -O
EXECUTABLEFLAGS += -g1 -O
endif

#Strict flags
ifeq ($(STRICT),1)
OBJECTFLAGS += -Werror
LIBRARYFLAGS += -Werror
EXECUTABLEFLAGS += -Werror
endif

#a function for modifying library refs
#syntax - $(call fixLibraryRef <working dir> <library name> <library ref name>
fixLibraryRef = cd $(1) && setupLibraryReferences.pl $(2)$(LIBRARYSUFFIX) $(3)$(LIBRARYSUFFIX)
