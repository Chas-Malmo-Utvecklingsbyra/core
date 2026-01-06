#include "cache.h"

#define CACHE_DIR "cached_weather_info/"
#define CACHE_TIME 900

void cache_create_folder(void){
#ifdef _WIN32
    mkdir(CACHE_DIR);
#else
    mkdir(CACHE_DIR, 0755);
#endif
}

/* Creating filename based only on latitude and longitude of location */
void cache_create_file(char *buf, size_t size, double latitude, double longitude){

    assert(buf != NULL);
    assert(size > 0);

    snprintf(buf, size, CACHE_DIR "/%.4f_%.4f.json", latitude, longitude);
}

int cache_if_file_is_fresh(const char *filename){
    
    struct stat st;
    if(stat(filename, &st) != 0){
        return 0;
    }

    time_t time_now = time(NULL);
    double age = difftime(time_now, st.st_mtime);
    return age < CACHE_TIME;
}

char *cache_load_file(double latitude, double longitude){

    char filename[256];
    cache_create_file(filename, sizeof(filename), latitude, longitude);

    if(!cache_if_file_is_fresh(filename)){
        return NULL;
    }

    FILE *file = fopen(filename, "r");
    if(!file){
        return NULL;
    }

    fseek(file, 0, SEEK_END);
    long size = ftell(file);
    rewind(file);

    char *data = malloc(size + 1);
    if(!data){
        fclose(file);
        return NULL;
    }

    fread(data, 1, size, file);
    data[size] = '\0';

    fclose(file);

    printf("\nLoaded weather info from: %s\n", filename);
    return data;
}

void cache_save_info_to_file(double latitude, double longitude, const char *data){
    
    char filename[256];
    cache_create_file(filename, sizeof(filename), latitude, longitude);

    FILE *file = fopen(filename, "w");
    if(!file){
        return;
    }

    fputs(data, file);
    fclose(file);

    printf("\nSaved weather info to %s\n", filename);
}
