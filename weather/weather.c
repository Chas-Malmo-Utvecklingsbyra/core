#include "weather.h"
#include "http.h"
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include "../string/strdup.h"

Weather_Response weather_get_data(const char* api_url)
{
    char* http_resp_data = NULL;
    Weather_Response response = {0};
    response.error = true;

    if (http_get(api_url, &http_resp_data, NULL) == HTTP_ERROR_FAILED_TO_PERFORM)
    {
        printf("Http_Perform failed in weather_get_data\n");
        return response;
    }
    
    if (http_resp_data == NULL)
    {
        printf("response from http_get is NULL\n");
        return response;
    }

    response.error = false;
    response.data = http_resp_data;
    return response;
}

void weather_dispose(Weather_Response* response)
{
    if (response->data == NULL)
    {
        printf("*response->data == NULL\n");
        return;
    }

    free(response->data);
    response->data = NULL;
}
