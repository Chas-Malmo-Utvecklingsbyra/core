#include "route_registry.h"
#include "../route_matcher/route_matcher.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

bool Route_Registry_Create(RouteRegistry *registry, size_t capacity)
{
    if (registry == NULL || capacity <= 0)
        return false;

    registry->entries = malloc(sizeof(RouteRegistry_Entry) * capacity);
    if (registry->entries == NULL)
    {
        return false;
    }

    registry->count = 0;
    registry->capacity = capacity;
    return true;
}

/* TODO: LS remove args count, should be done dynamically */
RouteRegistry_Result_t Route_Registry_Register(RouteRegistry *registry, const char *path, Http_Method method, size_t args_count, RouteHandler handler, void *context)
{
    if (registry == NULL || path == NULL || handler == NULL)
    {
        return ROUTE_REGISTRY_RESULT_ERROR;
    }
    
    if (registry->count >= registry->capacity)
    {
        fprintf(stderr, "Route registry at capacity (%zu). Cannot register more routes.\n", registry->capacity);
        return -1; /* Registry full */
    }

    char* method_str = Http_Get_Method_String(method);
    
    if (!method_str || strcmp(method_str, "UNDEFINED") == 0)
    {
        return ROUTE_REGISTRY_RESULT_ERROR; /* Invalid method */
    }
    
    registry->entries[registry->count].path = path;
    registry->entries[registry->count].method = method_str;
    registry->entries[registry->count].args_count = args_count;
    registry->entries[registry->count].handler = handler;
    registry->entries[registry->count].context = context;
    registry->count++;
    return ROUTE_REGISTRY_RESULT_OK; /* Success */
}

HTTP_Status_Code Route_Registry_Dispatch(RouteRegistry *registry, const char *path, const char *method, Request_Handler_Response_t *request_handler_response)
{
    
    if (registry == NULL || path == NULL || method == NULL || request_handler_response == NULL)
    {
        return HTTP_STATUS_CODE_INTERNAL_SERVER_ERROR;
    }
    
    /* Search for matching route */
    size_t i = 0;
    for (; i < registry->count; i++)
    {
        if (route_matcher_matches(path, method, registry->entries[i].path, registry->entries[i].method))
        {
            QueryParameters_t query_parameters = {0};
            memset(&query_parameters, 0, sizeof(QueryParameters_t));
            
            if (registry->entries[i].args_count > 0) /* Parse query parameters if expected */
            {
                if (query_parameter_create(&query_parameters, registry->entries[i].args_count) != 0)
                {
                    return HTTP_STATUS_CODE_INTERNAL_SERVER_ERROR;
                }
                
                if (query_parameter_parse(&query_parameters, path) != 0)
                {
                    query_parameter_dispose(&query_parameters);
                    return HTTP_STATUS_CODE_BAD_REQUEST;
                }
            }
            /* Call the handler, expected to return HTTP status code */
            HTTP_Status_Code status_code = registry->entries[i].handler(&query_parameters, request_handler_response, registry->entries[i].context);
            query_parameter_dispose(&query_parameters);
            
            return status_code;
        }
    }
    return HTTP_STATUS_CODE_NOT_FOUND; /* No matching route found */
}

void Route_Registry_Dispose(RouteRegistry *registry)
{
    if (registry == NULL)
        return;

    if (registry->entries != NULL)
    {
        /* Free context pointers for each entry */
        for (size_t i = 0; i < registry->count; i++)
        {
            if (registry->entries[i].context != NULL)
            {
                free(registry->entries[i].context);
                registry->entries[i].context = NULL;
            }
        }

        free(registry->entries);
        registry->entries = NULL;
    }

    registry->count = 0;
    registry->capacity = 0;
}
