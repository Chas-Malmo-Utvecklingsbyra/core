#ifndef CORE_FILE_H
#define CORE_FILE_H

#include <assert.h>

// TODO: SS - Make this generic, not restricted to read-only.
char *file_open_read_only(const char *path, uint32_t *out_file_size) {
    if(path == NULL) {
        printf("Path NULL.\n");
        return NULL;
    }

    FILE *file = fopen(path, "r");
    if (file == NULL) {
        printf("Failed to open file '%s'.\n", path);
        return NULL;
    }

    fseek(file, 0, SEEK_END);
    long length = ftell(file);
    rewind(file);
    
    *out_file_size = length + 1;

    char *str = calloc(1, *out_file_size);
    assert(str != NULL);
    fread(str, 1, length, file);
    fclose(file);

    return str;
}

#endif