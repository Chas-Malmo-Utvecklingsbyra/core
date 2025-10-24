#include <stdio.h>
#include <stdlib.h>
#include "newcache.h"


const char* test_fetch_weather_data(void)
{
    return "{\"city\": \"Säffle\", \"Temperature\": 10.5, \"Humidity\": 68}";
}


int main() {
    cache_struct cache = {.time_stamp = 900};
    char *data = NULL;
    size_t size = 0;

    if(get_data_from_file(&cache, "weather.json", &data, &size, test_fetch_weather_data))
    {
        printf("Data (%zu bytes):\n%s\n", size, data);
        free(data);
    } else 
    {
        printf("Couldn't retrieve data.\n");
    }

    return 0;    
}