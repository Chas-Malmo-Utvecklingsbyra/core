#include "http_router.h"
#include <stdio.h>

/* TODO: LS - Change response types to enums */
/* TODO: LS - Add logging for request handling */
/* TODO: LS - request_handler_register_routes fix parameter count magic number */

static int request_handler_response_init(Request_Handler_Response_t *request_handler_response)
{
    if (request_handler_response == NULL)
        return -1;

    request_handler_response->status_code = HTTP_STATUS_CODE_UNDEFINED;
    request_handler_response->response_data = NULL;
    request_handler_response->content_type = HTTP_CONTENT_TYPE_JSON;

    return 0;
}

static void request_handler_set_error_response(Request_Handler_Response_t *request_handler_response)
{
    if (request_handler_response == NULL)
        return;

    const char *error_msg = NULL;
    
    switch (request_handler_response->status_code)
    {
        case HTTP_STATUS_CODE_BAD_REQUEST:
            error_msg = "{\"error\":\"Bad Request\"}";
            break;
        case HTTP_STATUS_CODE_NOT_FOUND:
            error_msg = "{\"error\":\"Not Found\"}";
            break;
        case HTTP_STATUS_CODE_INTERNAL_SERVER_ERROR:
            error_msg = "{\"error\":\"Internal Server Error\"}";
            break;
        case HTTP_STATUS_CODE_SERVICE_UNAVAILABLE:
            error_msg = "{\"error\":\"Service Unavailable\"}";
            break;
        case HTTP_STATUS_CODE_BAD_GATEWAY:
            error_msg = "{\"error\":\"Bad Gateway\"}";
            break;
        default:
            request_handler_response->status_code = HTTP_STATUS_CODE_INTERNAL_SERVER_ERROR;
            error_msg = "{\"error\":\"Internal Server Error\"}";
            break;
    }
    
    request_handler_response->response_data = strdup(error_msg);
    if (request_handler_response->response_data == NULL)
    {
        fprintf(stderr, "Warning: Failed to allocate memory for error response\n");
    }
}

void Http_Router_Set_Response(Request_Handler_Response_t *request_handler_response, const HTTP_Status_Code status_code, const Http_Content_Type content_type, const char *response_data)
{
    if (request_handler_response == NULL)
        return;
    
    if (status_code != HTTP_STATUS_CODE_OK)
    {
        request_handler_response->status_code = status_code;
        request_handler_response->content_type = content_type;
        request_handler_set_error_response(request_handler_response);
        return;
    }

    request_handler_response->status_code = status_code;
    request_handler_response->content_type = content_type;
    request_handler_response->response_data = strdup(response_data);
    if (request_handler_response->response_data == NULL)
    {
        fprintf(stderr, "Warning: Failed to allocate memory for response data\n");
    }
}

Request_Handler_Response_t Http_Router_Handle_Request(RouteRegistry *registry, Http_Request *request)
{
    Request_Handler_Response_t response = {0};
    
    if (request == NULL)
    {
        Http_Router_Set_Response(&response, HTTP_STATUS_CODE_BAD_REQUEST, HTTP_CONTENT_TYPE_JSON, NULL);
        return response;
    }
    if (registry == NULL)
    {
        Http_Router_Set_Response(&response, HTTP_STATUS_CODE_INTERNAL_SERVER_ERROR, HTTP_CONTENT_TYPE_JSON, NULL);
        return response;
    }

    request_handler_response_init(&response);

    char *method = Http_Request_Get_Method_String(request);
    if (strcmp(method, "OPTIONS") == 0) /* Handle ALL OPTIONS requests */
    {
        Http_Router_Set_Response(&response, HTTP_STATUS_CODE_OK, HTTP_CONTENT_TYPE_HTML, "<h1>OPTIONS</h1>");
        return response;
    }
    
    HTTP_Status_Code result_code = Route_Registry_Dispatch(registry, request->start_line.path, method, &response);
    if (result_code != HTTP_STATUS_CODE_OK) /* Ensure error response is set */
    {
        Http_Router_Set_Response(&response, result_code, response.content_type, NULL);
    }
    return response;
}

void Http_Router_Dispose_Response(Request_Handler_Response_t *request_handler_response)
{
    if (request_handler_response->response_data != NULL)
    {
        free(request_handler_response->response_data);
        request_handler_response->response_data = NULL;
    }
}