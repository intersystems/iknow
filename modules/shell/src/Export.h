#ifndef IKNOW_SHELL_EXPORT_H_
#define IKNOW_SHELL_EXPORT_H_
#ifdef _WIN32
    #ifdef SHELL_EXPORTS
		#define SHELL_API __declspec(dllexport)
    #else
		#define SHELL_API __declspec(dllimport)
    #endif
#else
    #define SHELL_API
#endif
#endif //IKNOW_SHELL_EXPORT_H_

