#ifndef IKNOW_BASE_SAFESTRING_H_
#define IKNOW_BASE_SAFESTRING_H_
//Needed for Mac OS X when hidden symbol libraries are used.
//See: http://developer.apple.com/library/mac/#technotes/tn2185/_index.html
#ifdef __GNUC__
#pragma GCC visibility push(default) //added change
#endif
#include <string>
#ifdef __GNUC__
#pragma GCC visibility pop                 //added change
#endif
#endif //IKNOW_BASE_SAFESTRING_H_
