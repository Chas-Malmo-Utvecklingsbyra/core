#include "weather.h"
#include "http.h"
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include "../string/strdup.h"
#include "api/openmeteo.h"
#include "../json/cJSON/cJSON.h"

char* get_string_from_weathercode(int weather_code)
{
    if (weather_code == 0)
        return "Clear Sky";

    if (weather_code >= 1 && weather_code <= 3)
        return "Cloudy";

    if (weather_code >= 45 && weather_code <= 48)
        return "Fog";

    if (weather_code >= 51 && weather_code <= 67)
        return "Rain";

    if (weather_code >= 71 && weather_code <= 77)
        return "Snow";

    if (weather_code >= 80 && weather_code <= 82)
        return "Showers";

    if (weather_code >= 95 && weather_code <= 99)
        return "Thunderstorms";

    return "No weathercode found, Contact: Chas Malmö Utvecklingsbyrå ASAP";
}


Weather_Response weather_get_data_from_json(char* json)
{
    printf("Hello2!\n");

    cJSON* parsed_json = cJSON_Parse(json);

    cJSON* current = cJSON_GetObjectItemCaseSensitive(parsed_json, "current");


    cJSON* current_temperature_2m = cJSON_GetObjectItemCaseSensitive(current, "temperature_2m");
    cJSON* current_apparent_temperature = cJSON_GetObjectItemCaseSensitive(current, "apparent_temperature");
    cJSON* current_weather_code = cJSON_GetObjectItemCaseSensitive(current, "weather_code");
    cJSON* current_relativehumidity_2m = cJSON_GetObjectItemCaseSensitive(current, "relativehumidity_2m");
    cJSON* current_wind_speed_10m = cJSON_GetObjectItemCaseSensitive(current, "wind_speed_10m");
    cJSON* current_wind_drection_10m = cJSON_GetObjectItemCaseSensitive(current, "wind_direction_10m");

    cJSON* daily = cJSON_GetObjectItemCaseSensitive(parsed_json, "daily");
    cJSON* daily_sunrise_array = cJSON_GetObjectItemCaseSensitive(daily, "sunrise");
    cJSON* daily_sunrise_today = cJSON_GetArrayItem(daily_sunrise_array, 0);

    cJSON* daily_sunset_array = cJSON_GetObjectItemCaseSensitive(daily, "sunset");
    cJSON* daily_sunset_today = cJSON_GetArrayItem(daily_sunset_array, 0);


    Weather_Response response = {0};

    response.location = "Coming soon";

    response.temperature = (float)cJSON_GetNumberValue(current_temperature_2m);

    response.feels_like = (float)cJSON_GetNumberValue(current_apparent_temperature);
    response.unit = 'C';

    response.condition = get_string_from_weathercode((int)cJSON_GetNumberValue(current_weather_code));
    response.humidity = (float)cJSON_GetNumberValue(current_relativehumidity_2m);
    response.wind = (Wind){(float)cJSON_GetNumberValue(current_wind_speed_10m), (int)cJSON_GetNumberValue(current_wind_drection_10m)};

    sscanf(cJSON_GetStringValue(daily_sunrise_today), "%*[^T]T%5s", response.sunrise);
    sscanf(cJSON_GetStringValue(daily_sunset_today), "%*[^T]T%5s", response.sunset);

    response.icon_url = "NO ICONS AVAILABLE RIGHT NOW";

    cJSON_Delete(parsed_json);

    return response;
}

Weather_Response weather_get_data(const char* latitude, const char* longitude)
{
    char* http_resp_data = NULL;
    Weather_Response response = {0};
    response.error = true;

    /* -4 for both %d */
    char* buffer = (char*)malloc(strlen(OPENMETEO_API_TEMPLATE) - 4 + 1);


    sprintf(buffer, OPENMETEO_API_TEMPLATE, latitude, longitude);


    if (http_get(buffer, &http_resp_data, NULL) == HTTP_ERROR_FAILED_TO_PERFORM)
    {
        printf("Http_Perform failed in weather_get_data\n");
        free(buffer);
        return response;
    }

    if (http_resp_data == NULL)
    {
        printf("response from http_get is NULL\n");
        free(buffer);
        return response;
    }



    response = weather_get_data_from_json(http_resp_data);
    free(http_resp_data);
    free(buffer);
    response.error = false;
    return response;
}