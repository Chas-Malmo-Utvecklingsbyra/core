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

Weather_Response weather_get_data_from_json(char* json, char* locationiq_json)
{
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

    cJSON* parsed_locationiq_json = cJSON_Parse(locationiq_json);
    cJSON* address = cJSON_GetObjectItemCaseSensitive(parsed_locationiq_json, "address");
    cJSON* country = cJSON_GetObjectItemCaseSensitive(address, "country");
    cJSON* village = cJSON_GetObjectItemCaseSensitive(address, "village");
    cJSON* town = cJSON_GetObjectItemCaseSensitive(address, "town");
    cJSON* city = cJSON_GetObjectItemCaseSensitive(address, "city");
    cJSON* county = cJSON_GetObjectItemCaseSensitive(address, "county");


    if (village)
    {
        sprintf(response.location, "%s, %s", cJSON_GetStringValue(village), cJSON_GetStringValue(country));
    }
    else if (town)
    {
        sprintf(response.location, "%s, %s", cJSON_GetStringValue(town), cJSON_GetStringValue(country));
    }
    else if (city)
    {
        sprintf(response.location, "%s, %s", cJSON_GetStringValue(city), cJSON_GetStringValue(country));
    }
    else if (county)
    {
        sprintf(response.location, "%s, %s", cJSON_GetStringValue(county), cJSON_GetStringValue(country));
    }
    else
    {
        sprintf(response.location, "%s", "Unknown");
    }

    cJSON_Delete(parsed_locationiq_json);

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
    Weather_Response response = {0};

    if (latitude == NULL || longitude == NULL)
    {
        response.error = true;
        return response;
    }

    char* endptr = NULL;
    double latitude_double = strtod(latitude, &endptr);

    if (endptr == latitude || *endptr != '\0')
    {
        response.error = true;
        return response;
    }

    if (latitude_double < -90.0 || latitude_double > 90.0)
    {
        response.error = true;
        return response;
    }

    char* endptr2 = NULL;
    double longitude_double = strtod(longitude, &endptr2);

    if (endptr2 == longitude || *endptr2 != '\0')
    {
        response.error = true;
        return response;
    }

    if (longitude_double < -90.0 || longitude_double > 90.0)
    {
        response.error = true;
        return response;
    }


    char* http_resp_data = NULL;
    char* http_locationiq_data = NULL;
    response.error = true;

    /* -4 for both %s */
    char* buffer = (char*)malloc(strlen(OPENMETEO_API_TEMPLATE) + strlen(latitude) + strlen(longitude) - 4 + 1);

    Config_t* cfg = config_get_instance("settings.json");

    char* location_buffer = (char*)malloc(strlen(LOCATIONIQ_API_TEMPLATE) + strlen(latitude) + strlen(longitude) + strlen(cfg->locationiq_access_token) - 6 + 1);

    sprintf(location_buffer, LOCATIONIQ_API_TEMPLATE, cfg->locationiq_access_token, latitude, longitude);
    sprintf(buffer, OPENMETEO_API_TEMPLATE, latitude, longitude);

    if (http_get(buffer, &http_resp_data, NULL) == HTTP_ERROR_FAILED_TO_PERFORM || http_get(location_buffer, &http_locationiq_data, NULL) == HTTP_ERROR_FAILED_TO_PERFORM)
    {
        printf("Http_Perform failed in weather_get_data\n");
        free(location_buffer);
        free(buffer);
        return response;
    }

    if (http_resp_data == NULL || http_locationiq_data == NULL)
    {
        printf("response from http_get is NULL\n");
        free(location_buffer);
        free(buffer);
        return response;
    }

    response = weather_get_data_from_json(http_resp_data, http_locationiq_data);

    free(http_resp_data);
    free(http_locationiq_data);
    free(buffer);
    free(location_buffer);
    response.error = false;
    return response;
}

char* weather_convert_response_to_json(Weather_Response* response)
{
    cJSON* root = cJSON_CreateObject();
    cJSON_AddStringToObject(root, "location", response->location);
    cJSON_AddNumberToObject(root, "temperature", response->temperature, true);
    cJSON_AddStringToObject(root, "unit", &response->unit);
    cJSON_AddStringToObject(root, "condition", response->condition);
    cJSON_AddNumberToObject(root, "feels_like", response->feels_like, true);
    cJSON_AddNumberToObject(root, "humidity", response->humidity, false);
    cJSON* wind_object = cJSON_AddObjectToObject(root, "wind");
    cJSON_AddNumberToObject(wind_object, "speed", response->wind.speed, true);
    cJSON_AddNumberToObject(wind_object, "direction", response->wind.direction, true);
    cJSON_AddStringToObject(root, "sunrise", response->sunrise);
    cJSON_AddStringToObject(root, "sunset", response->sunset);
    cJSON_AddStringToObject(root, "icon_url", response->icon_url);

    return cJSON_Print(root);
}