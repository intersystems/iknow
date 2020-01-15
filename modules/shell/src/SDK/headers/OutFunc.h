#ifndef YMCA_SHELL_OUTFUNC_H_
#define YMCA_SHELL_OUTFUNC_H_
#include "IkIndexOutput.h"
#include "IkIndexDebug.h"
#include "IkStemmer.h"

typedef iknow::core::IkStemmer<iknow::base::String, iknow::base::String> Stemmer;
typedef void(*OutputFunc)(iknow::core::IkIndexOutput*, iknow::core::IkIndexDebug<TraceListType>*, void*, Stemmer*);

#endif // YMCA_SHELL_OUTFUNC_H_
