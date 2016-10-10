/********************************************************************
 * File: symbols.h
 *
 * Symbols types and prototypes file.
 *
 * Author: M. Tim Jones <mtj@mtjones.com>
 *
 */

#ifndef __SYMBOLS_H
#define __SYMBOLS_H
#include <hiredis.h>

#define MAX_FUNCTIONS		10000
#define MAX_FUNCTION_NAME	100

#define REDIS_KEY "redis_key"
#define REDIS_FLAG "redis_flag"
#define TRACE_LEVEL "trace_level"
#define IGNORE_LIST "ignore_list"
#define FUNCTION_NAME_CACHE "function_name"

typedef struct {
  unsigned int address;
  char funcName[MAX_FUNCTION_NAME+1];
} func_t;

typedef struct {
    char* funciton;
    unsigned int address;
} ignore_t;

extern const char *redis_key;
extern int read_redis;
extern redisContext *redis_context;
extern redisReply *reply;

void initSymbol( char *imageName1 );

int lookupSymbol( unsigned int address );

void addSymbol( unsigned int address );

void addCallTrace( unsigned int address );

void emitSymbols( int pid_tid );


#endif /* __SYMBOLS_H */
