#ifndef WEATHER_H
#define WEATHER_H

#include <stdbool.h>
#include <stdint.h>
#include "http.h"

#define LATITUDE_MIN -90.0f
#define LATITUDE_MAX  90.0f

#define LONGITUDE_MIN -180.0f
#define LONGITUDE_MAX  180.0f

typedef struct
{
    float speed;
    int direction;
} Wind;

typedef struct 
{
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
} Weather_Response;

Weather_Response weather_get_data(const char* latitude, const char* longitude);
bool weather_get_data_double(double latitude, double longitude, Weather_Response *out_weather_response);

#endif