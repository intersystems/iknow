#includes
-include $(ROOT_DIR)/build/make/platforms/superplatforms/unix.mak

###Universal tools
MKDIR = test -e $(dir $@) || mkdir -p $(dir $@)
DELETE = rm -f

###Stage 1: Source->Objects
ifneq ($(CROSSBUILD),)
C_COMPILER = arm-linux-gnueabihf-gcc
CPP_COMPILER = arm-linux-gnueabihf-g++
else
C_COMPILER = gcc
CPP_COMPILER = g++
endif
HEADERINCLUDEFLAG = -I
OBJECTOUTPUTFLAG = -o
OBJECTSUFFIX = .o
LIBFLAGS = -fPIC
OBJECTFLAGS += -march=armv7 -c -O2 -Wno-non-template-friend -DLINUX -DUNIX $(if $(CREATELIBRARY),$(LIBFLAGS))

###Stage 2a: Objects->Library
ifneq ($(CROSSBUILD),)
LIBRARIAN = arm-linux-gnueabihf-g++
else
LIBRARIAN = g++
endif
LIBRARYDIRFLAG = -L
LIBRARYSEARCHFLAG = -l
LIBRARYFLAGS += -shared -Wl,-rpath,. 
LIBRARYOUTPUTFLAG = -o
LIBRARYSUFFIX = .so

###Stage 2b: Objects->Executable
ifneq ($(CROSSBUILD),)
LINKER = arm-linux-gnueabihf-g++
else
LINKER = g++
endif
EXECUTABLEDIRFLAG = -L
EXECUTABLESEARCHFLAG = -l
EXECUTABLEFLAGS += -Wno-non-template-friend -DLINUX 
EXECUTABLEOUTPUTFLAG = -o
EXECUTABLESUFFIX = 
RUNTIMELOADPATHVAR = LD_LIBRARY_PATH



ifeq ($(MULTITHREADED),1)
C_COMPILER += -pthread
CPP_COMPILER += -pthread
LINKER += -pthread
LIBRARIAN += -pthread
endif


ifeq ($(HIDE_SYMBOLS),1)
LIBRARYFLAGS += -Wl,-Bsymbolic
EXECUTABLEFLAGS += -Wl,-Bsymbolic
endif


#Thirdparty definitions
BUILD_THIRDPARTY = 1
RUN_CONFIGURE_ICU_PARAMS = LinuxRedHat --enable-threads=no LDFLAGS="-Wl,-rpath,."

PLAT_CMQL_CONFIGURE_OPTIONS = --disable-abiflags

PLAT_ANTLR_CONFIGURE_OPTIONS = --disable-abiflags

PLAT_XERCES_CONFIGURE_OPTIONS = LDFLAGS="-Wl,-rpath,."
RUN_CONFIGURE_XALAN_PARAMS = -p linux -c $(C_COMPILER) -x $(CPP_COMPILER) -m inmem -t icu -b 32
SHARED_LIB_VAR = LD_LIBRARY_PATH
PLAT_OPENSSL_CONFIGURE_OPTIONS = linux-armv4
PLATSOEXT=so
PLAT_MQ_LIBRARIES = mqic
PLAT_MQ_LIBRARY_FLAGS =

PLAT_MOD_CSP22_INC = linux 
PLAT_MOD_CSP22_OBJECTFLAGS = -fpic -DLINUX=2 -D_REENTRANT -D_GNU_SOURCE -D_LARGEFILE64_SOURCE
PLAT_MOD_CSP22_LIBRARYFLAGS = 

PLAT_LIBSSH2_LIBS = -ldl

PLAT_ZLIB_COMPILER = $(C_COMPILER)
PLAT_ZLIB_LIBRARIAN = $(C_COMPILER)
PLAT_ZLIB_OBJECTFLAGS = -O3 -DLINUX -DUNIX $(if $(CREATELIBRARY),$(LIBFLAGS))

#Debugging flags
ifeq ($(MODE),debug)
OBJECTFLAGS += -g -O0 -DDEBUG
LIBRARYFLAGS += -g -DDEBUG
EXECUTABLEFLAGS += -g -DDEBUG
RUN_CONFIGURE_ICU_PARAMS += --enable-debug

RUN_CONFIGURE_XALAN_PARAMS += -d
CONFIG_PLAT_CFLAGS=-g -O0 -DDEBUG
CONFIG_PLAT_LDFLAGS=-g -DDEBUG
endif

#Strict flags
ifeq ($(STRICT),1)
OBJECTFLAGS += -Werror
LIBRARYFLAGS += -Werror
EXECUTABLEFLAGS += -Werror
endif
