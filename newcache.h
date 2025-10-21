#ifndef __NEWCACHE_H_
#define __NEWCACHE_H_

#include <stddef.h>
#include <time.h>
#include <stdint.h>
#include <stdbool.h>
#include <sys/stat.h>

typedef struct{
    unsigned int timeStamp;
} CacheStruct;

typedef const char* (*CacheCallback)(void);

bool doesFileExist(const char* fileName);

bool readDataFromFileIfFresh(const CacheStruct *cache, const char *fileName, char **outData, size_t *dataSize);

bool getDataFromFile(CacheStruct *cache, const char *fileName, char **outData, size_t *dataSize, CacheCallback fetchCallback);

#endif