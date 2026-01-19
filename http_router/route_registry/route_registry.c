#include "route_registry.h"
#include <stdlib.h>
#include <string.h>

#include "../route_matcher/route_matcher.h"
#include "string/strdup.h"

bool Route_Registry_Create(RouteRegistry *registry)
{
    if (registry == NULL)
        return false;

    registry->entries = NULL;
    registry->count = 0;
    
    return true;
}

RouteRegistry_Result_t Route_Registry_Register(RouteRegistry *registry, const char *path, Http_Method method, RouteHandler handler, void *context)
{
    if (registry == NULL || path == NULL || handler == NULL || method == HTTP_METHOD_UNDEFINED)
    {
        return ROUTE_REGISTRY_RESULT_ERROR;
    }

    if(registry->entries == NULL)
    {
        registry->entries = malloc(sizeof(RouteRegistry_Entry) * 1);
        if (!registry->entries)
            return ROUTE_REGISTRY_RESULT_ERROR; /* Memory allocation failure */
    }
    else
    {
        RouteRegistry_Entry* resized_entries = realloc(registry->entries, sizeof(RouteRegistry_Entry) * (registry->count + 1));
        if (!resized_entries)
            return ROUTE_REGISTRY_RESULT_ERROR; /* Memory allocation failure */
        
        registry->entries = resized_entries;
    }
    
    for (size_t i = 0; i < registry->count; i++)
    {
        if (strcmp(registry->entries[i].path, path) == 0 && registry->entries[i].method == method)
        {
            return ROUTE_REGISTRY_RESULT_ERROR; /* Route already registered */
        }
    }
    
    registry->entries[registry->count].path = strdup(path);
    registry->entries[registry->count].method = method;
    registry->entries[registry->count].handler = handler;
    registry->entries[registry->count].context = context;
    registry->count++;
    return ROUTE_REGISTRY_RESULT_OK; /* Success */
}

HTTP_Status_Code Route_Registry_Dispatch(RouteRegistry *registry, const char *path, Http_Method method, Request_Handler_Response_t *request_handler_response)
{
    if (!registry || !path || method == HTTP_METHOD_UNDEFINED || !request_handler_response)
        return HTTP_STATUS_CODE_INTERNAL_SERVER_ERROR;
    
    /* Search for matching route */
    size_t i = 0;
    for (; i < registry->count; i++)
    {
        if (Route_Matcher_Matches(path, method, registry->entries[i].path, registry->entries[i].method))
        {
            QueryParameters_t query_parameters = {0};
            size_t param_count = Query_Parameter_Get_Param_Count(path);
            
            if (param_count > 0) /* Parse query parameters if expected */
            {
                if (Query_Parameter_Create(&query_parameters, param_count) != 0)
                {
                    return HTTP_STATUS_CODE_INTERNAL_SERVER_ERROR;
                }
                
                if (Query_Parameter_Parse(&query_parameters, path) != 0)
                {
                    Query_Parameter_Dispose(&query_parameters);
                    return HTTP_STATUS_CODE_BAD_REQUEST;
                }
            }
            /* Call the handler, expected to return HTTP status code */
            HTTP_Status_Code status_code = registry->entries[i].handler(&query_parameters, request_handler_response, registry->entries[i].context);
            Query_Parameter_Dispose(&query_parameters);
            
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
}
