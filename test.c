#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <stdbool.h>
#include "newcache.h"


const char* testFetchWeatherData(void)
{
    return "{\"city\": \"Stockholm\", \"Temperature\": 8.5, \"Humidity\": 58 }";
}


int main() {
    CacheStruct cache = { .timeStamp = 900 };
    char *data = NULL;
    size_t size = 0;

    if(getDataFromFile(&cache, "weather.json", &data, &size, testFetchWeatherData))
    {
        printf("Data (%zu bytes):\n%s\n", size, data);
        free(data);
    } else 
    {
        printf("Couldn't retrieve data.\n");
    }

    return 0;    
}