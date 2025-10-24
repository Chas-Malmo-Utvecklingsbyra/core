#ifndef __NEWCACHE_H_
#define __NEWCACHE_H_

#include <stdbool.h>
#include <sys/stat.h>
#include <stddef.h>
#include <stdint.h>


typedef struct{
    uint32_t time_stamp;
} cache_struct;

typedef const char* (*cache_callback)(void);

bool does_file_exist(const char* file_name);

bool read_data_from_file_if_fresh(const cache_struct *cache, const char *file_name, char **out_data, size_t *data_size);

bool get_data_from_file(cache_struct *cache, const char *file_name, char **out_data, size_t *data_size, cache_callback fetch_callback);

#endif