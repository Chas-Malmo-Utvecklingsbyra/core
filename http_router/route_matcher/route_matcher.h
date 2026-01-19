#ifndef ROUTE_MATCHER_H
#define ROUTE_MATCHER_H

#include <stdbool.h>
#include <stddef.h>

#include "http/http.h"

/**
 * @brief Checks if a request path and method match a route pattern and method.
 *
 * @param request_path The full path from the HTTP request (may include query params)
 * @param request_method The HTTP method of the request
 * @param route_pattern The route pattern to match against
 * @param route_method The HTTP method of the route
 * @return true if both path and method match, false otherwise
 */
bool Route_Matcher_Matches(const char *request_path, Http_Method request_method, const char *route_pattern, Http_Method route_method);

/**
 * @brief Extracts the path component without query parameters.
 *
 * @param full_path The full path including potential query parameters
 * @param buffer Buffer to store the extracted path (must be pre-allocated)
 * @param buffer_size Size of the buffer
 * @return int 0 on success, -1 on error
 * @note Not in use yet
 */
int Route_Matcher_Extract_Path(const char *full_path, char *buffer, size_t buffer_size);

#endif /* ROUTE_MATCHER_H */