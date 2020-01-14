#ifndef IKNOW_AHO_EXPORT_H_
#define IKNOW_AHO_EXPORT_H_

#ifdef _WIN32
    #ifdef AHO_EXPORTS
		#define AHO_API __declspec(dllexport)
    #else
		#define AHO_API __declspec(dllimport)
    #endif
#else
    #define AHO_API
#endif
#ifdef _WIN32
    #ifdef MODEL_EXPORTS
		#define MODEL_API __declspec(dllexport)
    #else
		#define MODEL_API __declspec(dllimport)
    #endif
#else
    #define MODEL_API
#endif

#endif //IKNOW_AHO_EXPORT_H_
