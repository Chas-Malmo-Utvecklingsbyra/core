#ifndef WEATHER_H
#define WEATHER_H

#include <stdbool.h>
#include "http.h"

typedef struct
{
    float speed;
    int direction;
}Wind;

typedef struct 
{
    char location[143];
    float temperature;
    char unit;
    char* condition;
    float feels_like;
    float humidity;
    Wind wind;
    char sunrise[6]; /* 24h clock, will always be 6 bytes (null terminated) */
    char sunset[6];
    char* icon_url;
    int error;
}Weather_Response;

Weather_Response weather_get_data(const char* latitude, const char* longitude);

#endif