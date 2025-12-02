#include "weather.h"
#include "http.h"
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include "../string/strdup.h"
#include "api.h"
#include "../json/cJSON/cJSON.h"
#include "../config/config.h"

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

Weather_Response weather_get_data_from_json(char* json) {
    cJSON* parsed_json = cJSON_Parse(json);

    cJSON* current = cJSON_GetObjectItemCaseSensitive(parsed_json, "current");
    cJSON* current_temperature_2m = cJSON_GetObjectItemCaseSensitive(current, "temperature_2m");
    cJSON* current_apparent_temperature = cJSON_GetObjectItemCaseSensitive(current, "apparent_temperature");
    cJSON* current_weather_code = cJSON_GetObjectItemCaseSensitive(current, "weather_code");
    cJSON* current_relativehumidity_2m = cJSON_GetObjectItemCaseSensitive(current, "relativehumidity_2m");
    cJSON* current_wind_speed_10m = cJSON_GetObjectItemCaseSensitive(current, "wind_speed_10m");
    cJSON* current_wind_drection_10m = cJSON_GetObjectItemCaseSensitive(current, "wind_direction_10m");

    Weather_Response response = {0};
    response.temperature = (float)cJSON_GetNumberValue(current_temperature_2m);
    response.feels_like = (float)cJSON_GetNumberValue(current_apparent_temperature);
    response.unit = 'C'; // TODO: SS - Don't hardcode.
    response.condition = get_string_from_weathercode((int)cJSON_GetNumberValue(current_weather_code));
    response.humidity = (float)cJSON_GetNumberValue(current_relativehumidity_2m);
    response.wind = (Wind){(float)cJSON_GetNumberValue(current_wind_speed_10m), (int)cJSON_GetNumberValue(current_wind_drection_10m)};

    response.icon_url = "TODO: Parse icon url.";

    cJSON_Delete(parsed_json);

    return response;
}

bool weather_get_data_double(double latitude, double longitude, Weather_Response *out_weather_response) {
    if(latitude <= LATITUDE_MIN || latitude >= LATITUDE_MAX) {
        return false;
    }
    if(longitude <= LONGITUDE_MIN || longitude >= LONGITUDE_MAX) {
        return false;
    }
    
    char latitude_str[32];
    memset(&latitude_str[0], 0, sizeof(latitude_str));
    snprintf(&latitude_str[0],  sizeof(latitude_str), "%f", latitude);

    char longitude_str[32];
    memset(&longitude_str[0], 0, sizeof(latitude_str));
    snprintf(&longitude_str[0], sizeof(longitude_str), "%f", longitude);

    *out_weather_response = weather_get_data(latitude_str, longitude_str);
    return true;
}

Weather_Response weather_get_data(const char* latitude, const char* longitude) {
    char* http_resp_data = NULL;
    Weather_Response response = {0};
    response.error = true;

    /* -4 for both %s */
    char* buffer = (char*)malloc(strlen(OPENMETEO_API_TEMPLATE) + strlen(latitude) + strlen(longitude) - 4 + 1);

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