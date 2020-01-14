SUPER_PLATFORM = windows
REPLICATE_TREE_CMD = $(ROOT_DIR)/build/tools/win/cptree
SYMBOLIC_COPY = cp -Rf

#see http://www.cygwin.com/ml/cygwin/1999-10/msg00342.html for
#why we have to clear MAKEFLAGS before calling nmake.
PLAT_OPENSSL_MAKE_SCRIPT = set MAKEFLAGS= && set CYGWIN=winsymlinks:nativestrict && nmake /f ms\\ntdll.mak

