#include "weather.h"
#include "http.h"
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

Weather_Response weather_get_data(const char* api)
{
    Http h = {0};

    Weather_Response response = {0};
    response.error = true;

    if (http_initialize(&h) == -1)
    {
        printf("Http_Initialize returned -1 in weather_get_data\n");
        return response;
    }

    Http_Response http_resp = {0};
    if (http_get(&h, api, &http_resp) == -1)
    {
        printf("Http_Perform failed in weather_get_data\n");
        return response;
    }

    response.error = false;
    response.data = (char*)malloc(http_resp.size + 1);
    strcpy(response.data, http_resp.data);


    http_dispose_response(&http_resp);
    http_dispose(&h);
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
