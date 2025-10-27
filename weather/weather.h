#ifndef WEATHER_H
#define WEATHER_H

#include <stdbool.h>
#include "http.h"

typedef struct 
{
    char* data;
    bool error;
}Weather_Response;

Weather_Response weather_get_data(const char* api);
void weather_dispose(Weather_Response* response);


#endif