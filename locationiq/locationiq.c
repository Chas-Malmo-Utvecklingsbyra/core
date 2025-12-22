#include "locationiq.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <curl/curl.h>


size_t write_chunk(void* data, size_t item_size, size_t nmemb, void* user_data)
{
    size_t real_size = item_size * nmemb;

    Response* response = (Response*) user_data;

    char* ptr = realloc(response->string, response->size + real_size + 1);
    if (ptr == NULL)
    {
        return CURL_WRITEFUNC_ERROR;
    }

    response->string = ptr;
    memcpy(&(response->string[response->size]), data, real_size);
    response->size += real_size;
    response->string[response->size] = '\0';
    
    return real_size;
}

char *locationiq_api_call(const char *location, int limit, const char *access_token)
{
    char url[256];

    sprintf(url, "https://eu1.locationiq.com/v1/search?key=%s&q=%s&format=json&limit=%d", access_token, location, limit);
    
    CURL* curl = curl_easy_init();
    CURLcode result;
    if (curl == NULL)
    {
        printf("HTTP request failed\n");
        return NULL;
    }

    Response response;
    response.string = malloc(1);
    response.size = 0;

    curl_easy_setopt(curl, CURLOPT_URL, url);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_chunk);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void*)&response);

    result = curl_easy_perform(curl);
    if (result != CURLE_OK)
    {
        printf("Error: %s\n", curl_easy_strerror(result));
        return NULL;
    }

    curl_easy_cleanup(curl);

    int len = strlen(response.string);
    
    char* full_string = malloc(len + 1);
    strncpy(full_string, response.string, len);

    free(response.string);

    return full_string;
}

int locationiq_get_coordinates(Coordinates *coords, const char *location, const char *access_token)
{   
    char* json_string = locationiq_api_call(location, 1, access_token); /* limit set to 1 for single result */
    if (json_string == NULL)
    {
        return LOCATIONIQ_RESULT_ERROR;
    }

    cJSON* cjson_parsed = cJSON_Parse(json_string);
    cJSON* cjson_selector = cjson_parsed;
    cJSON* display_name = NULL;
    cJSON* cjson_lat = NULL;
    cJSON* cjson_lon = NULL;

    cJSON_Print(cjson_parsed);

    char lat_string[64];
    char lon_string[64];
    
    int array_length = cJSON_GetArraySize(cjson_parsed);

    int i;

    for (i = 0; i < array_length; i++)
    {
        cjson_selector = cJSON_GetArrayItem(cjson_parsed, i);
        display_name = cJSON_GetObjectItem(cjson_selector, "display_name");
        if (display_name == NULL)
        {
            printf("No results found\n");
            return LOCATIONIQ_RESULT_ERROR;
        }
        printf("%d.\t%s\n", i + 1, cJSON_Print(display_name));
    }

    int user_selection = 0;
    int valid_selection = 0;

    /* This still breaks if user doesn't input numbers but it'll have to do for now. */
    while (valid_selection != 1)
    {
        printf("\nEnter a number to select a location from the list above (or 0 to search for another location): ");
        if(scanf("%d", &user_selection) != 1){
            while(getchar() != '\n');
            continue;
        }

        if(user_selection == 0){
            cJSON_Delete(cjson_parsed);
            free(json_string);
            return LOCATIONIQ_RESULT_USER_ABORTED;
        }

        if (user_selection > 0 && user_selection < array_length)
        {
            valid_selection = 1;
            break;
        }

        printf("Invalid selection, please try again.");
    }

    cjson_selector = cJSON_GetArrayItem(cjson_parsed, user_selection - 1);
    display_name = cJSON_GetObjectItem(cjson_selector, "display_name");
    cjson_lat = cJSON_GetObjectItem(cjson_selector, "lat");
    cjson_lon = cJSON_GetObjectItem(cjson_selector, "lon");
    strcpy(lat_string, cJSON_Print(cjson_lat));
    strcpy(lon_string, cJSON_Print(cjson_lon));
    coords->location = cJSON_Print(display_name);

    /* LocationIQ gives the lat and lon as strings instead of numbers in the JSON response.
    This code replaces the first " with a whitespace and then uses strtod to convert the rest of the string to a double. */
    char* endptr;
    lat_string[0] = ' ';
    lon_string[0] = ' ';
    coords->lat = strtod(lat_string, &endptr);
    coords->lon = strtod(lon_string, &endptr);

    cJSON_free(cjson_parsed);
    free(json_string);

    return LOCATIONIQ_RESULT_OK;
}