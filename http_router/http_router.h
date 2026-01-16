#ifndef HTTP_ROUTER_H
#define HTTP_ROUTER_H

#include <string.h>
#include <stdlib.h>

#include "route_registry/route_registry.h"
#include "query_parameters/query_parameters.h"
#include "../string/strdup.h"
#include "../http/parser.h"
#include "../http/http.h"
#include "../tcp/server/tcp_server.h"

/**
 * @brief Initialize the request handler and register all routes.
 * Must be called once at application startup.
 * @return 0 on success, -1 on error
 * @note The route registry is created inside this function.
 */
//int request_handler_register_routes(RouteRegistry *registry, int capacity); should be done somewhere else --- IGNORE ---

/**
 * @brief Sets API result for responses with JSON content.
 * @param request_handler_response Pointer to the HTTP response structure.
 * @param status_code HTTP status status_code.
 * @param content_type Content type of the response.
 * @param response_data Response data (will be duplicated).
 * @return void
 * @note response_data should be NULL for error responses.
 */
void request_handler_set_response(Request_Handler_Response_t *request_handler_response, const HTTP_Status_Code status_code, const Http_Content_Type content_type, const char *response_data);

/**
 * @brief Handles a HTTP request and returns the response structure.
 * @param registry Pointer to the route registry.
 * @param request Pointer to the HTTP request structure.
 * @return Request_Handler_Response_t The HTTP response structure.
 */
Request_Handler_Response_t request_handler_handle_request(RouteRegistry *registry, Http_Request *request);

/**
 * @brief Frees the memory allocated for a Request_Handler_Response_t structure.
 * @param request_handler_response Pointer to the Request_Handler_Response_t structure to free.
 */
void dispose_request_handler_response(Request_Handler_Response_t *request_handler_response);

#endif /* HTTP_ROUTER_H */