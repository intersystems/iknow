/***************************************************************************
*	This file must be included in every HEADER FILE that has to be EXPORTED
****************************************************************************/

#ifdef WIN32
    #if _MSC_VER > 1000
    #pragma once
    #endif // _MSC_VER > 1000

    #ifdef CORE_EXPORTS
    #define CORE_API __declspec(dllexport)
    #else
    #define CORE_API __declspec(dllimport)
    #endif
#else
    #define CORE_API
#endif
