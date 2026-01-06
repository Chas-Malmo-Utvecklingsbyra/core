#ifndef CACHE_H
#define CACHE_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <time.h>
#include <assert.h>

void cache_create_folder(void);

void cache_create_file(char *buf, size_t size, double latitude, double longitude);

int cache_if_file_is_fresh(const char *filename);

char *cache_load_file(double latitude, double longitude);

void cache_save_info_to_file(double latitude, double longitude, const char *data);

#endif