#includes
-include $(ROOT_DIR)/build/make/platforms/superplatforms/unix.mak

###Universal tools
MKDIR = mkdir -p $(dir $@)
DELETE = rm -f

SYMBOLIC_COPY = cp -Rf -P

C_COMPILER = cc
CPP_COMPILER = CC

ifeq ($(MULTITHREADED),1)
C_COMPILER += -mt
CPP_COMPILER += -mt
endif

# For different versions of the compiler, set cc-xarch-flag options.
# E.G.  Existing:  cc: Sun C 5.8 Patch 121016-03 2006/06/07
#                  cc location:  /opt/SUNWspro/... 
#
#       New:       cc: Sun C 5.12 SunOS_i386 2011/11/16
#                  cc location: /opt/solarisstudio12.3/...
#
# If NEW_SOLARIS_STUDIO is defined in .../build/environments/setupenv_solx64
# as an environment variable, then we are using the Sun C 5.12 compiler.
# Added special processing for XERCES.
# Utilizing CCARCHFLAG in .../kernel/tools/solx64 files, and
# in .../modules/Node.js files hence must always retain 
# CCARCHFLAG.
ifdef NEW_SOLARIS_STUDIO
CCARCHFLAG = -m64 -xarch=sse2
LDARCHFLAG = -m64
OPENSSL_CONFIGURE = solaris64-x86_64-cc-m64
else
CCARCHFLAG = -xarch=amd64
LDARCHFLAG = -xarch=amd64
OPENSSL_CONFIGURE  = solaris64-x86_64-cc
endif 

PLATCFLAGS = $(CCARCHFLAG)
PLATCXXFLAGS = $(CCARCHFLAG)
PLATLDFLAGS = $(LDARCHFLAG)



###Stage 1: Source->Objects
HEADERINCLUDEFLAG = -I
OBJECTOUTPUTFLAG = -o
OBJECTSUFFIX = .o

OBJECTFLAGS += -c $(CCARCHFLAG) -DBIT64PLAT=1 -DUNIX
ifdef CMODE
OBJECTFLAGS += -DSOLARIS
else
OBJECTFLAGS += -DSOLARIS -DSPARC -DSIZEOF_LONG=8 $(if $(CREATELIBRARY), -KPIC)
ifneq ($(MODE),debug)
# Only optimize for a release build, otherwise dbx can't debug ...
OBJECTFLAGS += -O2
endif
endif

###Stage 2a: Objects->Library

LIBRARYDIRFLAG = -L
LIBRARYSEARCHFLAG = -l

LIBRARYFLAGS += $(CCARCHFLAG) -DBIT64PLAT=1 -DUNIX
LIBRARIAN = CC
ifdef CMODE
LIBRARIAN = cc
LIBRARYFLAGS += -G
else
LIBRARYFLAGS += -DSPARC -lCrun -lc -lm -ldl -z defs -lCstd -KPIC -G
endif

LIBRARYOUTPUTFLAG = -o
LIBRARYSUFFIX = .so

###Stage 2b: Objects->Executable
LINKER = CC

EXECUTABLEFLAGS += $(CCARCHFLAG) -DBIT64PLAT=1 -DUNIX
ifdef CMODE
LINKER = cc
EXECUTABLEFLAGS += -DSOLARIS -Xa -w -ldl 
else
EXECUTABLEFLAGS += -DSPARC -lCrun -lc -lm -z defs -lCstd -ldl
endif

EXECUTABLEDIRFLAG = -L
EXECUTABLESEARCHFLAG = -l

EXECUTABLEOUTPUTFLAG = -o
EXECUTABLESUFFIX = 


ifeq ($(HIDE_SYMBOLS),1)
OBJECTFLAGS += -xldscope=symbolic
LIBRARYFLAGS += -xldscope=symbolic
EXECUTABLEFLAGS += -xldscope=symbolic
endif


#Thirdparty definitions
BUILD_THIRDPARTY = 1
RUN_CONFIGURE_ICU_PARAMS = SolarisX86 --enable-threads=no --enable-64bit-libs
PLAT_XERCES_CONFIGURE_OPTIONS = CC=cc CXX=CC CFLAGS='$(CCARCHFLAG)' CXXFLAGS='$(CCARCHFLAG)' LDFLAGS=$(LDARCHFLAG)
RUN_CONFIGURE_XALAN_PARAMS = -p solaris-x86 -c cc -x CC -minmem -ticu -b64 -z -m64 -z -xarch=sse2 -l $(LDARCHFLAG)
SHARED_LIB_VAR = LD_LIBRARY_PATH_64
PLAT_OPENSSL_CONFIGURE_OPTIONS = $(OPENSSL_CONFIGURE)

#OpenLDAP's configuration files are terrible: we can't
#add 64-bit support without aliasing the compiler to include
#the 64-bit flag. The avl.h file in /usr/include gets in the way of the openldap one so we exclus it here
PLAT_OPENLDAP_COMPILER_OPTIONS = $(CCARCHFLAG) -D_AVL_H
PLAT_OPENLDAP_COMPILER = "cc $(CCARCHFLAG) -D_AVL_H"

PLAT_CMQL_CONFIGURE_OPTIONS = --enable-64bit --build=x86_64-pc-solaris CC='cc $(CCARCHFLAG)'
PLAT_ANTLR_CONFIGURE_OPTIONS = --enable-64bit --build=x86_64-pc-solaris CC='cc $(CCARCHFLAG)'

PLATSOEXT=so
CONFIG_PLAT_CFLAGS=$(CCARCHFLAG)
CONFIG_PLAT_LDFLAGS=$(CCARCHFLAG) 
PLAT_MQ_LIBRARIES = imqb23as imqc23as mqic mqmcs mqmzse socket nsl
PLAT_MQ_LIBRARY_FLAGS = -R/opt/mqm/lib64 -R/usr/lib/64
PLAT_MOD_CSP22_INC = solaris
PLAT_MOD_CSP22_OBJECTFLAGS = -KPIC
PLAT_MOD_CSP22_LIBRARYFLAGS = 
PLAT_ZLIB_COMPILER=cc
PLAT_ZLIB_LIBRARIAN=cc
PLAT_ZLIB_OBJECTFLAGS=-c $(CCARCHFLAG) -DBIT64PLAT=1 -DUNIX -DSPARC -DSIZEOF_LONG=8 $(if $(CREATELIBRARY), -KPIC)
PLAT_LIBSSH2_COMPILER=cc
PLAT_LIBSSH2_LIBRARIAN=cc
PLAT_LIBSSH2_OBJECTFLAGS += $(CCARCHFLAG)
#PLAT_LIBSSH2_LIBS += -lsocket -lnsl
PLAT_MDNSRESPONDER_OS = solaris
PLAT_MDNSRESPONDER_CC = cc -KPIC $(CCARCHFLAG)
PLAT_MDNSRESPONDER_LD = ld -lc -ldl -z defs -G
PLAT_BJAM_OPTIONS = toolset=sun address-model=64
PLAT_BJAM_BOOTSTRAP_OPTIONS = --with-toolset=sun
#Debugging flags
ifeq ($(MODE),debug)
OBJECTFLAGS += -g -DDEBUG
LIBRARYFLAGS += -g -DDEBUG
EXECUTABLEFLAGS += -g -DDEBUG
RUN_CONFIGURE_ICU_PARAMS += --enable-debug
RUN_CONFIGURE_XERCES_PARAMS += -d
RUN_CONFIGURE_XALAN_PARAMS += -d
CONFIG_PLAT_CFLAGS+=-g -DDEBUG
CONFIG_PLAT_LDFLAGS+=-g -DDEBUG
endif
