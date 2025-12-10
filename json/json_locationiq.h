#ifndef JSON_LOCATIONIQ_H
#define JSON_LOCATIONIQ_H

#include "json.h"

/**
 * @brief Parses LocationIQ JSON response and extracts city information.
 *
 * @param locationiq_api_response The raw JSON response from LocationIQ API.
 * @param response_count The maximum number of city results to extract.
 * @return char* A JSON string containing the extracted city information, or NULL on failure.
 *                The caller is responsible for freeing the returned string.
 */
char *locationiq_json_parse(const char *locationiq_api_response, int response_count);

#endif /* JSON_LOCATIONIQ_H */