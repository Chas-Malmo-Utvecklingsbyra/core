#include "weather.h"
#include "http.h"

void weather_get_data(const char* api)
{

    Http h = {0};
    if (Http_Initialize(&h) == -1)
    {
        printf("Http_Initialize returned -1 in weather_get_data\n");
        return;
    }


    Http_Response response = {0};
    if (Http_Perform(&h, api, &response) == -1)
    {
        printf("Http_Perform failed in weather_get_data\n");
        return;
    }


    Http_Dispose_Response(&response);
    Http_Dispose(&h);
}