#ifndef LOCATIONIQ_H
#define LOCATIONIQ_H

#include "../json/json.h"

/* LOCATIONIQ_ACCESS_TOKEN replaced with config_get_instance(NULL)->locationiq_access_token */

/*  Usage: Create a Coordinates struct and send it along with a const char* to locationiq_get_coordinates.
    If it finds places matching the string, it will display the options as a numbered list.
    When the user selects an option by entering the corresponding number, the relevant
    latitude and longitude as well as the full location name is stored in the Coordinates struct.
    The location names from LocationIQ are usually long so we might want to only take the
    first word from the string (but that would cause problems if the user looks up two places with the same name). */

typedef enum
{
    LOCATIONIQ_RESULT_ERROR = -1,
    LOCATIONIQ_RESULT_OK = 0,
    LOCATIONIQ_RESULT_USER_ABORTED = 1,
} LOCATIONIQ_RESULT;

typedef struct {
    char* location;
    double lat;
    double lon;

} Coordinates;

typedef struct 
{
    char* string;
    size_t size;
    
} Response;

size_t write_chunk(void* data, size_t item_size, size_t nmemb, void* user_data);

/**
 * @brief Makes an API call to LocationIQ to search for a location.
 *
 * @param location The location query string.
 * @param limit The maximum number of results to return (1-10).
 * @return char* The raw JSON response from LocationIQ API, or NULL on failure.
 *                The caller is responsible for freeing the returned string.
 */
char *locationiq_api_call(const char *location, int limit, const char *access_token);

int locationiq_get_coordinates(Coordinates *coords, const char *location, const char *access_token);

#endif