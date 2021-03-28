#ifndef DBG_H
#define DBG_H


#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <stdbool.h>

#ifdef NDEBUG
#define T_debug( M, ... )
#else
#define T_debug( M, ... ) _ftprintf( stderr, _T( "T DEBUG %s: line %d: " ) M _T ( "\n" ), _T( __FILE__ ), __LINE__, ##__VA_ARGS__ )
#endif

#endif
