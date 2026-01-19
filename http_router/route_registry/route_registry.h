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
    const char *method;         /* HTTP method (e.g., "GET") */
    size_t args_count;          /* Expected number of arguments */
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
    size_t capacity;
} RouteRegistry;

/**
 * @brief Create a new route registry
 * @param capacity Initial capacity of the registry
 * @return Pointer to the created registry, or NULL on error
 */
bool Route_Registry_Create(RouteRegistry *registry, size_t capacity);

/**
 * @brief Register a new route
 * @param registry Pointer to the registry
 * @param path Route path (e.g., "/v1/weather")
 * @param method HTTP method (e.g., "GET")
 * @param args_count Expected number of query parameters
 * @param handler Function pointer to handle this route
 * @return 0 on success, -1 on error
 */
RouteRegistry_Result_t Route_Registry_Register(RouteRegistry *registry, const char *path, Http_Method method, size_t args_count, RouteHandler handler, void *context);

/**
 * @brief Find and execute a handler for a request
 * @param registry Pointer to the registry
 * @param path Request path
 * @param method HTTP method
 * @param params Query parameters (may be NULL)
 * @param response HTTP response to populate
 * @return HTTP status code from the handler
 */
HTTP_Status_Code Route_Registry_Dispatch(RouteRegistry *registry, const char *path, const char *method, Request_Handler_Response_t *request_handler_response);

/**
 * @brief Free all resources associated with the registry
 * @param registry Pointer to the registry
 */
void Route_Registry_Dispose(RouteRegistry *registry);

#endif /* ROUTE_REGISTRY_H */
