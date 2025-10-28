#include "Database_PostgreSQL.h"
#include <curl/curl.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

char* city_fields[DATABASE_POSTGRESQL_CITY_FIELD_COUNT] = {"cityname", "latitude", "longitude", "timeLongNumber", "timeISO8601", "precipitation", "precipitationUnits", "temperature", "temperatueUnits", "elevation", "weathercode", "weathercodeVerbose", "windspeed", "windspeedUnits", "windspeedMS", "windspeedKMPH", "windDirection", "windDirectionUnits", "windDirectionVerbose", "isDay"};

/* Work in progress but should probly move this function to a more appropriate location */
/* Function to build JSON payload from CitySqlData, caller responsible for freeing the returned string */
char* buildJSONPayload(CitySqlData* city_data)
{
    char* data = (char*)malloc(DATABASE_POSTGRESQL_MAX_DATA_SIZE);
    char* temp;
    data[0] = '{';
    size_t memoryNeeded;
    
    int i = 0;
    for(i = 0; i < DATABASE_POSTGRESQL_CITY_FIELD_COUNT; i++) 
    {
        switch (i)
        {
            case 0:
                if (city_data->cityname != NULL) 
                {
                    memoryNeeded = strlen(city_fields[i]) + strlen(city_data->cityname) + 7;
                    temp = (char*)malloc(memoryNeeded);
                    sprintf(temp, "\"%s\":\"%s\",", city_fields[i], city_data->cityname);
                    strcat(data, temp);
                    free(temp);
                }
            break;
        
            case 1:
                memoryNeeded = strlen(city_fields[i]) + 20;
                temp = (char*)malloc(memoryNeeded);
                sprintf(temp, "\"%s\":%f,", city_fields[i], city_data->latitude);
                strcat(data, temp);
                free(temp);
            break;
            
            case 2:
                memoryNeeded = strlen(city_fields[i]) + 20;
                temp = (char*)malloc(memoryNeeded);
                sprintf(temp, "\"%s\":%f,", city_fields[i], city_data->longitude);
                strcat(data, temp);
                free(temp);
            break;
            
            case 7:
                memoryNeeded = strlen(city_fields[i]) + 20;
                temp = (char*)malloc(memoryNeeded);
                sprintf(temp, "\"%s\":%d,", city_fields[i], city_data->temperature);
                strcat(data, temp);
                free(temp);
            break;
            
            case 8:
                if (city_data->temperatureUnits != NULL) 
                {
                    memoryNeeded = strlen(city_fields[i]) + strlen(city_data->temperatureUnits) + 7;
                    temp = (char*)malloc(memoryNeeded);
                    sprintf(temp, "\"%s\":\"%s\",", city_fields[i], city_data->temperatureUnits);
                    strcat(data, temp);
                    free(temp);
                }
            break;
            
            default:
                break;
        }
    }
    /* remove last comma and close JSON object */
    data[strlen(data) - 1] = '}';
    return data;
}

int Database_PostgreSQL_Perform_Insert()
{
    CURL *curl;
    CURLcode res;
    
    /* Sample data for testing */
    CitySqlData city_data;
    city_data.cityname = "SampleCity";
    city_data.latitude = 12.34;
    city_data.longitude = 56.78;
    city_data.temperature = 25;
    city_data.temperatureUnits = "Celsius";

    char* data = buildJSONPayload(&city_data);

    printf("Data to send: %s\n", data);
    struct curl_slist *headers = NULL;
    
    curl = curl_easy_init();
    if(curl) 
    {
        /* WIP need to decide how to handle url's and api routes */
        char full_url[256];
        snprintf(full_url, sizeof(full_url), "%s%s", DATABASE_POSTGRESQL_URL, DATABASE_POSTGRESQL_TABLE_CITY_WEATHER);
        
        curl_easy_setopt(curl, CURLOPT_URL, full_url);
        curl_easy_setopt(curl, CURLOPT_URL, DATABASE_POSTGRESQL_URL);

        /* set auth headers */
        headers = curl_slist_append(headers, DATABASE_POSTGRESQL_API_KEY);
        headers = curl_slist_append(headers, DATABASE_POSTGRESQL_AUTH_HEADER);
        headers = curl_slist_append(headers, "Content-Type: application/json");
        headers = curl_slist_append(headers, "Prefer: return=minimal");
        printf("Headers set for request.\n");
        
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, data);

        res = curl_easy_perform(curl);
        if(res != CURLE_OK) 
        {
            fprintf(stderr, "curl_easy_perform() failed: %s\n", curl_easy_strerror(res));
        }
        
        curl_slist_free_all(headers);
        curl_easy_cleanup(curl);
        printf("Data sent to database successfully.\n");
    }
    
    free(data);
    return 0;
}