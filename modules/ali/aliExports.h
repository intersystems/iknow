#ifdef WIN32
    #if _MSC_VER > 1000
    #pragma once
    #endif // _MSC_VER > 1000

    #ifdef ALI_EXPORTS
		#define ALI_API __declspec(dllexport)
    #else
		#define ALI_API __declspec(dllimport)
    #endif
#else
    #define ALI_API
#endif
