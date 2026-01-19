#ifndef ROUTE_REGISTRY_H
#define ROUTE_REGISTRY_H

#include "../query_parameters/query_parameters.h"
#include "http/http.h"

/**
 * @brief Result codes for route registry operations
 */
typedef enum RouteRegistry_Result_t
{
    ROUTE_REGISTRY_RESULT_ERROR = -1,
    ROUTE_REGISTRY_RESULT_OK = 0
} RouteRegistry_Result_t;

/**
 * @brief Structure to hold the result of an API call.
 * @param status_code HTTP status status_code (200, 400, 404, 500, etc.)
 * @param response_data Response data (JSON string, HTML, etc.) - caller must free
 * @param content_type Content type of the response (JSON, HTML, etc.)
 * @note The response string has to be freed by the caller if not NULL.
 */
typedef struct Request_Handler_Response_t
{
    HTTP_Status_Code status_code;
    char *response_data;
    Http_Content_Type content_type;
} Request_Handler_Response_t;

/**
  * @brief Function pointer type for route handlers
  * @param params Query parameters extracted from the request
  * @param response HTTP response structure to populate
  */
typedef HTTP_Status_Code (*RouteHandler)(QueryParameters_t *params, Request_Handler_Response_t *response, void *context);

/**
 * @brief Single route entry in the registry
 */
typedef struct RouteRegistry_Entry
{
    const char *path;           /* Route path (e.g., "/v1/weather") */
    Http_Method method;         /* HTTP method (e.g., "GET") */
    RouteHandler handler;       /* Handler function for this route */
    void *context;              /* Optional context pointer for user data */
} RouteRegistry_Entry;

/**
 * @brief Route registry - manages all registered routes
 */
typedef struct RouteRegistry
{
    RouteRegistry_Entry *entries;
    size_t count;
} RouteRegistry;

/**
 * @brief Create a new route registry
 * @param capacity Initial capacity of the registry
 * @return Pointer to the created registry, or NULL on error
 */
bool Route_Registry_Create(RouteRegistry *registry);

/**
 * @brief Register a new route in the registry
 * @param registry Pointer to the registry
 * @param path Route path (e.g., "/v1/weather")
 * @param method HTTP method (e.g., HTTP_METHOD_GET)
 * @param handler Handler function for this route
 * @param context Optional context pointer for user data
 * @return Result code indicating success or failure
 */
RouteRegistry_Result_t Route_Registry_Register(RouteRegistry *registry, const char *path, Http_Method method, RouteHandler handler, void *context);

/**
 * @brief Dispatch a request to the appropriate route handler
 * @param registry Pointer to the registry
 * @param path Request path (e.g., "/v1/weather?lat=1&lon=2")
 * @param method HTTP method of the request
 * @param request_handler_response Pointer to the response structure to populate
 * @return HTTP status code from the handler, or error code if no match found
 */
HTTP_Status_Code Route_Registry_Dispatch(RouteRegistry *registry, const char *path, Http_Method method, Request_Handler_Response_t *request_handler_response);

/**
 * @brief Free all resources associated with the registry
 * @param registry Pointer to the registry
 */
void Route_Registry_Dispose(RouteRegistry *registry);

#endif /* ROUTE_REGISTRY_H */
