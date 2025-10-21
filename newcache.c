#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <stdbool.h>
#include "newcache.h"


/* Checking if file exists, if it does it returns true, otherwise it returns a false value */
bool doesFileExist(const char* fileName)
{
    if (!fileName)
    {
        return false;
    }

    struct stat fileInfo;
    
    return stat(fileName, &fileInfo) == 0;    
}


/* The program will check if the information stored in the file is fresh (modified within the last 15 minutes). 
If it isn't fresh, the program will return false, so that new data can be collected. 
If it is fresh, reads it's content and it will return true */
bool readDataFromFileIfFresh(const CacheStruct *cache, const char* fileName, char **outData, size_t* dataSize)
{
    if(doesFileExist(fileName))
    {
        struct stat fileInfo;
        if(stat(fileName, &fileInfo) == -1)
        {
            perror("Error: File does not exist!\n");
            return false;
        }

        time_t timeNow = time(NULL);
        double timeDifference = difftime(timeNow, fileInfo.st_mtime);
        if(timeDifference > cache->timeStamp)
        {
            printf("Data in file is too old (cached %.0lf seconds ago).\n", timeDifference);
            return false;
        }

        FILE *file = fopen(fileName, "r");
        if(!file){
            return false;
        }

        fseek(file, 0, SEEK_END);
        long length = ftell(file);
        fseek(file, 0, SEEK_SET);

        if(length <= 0)
        {
            fclose(file);
            return false;
        }
        
        *outData = (char *)malloc(length + 1);
        if(!*outData)
        {
            fclose(file);
            return false;
        }

        size_t bytesRead = fread(*outData, 1, length, file);
        fclose(file);

        (*outData)[bytesRead] = '\0';
        *dataSize = bytesRead;

        return true;
    }

}

/* If the file doesn't exist, it returns false. If the file exists, then it retrieves the data from the file, if the data is missing or too old, 
it calls the fetchCallback to get new data, writes the new data into the cached file and returns the fresh data via outData. */
bool getDataFromFile(CacheStruct *cache, const char *fileName, char **outData, size_t *dataSize, CacheCallback fetchCallback)
{
    if(!doesFileExist(fileName))
    {
        return false;
    }

    if(readDataFromFileIfFresh(cache, fileName, outData, dataSize))
    {
        printf("Cache: Valid file found: %s\n", fileName);
        return true;
    }

    printf("Cache: Invalid or missing data. Fetching new data...\n");

    if(!fetchCallback)
    {
        return false;
    }

    const char *newData = fetchCallback();
    if(!newData)
    {
        return false;
    }

    size_t len = strlen(newData);

    FILE *file = fopen(fileName, "w");
    if(!file)
    {
        return false;
    }

    fwrite(newData, 1, len, file);
    fclose(file);

    *outData = (char *)malloc(len + 1);
    if(!*outData)
    {
        return false;
    }

    memcpy(*outData, newData, len + 1);
    *dataSize = len;

    return true;
}