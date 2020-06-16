# -*- Makefile -*-
#includes
-include $(ROOT_DIR)/build/make/platforms/superplatforms/unix.mak

###Universal tools
MKDIR = mkdir -p $(dir $@)
DELETE = rm -f


CLANG_MODE = 1
# Use LLVM's libc++, rather than the older GNU libstdc++
STDLIB=libc++

# Compile C++ code as version 11 of the language?
ifeq ($(CPP_LANGUAGE),11)
CPPLANGFLAG=-std=c++11
endif

# Compile C++ code as version 14 of the language?
ifeq ($(CPP_LANGUAGE),14)
CPPLANGFLAG=-std=c++14
endif

# Compile C++ code as version 17 of the language?
ifeq ($(CPP_LANGUAGE),17)
CPPLANGFLAG=-std=c++17
endif

PLATARCH = -arch x86_64 -mmacosx-version-min=10.9 -stdlib=$(STDLIB)

###Stage 1: Source->Objects
ifeq ($(CLANG_MODE),1)
C_COMPILER = clang
CPP_COMPILER = clang++ $(CPPLANGFLAG)
else
C_COMPILER = gcc
CPP_COMPILER = g++ $(CPPLANGFLAG)
endif
###C vs. C++
GNUTOOL = $(if $(CMODE),$(C_COMPILER),$(CPP_COMPILER))

COMPILER = $(GNUTOOL)

HEADERINCLUDEFLAG = -I
OBJECTOUTPUTFLAG = -o
OBJECTSUFFIX = .o
OBJECTFLAGS += -c $(PLATARCH) -DMY_BIG_ENDIAN=__BIG_ENDIAN__ -D_ISC_BIGENDIAN=__BIG_ENDIAN__ -DBIT64PLAT=__LP64__ -DSIZEOF_LONG=8 -DMACOSX $(if $(CREATELIBRARY), -fPIC) -DUNIX -stdlib=$(STDLIB)

###Stage 2a: Objects->Library
LIBRARIAN = $(GNUTOOL)
LIBRARYDIRFLAG = -L
LIBRARYSEARCHFLAG = -l

LFLAGS = $(PLATARCH) -Wl,-multiply_defined,suppress -stdlib=$(STDLIB)
# Note that dylib's need a -install_name to be set
LIBRARYFLAGSDYN = -dynamiclib -install_name @loader_path/$(@F)
LIBRARYFLAGSSO = -dynamic -bundle

LIBRARYFLAGS += $(LFLAGS) $(if $(IS_DYLIB), $(LIBRARYFLAGSDYN), $(LIBRARYFLAGSSO))
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
EXECUTABLEFLAGS += $(PLATARCH) -stdlib=$(STDLIB)
ifeq ($(MULTITHREADED),1)
EXECUTABLEFLAGS += -lpthread
endif
EXECUTABLEOUTPUTFLAG = -o
EXECUTABLESUFFIX =
RUNTIMELOADPATHVAR = DYLD_LIBRARY_PATH


ifeq ($(HIDE_SYMBOLS),1)
OBJECTFLAGS += -fvisibility=hidden
endif


#Thirdparty definitions
BUILD_THIRDPARTY = 1
PLAT_OPENSSL_CONFIGURE_OPTIONS = darwin64-x86_64-cc
PLATSOEXT=dylib
APACHE_XML_FLAGS = $(foreach f,$(PLATARCH),-z $f) $(foreach f,$(PLATARCH),-l $f)
RUN_CONFIGURE_ICU_PARAMS = MacOSX64 --enable-threads=yes
PLAT_XERCES_CONFIGURE_OPTIONS = CFLAGS="-arch x86_64 -mmacosx-version-min=10.9" CXXFLAGS="-arch x86_64 -mmacosx-version-min=10.9" LDFLAGS="-arch x86_64 -mmacosx-version-min=10.9"
RUN_CONFIGURE_XERCES_PARAMS = -p macosx -c gcc -x g++ -m inmem -t icu -n socket -r none -b 64 $(APACHE_XML_FLAGS)
RUN_CONFIGURE_XALAN_PARAMS = -p macosx -c gcc -x g++ -m inmem -t icu -b 64 $(APACHE_XML_FLAGS)
SHARED_LIB_VAR = DYLD_LIBRARY_PATH
PLAT_MOD_CSP22_INC = freebsd
PLAT_MOD_CSP22_OBJECTFLAGS =
PLAT_MOD_CSP22_LIBRARYFLAGS = -undefined suppress -flat_namespace
CONFIG_PLAT_CFLAGS=-arch x86_64 -mmacosx-version-min=10.9
CONFIG_PLAT_LDFLAGS=-arch x86_64 -mmacosx-version-min=10.9
PLAT_LIBSSH2_OBJECTFLAGS += -arch x86_64
PLAT_CMQL_CONFIGURE_OPTIONS = --enable-64bit
PLAT_ANTLR_CONFIGURE_OPTIONS = --enable-64bit
#The following corresponds to openldap version openldap-2.4.11
#OpenLDAP's configuration files are terrible: we can't
#add 64-bit support without aliasing the compiler to include
#the 64-bit flag
#Note that on mac we link statically to avoid collision with the system level ldap calls STC1906
PLAT_OPENLDAP_OUTPUT = libldap.a liblber.a
PLAT_OPENLDAP_COMPILER_OPTIONS = -arch x86_64
PLAT_OPENLDAP_COMPILER = "cc -arch x86_64"
PLAT_OPENLDAP_CONFIGURE_OPTIONS = --enable-static --disable-shared --enable-slapd=no --enable-slurpd=no --with-threads=no --prefix=$(BUILD_DIR) --without-cyrus-sasl --with-tls=openssl $(FLAGS)

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
OBJECTFLAGS += -g -O3
LIBRARYFLAGS += -g -O3
EXECUTABLEFLAGS += -g -O3
endif

###Japanese language model is optimized forever...
ifeq ($(UNOPTIMIZED), 1)
OBJECTFLAGS += -O0
endif


#Strict flags
ifeq ($(STRICT),1)
#"long long" is used in sysTypes.h
OBJECTFLAGS += -Wno-long-long -Werror -Wall -Wextra -pedantic-errors -fdiagnostics-show-option -Wno-parentheses -Wno-missing-field-initializers -Wno-unused-parameter -Wno-unused-local-typedef -Wno-unknown-warning-option
#OBJECTFLAGS += -Wno-long-long -Werror -Wall -Wextra -pedantic-errors -fdiagnostics-show-option
LIBRARYFLAGS +=  -Werror -Wall -Wextra -Werror
EXECUTABLEFLAGS += -Werror -Wall -Wextra
endif

#a function for modifying library refs
#syntax - $(call fixLibraryRef <working dir> <library name> <library ref name>
fixLibraryRef = cd $(1) && setupLibraryReferences.pl $(2)$(LIBRARYSUFFIX) $(3)$(LIBRARYSUFFIX)
